#include <stdio.h>
#include <glib/gprintf.h>
#include <glib.h>
#include <cairo.h>
#include "tiles.h"
#include "tiles_makers.h"

potash_tiles po_tiles_create(potash_confdir cd,guint32 size,guint32 cache_size) {
	potash_tiles ts;
	
	ts=g_new(struct _potash_tiles,1);
	ts->next_id=1;
	ts->population=0;
	ts->size=size;
	ts->cache_size=cache_size;
	ts->cd=cd;
	ts->first_cache=ts->last_cache=0;
	return ts;
}

void po_tiles_destroy(potash_tiles ts) {
	g_free(ts);
}

static void cache_list_add(potash_tile t) {
	if(t->tiles->last_cache) {
		/* Not empty, add to end */
		t->prev_cache=t->tiles->last_cache;
		t->next_cache=0;
		t->prev_cache->next_cache=t;
		t->tiles->last_cache=t;
	} else {
		/* Empty */
		t->tiles->first_cache=t->tiles->last_cache=t;
		t->next_cache=t->prev_cache=0;
	}
	t->tiles->population++;
}

static void cache_list_remove(potash_tile t) {
	if(t->prev_cache)
		t->prev_cache->next_cache=t->next_cache;
	else
		t->tiles->first_cache=t->next_cache;
	if(t->next_cache)
		t->next_cache->prev_cache=t->prev_cache;
	else
		t->tiles->last_cache=t->prev_cache;
	t->tiles->population--;
	t->next_cache=t->prev_cache=0;
}

static void create_cairo_surface(potash_tile t) {
	int stride;
	potash_tiles ts;

	ts=t->tiles;
	stride=cairo_format_stride_for_width(CAIRO_FORMAT_ARGB32,ts->size);
	t->surface_data=g_new(unsigned char,stride*ts->size);
	t->surface=cairo_image_surface_create_for_data(t->surface_data,
																  CAIRO_FORMAT_ARGB32,
																  ts->size,ts->size,stride);
}

static potash_tile create_tile_internal(potash_tiles ts,int x,int y) {
	potash_tile t;
	unsigned char * data;
	int stride;
	
	t=g_new(struct _potash_tile,1);
	t->id=ts->next_id++;
	t->tiles=ts;
	t->type=PO_TILE_TYPE_LOCKED;
	t->age=0;
	t->x=x;
	t->y=y;
	create_cairo_surface(t);
	return t;
}

potash_tile po_tile_create(potash_tiles ts,int x,int y,int flags,
									potash_tile_maker c,gpointer payload) {
	potash_tile t;
	
	t=create_tile_internal(ts,x,y);
	t->constructor=c;
	t->payload=payload;
	cache_list_add(t);
	g_debug("Creating");
	(t->constructor)(t->tiles,x,y,t->surface,t->surface_data,t->payload);
	t->type|=(flags&PO_TILE_FLAGS);
	return t;
}

void pod_tile_debug(potash_tile t,gchar *base) {
	gchar * image_filename;

	image_filename="[unloaded]";
	if(PO_TILE_TYPE(t)!=PO_TILE_TYPE_UNLOADED) {
		image_filename=g_strdup_printf("%s-%8.8X.png",base,t->id);
		cairo_surface_write_to_png(t->surface,image_filename);
	}
	if(t->next_cache && t->next_cache->prev_cache!=t)
		g_critical("Bad tile map");
	if(t->prev_cache && t->prev_cache->next_cache!=t)
		g_critical("Bad tile map");			
	fprintf(stderr,"Tile type=%8.8X id=%8.8X filename=%s age=%d\n",
  		 	  t->type,t->id,image_filename,t->age);
	if(PO_TILE_TYPE(t)!=PO_TILE_TYPE_UNLOADED) {
		g_free(image_filename);
	}
}

void pod_tiles_debug(potash_tiles ts) {
	potash_tile t;

	fprintf(stderr,"Tiles next_id=%d size=%d cache_size=%d\n",
			  ts->next_id,ts->size,ts->cache_size);
	fprintf(stderr,"Fwd cache: ");
	for(t=ts->first_cache;t;t=t->next_cache) {
		fprintf(stderr,"%d ",t->id);
		if(t->next_cache && t->next_cache->prev_cache!=t)
			g_critical("Bad tile map id=%d",t->id);
		if(t->prev_cache && t->prev_cache->next_cache!=t)
			g_critical("Bad tile map id=%d",t->id);			
	}
	fprintf(stderr,"\nRev cache: ");
	for(t=ts->last_cache;t;t=t->prev_cache)
		fprintf(stderr,"%d ",t->id);
	fprintf(stderr,"\n");
}

void pod_tile_scribble(potash_tile t,int colour) {
	cairo_t *cr;

	if(PO_TILE_TYPE(t)!=PO_TILE_TYPE_LOCKED)
		g_critical("Can only draw on locked tiles (id=%d)",t->id);

	cr=cairo_create(t->surface);
	cairo_set_source_rgb (cr, (colour&4)?1.0:0.0,(colour&2)?1.0:0.0,(colour&1)?1.0:0.0);
	cairo_rectangle(cr,t->tiles->size/3,t->tiles->size/3,
						 t->tiles->size/3,t->tiles->size/3);
	cairo_fill(cr);
	cairo_destroy(cr);
}

