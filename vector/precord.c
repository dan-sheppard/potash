#include <glib.h>

#include "precord.h"
#include "pfile.h"
#include "int4.h"

potash_pstream po_pstream_create(potash_pfile pf) {
	potash_pstream ps;
	
	ps=g_new(struct _potash_pstream,1);
	ps->pf=pf;
	return ps;
}

void po_pstream_destroy(potash_pstream ps) {
	g_free(ps);
}

void po_pstream_add_record(potash_pstream ps,potash_precord pr) {
	int i;
	
	/* XXX seek end etc when we also support reading */
	for(i=0;i<pr->buffer_offset;i++)
		po_pfile_put(ps->pf,pr->buffer[i]);
}

static void record_write(potash_precord pr,vint4 val) {
	if(pr->buffer_offset==pr->buffer_len) {
		if(pr->buffer) {
			pr->buffer_len=(pr->buffer_len*3)/2+1;
			pr->buffer=g_renew(vint4,pr->buffer,pr->buffer_len);
		} else {
			pr->buffer_len=32;
			pr->buffer=g_new(vint4,pr->buffer_len);
		}
	}
	pr->buffer[pr->buffer_offset++]=val;
}

potash_precord po_precord_create(int type) {
	potash_precord pr;
	
	pr=g_new(struct _potash_precord,1);
	pr->buffer=0;
	pr->buffer_offset=0;
	pr->buffer_len=0;
	record_write(pr,PRECORD_RECORD_REGULAR);
	record_write(pr,type);
	/* XXX write local context */
	record_write(pr,PRECORD_HEADBODY);	
	return pr;
}

void po_precord_destroy(potash_precord pr) {
	if(pr->buffer)
		g_free(pr->buffer);
	g_free(pr);
}
