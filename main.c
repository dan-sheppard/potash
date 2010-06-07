#include <stdlib.h>

#include <glib.h>

#include "tiles/tiles.h"
#include "tiles/tiles_makers.h"
#include "tiles/layer.h"
#include "tiles/stack.h"
#include "tiles/stack_composers.h"
#include "confdir/confdir.h"
#include "vector/int4.h"
#include "vector/pfile.h"
#include "vector/precord.h"

void pbuf(vint4 v,unsigned char *in,int n,vint4 w) {
	int i;
	
	fprintf(stderr,"%16.16lX ::: ",v,n);
	for(i=0;i<n;i++)
	  fprintf(stderr,"%2.2X ",in[i]);
	fprintf(stderr," ::: %16.16lX\n",w);
}

static guint32 r() {
	guint32 out;
	
	return ((rand()&0xFF)<<0)|
		    ((rand()&0xFF)<<8)|
		    ((rand()&0xFF)<<16)|
		    ((rand()&0xFF)<<24);
}

static void pb_read(struct potash_buffer *pb) {
	int v;
	
	v=pb->data[pb->offset++];
	fprintf(stderr,"{offset=%d len=%d size=%d} -> %2.2X\n",pb->offset,pb->len,pb->size,v);
}

/* XXX rev/ffwd */
/* XXX open and close pf in ps */
int main(int argc,char **argv) {
	potash_pfile pf;
	potash_pstream ps;
	potash_precord pr;
	gint64 off1;
	
	pf=po_pfile_open("test.ash",POTASH_PFILE_RDWR|POTASH_PFILE_CREATE|POTASH_PFILE_TRUNCATE);
	ps=po_pstream_create(pf);
	pr=po_precord_create(42);
	po_pstream_add_record(ps,pr);
	po_precord_destroy(pr);
	po_pstream_destroy(ps);
	po_pfile_close(pf);
	pf=po_pfile_open("test.ash",POTASH_PFILE_RDWR);	
	g_debug("good %d",po_pfile_ffwd(pf,PRECORD_HEADBODY));
	off1=po_pfile_get_pos(pf);
	g_debug("off1=%ld",off1);
	g_debug("good %d",po_pfile_rev(pf,PRECORD_RECORD_REGULAR));
	off1=po_pfile_get_pos(pf);
	g_debug("off1=%ld",off1);
	g_debug("bad %d",po_pfile_ffwd(pf,PRECORD_SEP_TRIVIAL));
	off1=po_pfile_get_pos(pf);
	g_debug("off1=%ld",off1);
	g_debug("bad %d",po_pfile_rev(pf,PRECORD_SEP_TRIVIAL));
	off1=po_pfile_get_pos(pf);
	g_debug("off1=%ld",off1);
	po_pfile_close(pf);
}

#if 0
int third_main(int argc,char **argv) {
	potash_pfile pf;
	struct potash_buffer *pb;
	gint64 off0,off1;
	vint4 v0;
	
	pf=po_pfile_open("test.ash",POTASH_PFILE_RDWR|POTASH_PFILE_CREATE|POTASH_PFILE_TRUNCATE);
	off0=po_pfile_get_pos(pf);
	g_debug("off0=%ld",off0);
	v0=PO_NUMBER_VAL(42);
	po_pfile_put(pf,v0);
	off1=po_pfile_get_pos(pf);
	g_debug("off1=%ld",off1);
	v0=PO_NUMBER_VAL(4242);
	po_pfile_put(pf,v0);
	v0=PO_NUMBER_VAL(42424);
	po_pfile_put(pf,v0);
	v0=PO_NUMBER_VAL(424242);
	po_pfile_put(pf,v0);
	v0=PO_NUMBER_VAL(42424242);
	po_pfile_put(pf,v0);
	v0=PO_FLAG2_VAL(4,42);
	po_pfile_put(pf,v0);
	po_pfile_set_pos(pf,off1);
	v0=po_pfile_get(pf);
	g_debug("Got %16.16lX %d",v0,PO_GET_NUMBER(v0));
	v0=po_pfile_get(pf);
	g_debug("Got %16.16lX %d",v0,PO_GET_NUMBER(v0));
	v0=po_pfile_get(pf);
	g_debug("Got %16.16lX %d",v0,PO_GET_NUMBER(v0));
	v0=po_pfile_get(pf);
	g_debug("Got %16.16lX %d",v0,PO_GET_NUMBER(v0));
	v0=po_pfile_get(pf);
	g_debug("Got %16.16lX %d/%d",v0,PO_GET_FLAG_INDEX(v0),PO_GET_FLAG2_VALUE(v0));
	po_pfile_set_pos(pf,off0);
	v0=po_pfile_get(pf);
	g_debug("Got %16.16lX %d",v0,PO_GET_NUMBER(v0));
	v0=po_pfile_get(pf);
	g_debug("Got %16.16lX %d",v0,PO_GET_NUMBER(v0));
	po_pfile_close(pf);
}

