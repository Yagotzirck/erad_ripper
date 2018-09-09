#ifndef PIC_UTILS_H
#define PIC_UTILS_H

#include "utils.h"

typedef struct erad_palette_s{
    BYTE red, green, blue;
}erad_palette_t;


typedef struct picAttr_s{
    BYTE *rawData;
    BYTE *flippedBuf;
    BYTE *shrunkBuf;
    DWORD width;
    DWORD height;
    DWORD isFlipped;
}picAttr_t;

/* functions declarations */
void pic_handler(picAttr_t *picAttr);


#endif /* PIC_UTILS_H */
