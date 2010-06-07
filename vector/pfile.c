#define _FILE_OFFSET_BITS 64
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>

#include <glib.h>

#include "int4.h"
#include "pfile.h"

#define VINT4MAX       5
#define BUFFER_SIZE   12 /* XXX small to try to trigger errors */

static struct potash_buffer * buffer_create(int fd) {
	struct potash_buffer *pb;
	
	pb=g_new(struct potash_buffer,1);
	pb->fd=fd;
	pb->data=g_new(unsigned char,BUFFER_SIZE);
	pb->size=BUFFER_SIZE;
	pb->len=0;
	pb->offset=0;
	pb->base_offset=0L;
	return pb;
}

static void buffer_destroy(struct potash_buffer *pb) {
	g_free(pb->data);
	g_free(pb);
}

static gboolean buffer_read_ensure(struct potash_buffer *pb,int num) {
	struct potash_buffer new;
	int r;

	while(pb->len-pb->offset < num) {
		/* Copy remainder in */
		new=*pb;
		new.data=g_new(unsigned char,new.size);
		memcpy(new.data,pb->data+pb->offset,pb->len-pb->offset);
		new.len=pb->len-pb->offset;
		new.offset=0;
		g_free(pb->data);
		*pb=new;
		/* Read data to top up */
		pb->base_offset=lseek(pb->fd,0L,SEEK_CUR);
		if(pb->base_offset==(off_t)-1)
			return FALSE;
		pb->base_offset-=pb->len;
		r=read(pb->fd,pb->data+pb->len,pb->size-pb->len);
		g_debug("read %d bytes",r);
		if(r==-1)
			return FALSE;
		pb->len+=r;
		if(r==0 && pb->len<pb->size) {
			/* Buffer to end with EOF */
			memset(pb->data+pb->len,0xFF,pb->size-pb->len);
			pb->len=pb->size;
		}
	}
	return TRUE;
}

static gboolean buffer_write_flush(struct potash_buffer *pb) {
	int r,n;

	g_debug("Write flush");
	/* write existing data */
	n=0;
	while(n<pb->len) {
		r=write(pb->fd,pb->data+n,pb->len-n);
		if(r==-1)
			return FALSE;
		n+=r;
	}
	/* sort out data structure */
	pb->len=0;
	return TRUE;	
}

static gboolean buffer_write_ensure(struct potash_buffer *pb,int num) {
	int r,n;

	if(pb->size-pb->len>=num)
		return TRUE;
	return buffer_write_flush(pb);
}

static gboolean buffer_read_flush(struct potash_buffer *pb) {
	off_t off;

	g_debug("Read flush");
	/* Seek to the right place in the file */
	off=lseek(pb->fd,pb->base_offset+pb->offset,SEEK_SET);
	if(off==(off_t)-1) {
		g_debug("error flushing");
		return FALSE;
	}
	/* sort out data structure */
	pb->base_offset=off;
	pb->offset=0;
	pb->len=0;
	return TRUE;
}

static void pf_err_close(potash_pfile pf) {
	g_debug("Error accessing file");
	pf->errval=errno;
	pf->type|=POTASH_PFILE_ERROR;
	buffer_destroy(pf->rdbuf);
	buffer_destroy(pf->wrbuf);
	close(pf->fd); /* Not much we can do about failure */
}

static void set_state(potash_pfile pf,int state) {
	gboolean flush_ok=TRUE;

	if((pf->type&POTASH_PFILE_STATE)==state)
		return;
	if(pf->type&POTASH_PFILE_ERROR)
		return;
	if(pf->type&POTASH_PFILE_RDSTATE)
		flush_ok=buffer_read_flush(pf->rdbuf);
	if(pf->type&POTASH_PFILE_WRSTATE)
		flush_ok=buffer_write_flush(pf->wrbuf);
	pf->type&=~POTASH_PFILE_STATE;
	if(!flush_ok) {
		pf_err_close(pf);
		return;
	}
	if((state&POTASH_PFILE_STATE)==POTASH_PFILE_WRSTATE &&
		(pf->type&POTASH_PFILE_MODE)==0) { /* Cannot write in read-only mode */
		pf_err_close(pf);
		return;
	}
	pf->type|=state&POTASH_PFILE_STATE;
}

potash_pfile po_pfile_open(char *filename,guint32 mode) {
	potash_pfile pf;
	int flags=0;
	
	pf=g_new(struct _potash_pfile,1);
	pf->type=mode&POTASH_PFILE_MODE;
	/* open */
	if(mode&POTASH_PFILE_RDWR)
		flags=O_RDWR;
	else
		flags=O_RDONLY;
	if(mode&POTASH_PFILE_CREATE)
		flags|=O_CREAT;
	if(mode&POTASH_PFILE_TRUNCATE)
		flags|=O_TRUNC;
	pf->fd=open(filename,flags,0777);
	if(pf->fd==-1) {
		pf->errval=errno;
		pf->type|=POTASH_PFILE_ERROR;
		return pf;
	}
	pf->rdbuf=buffer_create(pf->fd);
	pf->wrbuf=buffer_create(pf->fd);
	return pf;
}

void po_pfile_close(potash_pfile pf) {
	if(pf->type&POTASH_PFILE_ERROR)
		return;
	set_state(pf,0);
	buffer_destroy(pf->rdbuf);
	buffer_destroy(pf->wrbuf);
	close(pf->fd);
	g_free(pf);
}

