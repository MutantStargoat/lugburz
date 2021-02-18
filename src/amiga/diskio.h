#ifndef DISKIO_H_
#define DISKIO_H_

int read_sectors(void *sbuf, int start, int count);
int write_sectors(void *sbuf, int start, int count);

#endif	/* DISKIO_H_ */
