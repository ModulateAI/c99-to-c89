




typedef struct AVRational { int num, den; } AVRational;
struct AVRational2 { int num; int den; char **test[3]; };
typedef struct AVRational2 AVRational2;
typedef struct AVRational4 AVRational4;
typedef struct { int num, den; struct AVRational test; } AVRational3;

static AVRational  gap_test() {
    AVRational gap = { 0, 4        };
    gap.num = 1;
    return gap;
}

static AVRational call_function_2(AVRational x)
{
    AVRational y =                    { x.den, x.num };
    int z = -1;
    {     AVRational tmp__0 =   { y.den, y.num }; y = tmp__0                            ; }
    if (z == 0)
        {   AVRational tmp__1 =   { 5, -5 }; return tmp__1                     ; }
    else
        {               AVRational tmp__2 =   { x.num, x.den }; { 
                            AVRational tmp__3 =   { x.den, x.num }; { 

                            AVRational tmp__4 =   { 0, 0 }; return x.num > 0 ? tmp__2                             :
               x.den > 0 ? tmp__3                             :
                           tmp__4                    ; } } }
}

static int call_function_3(AVRational x)
{
    return x.num ^ x.den;
}

static int call_function(AVRational x)
{
    {                                   AVRational tmp__5 =   { x.num, x.den }; { 
                                                AVRational tmp__6 =   { x.den, x.num }; { 

                                                AVRational tmp__7 =   { 0, 0 }; AVRational y = x.num > 0 ? call_function_2(tmp__5                            ) :
                   x.den > 0 ? call_function_2(tmp__6                            ) :
                               call_function_2(tmp__7                    );
    int res;

    {                           AVRational tmp__8 =   { 5, -5 }; if ((res = call_function_3(tmp__8                     ) > 0)) {
        {    AVRational tmp__9 =   { -8, 8 }; return (tmp__9                     ).den; }
    } else {                                AVRational tmp__10 =   { 6, -6 }; if (1 && (res = call_function_3(tmp__10                     ) > 0)) {
        {                   AVRational tmp__11 =   { -5, 5 }; return call_function_3(tmp__11                     ); }
    } else
        return 0; } }
} } } }



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

static 
                   const int tmp__12[] =   { 0, 1 };static AVCodec decoder = {
    call_function                                   ,
    tmp__12                      ,
};

typedef struct AVFilterPad {
    const char *name;
} AVFilterPad;

typedef struct AVFilter {
    const char *name;
    const AVFilterPad *inputs;
} AVFilter;

static 

               const AVFilterPad tmp__13[] =   {{"pad"          ,},{(      void*)0,},};AVFilter filter = {
    "filter"               ,
    tmp__13                                                                  ,
};

int main(int argc, char *argv[])
{
    int var;


    {                  AVRational tmp__14 =  {1, 1}; switch (call_function(tmp__14                 )) {
    case 0:
        {   AVRational tmp__15 =  {2, 2}; call_function(tmp__15                 ); }
        break;
    default:
        {   AVRational tmp__16 =  {3, 3}; call_function(tmp__16                 ); }
        break;
    } }
    {      const int tmp__17[2] =  {1,2}; var = (tmp__17                  )[argc]; }
    {                   AVRational tmp__18 =  {1, 2}; var = call_function(tmp__18                 ); }
    if (var == 0) {                 AVRational tmp__19 =  {3, 2}; return call_function(tmp__19                 ); }
    else          {                 AVRational tmp__20 =  {2, 3}; return call_function(tmp__20                 ); }

}