#if 0
int another_main(int argc,char **argv) {
	vint4 v,w;
	int i,j;
	unsigned char buf[5];
	
	srand(time(NULL));	
	
	PO_SET_FLAG1(v,3);
	i=po_int4_encode(buf,v);
	po_int4_decode(buf,&w);
	pbuf(v,buf,i,w);

	PO_SET_FLAG1(v,7);
	i=po_int4_encode(buf,v);
	po_int4_decode(buf,&w);
	pbuf(v,buf,i,w);

	PO_SET_FLAG2(v,5,42);
	i=po_int4_encode(buf,v);
	po_int4_decode(buf,&w);
	pbuf(v,buf,i,w);
	
	PO_SET_EOF(v);
	i=po_int4_encode(buf,v);
	po_int4_decode(buf,&w);
	pbuf(v,buf,i,w);
	
	PO_SET_NUMBER(v,0);
	i=po_int4_encode(buf,v);
	po_int4_decode(buf,&w);
	pbuf(v,buf,i,w);		

	PO_SET_NUMBER(v,127);
	i=po_int4_encode(buf,v);
	po_int4_decode(buf,&w);
	pbuf(v,buf,i,w);		

	PO_SET_NUMBER(v,128);
	i=po_int4_encode(buf,v);
	po_int4_decode(buf,&w);
	pbuf(v,buf,i,w);

	PO_SET_NUMBER(v,8191);
	i=po_int4_encode(buf,v);
	po_int4_decode(buf,&w);
	pbuf(v,buf,i,w);

	PO_SET_NUMBER(v,8192);
	i=po_int4_encode(buf,v);
	po_int4_decode(buf,&w);
	pbuf(v,buf,i,w);

	PO_SET_NUMBER(v,262143);
	i=po_int4_encode(buf,v);
	po_int4_decode(buf,&w);
	pbuf(v,buf,i,w);

	PO_SET_NUMBER(v,262144);
	i=po_int4_encode(buf,v);
	po_int4_decode(buf,&w);
	pbuf(v,buf,i,w);

	PO_SET_NUMBER(v,2097151);
	i=po_int4_encode(buf,v);
	po_int4_decode(buf,&w);
	pbuf(v,buf,i,w);

	PO_SET_NUMBER(v,2097152);
	i=po_int4_encode(buf,v);
	po_int4_decode(buf,&w);
	pbuf(v,buf,i,w);

	PO_SET_NUMBER(v,0xFFFFFFFF);
	i=po_int4_encode(buf,v);
	po_int4_decode(buf,&w);
	pbuf(v,buf,i,w);
	
	for(j=0;j<100000000;j++) {
		PO_SET_NUMBER(v,r());
		i=po_int4_encode(buf,v);
		po_int4_decode(buf,&w);
		if(v!=w)
			g_error("Mismatch1 %16.16X",v);
		if(!(j%1000000))
			pbuf(v,buf,i,w);
	}
	for(j=0;j<100000000;j++) {
		PO_SET_NUMBER(v,r()%300000);
		i=po_int4_encode(buf,v);
		po_int4_decode(buf,&w);
		if(v!=w)
			g_error("Mismatch1 %16.16X",v);
		if(!(j%1000000))
			pbuf(v,buf,i,w);
	}
	for(j=0;j<100000000;j++) {
		PO_SET_NUMBER(v,r()%10000);
		i=po_int4_encode(buf,v);
		po_int4_decode(buf,&w);
		if(v!=w)
			g_error("Mismatch1 %16.16X",v);
		if(!(j%1000000))
			pbuf(v,buf,i,w);
	}
	for(j=0;j<100000000;j++) {
		PO_SET_NUMBER(v,r()%300);
		i=po_int4_encode(buf,v);
		po_int4_decode(buf,&w);
		if(v!=w)
			g_error("Mismatch1 %16.16X",v);
		if(!(j%1000000))
			pbuf(v,buf,i,w);
	}
	for(j=0;j<100000000;j++) {
		PO_SET_FLAG1(v,r()%8);
		i=po_int4_encode(buf,v);
		po_int4_decode(buf,&w);
		if(v!=w)
			g_error("Mismatch2 %16.16X",v);
	}
	for(j=0;j<100000000;j++) {
		PO_SET_FLAG2(v,r()%7,r()%128);
		i=po_int4_encode(buf,v);
		po_int4_decode(buf,&w);
		if(v!=w)
			g_error("Mismatch3 %16.16X %16.16X",v,w);
	}

}
#endif

/* TODO something faster than PNG for tiles */

