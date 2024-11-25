/*
 * C99-to-MSVC-compatible-C89 syntax converter
 * Copyright (c) 2012 Martin Storsjo <martin@martin.st>
 * Copyright (c) 2018 Ray Donnelly, Anaconda Inc. <mingw.android@gmail.com>
 * Copyright (c) 2024 Modulate, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include <windows.h>
#define getpid GetCurrentProcessId
#else
#include <unistd.h>
#include <sys/wait.h>
#include <wordexp.h>
#endif

#ifdef _MSC_VER
#define strtoll _strtoi64
#endif

#define CONVERTER "c99conv"

/* 0 == no debugging */
/* 1 == print command-lines to all called processes */
static int DEBUG_LEVEL = 0;
/* 0 == pass -E to the pre-processor, 1 == pass -EP */
static int DEBUG_NO_LINE_DIRECTIVES = 0;

static char* create_cmdline(char **argv)
{
    int i;
    int len = 0, pos = 0;
    char *out;

    for (i = 0; argv[i]; i++)
        len += strlen(argv[i]);

    out = malloc(len + 4 * i + 10);

    for (i = 0; argv[i]; i++) {
        int contains_space = 0, j;

        for (j = 0; argv[i][j] && !contains_space; j++)
            if (argv[i][j] == ' ')
                contains_space = 1;

        if (contains_space)
            out[pos++] = '"';

        strcpy(&out[pos], argv[i]);
        pos += strlen(argv[i]);

        if (contains_space)
            out[pos++] = '"';

        if (argv[i + 1])
            out[pos++] = ' ';
    }

    out[pos] = '\0';

    return out;
}

/* On Windows, system() has very frugal length limits */
#ifdef _WIN32
static int exec_argv_out(char **argv, int out_0_err_1, const char *out)
{
    STARTUPINFO si = { 0 };
    PROCESS_INFORMATION pi = { 0 };
    DWORD exit_code;
    char *cmdline = create_cmdline(argv);
    FILE *fp = NULL;
    HANDLE pipe_read, pipe_write = NULL;
    BOOL inherit = FALSE;

    if (DEBUG_LEVEL > 0) {
        (void)fprintf(stderr, "Invoking sub-process: %s\n", cmdline);
    }

    if (out) {
        char temp[2048];
        /* When debugging this code I wasted a lot of time on this due to file locking.
           system del seems to be the most reliable way to work around that. Also the
           error message below is less cryptic than the one from perror(). */
        if(_access(out, 0) == 0) {
            sprintf(temp, "del \"%s\"", out);
            system(temp);
        }
        SECURITY_ATTRIBUTES sa = { 0 };
        fp = fopen(out, "wb");
        if (!fp) {
            printf("ERROR :: c99wrap failed to open out %s\n", out);
            perror(out);
            return 1;
        }
        sa.nLength = sizeof(sa);
        sa.bInheritHandle = TRUE;
        CreatePipe(&pipe_read, &pipe_write, &sa, 0);
        si.cb = sizeof(si);
        si.dwFlags = STARTF_USESTDHANDLES;
        if (out_0_err_1 == 0) {
            si.hStdOutput = pipe_write;
            si.hStdError = GetStdHandle(STD_ERROR_HANDLE);
        }
        else {
            si.hStdOutput = GetStdHandle(STD_OUTPUT_HANDLE);
            si.hStdError = pipe_write;
        }
        inherit = TRUE;
    }
    if (CreateProcess(NULL, cmdline, NULL, NULL, inherit, 0, NULL, NULL, &si, &pi)) {
        free(cmdline);
        if (out) {
            char buf[8192];
            DWORD n;
            CloseHandle(pipe_write);
            while (ReadFile(pipe_read, buf, sizeof(buf), &n, NULL))
                fwrite(buf, 1, n, fp);
            CloseHandle(pipe_read);
            fclose(fp);
        }

        WaitForSingleObject(pi.hProcess, INFINITE);

        if (!GetExitCodeProcess(pi.hProcess, &exit_code))
            exit_code = -1;

        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);

        if (DEBUG_LEVEL > 0) {
            (void)fprintf(stderr, "Sub-process exit code: %lu\n", exit_code);
        }

        return exit_code;
    } else {
        if (out) {
            fclose(fp);
            CloseHandle(pipe_read);
            CloseHandle(pipe_write);
        }
        free(cmdline);
        return -1;
    }
}
#else
static int exec_argv_out(char **argv, int out_0_err_1, const char *out)
{
    int fds[2];
    pid_t pid;
    int ret = 0;
    FILE *fp;
    if (!out) {
        char *cmdline = create_cmdline(argv);

        if (DEBUG_LEVEL > 0) {
            (void)fprintf(stderr, "Invoking sub-process: %s\n", cmdline);
        }

        ret = system(cmdline);
        free(cmdline);

        if (DEBUG_LEVEL > 0) {
            (void)fprintf(stderr, "Sub-process exit code: %d\n", ret);
        }

        return ret;
    }
    fp = fopen(out, "wb");
    if (!fp) {
        perror(out);
        return 1;
    }

    pipe(fds);
    if (!(pid = fork())) {
        close(fds[0]);
        dup2(fds[1], STDOUT_FILENO);

        if (DEBUG_LEVEL > 0) {
            char *cmdline = create_cmdline(argv);
            (void)fprintf(stderr, "Invoking sub-process: %s\n", cmdline);
        }

        if (execvp(argv[0], argv)) {
            perror("execvp");
            exit(1);
        }
    }
    close(fds[1]);
    while (1) {
        char buf[8192];
        int n = read(fds[0], buf, sizeof(buf));
        if (n <= 0)
            break;
        fwrite(buf, 1, n, fp);
    }
    close(fds[0]);
    fclose(fp);
    waitpid(pid, &ret, 0);

    if (DEBUG_LEVEL > 0) {
        printf("Sub-process exit code: %d\n", ret);
    }

    return WEXITSTATUS(ret);
}
#endif


