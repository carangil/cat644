#include <inttypes.h>


typedef uint16_t u16;


void mminit();
void mmdump();
void* mmalloc(u16 size);
void mmfree(void* v);
void* mmaddref(void* v);

