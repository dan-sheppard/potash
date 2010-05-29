#ifndef POTASH_TILES_MAKERS_H
#define POTASH_TILES_MAKERS_H

#include <glib.h>
#include <cairo.h>

#include "tiles.h"

/* tile constructors */
void po_tmaker_opaque_white(potash_tiles tiles,int x,int y,
								    cairo_surface_t *surface,unsigned char *surface_data,
						          gpointer data);
						          
void po_tmaker_transparent_white(potash_tiles tiles,int x,int y,
									      cairo_surface_t *surface,
									      unsigned char *surface_data,
									      gpointer payload);
									      
void po_tmaker_copy(potash_tiles tiles,int x,int y,
						  cairo_surface_t *surface,unsigned char *surface_data,
						  gpointer payload);									      
									      
									      
void pod_tmaker_debug(potash_tiles tiles,int x,int y,
							 cairo_surface_t *surface,unsigned char *surface_data,
						    gpointer data);
gpointer pod_tmaker_debug_data(int a,int r,int g,int b,gchar *text);
void pod_tmaker_debug_data_free(gpointer data);

#endif
