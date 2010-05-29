#ifndef POTASH_LAYER_H
#define POTASH_LAYER_H

#include <glib.h>
#include "tiles.h"

struct potash_matrix {
	int x_offset,y_offset;
	int x_size,y_size;
	potash_tile * tiles;
};

typedef struct _potash_layer {
	/* Metadata */
	potash_tiles tiles;
	int tile_flags;
	potash_tile_maker maker;
	gpointer maker_data;
	GDestroyNotify maker_free;

	/* Store */
	struct potash_matrix matrix;
} * potash_layer;

potash_layer po_layer_create(potash_tiles ts,int tile_flags,
									  potash_tile_maker maker,gpointer maker_data,
									  GDestroyNotify maker_free);
void po_layer_destroy(potash_layer);
potash_tile po_layer_get_tile(potash_layer,int,int);
void po_layer_put_tile(potash_tile t);

#endif