int old_main(int argc,char **argv) {
   potash_confdir cd;
	potash_tiles tiles;
	potash_tile t0,t1,t2,tile[100];
	int i,j,used[100];
	potash_layer layer,y0,y1,y2;
	potash_stack stack;
	cairo_surface_t *surface;
	cairo_t *cr;
	gchar *fn;

	cd=po_confdir_setup(".potash");
	tiles=po_tiles_create(cd,256,128);
#if 0
	t0=po_tile_create(tiles,0,0,0,po_tmaker_opaque_white,0);
	pod_tile_scribble(t0,1);
	po_tile_unref(t0);
	po_tile_ref(t0);
	pod_tile_scribble(t0,2);
	pod_tile_debug(t0,"test");
	po_tile_unref(t0);
	for(i=0;i<100;i++) {
		tile[i]=po_tile_create(tiles,0,0,0,po_tmaker_opaque_white,0);
		po_tile_unref(tile[i]);
		used[i]=0;
	}
	for(i=0;i<1000;i++) {
		for(j=rand()%100;used[j];j=rand()%100)
			;
		po_tile_ref(tile[j]);
		used[j]=1;
		for(j=rand()%100;!used[j];j=rand()%100)
			;
		po_tile_unref(tile[j]);
		used[j]=0;
		pod_tiles_debug(tiles);	
	}
	pod_tile_debug(t0,"test");
	t1=po_tile_create(tiles,0,0,PO_TILE_TMP,po_tmaker_opaque_white,0);
	po_tile_unref(t1);
	t2=po_tile_create(tiles,0,0,PO_TILE_SYNTHETIC,po_tmaker_opaque_white,0);
	po_tile_unref(t2);
	fprintf(stderr,"Synthetic\n");
	pod_tile_debug(t2,"test2");
	for(i=0;i<8;i++) {	
		po_tile_ref(tile[i]);
		po_tile_unref(tile[i]);
	}
	po_tile_ref(t2);
	po_tile_unref(t2);
	po_tile_destroy(t0);
	po_tile_destroy(t1);
	po_tile_destroy(t2);
	for(i=0;i<100;i++)
		po_tile_destroy(tile[i]);
	/**/
	layer=po_layer_create(tiles,0,po_tmaker_opaque_white,0,0);
	po_layer_destroy(layer);
	layer=po_layer_create(tiles,0,po_tmaker_opaque_white,0,0);
	for(j=0;j<10;j++)
		for(i=0;i<10;i++) {
			t0=po_layer_get_tile(layer,((i%2)?-1:1)*(i/2),((j%2)?-1:1)*(j/2));
			po_layer_put_tile(t0);
		}
	po_layer_destroy(layer);
#endif
	/**/
	stack=po_stack_create(tiles);
	y0=po_layer_create(tiles,0,pod_tmaker_debug,
		pod_tmaker_debug_data(255,255,0,0,"y0"),pod_tmaker_debug_data_free);
	y1=po_layer_create(tiles,0,pod_tmaker_debug,
			pod_tmaker_debug_data(255,0,255,0,"y1"),pod_tmaker_debug_data_free);
	y2=po_layer_create(tiles,0,po_tmaker_transparent_white,0,0);
	po_stack_layer_add(stack,y0,po_stack_compose_normal,255);
	po_stack_layer_add(stack,y1,po_stack_compose_normal,128);
	po_stack_layer_add(stack,y2,po_stack_compose_normal,255);	
	
	layer=po_stack_layer(stack);
	t0=po_layer_get_tile(layer,0,0,PO_TILE_RDONLY);
	pod_tile_debug(t0,"test3");
	po_layer_put_tile(layer,t0);
	
	surface=cairo_image_surface_create(CAIRO_FORMAT_ARGB32,500,500);
	cr=cairo_create(surface);
	cairo_set_source_rgb(cr,1.0,1.0,1.0);
	cairo_set_operator(cr,CAIRO_OPERATOR_CLEAR);
	cairo_rectangle(cr,0.0,0.0,500,500);
	cairo_fill(cr);
	cairo_destroy(cr);	
	cr=cairo_create(surface);		
	cairo_set_source_rgba(cr,0.0,0.0,1.0,0.5);
	cairo_rectangle(cr,0.0,0.0,500,500);
	cairo_fill(cr);
	cairo_destroy(cr);
	po_layer_print(y2,surface,100,100,po_stack_compose_normal,255);	
	cairo_surface_destroy(surface);
	/**/
	surface=cairo_image_surface_create(CAIRO_FORMAT_ARGB32,100,100);
	cr=cairo_create(surface);		
	cairo_set_source_rgba(cr,1.0,0.0,0.0,0.5);
	cairo_rectangle(cr,0.0,0.0,100,100);
	cairo_fill(cr);
	cairo_destroy(cr);
	po_layer_print(y2,surface,-70,-70,po_stack_compose_normal,255);	
	cairo_surface_destroy(surface);	
	/**/
	surface=cairo_image_surface_create(CAIRO_FORMAT_ARGB32,600,600);
	po_layer_develop(surface,layer,-100,-100,100,100,400,400);
	cairo_surface_write_to_png(surface,"develop.png");
	cairo_surface_destroy(surface);	
	
	po_stack_destroy(stack);
	po_layer_destroy(y0);
	po_layer_destroy(y1);	
	po_layer_destroy(y2);
	/**/
	po_tiles_destroy(tiles);
	po_confdir_destroy(cd,FALSE);
	return 0;
}

#endif