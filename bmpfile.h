#ifndef __BMPFILE_H__
#define __BMPFILE_H__

/* BMP ��������Ͷ��� */
typedef struct {
    int   width;   /* ��� */
    int   height;  /* �߶� */
    int   stride;  /* ���ֽ��� */
    int   cdepth;  /* ����λ�� */
    void *pdata;   /* ָ������ */
} BMP;

int  bmp_load(BMP *pb, char *file);
int  bmp_save(BMP *pb, char *file);
void bmp_free(BMP *pb);
void bmp_setpixel(BMP *pb, int x, int y, int r, int g, int b);
void bmp_getpixel(BMP *pb, int x, int y, int *r, int *g, int *b);

#endif
