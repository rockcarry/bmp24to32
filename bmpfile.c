#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "bmpfile.h"

typedef unsigned char  uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int   uint32_t;

/* 内部函数实现 */
static int ALIGN(int x, int y) {
    // y must be a power of 2.
    return (x + y - 1) & ~(y - 1);
}

//++ for bmp file ++//
// 内部类型定义
#pragma pack(1)
typedef struct {
    uint16_t  bfType;
    uint32_t  bfSize;
    uint16_t  bfReserved1;
    uint16_t  bfReserved2;
    uint32_t  bfOffBits;
    uint32_t  biSize;
    uint32_t  biWidth;
    uint32_t  biHeight;
    uint16_t  biPlanes;
    uint16_t  biBitCount;
    uint32_t  biCompression;
    uint32_t  biSizeImage;
    uint32_t  biXPelsPerMeter;
    uint32_t  biYPelsPerMeter;
    uint32_t  biClrUsed;
    uint32_t  biClrImportant;
} BMPFILEHEADER;
#pragma pack()

/* 函数实现 */
int bmp_load(BMP *pb, char *file)
{
    BMPFILEHEADER header = {0};
    FILE         *fp     = NULL;
    uint8_t      *pdata  = NULL;
    int           i;

    fp = fopen(file, "rb");
    if (!fp) return -1;

    fread(&header, sizeof(header), 1, fp);
    pb->width  = header.biWidth;
    pb->height = header.biHeight;
    pb->stride = ALIGN(header.biWidth * 3, 4);
    pb->cdepth = 24;
    pb->pdata  = malloc(pb->stride * pb->height);
    if (pb->pdata) {
        pdata  = (uint8_t*)pb->pdata + pb->stride * pb->height;
        for (i=0; i<pb->height; i++) {
            pdata -= pb->stride;
            fread(pdata, pb->stride, 1, fp);
        }
    }

    fclose(fp);
    return pb->pdata ? 0 : -1;
}

int bmp_create(BMP *pb)
{
    pb->stride = ALIGN(pb->width * (pb->cdepth / 8), 4);
    pb->pdata  = calloc(1, pb->width * pb->stride);
    return pb->pdata ? 0 : -1;
}

int bmp_save(BMP *pb, char *file)
{
    BMPFILEHEADER header = {0};
    FILE         *fp     = NULL;
    uint8_t      *pdata;
    int           i;

    header.bfType     = ('B' << 0) | ('M' << 8);
    header.bfSize     = sizeof(header) + pb->stride * pb->height;
    header.bfOffBits  = sizeof(header);
    header.biSize     = 40;
    header.biWidth    = pb->width;
    header.biHeight   = pb->height;
    header.biPlanes   = 1;
    header.biBitCount = pb->cdepth;
    header.biSizeImage= pb->stride * pb->height;

    fp = fopen(file, "wb");
    if (fp) {
        fwrite(&header, sizeof(header), 1, fp);
        pdata = (uint8_t*)pb->pdata + pb->stride * pb->height;
        for (i=0; i<pb->height; i++) {
            pdata -= pb->stride;
            fwrite(pdata, pb->stride, 1, fp);
        }
        fclose(fp);
    }

    return fp ? 0 : -1;
}

void bmp_free(BMP *pb)
{
    if (pb->pdata) {
        free(pb->pdata);
        pb->pdata = NULL;
    }
    pb->width  = 0;
    pb->height = 0;
    pb->stride = 0;
    pb->cdepth = 0;
}

void bmp_setpixel(BMP *pb, int x, int y, int r, int g, int b)
{
    uint8_t *pbyte = pb->pdata;
    if (x < 0 || x >= pb->width || y < 0 || y >= pb->height) return;
    r = r < 0 ? 0 : r < 255 ? r : 255;
    g = g < 0 ? 0 : g < 255 ? g : 255;
    b = b < 0 ? 0 : b < 255 ? b : 255;
    pbyte[x * (pb->cdepth / 8) + 0 + y * pb->stride] = r;
    pbyte[x * (pb->cdepth / 8) + 1 + y * pb->stride] = g;
    pbyte[x * (pb->cdepth / 8) + 2 + y * pb->stride] = b;
}

void bmp_getpixel(BMP *pb, int x, int y, int *r, int *g, int *b)
{
    uint8_t *pbyte = pb->pdata;
    if (x < 0 || x >= pb->width || y < 0 || y >= pb->height) {
        *r = *g = *b = 0;
        return;
    }
    *r = pbyte[x * (pb->cdepth / 8) + 0 + y * pb->stride];
    *g = pbyte[x * (pb->cdepth / 8) + 1 + y * pb->stride];
    *b = pbyte[x * (pb->cdepth / 8) + 2 + y * pb->stride];
}

int main(int argc, char *argv[])
{
    BMP  bmp24 = {}, bmp32 = {};
    int  x = 0, y = 0, r, g, b, i, j;
    char file[256];

    if (argc < 2) {
        printf("\n");
        printf("tool for convert 24bit bitmap to 32bit\n\n");
        printf("usage:\n");
        printf("  bmp24to32 bmpfile \n\n");
        return 0;
    }

    bmp_load(&bmp24, argv[1]);
    bmp32.width  = bmp24.width;
    bmp32.height = bmp24.height;
    snprintf(file, sizeof(file), "%s.32", argv[1]);

    for (i = 2; i < argc; i++) {
        if      (strstr(argv[i], "--srcx=") == argv[i]) x = atoi(argv[i] + 7);
        else if (strstr(argv[i], "--srcy=") == argv[i]) y = atoi(argv[i] + 7);
        else if (strstr(argv[i], "--srcw=") == argv[i]) bmp32.width  = atoi(argv[i] + 7);
        else if (strstr(argv[i], "--srch=") == argv[i]) bmp32.height = atoi(argv[i] + 7);
        else if (strstr(argv[i], "--out=" ) == argv[i]) strncpy(file, argv[i] + 6, sizeof(file));
    }

    bmp32.cdepth = 32;
    bmp_create(&bmp32);

    for (i = 0; i < bmp32.height; i++) {
        for (j = 0; j < bmp32.width; j++) {
            bmp_getpixel(&bmp24, x + j, y + i, &r, &g, &b);
            bmp_setpixel(&bmp32, 0 + j, 0 + i,  r,  g,  b);
        }
    }

    bmp_save(&bmp32, file);
    bmp_free(&bmp24);
    bmp_free(&bmp32);
}


