#ifndef _GZIP_H_
#define _GZIP_H_


int gzCompress(unsigned char *data, unsigned long ndata, unsigned char *zdata, unsigned long *nzdata);
int gzDecompress(unsigned char *zdata, unsigned long nzdata, unsigned char *data, unsigned long *ndata);
unsigned long gzCRC32(unsigned char *buf, unsigned int len);

#endif