gint64 po_pfile_get_pos(potash_pfile pf) {	
	off_t offset;

	if(po_pfile_error(pf))
		return -1L;
	set_state(pf,0);
	if(po_pfile_error(pf))
		return -1L;
	offset=lseek(pf->fd,0L,SEEK_CUR);
	if(offset==(off_t)-1) {
		pf_err_close(pf);
		return -1L;
	}
	return (gint64)offset;
}

void po_pfile_set_pos(potash_pfile pf,gint64 pos) {
	off_t offset;
	
	if(po_pfile_error(pf))
		return;
	set_state(pf,0);
	if(po_pfile_error(pf))
		return;
	offset=lseek(pf->fd,pos,SEEK_SET);
	if(offset==(off_t)-1) {
		pf_err_close(pf);
		return;
	}
}

gboolean po_pfile_error(potash_pfile pf) {
	return !!(pf->type&POTASH_PFILE_ERROR);
}

int po_pfile_errno(potash_pfile pf) {
	return pf->errval;
}

vint4 po_pfile_get(potash_pfile pf) {
	vint4 out;
	int n;

	if(po_pfile_error(pf)) {
		out=PO_EOF_VAL;
		return out;
	}
	set_state(pf,POTASH_PFILE_RDSTATE);
	if(po_pfile_error(pf)) {
		out=PO_EOF_VAL;
		return out;
	}
	if(!buffer_read_ensure(pf->rdbuf,VINT4MAX)) {
		pf_err_close(pf);
		out=PO_EOF_VAL;
		return out;		
	}
	if(po_pfile_error(pf)) {
		out=PO_EOF_VAL;
		return out;
	}
	n=po_int4_decode(pf->rdbuf->data+pf->rdbuf->offset,&out);
	pf->rdbuf->offset+=n;
	return out;
}

void po_pfile_put(potash_pfile pf,vint4 in) {
	int n;

	if(po_pfile_error(pf))
		return;
	set_state(pf,POTASH_PFILE_WRSTATE);
	if(po_pfile_error(pf))
		return;
	if(!buffer_write_ensure(pf->wrbuf,VINT4MAX)) {
		pf_err_close(pf);
		return;
	}
	if(po_pfile_error(pf))
		return;
	n=po_int4_encode(pf->wrbuf->data+pf->wrbuf->len,in);
	pf->wrbuf->len+=n;
}

#define SPOOL_BUFFER 8 /* XXX small to try to exercise bugs */

static void spool_jump(potash_pfile pf,unsigned char * buffer,int *n,off_t *offset,gboolean rev) {
	off_t old_offset,new;
	int r,c;

	/* Where do we want to seek to? */
	old_offset=*offset;
	if(rev)
		*offset-=SPOOL_BUFFER;
	else
		*offset+=SPOOL_BUFFER;
	if(*offset<0)
		*offset=0;
	g_debug("seeking to %ld",*offset);
	/* Try it */
	*n=abs((*offset)-old_offset);
	if(!*n)
		return;
	new=lseek(pf->fd,*offset,SEEK_SET);
	if(new==(off_t)-1) {
		if(errno==EINVAL && !rev) {
			*n=0; /* No more data */
			return;
		}
		pf_err_close(pf);
		return;
	}
	/* Fill buffer */
	c=0;
	while(c<SPOOL_BUFFER) {
		r=read(pf->fd,buffer+c,SPOOL_BUFFER-c);
		if(r==-1) {
			pf_err_close(pf);
			return;
		}
		c+=r;
		if(!r) {
			*n=c;
			return;
		}
	}
}

gboolean spool(potash_pfile pf,vint4 flag,gboolean rev) {
	unsigned char byte;
	off_t orig,offset,jump;
	unsigned char * buffer;
	int i,n,j;

	/* Which byte? */
	if(po_pfile_error(pf))
		return FALSE;
	if(PO_IS_TYPE_FLAG1(flag))
		byte=PO_FLAG1_BYTE(flag);
	else if(PO_IS_TYPE_FLAG2(flag))
		byte=PO_FLAG2_BYTE(flag);
	else
		return FALSE;		
	/* Find it */
	buffer=g_new(unsigned char,SPOOL_BUFFER);
	set_state(pf,0);
	offset=lseek(pf->fd,0L,SEEK_CUR);
	orig=offset;
	if(!rev)
		offset+=SPOOL_BUFFER; /* Re-subtracted in following reverse jump */
	spool_jump(pf,buffer,&n,&offset,TRUE);
	if(po_pfile_error(pf)) {
		g_free(buffer);
		return FALSE;
	}
	while(n) {
		/* Search for it */
		j=-1;
		if(rev) {
			for(i=0;i<n;i++)
				if(buffer[n-1-i]==byte) {
					j=n-1-i;
					break;
				}
		} else {
			for(i=0;i<n;i++)
				if(buffer[i]==byte) {
					j=i;
					break;
				}
		}
		if(j!=-1) {
			/* Found it */
			g_free(buffer);
			jump=lseek(pf->fd,offset+j,SEEK_SET);
			if(jump==(off_t)-1) {
				pf_err_close(pf);
				return FALSE;
			}
			return TRUE;
		}
		/* Try again */
		spool_jump(pf,buffer,&n,&offset,rev);
		if(po_pfile_error(pf)) {
			g_free(buffer);
			return FALSE;
		}
	}	
	g_free(buffer);
	jump=lseek(pf->fd,orig,SEEK_SET);
	if(jump==(off_t)-1) {
		pf_err_close(pf);
		return FALSE;
	}	
	return FALSE;
}

gboolean po_pfile_ffwd(potash_pfile pf,vint4 flag) {
	return spool(pf,flag,FALSE);
}

gboolean po_pfile_rev(potash_pfile pf,vint4 flag) {
	return spool(pf,flag,TRUE);
}
