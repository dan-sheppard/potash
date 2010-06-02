#ifndef POTASH_TILES_H
#define POTASH_TILES_H

#include <glib.h>
#include <gtk/gtk.h>
#include <cairo.h>
#include "../confdir/confdir.h"

#define PO_TILE_TYPE_MASK     0x00000003 /* Cache status */
#define PO_TILE_FLAGS         0x0000001C /* Invariable tile type info */
#define PO_TILE_MODES         0x00000020 /* Lock status */

#define PO_TILE_TYPE_UNLOADED 0x00000001
#define PO_TILE_TYPE_LOADED   0x00000002
#define PO_TILE_TYPE_LOCKED   0x00000003
#define PO_TILE_TYPE(t) ((t)->type&PO_TILE_TYPE_MASK)
#define PO_TILE_TMP           0x00000004
#define PO_TILE_SYNTHETIC     0x00000008
#define PO_TILE_VARIABLE      0x00000010 /* XXX Unused now? */
#define PO_TILE_RDWR          0x00000020

#define PO_TILE_RDWR          0x00000020
#define PO_TILE_RDONLY        0x00000000

struct _potash_tile;
struct _potash_layer; /* From layer.h */

typedef struct _potash_tiles {
   /* Metadata */
   guint32 next_id;
  
   /* Configuration */
   guint32 size,cache_size;
   potash_confdir cd;	

	/* Cache */
	int population;
	struct _potash_tile *first_cache,*last_cache;
  
} * potash_tiles;

typedef void (*potash_tile_maker)(potash_tiles tiles,int x,int y,
								          cairo_surface_t *surface,
								          unsigned char *surface_data,
								          gpointer payload);

typedef struct _potash_tile {
   /* Metadata */
   guint32 type;
   guint32 id;
   potash_tiles tiles;
   int x,y;

	/* Cache */
	int age;
	struct _potash_tile *prev_cache,*next_cache;
	
	/* Construction */
	potash_tile_maker constructor;
	gpointer payload;

   /* Surface */
   cairo_surface_t *surface;
   unsigned char *surface_data;   
} * potash_tile;

potash_tiles po_tiles_create(potash_confdir cd,guint32 size,guint32 cache_size);
void po_tiles_destroy(potash_tiles ts);

potash_tile po_tile_create(potash_tiles ts,int x,int y,int flags,
						 			potash_tile_maker t,gpointer payload);
void po_tile_destroy(potash_tile tile);
void po_tile_ref(potash_tile t,int mode);
void po_tile_unref(potash_tile t);
cairo_surface_t * po_tile_surface(potash_tile t);
void po_tile_flush(potash_tile t);
gboolean po_tile_ref_is_rdwr(potash_tile t);

void pod_tiles_debug(potash_tiles ts);
void pod_tile_debug(potash_tile t,gchar *base);
void pod_tile_scribble(potash_tile t,int colour);

#endif
