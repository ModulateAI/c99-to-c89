#!/bin/bash

set -x

# $(dirname $(dirname $(dirname $(which 7z))))/usr/lib/p7zip/7z x -aoa -o$PWD/clang LLVM*${ARCH}.exe || exit 1
7z x -aoa -o$PWD/clang LLVM*.exe || exit 1

CLANGDIR=${PWD}/clang
# Rename libclang.dll to avoid conflicts. Sorry. No one builds static libclangs on Windows for some reason?
cp ${CLANGDIR}/bin/libclang.dll ${CLANGDIR}/bin/c99-to-c89-libclang.dll
# Create an import library for c99-to-c89-libclang.dll
gendef ${CLANGDIR}/bin/c99-to-c89-libclang.dll - > c99-to-c89-libclang.dll.def
sed -i 's|LIBRARY "libclang.dll"|LIBRARY "c99-to-c89-libclang.dll"|' c99-to-c89-libclang.dll.def
if [[ ${ARCH} == 32 ]]; then
  MS_MACH=X86
else
  MS_MACH=X64
fi
if [[ ${vc} == 9 ]]; then
  SNPRINTF=-Dsnprintf=_snprintf
fi
lib /def:c99-to-c89-libclang.dll.def /out:$(cygpath -m ${CLANGDIR}/lib/c99-to-c89-libclang.lib) /machine:${MS_MACH}

CFLAGS="${CFLAGS} -I${BUILD_PREFIX}/Library/include ${SNPRINTF}" CLANGDIR=${PWD}/clang make -f Makefile.w32 -n
CFLAGS="${CFLAGS} -I${BUILD_PREFIX}/Library/include ${SNPRINTF}" CLANGDIR=${PWD}/clang make -f Makefile.w32

cp c99*.exe ${CLANGDIR}/bin/c99-to-c89-libclang.dll ${PREFIX}/Library/bin

# This file should be passed to the CMake command-line as:
# -DCMAKE_C_COMPILER="c99-to-c89-cmake-nmake-wrap.bat"
# I have given it a very specific name as I do not know that it will work for any other CMake
# generators .. it assumes the only argument is a response file beginning with @ for example.
# It also prints out the contents of the response file as not being able to see command-lines
# passed to the compiler drives me up the wall.
pushd ${PREFIX}/Library/bin
  echo "@echo off"                                          > c99-to-c89-cmake-nmake-wrap.bat
  echo "echo cd %CD%"                                      >> c99-to-c89-cmake-nmake-wrap.bat
  echo "setlocal EnableExtensions EnableDelayedExpansion"  >> c99-to-c89-cmake-nmake-wrap.bat
  echo "set respo=%1"                                      >> c99-to-c89-cmake-nmake-wrap.bat
  echo "echo ~dp0c99wrap.exe cl"                           >> c99-to-c89-cmake-nmake-wrap.bat
  echo "more !respo:~1!"                                   >> c99-to-c89-cmake-nmake-wrap.bat
  echo "~dp0c99wrap.exe cl %*"                             >> c99-to-c89-cmake-nmake-wrap.bat
popd