/* Permits flags beginning with either - or / */
int flagstrcmp ( const char * str1, const char * str2 )
{
    int realres = strcmp(str1, str2);
    if (!realres)
        return 0;
    if ((str1[0] == '-' && str2[0] == '/') || (str1[0] == '/' && str2[0] == '-'))
        return strcmp(str1+1, str2+1);
    return realres;
}


/* Permits flags beginning with either - or / */
int flagstrncmp ( const char * str1, const char * str2, size_t num )
{
    int realres = strncmp(str1, str2, num);
    if (!realres)
        return 0;
    if ((str2[0] == '-' && str1[0] == '/') || (str2[0] == '/' && str1[0] == '-'))
        return strncmp(str1+1, str2+1, num-1);
    return realres;
}


char **split_commandline(char *cmdline, int *argc)
{
    int i;
    char **argv = NULL;

    if (!cmdline) {
        return NULL;
    }

    /* Get rid of anything after the first newline (0d 0a) */
    if (strchr(cmdline, 0x0d)) {
        *strchr(cmdline, 0x0d) = '\0';
    }
    if (strchr(cmdline, 0x0a)) {
        *strchr(cmdline, 0x0a) = '\0';
    }

 #ifndef _WIN32
    {
        wordexp_t p;

        // Note! This expands shell variables.
        if (wordexp(cmdline, &p, 0))
        {
            return NULL;
        }

        *argc = p.we_wordc;

        if (!(argv = calloc(*argc, sizeof(char *))))
        {
            goto fail;
        }

        for (i = 0; i < p.we_wordc; i++)
        {
            if (!(argv[i] = strdup(p.we_wordv[i])))
            {
                goto fail;
            }
        }

        wordfree(&p);

        return argv;
    fail:
        wordfree(&p);
    }
#else
    {
        wchar_t **wargs = NULL;
        size_t needed = 0;
        wchar_t *cmdlinew = NULL;
        size_t len = strlen(cmdline) + 1;

        if (!(cmdlinew = calloc(len, sizeof(wchar_t))))
            goto fail;

        if (!MultiByteToWideChar(CP_ACP, 0, cmdline, -1, cmdlinew, len))
            goto fail;

        if (!(wargs = CommandLineToArgvW(cmdlinew, argc)))
            goto fail;

        if (!(argv = calloc(*argc, sizeof(char *))))
            goto fail;

        // Convert from wchar_t * to ANSI char *
        for (i = 0; i < *argc; i++)
        {
            // Get the size needed for the target buffer.
            // CP_ACP = Ansi Codepage.
            needed = WideCharToMultiByte(CP_ACP, 0, wargs[i], -1,
                                        NULL, 0, NULL, NULL);

            if (!(argv[i] = malloc(needed)))
                goto fail;

            // Do the conversion.
            needed = WideCharToMultiByte(CP_ACP, 0, wargs[i], -1,
                                        argv[i], needed, NULL, NULL);
        }

        if (wargs) LocalFree(wargs);
        if (cmdlinew) free(cmdlinew);
        return argv;

    fail:
        if (wargs) LocalFree(wargs);
        if (cmdlinew) free(cmdlinew);
    }
    #endif // WIN32

    if (argv)
    {
        for (i = 0; i < *argc; i++)
        {
            if (argv[i])
            {
                free(argv[i]);
            }
        }

        free(argv);
    }

    return NULL;
}

