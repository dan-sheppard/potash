#ifndef POTASH_INT4_H
#define POTASH_INT4_H

#include <glib.h>

/* INT4 is the underlying format used in potash data streams. Other data is first
 * transformed into an INT4 compatible format and it is then encoded as INT4.
 * INT4 hangles only 32-bit unsigned integers. It stores small numbers efficiently,
 * and larger ones less so. Other formats are first encoded into a format which
 * maximally exploits this property. As well as integers, a number of flags (end of
 * field/record) type flags can be stored, which are distinct from the integers.
 * 
 * The underlying format is byte-based. The flags can be recognised in the byte
 * stream, as they have values never taken by integers. This allows 
 * resynchronization at flags, and mid-stream interpretation (albeit less so than 
 * a format like UTF8).
 *
 * The format is inspired by UTF8, however it is more compact and includes a large
 * number of flag bytes. The cost is that resynchronization and bytewise
 * interpretation are more difficult. This is acceptable here because the higher
 * levels of interpretation make such low-level interpretation less valuable than
 * in the case of UTF8 character streams.
 *
 * 0x0xxxxxxx  --> 0x0000007F
 * 0x10xxxxxx 0x0xxxxxxx  --> 0x00001FFF
 * 0x1100xxxx 0x0xxxxxxx 0x0xxxxxxx --> 0x0003FFFF
 * 0x1101xxxx 0x0xxxxxxx 0x0xxxxxxx 0x0xxxxxxx -> 0x001FFFFF
 * 0x1110xxxx 0x0xxxxxxx 0x0xxxxxxx 0x0xxxxxxx 0x0xxxxxxx -> 0xFFFFFFFF
 * 0x11110xxx -- F1(0-7)
 * 0x11111xxx 0x0xxxxxxx -- F2(0-6,0-127)
 * 0x11111111 -- EOF
 */

/* Internal, don't use these */
#define PO_NUMBER      0x00000000FFFFFFFFULL
#define PO_TYPE        0xF000000000000000ULL
#define PO_TYPE_NUMBER 0x0000000000000000ULL
#define PO_TYPE_FLAG1  0x1000000000000000ULL
#define PO_TYPE_FLAG2  0x2000000000000000ULL
#define PO_TYPE_EOF    0x3000000000000000ULL
#define PO_FLAG_INDEX  0x000000000000000FULL
#define PO_FLAG2_VALUE 0x00000000000007F0ULL
#define PO_FLAG2_VALUE_SHIFT 4
#define PO_GET_TYPE(x) ((x)&PO_TYPE)

/* Use these instead */
typedef guint64 vint4;

#define PO_GET_NUMBER(x) ((guint32)((x)&PO_NUMBER))
#define PO_NUMBER_VAL(x) ((vint4)(PO_TYPE_NUMBER|((x)&PO_NUMBER)))

#define PO_IS_TYPE_NUMBER(x) (PO_GET_TYPE(x)==PO_TYPE_NUMBER)
#define PO_IS_TYPE_FLAG1(x) (PO_GET_TYPE(x)==PO_TYPE_FLAG1)
#define PO_IS_TYPE_FLAG2(x) (PO_GET_TYPE(x)==PO_TYPE_FLAG2)
#define PO_IS_TYPE_EOF(x) ((x)==PO_TYPE_EOF)
#define PO_GET_FLAG_INDEX(x) ((guint32)((x)&PO_FLAG_INDEX))
#define PO_GET_FLAG2_VALUE(x) ((guint32)(((x)&PO_FLAG2_VALUE)>>PO_FLAG2_VALUE_SHIFT))
#define PO_FLAG1_VAL(x) ((vint4)(PO_TYPE_FLAG1|((x)&PO_FLAG_INDEX)))
#define PO_FLAG2_VAL(x,y) ((vint4)(PO_TYPE_FLAG2|((y)<<PO_FLAG2_VALUE_SHIFT)|((x)&PO_FLAG_INDEX)))
#define PO_EOF_VAL PO_TYPE_EOF

/* Internal to ffwd/rev routines */
#define PO_FLAG1_BYTE(x) ((unsigned char)(0xF0|(x)))
#define PO_FLAG2_BYTE(x) ((unsigned char)(0xF8|(x)))

/* Must have at least five bytes left in buffer */
int po_int4_encode(guint8 *buffer,vint4 data);
/* Doesn't check for EOF at end: make sure you do! */
int po_int4_decode(guint8 *buffer,vint4 *number);

#endif
