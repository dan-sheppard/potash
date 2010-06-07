#ifndef POTASH_PFILE_H
#define POTASH_PFILE_H

#include <sys/types.h>
#include <unistd.h>
#include <glib.h>

#include "int4.h"

struct potash_buffer {
	int fd;
	unsigned char *data;
	int size,len;
	int offset;
	off_t base_offset;
};

typedef struct _potash_pfile {
	/* Metadata */
	int type;

	/* Buffers */
	struct potash_buffer *rdbuf,*wrbuf;

	/* OS stuff */
	int fd;
	int errval;
} * potash_pfile;

#define POTASH_PFILE_RDONLY   0x00000000
#define POTASH_PFILE_RDWR     0x00000001
#define POTASH_PFILE_CREATE   0x00000002
#define POTASH_PFILE_TRUNCATE 0x00000004
#define POTASH_PFILE_ERROR    0x00000010
#define POTASH_PFILE_RDSTATE  0x00000020
#define POTASH_PFILE_WRSTATE  0x00000040

#define POTASH_PFILE_STATE    0x000000F0
#define POTASH_PFILE_MODE     0x0000000F

potash_pfile po_pfile_open(char *filename,guint32 mode);
void po_pfile_close(potash_pfile);

vint4 po_pfile_get(potash_pfile);
void po_pfile_put(potash_pfile,vint4);

gint64 po_pfile_get_pos(potash_pfile);
void po_pfile_set_pos(potash_pfile,gint64);

gboolean po_pfile_error(potash_pfile);
int po_pfile_errno(potash_pfile pf);

gboolean po_pfile_ffwd(potash_pfile,vint4 flag);
gboolean po_pfile_rev(potash_pfile,vint4 flag);

#endif
