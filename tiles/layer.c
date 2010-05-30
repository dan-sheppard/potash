#include <glib.h>

#include "layer.h"
#include "tiles_makers.h"

potash_layer po_layer_create(potash_tiles ts,int tile_flags,
									  potash_tile_maker maker,gpointer maker_data,
									  GDestroyNotify maker_free) {
	potash_layer ly;
	
	ly=g_new(struct _potash_layer,1);
	ly->tiles=ts;
	ly->matrix.x_offset=ly->matrix.y_offset=0;
	ly->matrix.x_size=ly->matrix.y_size=1;
	ly->matrix.tiles=g_new(potash_tile,1);
	ly->matrix.tiles[0]=0;
	ly->maker=maker;
	ly->maker_data=maker_data;
	ly->maker_free=maker_free;
	ly->tile_flags=tile_flags;
	return ly;
}

void po_layer_destroy(potash_layer ly) {
	int x;

	for(x=0;x<ly->matrix.x_size*ly->matrix.y_size;x++)
		if(ly->matrix.tiles[x])
			po_tile_destroy(ly->matrix.tiles[x]);
	g_free(ly->matrix.tiles);
	if(ly->maker_free)
		(*ly->maker_free)(ly->maker_data);
	g_free(ly);
}

static int matrix_get_offset(struct potash_matrix *matrix,int x,int y) {
	x-=matrix->x_offset;
	y-=matrix->y_offset;
	return x+y*(matrix->x_size);
}

static int matrix_ensure_bounds(struct potash_matrix *matrix,int x,int y) {
	struct potash_matrix new;
	gboolean redo=FALSE;
	int i,j,x_max,y_max;
	
	new=*matrix;
	x_max=new.x_size+new.x_offset-1;
	y_max=new.y_size+new.y_offset-1;
	if(x<new.x_offset) {
		g_debug("Extending layer left");
		new.x_offset=x;
		redo=TRUE;
	}
	if(y<new.y_offset) {
		g_debug("Extending layer up");
		new.y_offset=y;
		redo=TRUE;
	}
	if(x>x_max) {
		g_debug("Extending layer right");
		x_max=x;
		redo=TRUE;
	}
	if(y>y_max) {
		g_debug("Extending layer down");
		y_max=y;
		redo=TRUE;
	}
	new.x_size=x_max-new.x_offset+1;
	new.y_size=y_max-new.y_offset+1;
	if(redo) {
		g_debug("Shuffling matrix");
		new.tiles=g_new(potash_tile,new.x_size*new.y_size);
		for(j=0;j<new.x_size*new.y_size;j++)
			new.tiles[j]=0;
		for(j=0;j<matrix->y_size;j++)
			for(i=0;i<matrix->x_size;i++) {
				new.tiles[matrix_get_offset(&new,i+new.x_offset,j+new.y_offset)]=
					matrix->tiles[matrix_get_offset(matrix,i+matrix->x_offset,
															  j+matrix->y_offset)];
			}
		g_free(matrix->tiles);
		*matrix=new;
	}
}

potash_tile po_layer_get_tile(potash_layer ly,int x,int y) {
	int offset;

	matrix_ensure_bounds(&ly->matrix,x,y);
	offset=matrix_get_offset(&ly->matrix,x,y);
	if(!ly->matrix.tiles[offset]) {
		ly->matrix.tiles[offset]=po_tile_create(ly->tiles,x,y,ly->tile_flags,
															 ly->maker,ly->maker_data);
	} else {
		po_tile_ref(ly->matrix.tiles[offset]);
	}
	g_debug("Got tile (%d,%d)",x,y);
	return ly->matrix.tiles[offset];
}

void po_layer_put_tile(potash_tile t) {
	po_tile_unref(t);
}

static void print_tile(potash_tile tile,cairo_surface_t *src,
							  int x,int y,
							  guint32 src_origin_x,guint32 src_origin_y,
							  guint32 dst_origin_x,guint32 dst_origin_y,
							  guint32 size_x,guint32 size_y,
							  potash_stack_compose compose,guint32 alpha) {
	potash_tile tmp;							  
							  
	g_debug("  writing to tile src=[(%d,%d)x(%d,%d)] dst=[(%d,%d)x(%d,%d)]",
			 src_origin_x,src_origin_y,size_x,size_y,
			 dst_origin_x,dst_origin_y,size_x,size_y);	
	tmp=po_tile_create(tile->tiles,0,0,PO_TILE_TMP,po_tmaker_copy,src);
	po_cairo_util_fade_alpha(po_tile_surface(tmp),alpha);
	(*compose)(po_tile_surface(tile),po_tile_surface(tmp),x,y,
				  dst_origin_x-src_origin_x,dst_origin_y-src_origin_y,
				  dst_origin_x,dst_origin_y,size_x,size_y);
	po_tile_unref(tmp);
	po_tile_destroy(tmp);
}  

void po_layer_print(potash_layer dst,cairo_surface_t *src,
						  gint64 xo,gint64 yo,
						  potash_stack_compose compose,guint32 alpha) {
	int i,j;
	gint64 dst_base_x,dst_base_y;
	guint32 xs,ys,dst_origin_x,dst_origin_y,src_origin_x,src_origin_y;
	guint32 size_x,size_y;
	potash_tile tile;
	
	g_debug("Writing tiles");
	xs=cairo_image_surface_get_width(src);
	ys=cairo_image_surface_get_height(src);
	for(j=yo/dst->tiles->size;j<=(yo+ys-1)/dst->tiles->size;j++)
		for(i=xo/dst->tiles->size;i<=(xo+xs-1)/dst->tiles->size;i++) {
			g_debug("Writing to tile (%d,%d)",i,j);
			dst_base_x=i*dst->tiles->size;
			dst_base_y=j*dst->tiles->size;
			dst_origin_x=(guint32)MAX(0,xo-dst_base_x);
			dst_origin_y=(guint32)MAX(0,yo-dst_base_y);
			src_origin_x=(guint32)MAX(0,dst_base_x-xo);
			src_origin_y=(guint32)MAX(0,dst_base_y-yo);
			size_x=dst->tiles->size-dst_origin_x;
			if(dst_base_x+size_x>xo+xs)
				size_x=(guint32)(xo+xs-dst_base_x);
			size_y=dst->tiles->size-dst_origin_y;			
			if(dst_base_y+size_y>yo+ys)
				size_y=(guint32)(yo+ys-dst_base_y);
			tile=po_layer_get_tile(dst,i,j);
			print_tile(tile,src,src_origin_x,i,j,
						  src_origin_y,dst_origin_x,dst_origin_y,size_x,size_y,
						  compose,alpha);
			po_layer_put_tile(tile);
		}
}
