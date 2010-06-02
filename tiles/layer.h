#ifndef POTASH_LAYER_H
#define POTASH_LAYER_H

#include <glib.h>
#include "tiles.h"

typedef void (*potash_stack_compose)(cairo_surface_t *dst,cairo_surface_t *src,
									   		 int x,int y,
									   		 int xsrc,int ysrc,int xdst,int ydst,
									   		 int xs,int ys);

struct potash_matrix {
	int x_offset,y_offset;
	int x_size,y_size;
	potash_tile * tiles;
};

struct potash_layer_list;

typedef struct _potash_layer {
	/* Metadata */
	potash_tiles tiles;
	int tile_flags;
	potash_tile_maker maker;
	gpointer maker_data;
	GDestroyNotify maker_free;

	/* Dirty lists */
	struct potash_layer_list *dirties,*is_dirtied_by;

	/* Store */
	struct potash_matrix matrix;
} * potash_layer;

struct potash_layer_list {
	potash_layer layer;
	struct potash_layer_list *next;
};

potash_layer po_layer_create(potash_tiles ts,int tile_flags,
									  potash_tile_maker maker,gpointer maker_data,
									  GDestroyNotify maker_free);
void po_layer_destroy(potash_layer);
potash_tile po_layer_get_tile(potash_layer,int x,int y,int mode);
void po_layer_put_tile(potash_layer,potash_tile t);
void po_layer_flush_tile(potash_layer,int,int);

void po_layer_dirties(potash_layer dirties,potash_layer dirtier);

void po_layer_print(potash_layer,cairo_surface_t *,gint64 x,gint64 y,
						  potash_stack_compose,guint32);

void po_layer_develop(cairo_surface_t *,potash_layer,
							 gint64 x_orig,gint64 y_orig,
							 guint32 x_surface,guint32 y_surface,
							 guint32 x_size,guint32 y_size);

void pod_layer_debug(potash_layer);

#endif
