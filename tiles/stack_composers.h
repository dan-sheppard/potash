#ifndef POTASH_STACK_COMPOSERS
#define POTASH_STACK_COMPOSERS

#include <cairo.h>

void po_stack_compose_normal(cairo_surface_t *dst,cairo_surface_t *src,
									  int x,int y,int xsrc,int ysrc,int xdst,int ydst,
									  int xs,int ys);

#endif
