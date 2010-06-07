#ifndef POTASH_PRECORD_H
#define POTASH_PRECORD_H

#include "pfile.h"
#include "int4.h"

/* FLAG1/0      -- Context record starts
 * FLAG1/1      -- Regular record starts
 * FLAG1/2      -- Record header separator
 * FLAG1/3      -- Major separator (free to use in record data)
 * FLAG1/4      -- Minor separator (free to use in record data)
 * FLAG1/5      -- Trivial separator (free to use in record data)
 * FLAG1/6      -- Type escape 1
 * FLAG1/7      -- Type escape 2
 * FLAG2/0/x    -- Type escape 3
 * FLAG2/1/x    -- Type escape 4
 * FLAG2/2/x    -- General record flag 1
 * FLAG2/3/x    -- General record flag 2
 * FLAG2/4/x    -- Misc Meta Record values / Reserved
 * FLAG2/5/x    -- Reserved
 * FLAG2/6/x    -- Reserved
 */

#define PRECORD_RECORD_CONTEXT PO_FLAG1_VAL(0)
#define PRECORD_RECORD_REGULAR PO_FLAG1_VAL(1)
#define PRECORD_HEADBODY       PO_FLAG1_VAL(2)
#define PRECORD_SEP_MAJOR      PO_FLAG1_VAL(3)
#define PRECORD_SEP_MINOR      PO_FLAG1_VAL(4)
#define PRECORD_SEP_TRIVIAL    PO_FLAG1_VAL(5)
#define PRECORD_TYPE_ESC1      PO_FLAG1_VAL(6)
#define PRECORD_TYPE_ESC2      PO_FLAG1_VAL(7)
#define PRECORD_TYPE_ESC3(x)   PO_FLAG2_VAL(0,x)
#define PRECORD_TYPE_ESC4(x)   PO_FLAG2_VAL(1,x)
#define PRECORD_GENERAL1(x)    PO_FLAG2_VAL(2,x)
#define PRECORD_GENERAL2(x)    PO_FLAG2_VAL(3,x)
#define PRECORD_PLANNED_EOF    PO_FLAG2_VAL(4,0)

typedef struct _potash_pstream {
	/* Metadata */
	potash_pfile pf;
} * potash_pstream;

typedef struct _potash_precord {
	/* Buffer */
	vint4 *buffer;
	int buffer_offset,buffer_len;
} * potash_precord;

potash_pstream po_pstream_create(potash_pfile pf);
void po_pstream_destroy(potash_pstream ps);
void po_pstream_add_record(potash_pstream,potash_precord);

potash_precord po_precord_create(int type);
void po_precord_destroy(potash_precord);

#endif
