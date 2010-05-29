#ifndef POTASH_FILE_STACK
#define POTASH_FILE_STACK

#include <glib.h>

#include "tiles.h"
#include "layer.h"

typedef void (*potash_stack_compose)(cairo_surface_t *dst,cairo_surface_t *src,
									   		 int x,int y);

struct potash_stack_el {
	potash_layer layer;
	potash_stack_compose compose;
	guint32 alpha;
};

typedef struct _potash_stack {
	/* Metadata */
	potash_tiles tiles;

	/* Layer stack */
	struct potash_stack_el *stack;
	int size,len;
	
	/* Main layer */
	potash_layer layer;
} *potash_stack;

potash_stack po_stack_create(potash_tiles);
void po_stack_destroy(potash_stack);
void po_stack_layer_add(potash_stack,potash_layer,potash_stack_compose,guint32);
potash_layer po_stack_layer(potash_stack);

#endif