void po_tile_unlock(potash_tile t) {
	if(PO_TILE_TYPE(t)!=PO_TILE_TYPE_LOCKED)
		g_critical("Can only unlock locked tiles (id=%d)",t->id);
	t->type&=~PO_TILE_TYPE_MASK;
	t->type|=PO_TILE_TYPE_LOADED;
}

static void po_tile_lock(potash_tile t) {
	if(PO_TILE_TYPE(t)!=PO_TILE_TYPE_LOADED)
		g_critical("Can only lock loaded tiles (id=%d)",t->id);
	t->type&=~PO_TILE_TYPE_MASK;
	t->type|=PO_TILE_TYPE_LOCKED;
	t->age=0;
}

static void po_tile_unload(potash_tile t) {
	gchar *filename;

	if(PO_TILE_TYPE(t)!=PO_TILE_TYPE_LOADED)
		g_critical("Can only unload loaded tiles (id=%d)",t->id);
	if(!(t->type&PO_TILE_TMP) && !(t->type&PO_TILE_SYNTHETIC)) {
		filename=po_confdir_tmp_file(t->tiles->cd,"tile",t->id,"png");
		g_debug("Saving tile to '%s'",filename);
		cairo_surface_write_to_png(t->surface,filename);
		g_free(filename);
	}
	cairo_surface_destroy(t->surface);
	g_free(t->surface_data);
	t->type&=~PO_TILE_TYPE_MASK;	
	t->type|=PO_TILE_TYPE_UNLOADED;
	t->surface=0;
	t->surface_data=0;
	cache_list_remove(t);
}

static void po_tile_load(potash_tile t) {
	gchar *filename;
	cairo_surface_t *png;
	cairo_t *ct;

	if(PO_TILE_TYPE(t)!=PO_TILE_TYPE_UNLOADED)
		g_critical("Can only load unloaded tiles (id=%d)",t->id);
	create_cairo_surface(t);
	if(t->type&PO_TILE_SYNTHETIC) {
		g_debug("Recreating synthetic tile");
		(t->constructor)(t->tiles,t->x,t->y,t->surface,t->surface_data,t->payload);
	} else {	
		filename=po_confdir_tmp_file(t->tiles->cd,"tile",t->id,"png");
		png=cairo_image_surface_create_from_png(filename);
		g_free(filename);
		ct=cairo_create(t->surface);
		cairo_set_source_surface(ct,png,0.0,0.0);
		cairo_set_operator(ct,CAIRO_OPERATOR_SOURCE);		
		cairo_paint(ct);
		cairo_destroy(ct);
		cairo_surface_destroy(png);
	}
	t->type&=~PO_TILE_TYPE_MASK;
	t->type|=PO_TILE_TYPE_LOADED;
	cache_list_add(t);
}

static void prune_cache(potash_tiles ts,int limit) {
	potash_tile t,victim;
	int maxage;

	if(ts->population<=limit) {
		g_debug("cache not overflowing %d/%d",ts->population,limit);
		/* Cache not full */
		return;
	}
	/* Bump the age of all */
	for(t=ts->first_cache;t;t=t->next_cache)
		t->age++;
	/* Find oldest */
	victim=0;
	maxage=-1;
	for(t=ts->first_cache;t;t=t->next_cache)
		if(PO_TILE_TYPE(t)==PO_TILE_TYPE_LOADED) {
			if(t->age>maxage) {
				maxage=t->age;
				victim=t;
			}
		}
	/* Prune oldest */
	if(victim) {
		g_debug("cache overflowing evicting %d",victim->id);
		po_tile_unload(victim);
	}
}

void po_tile_destroy(potash_tile t) {
	if(PO_TILE_TYPE(t)==PO_TILE_TYPE_LOCKED)
		po_tile_unlock(t);
	if(PO_TILE_TYPE(t)==PO_TILE_TYPE_LOADED)
		po_tile_unload(t);
	g_free(t);
}

void po_tile_ref(potash_tile t) {
	if(t->type&PO_TILE_TMP)
		g_error("Cannot ref tmp tile");
	if(PO_TILE_TYPE(t)==PO_TILE_TYPE_UNLOADED) {
		prune_cache(t->tiles,t->tiles->cache_size-1);
		po_tile_load(t);
	} else
		g_debug("Already loaded tile");
	if(PO_TILE_TYPE(t)!=PO_TILE_TYPE_LOCKED)
		po_tile_lock(t);
}

void po_tile_unref(potash_tile t) {
	po_tile_unlock(t);
	if(t->type&PO_TILE_TMP || t->type&PO_TILE_VARIABLE)
		po_tile_unload(t);	
	prune_cache(t->tiles,t->tiles->cache_size);
}

cairo_surface_t * po_tile_surface(potash_tile t) {
	return t->surface;
}
