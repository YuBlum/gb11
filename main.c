#include <stdio.h>

typedef char                b8;
typedef char                s8;
typedef short               s16;
typedef int                 s32;
typedef unsigned char       u8;
typedef unsigned short      u16;
typedef unsigned int        u32;

#define MEM_AMOUNT 1024*8

static u8 game_memory[MEM_AMOUNT]; /* basically all the memory we will ever need */

s32
main(void) {
  (void)game_memory;
  printf("game memory amount: %u\n", MEM_AMOUNT);
  return 0;
}
