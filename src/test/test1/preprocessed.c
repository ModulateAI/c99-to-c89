




typedef struct AVRational { int num, den; } AVRational;
struct AVRational2 { int num; int den; char **test[3]; };
typedef struct AVRational2 AVRational2;
typedef struct AVRational4 AVRational4;
typedef struct { int num, den; struct AVRational test; } AVRational3;

static AVRational  gap_test() {
    AVRational gap = { .den = 4 };
    gap.num = 1;
    return gap;
}

static AVRational call_function_2(AVRational x)
{
    AVRational y = (struct AVRational) { x.den, x.num };
    int z = -1; 
    y = (AVRational) { y.den, y.num };
    if (z == 0)
        return (AVRational) { 5, -5 };
    else
        return x.num > 0 ? (AVRational) { x.num, x.den } :
               x.den > 0 ? (AVRational) { x.den, x.num } :
                           (AVRational) { 0, 0 };
}

static int call_function_3(AVRational x)
{
    return x.num ^ x.den;
}

static int call_function(AVRational x)
{
    AVRational y = x.num > 0 ? call_function_2((AVRational) { x.num, x.den }) :
                   x.den > 0 ? call_function_2((AVRational) { x.den, x.num }) :
                               call_function_2((AVRational) { 0, 0 });
    int res;

    if ((res = call_function_3((AVRational) { 5, -5 }) > 0)) {
        return ((AVRational) { -8, 8 }).den;
    } else if (1 && (res = call_function_3((AVRational) { 6, -6 }) > 0)) {
        return call_function_3((AVRational) { -5, 5 });
    } else
        return 0;
}



static const int l[][8] = {
    { 0, 0+1, 0+2, 0+3, 0+4, 0+4+1, 0+4+2, 0+4+3 },
    { 16, 16+1, 16+2, 16+3, 16+4, 16+4+1, 16+4+2, 16+4+3 },
    { 32, 32+1, 32+2, 32+3, 32+4, 32+4+1, 32+4+2, 32+4+3 },
    { 48, 48+1, 48+2, 48+3, 48+4, 48+4+1, 48+4+2, 48+4+3 }
};

typedef struct AVCodec {
    int (*decode) (AVRational x);
    const int *samplefmts;
} AVCodec;

static AVCodec decoder = {
    .samplefmts = (const int[]) { 0, 1 },
    .decode = call_function,
};

typedef struct AVFilterPad {
    const char *name;
} AVFilterPad;

typedef struct AVFilter {
    const char *name;
    const AVFilterPad *inputs;
} AVFilter;

AVFilter filter = {
    .name = "filter",
    .inputs = (const AVFilterPad[]) {{.name="pad",},{.name=(void*)0,},},
};

int main(int argc, char *argv[])
{
    int var;


    switch (call_function((AVRational){1, 1})) {
    case 0:
        call_function((AVRational){2, 2});
        break;
    default:
        call_function((AVRational){3, 3});
        break;
    }
    var = ((const int[2]){1,2})[argc];
    var = call_function((AVRational){1, 2});
    if (var == 0) return call_function((AVRational){3, 2});
    else          return call_function((AVRational){2, 3});

}
