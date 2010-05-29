#include <glib.h>
#include <cairo.h>

#include "stack.h"
#include "layer.h"
#include "tiles.h"
#include "tiles_makers.h"

static void fade_alpha(cairo_surface_t *src,guint32 alpha) {
	cairo_t * cr;
	
	cr=cairo_create(src);
	cairo_set_source_rgba(cr,0.0,0.0,0.0,((double)alpha)/255);
	cairo_set_operator(cr,CAIRO_OPERATOR_DEST_IN);
	cairo_rectangle(cr,0.0,0.0,
						 cairo_image_surface_get_width(src),
						 cairo_image_surface_get_height(src));
	cairo_fill(cr);
	cairo_destroy(cr);
}

static void stack_maker(potash_tiles tiles,int x,int y,
								cairo_surface_t *surface,unsigned char *surface_data,
								gpointer payload) {
	potash_stack stack;
	potash_tile tsrc,tmixed;
	int i;
	
	stack=(potash_stack)payload;
	g_debug("Reconstructing stack surface at (%d,%d) ",x,y);
	for(i=0;i<stack->size;i++) {
		g_debug("Handling layer %d",i);
		tsrc=po_layer_get_tile(stack->stack[i].layer,x,y);
		tmixed=po_tile_create(tiles,0,0,PO_TILE_TMP,po_tmaker_copy,
									 po_tile_surface(tsrc));
		fade_alpha(po_tile_surface(tmixed),stack->stack[i].alpha);
		(*stack->stack[i].compose)(surface,po_tile_surface(tmixed),x,y);
		po_layer_put_tile(tsrc);
		po_tile_unref(tmixed);
		po_tile_destroy(tmixed);
	}
}

potash_stack po_stack_create(potash_tiles tiles) {
	potash_stack stack;
	
	stack=g_new(struct _potash_stack,1);
	stack->stack=0;
	stack->len=0;
	stack->size=0;
	stack->tiles=tiles;
	stack->layer=po_layer_create(tiles,PO_TILE_SYNTHETIC,stack_maker,stack,0);
	return stack;
}

void po_stack_destroy(potash_stack stack) {
	po_layer_destroy(stack->layer);
	if(stack->stack)
		g_free(stack->stack);
	g_free(stack);
}

void po_stack_layer_add(potash_stack stack,
								potash_layer layer,potash_stack_compose compose,
								guint32 alpha) {
	struct potash_stack_el *el;

	if(stack->len<=stack->size) {
		stack->len=stack->len*3/2+1; /* slow exp: 1 2 4 7 11 17 26 40 61 92 139 */
		if(stack->stack)
			stack->stack=g_renew(struct potash_stack_el,stack->stack,stack->len);
		else
			stack->stack=g_new(struct potash_stack_el,stack->len);
	}
	el=&stack->stack[stack->size++];
	el->layer=layer;
	el->compose=compose;
	el->alpha=alpha;
}

potash_layer po_stack_layer(potash_stack stack) {
	return stack->layer;
}