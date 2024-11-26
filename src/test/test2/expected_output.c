




typedef struct PixFmtInfo {
    int nb_channels, color_type, pixel_type, is_alpha, depth;
} PixFmtInfo;

enum PixelFormat {
    PIX_FMT_YUV420P,
    PIX_FMT_YUYV422,
    PIX_FMT_DUMMY3,
    PIX_FMT_YUVJ420P,
    PIX_FMT_RGB24,
    PIX_FMT_RGB555,
    PIX_FMT_RGBA,
    PIX_FMT_DUMMY,
    PIX_FMT_DUMMY2,
    PIX_FMT_GRAY8,
    PIX_FMT_NB,
};

enum ColorType {
    COLOR_RGB,
    COLOR_GRAY,
    COLOR_YUV,
    COLOR_YUV_JPEG,
};

enum PixelType {
    PIXEL_PLANAR = 2,
    PIXEL_PACKED = PIXEL_PLANAR + 1,
    PIXEL_PALETTE = 5,
};

static const struct PixFmtInfo pix_fmt_info[] = {
    {



















        3               ,
        COLOR_YUV                       ,
        PIXEL_PLANAR                          ,
        0, 12                ,
    },
    {






















        3               ,
        COLOR_YUV                       ,
        PIXEL_PACKED                          ,
        0, 16                ,
    }    ,
    {


























      2 },
    {
        3               ,
        COLOR_YUV_JPEG                            ,
        PIXEL_PLANAR                          ,
        0, 12                ,
    }, {3               ,
        COLOR_RGB                       ,
        PIXEL_PACKED                          ,
        0, 24                ,
    }










     , { 0 }, {4               ,
        COLOR_RGB                       ,
        PIXEL_PACKED                          ,
        1               ,
        32                ,
    }






















     , {1 }


























     ,
    { 0 }, {   1               ,
        COLOR_GRAY                        ,
                       42,
        0, 8               ,
    }





         ,
};

int mixed_array[] = {
    0, 1                    ,
    2,
    0, 0, 0, 3                 ,
    4,
};

static const struct {
    int a;
    struct { int c, d; } b;
} random_values2 = {
    0, {      0, 1     , },
}, random_values3 = {
    0, { 0, 1     , },
}, random_values4 = {
    1     , { 2, 3 },
};

static const struct {
    int a, b;
} random_values[] = {
    { 0, 1     , },
    { 0 }, { 0 }, {       0, 3      },
};

static const struct PixFmtInfo info2 = {
    3, COLOR_YUV_JPEG, PIX_FMT_YUVJ420P, 0, 12
};

typedef struct {
    const char *name;
    union {
        void *dst_ptr;
        int (*func_arg)(void);
        double dbl;
    } u;
} OptionDef;

static int do_nothing(void) { }
static const OptionDef options[] = {
  { "name", {(void*)0,},},
  { "name2", {(void*) (intptr_t) do_nothing                    },},
  { "name3", {(void*) 0x3fe0000000000000                      },},
};

union av_intfloat32 {
    int   i;
    float f;
};

typedef union {
    unsigned long long u64;
    unsigned int   u32[2];
    unsigned short u16[4];
    unsigned char  u8 [8];
    double   f64;
    float    f32[2];
} av_alias64;

unsigned long long func1(void);
unsigned long long func2(void);

int foo(float f) {
    union av_intfloat32 s ;s.f=f;{
    int other;
    return s.i;
}}

static double tget_double(int le)
{
    av_alias64 i ;i.u64=le?func1():func2();{
    return i.f64;
}}

extern __inline __attribute__ ((__always_inline__)) __attribute__ ((__gnu_inline__, __artificial__)) int
__attribute__ ((__nothrow__ , __leaf__)) __signbitl (long double __x)
{
  __extension__ union { long double __l; int __i[3]; } __u ;__u.__l=__x;{
  return (__u.__i[2] & 0x8000) != 0;
}}

int main(int argc, char *argv[])
{
    printf("Hi\n");
    { int j = PIX_FMT_NB;
    {   int i = 0; for (      ; i < j; i++)
        return pix_fmt_info[i].depth; }
    return j != 0; }
}
