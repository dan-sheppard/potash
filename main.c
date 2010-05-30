#include <stdlib.h>
#include "tiles/tiles.h"
#include "tiles/tiles_makers.h"
#include "tiles/layer.h"
#include "tiles/stack.h"
#include "tiles/stack_composers.h"
#include "confdir/confdir.h"

/* TODO something faster than PNG for tiles */

int main(int argc,char **argv) {
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
	tiles=po_tiles_create(cd,256,4);
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
	t0=po_layer_get_tile(layer,0,0);
	pod_tile_debug(t0,"test3");
	po_layer_put_tile(t0);
	
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
	for(j=0;j<5;j++)
		for(i=0;i<5;i++) {
			t0=po_layer_get_tile(layer,i,j);
			fn=g_strdup_printf("rec-%d-%d",i,j);
			pod_tile_debug(t0,fn);
			po_layer_put_tile(t0);
			g_free(fn);
		}	
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
