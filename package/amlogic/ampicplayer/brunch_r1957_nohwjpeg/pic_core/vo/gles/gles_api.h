#ifndef GLES_API_H__
#define GLES_API_H__

extern void* local_init(void* arg);
extern int local_release(void* arg);
extern int local_getCurrentRes(void* arg,int* width,int* height);
extern int local_display(const char *arg,unsigned char *rgbbuff, int x_size, int y_size, int x_pan, int y_pan, int x_offs, int y_offs);
#endif /* GLES_API_H__ */