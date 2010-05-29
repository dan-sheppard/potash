#include <glib.h>
#include <cairo.h>
#include "tiles.h"
#include "tiles_makers.h"

void po_tmaker_opaque_white(potash_tiles tiles,int x,int y,
									 cairo_surface_t *surface,unsigned char *surface_data,
									 gpointer payload) {
	cairo_t *cr;
	
	cr=cairo_create(surface);
	cairo_set_source_rgb(cr,1.0,1.0,1.0);
	cairo_rectangle(cr,0.0,0.0,tiles->size,tiles->size);
	cairo_fill(cr);
	cairo_destroy(cr);
}

void po_tmaker_transparent_white(potash_tiles tiles,int x,int y,
									      cairo_surface_t *surface,
									      unsigned char *surface_data,
									      gpointer payload) {
	cairo_t *cr;
	
	cr=cairo_create(surface);
	cairo_set_source_rgb(cr,1.0,1.0,1.0);
	cairo_set_operator(cr,CAIRO_OPERATOR_CLEAR);
	cairo_rectangle(cr,0.0,0.0,tiles->size,tiles->size);
	cairo_fill(cr);
	cairo_destroy(cr);
}

void po_tmaker_copy(potash_tiles tiles,int x,int y,
						  cairo_surface_t *surface,unsigned char *surface_data,
						  gpointer payload) {
	cairo_t *cr;
	
	cr=cairo_create(surface);
	cairo_set_source_surface(cr,(cairo_surface_t *)payload,0.0,0.0);
	cairo_set_operator(cr,CAIRO_OPERATOR_SOURCE);
	cairo_rectangle(cr,0.0,0.0,tiles->size,tiles->size);
	cairo_fill(cr);
	cairo_destroy(cr);
}

struct pod_tmaker_data {
	int a,r,g,b;
	gchar *text;
};

static double b2f(int in) {
	return ((double)in)/255;
}

void pod_tmaker_debug(potash_tiles tiles,int x,int y,
							 cairo_surface_t *surface,unsigned char *surface_data,
							 gpointer payload) {
	cairo_t *cr;
	struct pod_tmaker_data *data;
	int tx,ty;
	
	data=(struct pod_tmaker_data *)payload;
	cr=cairo_create(surface);
	po_tmaker_opaque_white(tiles,x,y,surface,surface_data,payload);
	/* Layer on colours */
	cairo_set_operator(cr,CAIRO_OPERATOR_SOURCE);
	cairo_set_source_rgba(cr,b2f(data->r),b2f(data->g),b2f(data->b),b2f(data->a));
	cairo_rectangle(cr,0.0,0.0,tiles->size,tiles->size);
	cairo_fill(cr);
	/* Add text */
	cairo_set_source_rgba(cr,1.0,1.0,1.0,b2f(data->a));
	cairo_select_font_face(cr,"Purisa",
								  CAIRO_FONT_SLANT_NORMAL,CAIRO_FONT_WEIGHT_BOLD);
	cairo_set_font_size(cr,50);
	tx=rand()%(tiles->size/3)+tiles->size/3;
	ty=rand()%(tiles->size/3)+tiles->size/3;
	cairo_move_to(cr,tx,ty);
	cairo_show_text(cr,data->text);
	cairo_destroy(cr);
}

gpointer pod_tmaker_debug_data(int a,int r,int g,int b,gchar *text) {
	struct pod_tmaker_data *data;
	
	data=g_new(struct pod_tmaker_data,1);
	data->a=a;
	data->r=r;
	data->g=g;
	data->b=b;
	data->text=g_strdup(text);
	return data;
}

void pod_tmaker_debug_data_free(gpointer data) {
	g_free(((struct pod_tmaker_data *)data)->text);
	g_free(data);
}