/* Converts a UTF-16 LE buffer to a UTF-8 string. */
char *utf16le_to_utf8(const unsigned char *utf16_buf, size_t utf16_len) {
    size_t utf8_len = 0;
    char *utf8_buf = NULL;
    size_t i;

    // First, calculate the required length for the UTF-8 buffer.
    for (i = 0; i < utf16_len; i += 2) {
        const unsigned char low = utf16_buf[i];
        const unsigned char high = utf16_buf[i + 1];
        const unsigned int codepoint = (high << 8) | low;

        if (codepoint < 0x80) {
            utf8_len += 1; // 1-byte UTF-8
        } else if (codepoint < 0x800) {
            utf8_len += 2; // 2-byte UTF-8
        } else {
            utf8_len += 3; // 3-byte UTF-8
        }
    }

    // Allocate the UTF-8 buffer.
    utf8_buf = malloc(utf8_len + 1); // +1 for NULL terminator
    if (!utf8_buf) {
        return NULL;
    }

    // Convert UTF-16 LE to UTF-8.
    char *out = utf8_buf;
    for (i = 0; i < utf16_len; i += 2) {
        const unsigned char low = utf16_buf[i];
        const unsigned char high = utf16_buf[i + 1];
        const unsigned int codepoint = (high << 8) | low;

        if (codepoint < 0x80) {
            *out++ = codepoint;
        } else if (codepoint < 0x800) {
            *out++ = 0xC0 | (codepoint >> 6);
            *out++ = 0x80 | (codepoint & 0x3F);
        } else {
            *out++ = 0xE0 | (codepoint >> 12);
            *out++ = 0x80 | ((codepoint >> 6) & 0x3F);
            *out++ = 0x80 | (codepoint & 0x3F);
        }
    }

    *out = '\0'; // NULL terminator
    return utf8_buf;
}

/* Allocates and returns a buffer containing the contents of filename as a UTF-8 string. */
char * read_file(const char *filename) {
    char * buf = NULL;
    long len;
    FILE * f = fopen (filename, "rb");

    if (f) {
        // Get the file length.
        fseek (f, 0, SEEK_END);
        len = ftell (f);
        fseek (f, 0, SEEK_SET);

        if (len > 0) {
            buf = malloc(len + 1);
            if (buf != NULL) {
                fread (buf, 1, len, f);
                buf[len] = '\0'; // Ensure NULL terminator

                // Check for UTF-16 LE BOM.
                if (len >= 2 && (unsigned char)buf[0] == 0xFF && (unsigned char)buf[1] == 0xFE) {
                    // Convert UTF-16 LE to UTF-8
                    char * utf8_buf = utf16le_to_utf8 ((unsigned char *)buf + 2, len - 2);

                    // Return the converted version of the data.
                    free (buf);
                    buf = utf8_buf;
                }
            }
        }
        fclose(f);
    }

    return buf;
}

void write_file(const char * buf, size_t len, const char * filename)
{
    if (strlen(buf) != len) {
        printf("ERROR: Not saving all of file. strlen(buf)=%zd, asked to save len=%zd\n", strlen(buf), len);
    }
    FILE * f = fopen(filename, "wb");
    fwrite (buf, 1, len, f);
    fclose(f);
}


void print_argv(char * name, char ** argv, int argc, int force_print)
{
    int i, j;

    if (DEBUG_LEVEL == 0 && force_print == 0) {
        return;
    }
    (void)name;
    for (i = 0; i < argc; i++) {
        if (argv[i]) {
            printf("%s ", argv[i]);
        }
    }
}

size_t remove_string(char * input, char * to_remove, size_t * initialsz)
{
    char * old_cursor = input;
    char * cursor = strstr(input, to_remove);
    size_t remain = strlen(input) + 1;
    size_t finalsz = remain;
    size_t to_remove_sz = strlen(to_remove);
    *initialsz = remain;
    while (cursor != NULL) {
        size_t gap = cursor - old_cursor;
        remain -= gap + to_remove_sz;
        finalsz -= to_remove_sz;
        memmove(cursor, cursor + to_remove_sz, remain);
        old_cursor = cursor;
        cursor = strstr(cursor, to_remove);
    }
    return finalsz;
}

