#include <glib.h>
#include <cairo.h>

#include "cairo_util.h"

void po_cairo_util_fade_alpha(cairo_surface_t *src,guint32 alpha) {
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