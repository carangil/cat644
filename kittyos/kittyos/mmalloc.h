#include <inttypes.h>





void mminit();
void mmdump();
void* mmalloc(u16 size);
void* mmalloc_handle(u16 size, u16 handle);
void mmfree(void* v);
void* mmaddref(void* v);

u16 mmrefcount(void* v);
u16 mmgethandle(void* v);

#define MM_REF_ONE	0x0200  /* 0000 0010 0000 0000 */

