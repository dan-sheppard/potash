#include <cairo.h>
#include "stack_composers.h"

void po_stack_compose_normal(cairo_surface_t *dst,cairo_surface_t *src,
									  int x,int y) {
	cairo_t *cr;
	
	cr=cairo_create(dst);
	cairo_set_source_surface(cr,src,0.0,0.0);
	cairo_rectangle(cr,0,0,cairo_image_surface_get_width(src),
						 cairo_image_surface_get_height(src));
	cairo_fill(cr);
	cairo_destroy(cr);
}
