#include <stdio.h>
#include <stdlib.h>

typedef char           s8;
typedef short          s16;
typedef int            s32;
typedef unsigned char  u8;
typedef unsigned short u16;
typedef unsigned int   u32;

s32
main(void) {
  u16 w, h;
  u32 *pixels;
  u16 i;
  FILE *out = fopen("atlas.h", "w");
  FILE *in  = fopen("./res/imgs/atlas.tga", "rb");
  if (!out) {
    printf("out\n");
    return 1;
  }
  if (!in) {
    printf("in\n");
    return 1;
  }
  fseek(in, 12, SEEK_CUR);
  fread(&w, sizeof (u16), 1, in);
  fread(&h, sizeof (u16), 1, in);
  fseek(in, 2, SEEK_CUR);
  printf("w: %u, h: %u\n", w, h);
  pixels = malloc(sizeof (u32) * w * h);
  fread(pixels, sizeof (u32), w * h, in);
  fprintf(out, "#ifndef __ATLAS_H__\n");
  fprintf(out, "#define __ATLAS_H__\n");
  fprintf(out, "#define ATLAS_W %u\n", w);
  fprintf(out, "#define ATLAS_H %u\n", h);
  fprintf(out, "static u8 atlas[ATLAS_W * ATLAS_H] = {");
  for (i = 0; i < w * h; i++) {
    switch (pixels[i]) {
      case 0xffffffff:
        fprintf(out, "0");
        break;
      case 0xffcccccc:
        fprintf(out, "1");
        break;
      case 0xff999999:
        fprintf(out, "2");
        break;
      case 0xff666666:
        fprintf(out, "3");
        break;
      default:
        fprintf(out, "4");
        break;
    }
    if (i < w * h - 1) fprintf(out, ",");
  }
  fprintf(out, "};\n");
  fprintf(out, "#endif/*__ATLAS_H__*/\n");
  free(pixels);
  fclose(out);
  fclose(in);
  return 0;
}