int main(int argc, char *argv[])
{
    int i = 1;
    int cpp_argc, cc_argc, pass_argc;
    int exit_code;
    int input_source = 0, input_obj = 0;
    int msvc = 0, icl = 0, keep = 0, noconv = 0, flag_compile = 0;
    char *ptr;
    char temp_file_1[2048], temp_file_2[2048], temp_file_3[2046],
         fo_buffer[2048], fi_buffer[2048];
    char **cpp_argv, **cc_argv, **pass_argv, **bitness_argv;
    char *conv_argv[6], *conv_tool;
    const char *source_file = NULL;
    const char *outname = NULL;
    char *response_file = NULL;
    const char *envvar = NULL;
    char convert_options[20] = "";
    char convert_bitness[4] = "-64";

    conv_tool = malloc(strlen(argv[0]) + strlen(CONVERTER) + 1);
    strcpy(conv_tool, argv[0]);

    envvar = getenv("C99_TO_C89_WRAP_DEBUG_LEVEL");
    if (envvar != NULL) {
        DEBUG_LEVEL = strtoll(envvar, NULL, 10);
        envvar = NULL;
    }
    envvar = getenv("C99_TO_C89_WRAP_SAVE_TEMPS");
    if (envvar != NULL) {
        keep = strtoll(envvar, NULL, 10);
        envvar = NULL;
    }
    envvar = getenv("C99_TO_C89_WRAP_NO_LINE_DIRECTIVES");
    if (envvar != NULL) {
        DEBUG_NO_LINE_DIRECTIVES = strtoll(envvar, NULL, 10);
        envvar = NULL;
    }

    ptr = strrchr(conv_tool, '\\');
    if (!ptr)
        ptr = strrchr(conv_tool, '/');

    if (!ptr)
        conv_tool[0] = '\0';
    else
        ptr[1] = '\0';
    strcat(conv_tool, CONVERTER);

    for (; i < argc; i++) {
        if (!strcmp(argv[i], "-keep")) {
            keep = 1;
        } else if (!strcmp(argv[i], "-noconv")) {
            noconv = 1;
        } else
            break;
    }

    if (keep && noconv) {
        fprintf(stderr, "Using -keep with -noconv doesn't make any sense!\n "
                        "You cannot keep intermediate files that doesn't exist.\n");
        return 1;
    }

    if (i < argc && !strncmp(argv[i], "cl", 2) && (argv[i][2] == '.' || argv[i][2] == '\0')) {
        msvc = 1;
        strcpy(convert_options, "-ms");
    } else if (i < argc && !strncmp(argv[i], "icl", 3) && (argv[i][3] == '.' || argv[i][3] == '\0')) {
        msvc = 1; /* for command line compatibility */
        icl = 1;
    }

    /* Are we using a response file? If so reform argv and argc from it. */
    if (argv[argc-1][0] == '@') {
        response_file = read_file(&(argv[argc-1][1]));
        if (response_file) {
            if (DEBUG_LEVEL > 1) {
                printf("Response file contents:\n%s\n", response_file);
            }
            char ** argv_temp = split_commandline(response_file, &argc);
            char *cl_or_icl = argv[i];
            argv = malloc((argc+1) * sizeof(char*));
            argv[0] = cl_or_icl;
            memcpy(&argv[1], argv_temp, argc * sizeof(char*));
            argc++;
            i = 0;
            /* Print the commandline as this is one of the most difficult aspects of dealing
            with CMake on Windows. If it uses a response file (often it will) you are forced
            to use something like procmon to see the flags passed. */
            if (icl == 0)
                printf("c99wrap cl ");
            else
                printf("c99wrap icl ");
            print_argv("argv", argv + 1, argc - 1, 1);

        }
    }

    sprintf(temp_file_1, "preprocessed_%d.c", getpid());
    sprintf(temp_file_2, "converted_%d.c", getpid());
    sprintf(temp_file_3, "bitness_%d.c", getpid());

    cpp_argv     = malloc((argc + 2) * sizeof(*cpp_argv));
    cc_argv      = malloc((argc + 3) * sizeof(*cc_argv));
    pass_argv    = malloc((argc + 3) * sizeof(*pass_argv));
    bitness_argv = malloc(2 * sizeof(*bitness_argv));
    if (icl)
        bitness_argv[0] = "icl";
    else
        bitness_argv[0] = "cl";
    bitness_argv[1] = NULL;
    exec_argv_out(bitness_argv, 1, temp_file_3);
    char * bitness_out = read_file(temp_file_3);
    if (bitness_out != NULL && strstr(bitness_out, "80x86"))
        strcpy(convert_bitness, "-32");
    unlink(temp_file_3);

    cpp_argc = cc_argc = pass_argc = 0;

    for (; i < argc; ) {
        int len           = strlen(argv[i]);
        int ext_inputfile = 0;

        if (len >= 2) {
            const char *ext = &argv[i][len - 2];

            if (!strcmp(ext, ".c") || !strcmp(ext, ".s") || !strcmp(ext, ".S")) {
                ext_inputfile = 1;
                input_source  = 1;
                source_file   = argv[i];
            } else if (!strcmp(ext, ".o") && argv[i][0] != '/' && argv[i][0] != '-') {
                ext_inputfile = 1;
                input_obj     = 1;
            }
        }
        if (!flagstrncmp(argv[i], "-Fo", 3) || !flagstrncmp(argv[i], "-Fi", 3) || !flagstrncmp(argv[i], "-Fe", 3) ||
            !flagstrcmp(argv[i], "-out") || !flagstrcmp(argv[i], "-o") || !flagstrcmp(argv[i], "-FI")) {

            // Copy the output filename only to cc
            if ((!flagstrcmp(argv[i], "-Fo") || !flagstrcmp(argv[i], "-out") || !flagstrcmp(argv[i], "-Fi") ||
                !flagstrcmp(argv[i], "-Fe")) && i + 1 < argc) {

                /* Support the nonstandard syntax -Fo filename or -out filename, to get around
                 * msys file name mangling issues. */
                if (!flagstrcmp(argv[i], "-out"))
                    sprintf(fo_buffer, "-out:%s", argv[i + 1]);
                else
                    sprintf(fo_buffer, "%s%s", argv[i], argv[i + 1]);

                outname = argv[i + 1];

                cc_argv[cc_argc++]     = fo_buffer;
                pass_argv[pass_argc++] = fo_buffer;

                i += 2;
            } else if (!flagstrcmp(argv[i], "-FI") && i + 1 < argc) {
                /* Support the nonstandard syntax -FI filename, to get around
                 * msys file name mangling issues. */
                sprintf(fi_buffer, "%s%s", argv[i], argv[i + 1]);

                cpp_argv[cpp_argc++]   = fi_buffer;
                pass_argv[pass_argc++] = fi_buffer;

                i += 2;
            } else if (!flagstrncmp(argv[i], "-Fo", 3) || !flagstrncmp(argv[i], "-Fi", 3) || !flagstrncmp(argv[i], "-Fe", 3)) {
                cc_argv[cc_argc++]     = argv[i];
                pass_argv[pass_argc++] = argv[i];

                outname = argv[i] + 3;

                i++;
            } else {
                /* -o */
                pass_argv[pass_argc++] = argv[i];
                cc_argv[cc_argc++]     = argv[i++];

                if (i < argc) {
                    outname = argv[i];

                    pass_argv[pass_argc++] = argv[i];
                    cc_argv[cc_argc++]     = argv[i++];
                }
            }

            /* Set easier to understand temp file names. temp_file_2 might
             * have been set in cc_argv already, but we're just updating
             * the same buffer. */
            sprintf(temp_file_1, "%s_preprocessed.c", outname);
            sprintf(temp_file_2, "%s_converted.c", outname);

        } else if (!flagstrcmp(argv[i], "-c")) {
            // Copy the compile flag only to cc, set the preprocess flag for cpp
            pass_argv[pass_argc++] = argv[i];
            cc_argv[cc_argc++]     = argv[i++];

            if (DEBUG_NO_LINE_DIRECTIVES == 0)
                cpp_argv[cpp_argc++] = "-E";
            else
                cpp_argv[cpp_argc++] = "-EP";

            if (!noconv)
                flag_compile = 1;
        } else if (ext_inputfile) {
            // Input filename, pass to cpp only, set the temp file input to cc
            pass_argv[pass_argc++] = argv[i];
            cpp_argv[cpp_argc++]   = argv[i++];
            cc_argv[cc_argc++]     = temp_file_2;
        } else if (!flagstrcmp(argv[i], "-MMD") || !flagstrncmp(argv[i], "-D", 2)) {
            // Preprocessor-only parameter
            if (!flagstrcmp(argv[i], "-D")) {
                // Handle -D DEFINE style
                pass_argv[pass_argc++] = argv[i];
                cpp_argv[cpp_argc++]   = argv[i++];
            }
            pass_argv[pass_argc++] = argv[i];
            cpp_argv[cpp_argc++]   = argv[i++];
        } else if (!flagstrcmp(argv[i], "-MF") || !flagstrcmp(argv[i], "-MT")) {
            // Deps generation, pass to cpp only
            pass_argv[pass_argc++] = argv[i];
            cpp_argv[cpp_argc++]   = argv[i++];

            if (i < argc) {
                pass_argv[pass_argc++] = argv[i];
                cpp_argv[cpp_argc++]   = argv[i++];
            }
        } else if (!flagstrncmp(argv[i], "-FI", 3)) {
            // Forced include, pass to cpp only
            pass_argv[pass_argc++] = argv[i];
            cpp_argv[cpp_argc++]   = argv[i++];
        } else {
            // Normal parameter, copy to both cc and cpp
            pass_argv[pass_argc++] = argv[i];
            cc_argv[cc_argc++]     = argv[i];
            cpp_argv[cpp_argc++]   = argv[i];

            i++;
        }
    }

    cpp_argv[cpp_argc++]   = NULL;
    cc_argv[cc_argc++]     = NULL;
    pass_argv[pass_argc++] = NULL;

    if (!flag_compile || !source_file || !outname) {
        print_argv("pass_argv", pass_argv, pass_argc, 0);
        /* Doesn't seem like we should be invoked, just call the parameters as such */
        exit_code = exec_argv_out(pass_argv, 0, NULL);

        goto exit;
    }

    print_argv("cpp_argv", cpp_argv, cpp_argc, 0);
    exit_code = exec_argv_out(cpp_argv, 0, temp_file_1);
    if (exit_code) {
        if (!keep)
            unlink(temp_file_1);

        goto exit;
    }
    /* VS2008 pre-processor does not remove line-continuations, so you end up with:
    #pragma comment(linker,"/manifestdependency:\"type='win32' " \
                    "name='" "Microsoft.VC90" ".DebugCRT' " \
                                        "version='" "9.0.21022.8" "' " \
                                        "processorArchitecture='amd64' " \
                                        "publicKeyToken='" "1fc8b3b9a1e18e3b" "'\"")
        .. and then clang will remove these continuations and the compiler cannot handle that
        (because it's not valid).
    */
    char * preproc_out = read_file(temp_file_1);
    if (preproc_out == NULL) {
        exit_code = 1;
        goto exit;
    }
    /* Two line ending styles to allow for other shells, nasty but this caused me a lot of trouble */
    static char line_cont1[] = "\\\r\n";
    static char line_cont2[] = "\\\n";
    /* We also remove #pragma once which is invalid in .c files and creates very noisy logs */
    static char pragma_once1[] = "#pragma once\r\n";
    static char pragma_once2[] = "#pragma once\n";
    size_t initialsz;
    size_t finalsz1 = remove_string(preproc_out, line_cont1, &initialsz);
    size_t finalsz2 = remove_string(preproc_out, line_cont2, &finalsz1);
    size_t finalsz3 = remove_string(preproc_out, pragma_once1, &finalsz2);
    size_t finalsz4 = remove_string(preproc_out, pragma_once2, &finalsz3);
    if (finalsz4 != initialsz) {
        write_file(preproc_out, finalsz4 - 1, temp_file_1);
    }

    conv_argv[0] = conv_tool;
    conv_argv[1] = convert_options;
    conv_argv[2] = convert_bitness;
    conv_argv[3] = temp_file_1;
    conv_argv[4] = temp_file_2;
    conv_argv[5] = NULL;

    exit_code = exec_argv_out(conv_argv, 0, NULL);
    print_argv("conv_argv", conv_argv, 5, 0);
    if (exit_code) {
        if (!keep) {
            unlink(temp_file_1);
            unlink(temp_file_2);
        }

        goto exit;
    }

    if (!keep)
        unlink(temp_file_1);

    exit_code = exec_argv_out(cc_argv, 0, NULL);

    if (!keep)
        unlink(temp_file_2);

exit:
    free(cc_argv);
    free(cpp_argv);
    free(pass_argv);
    free(conv_tool);
    if(response_file)
        free(response_file);

    return exit_code ? 1 : 0;
}
