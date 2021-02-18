#include <stdio.h>
#include <string.h>
#include "diskio.h"
#include "hwregs.h"

/* MFM encoding:
 * 1 -> 01
 * 0 -> 10 after 0
 * 0 -> 00 after 1
 */

#define MFM_SYNC		0x4489
#define NSECT_TRACK		11
#define NWORDS_TRACK	(NSECT_TRACK << 8)

/* XXX this seems like a good place for a track buffer */
#define TRACKBUF_ADDR		0x8000
static unsigned char *trackbuf = (unsigned char*)TRACKBUF_ADDR;
static int tbuf_track = -1;

static int read_track(int track);

int read_sectors(void *buf, int start, int count)
{
	int trk, soffs, sz;
	char *dest = buf;

	trk = start / NSECT_TRACK;
	soffs = start % NSECT_TRACK;

	while(count > 0) {
		if(read_track(trk) == -1) {
			return -1;
		}

		/* TODO MFM decode here instead of memcpy? or in read_track? */

		sz = (NSECT_TRACK - soffs) << 9;
		memcpy(dest, trackbuf, sz);
		dest += sz;
		count -= NSECT_TRACK - soffs;
		soffs = 0;
		trk++;
	}

	return 0;
}

int write_sectors(void *buf, int start, int count)
{
	return -1;
}


static int read_track(int track)
{
	REG_DSKLEN = 0;

	REG_DSKSYNC = MFM_SYNC;

	REG_ADKCON = CLRBITS(ADKCON_PRECOMP_MASK);	/* precompensation = 0 */
	REG_ADKCON = SETBITS(ADKCON_FAST | ADKCON_WORDSYNC | ADKCON_MFMPREC);

	REG_DMACON = SETBITS(DMA_DISK);				/* enable disk DMA */
	REG32_DSKPT = TRACKBUF_ADDR;				/* set DMA address */
	REG_DSKLEN = NWORDS_TRACK | DSKLEN_DMAEN;	/* set length and start */
	REG_DSKLEN = NWORDS_TRACK | DSKLEN_DMAEN;	/* ... twice */

	/* TODO wait */

	REG_DSKLEN = 0;

	tbuf_track = track;
	return -1;
}
