#include <cairo.h>
#include "stack_composers.h"

void po_stack_compose_normal(cairo_surface_t *dst,cairo_surface_t *src,
									  int x,int y,
									  int xsrc,int ysrc,int xdst,int ydst,
									  int xs,int ys) {
	cairo_t *cr;
	
	cr=cairo_create(dst);
	cairo_set_source_surface(cr,src,xsrc,ysrc);
	cairo_rectangle(cr,xdst,ydst,xs,ys);
	cairo_fill(cr);
	cairo_destroy(cr);
}
