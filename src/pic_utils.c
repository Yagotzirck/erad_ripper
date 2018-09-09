#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "pic_utils.h"
#include "tga_utils.h"

extern char path[];

/* local functions declarations */
static void imgFlipMirror(BYTE dest[], const BYTE src[], WORD width, WORD height);


/* functions definitions */
void pic_handler(picAttr_t *picAttr){
    FILE *tga_fp;

    BYTE *bufToShrink, *bufToWrite;

    int shrunkSize;
    WORD CMapLen;

    enum tgaImageType imgType;

    DWORD rawBytesToWrite;


    if((tga_fp = fopen(path, "wb")) == NULL){
        fprintf(stderr, "Couldn't create %s\n", path);
        exit(EXIT_FAILURE);
    }

    if(picAttr->isFlipped){
        bufToShrink = picAttr->flippedBuf;
        imgFlipMirror(picAttr->flippedBuf, picAttr->rawData, picAttr->width, picAttr->height);
    }
    else
        bufToShrink = picAttr->rawData;


    shrunkSize = shrink_tga(picAttr->shrunkBuf, bufToShrink, picAttr->width * picAttr->height, &CMapLen);

    /* RLE compression resulted in increased data size */
    if(shrunkSize == -1){
        bufToWrite = bufToShrink;
        imgType = IMGTYPE_COLORMAPPED;
        rawBytesToWrite = picAttr->width * picAttr->height;
    }
    /* RLE compression worked fine; save the compressed data */
    else{
        bufToWrite = picAttr->shrunkBuf;
        imgType = IMGTYPE_COLORMAPPED_RLE;
        rawBytesToWrite = shrunkSize;
    }

    /* set and write the tga header */
    set_tga_hdr(PALETTED, imgType, CMapLen, 32, picAttr->height, picAttr->width, 8, ATTRIB_BITS | TOP_LEFT);
    write_tga_hdr(tga_fp);


    /* write the tga palette */
    write_shrunk_tga_pal(tga_fp);

    /* write the image data */
    fwrite(bufToWrite, 1, rawBytesToWrite, tga_fp);

    fclose(tga_fp);
}

static void imgFlipMirror(BYTE dest[], const BYTE src[], WORD width, WORD height){
    unsigned int x, y;

    for(x = 0; x < height; ++x)
        for(y = 0; y < width; ++y)
            dest[y*height + x] = src[x*width + y];
}
