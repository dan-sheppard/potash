#include "int4.h"

int po_int4_encode(guint8 *buffer,vint4 x) {
	int offset=0;
	guint32 v;

	if(PO_IS_TYPE_NUMBER(x)) {
		v=PO_GET_NUMBER(x);
		if(v<0x00000080) {
			buffer[offset++]=v;
		} else if(v<0x00002000) {
			buffer[offset++]=0x80|(v>>7);
			buffer[offset++]=v&0x7F;
		} else if(v<0x00040000) {
			buffer[offset++]=0xC0|(v>>14);
			buffer[offset++]=(v>>7)&0x7F;
			buffer[offset++]=v&0x7F;
		} else if(v<0x00200000) {
			buffer[offset++]=0xD0|(v>>21);
			buffer[offset++]=(v>>14)&0x7F;
			buffer[offset++]=(v>>7)&0x7F;
			buffer[offset++]=v&0x7F;			
		} else {
			buffer[offset++]=0xE0|(v>>28);
			buffer[offset++]=(v>>21)&0x7F;
			buffer[offset++]=(v>>14)&0x7F;
			buffer[offset++]=(v>>7)&0x7F;
			buffer[offset++]=v&0x7F;
		}
	} else if(PO_IS_TYPE_FLAG1(x)) {
		buffer[offset++]=0xF0|PO_GET_FLAG_INDEX(x);
	} else if(PO_IS_TYPE_FLAG2(x)) {
		buffer[offset++]=0xF8|PO_GET_FLAG_INDEX(x);
		buffer[offset++]=PO_GET_FLAG2_VALUE(x);
	} else if(PO_IS_TYPE_EOF(x)) {
		buffer[offset++]=0xFF;
	} else {
		g_error("bad vint4");
	}
	return offset;
}

int po_int4_decode(guint8 *buffer,vint4 *number) {
	int offset=0;
	guint8 b[5],c;	
	
	b[0]=buffer[offset++];
	if(!(b[0]&0x80)) {
		*number=PO_NUMBER_VAL(b[0]);
	} else if((b[0]&0xC0)==0x80) {
		b[1]=buffer[offset++];
		*number=PO_NUMBER_VAL(b[1]|((b[0]&0x3F)<<7));
	} else if((b[0]&0xF0)==0xC0) {
		b[1]=buffer[offset++];
		b[2]=buffer[offset++];	
		*number=PO_NUMBER_VAL(b[2]|(b[1]<<7)|((b[0]&0x0F)<<14));
	} else if((b[0]&0xF0)==0xD0) {
		b[1]=buffer[offset++];
		b[2]=buffer[offset++];	
		b[3]=buffer[offset++];	
		*number=PO_NUMBER_VAL(b[3]|(b[2]<<7)|(b[1]<<14)|((b[0]&0x0F)<<21));
	} else if((b[0]&0xF0)==0xE0) {
		b[1]=buffer[offset++];
		b[2]=buffer[offset++];	
		b[3]=buffer[offset++];	
		b[4]=buffer[offset++];	
		*number=PO_NUMBER_VAL(b[4]|(b[3]<<7)|(b[2]<<14)|(b[1]<<21)|((b[0]&0x0F)<<28));
	} else if((b[0]&0xF8)==0xF0) {
		*number=PO_FLAG1_VAL(b[0]&0x07);
	} else if((b[0]&0xF8)==0xF8 && b[0]!=0xFF) {
		b[1]=buffer[offset++];
		*number=PO_FLAG2_VAL(b[0]&0x07,b[1]);
	} else {
		*number=PO_EOF_VAL;
	}
	return offset;
}
