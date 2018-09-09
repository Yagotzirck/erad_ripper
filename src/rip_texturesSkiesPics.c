#include <stdio.h>
#include <stdlib.h>

#include "rip_texturesSkiesPics.h"
#include "utils.h"
#include "tga_utils.h"

extern FILE *in_fp;
extern char path[];
extern char *currDirPtr;


void rip_texturesSkiesPics(void){
    typedef struct textureLumpHeader_s{
        DWORD height;
        DWORD width;
        DWORD unk;  // always zero. maybe it's 0 for textures and 1 for sprites?
    }textureLumpHeader_t;

    enum lumpHeaderType_e{
        TEXTURES_SKIES, // 12 bytes
        PICTURES        // 8 bytes
    }lumpHdrType;


    ridHeader_t ridHdr;
    ridEntry_t *ridEntries;
    size_t lumpHdrSize[2] = {12, 8};

    size_t biggestSize;
    textureLumpHeader_t textLumpHdr = {0};
    picAttr_t picAttr;

    long hdrPos;


    unsigned int i;

    /* a rather clunky way to know whether we're ripping PICTURES or TEXTURES/SKIES lumps,
    ** by checking if the 6th character(7th, if we count the \0 terminating character) from the end
    ** of the directory name is 'C' or not
    */
    if(*(currDirPtr - 7) == 'C')
        lumpHdrType = PICTURES;
    else
        lumpHdrType = TEXTURES_SKIES;



    hdrPos = ftell(in_fp);

    fread(&ridHdr, sizeof(ridHdr), 1, in_fp);

    fseek(in_fp, ridHdr.entriesOff + hdrPos, SEEK_SET);

    /* allocate space for the rid entries array */
    if((ridEntries = malloc(ridHdr.numEntries * sizeof(ridEntry_t))) == NULL){
        fputs("Couldn't allocate memory for the entries array\n", stderr);
        exit(EXIT_FAILURE);
    }

    fread(ridEntries, sizeof(ridEntry_t), ridHdr.numEntries, in_fp);

    /* get the biggest entry size so that we do a single malloc for the entry buffer */
    biggestSize = getBiggestSize(ridEntries, ridHdr.numEntries);

    /* allocate all the image-related buffers */
    buffersAlloc(&picAttr, biggestSize - lumpHdrSize[lumpHdrType]);

    for(i = 0; i < ridHdr.numEntries; ++i){
        if(ridEntries[i].size == 0)
            continue;

        sprintf(currDirPtr, "%s.tga", ridEntries[i].name);

        fseek(in_fp, ridEntries[i].offset + hdrPos, SEEK_SET);
        fread(&textLumpHdr, lumpHdrSize[lumpHdrType], 1, in_fp);

        fread(picAttr.rawData, 1, ridEntries[i].size - lumpHdrSize[lumpHdrType], in_fp);

        picAttr.width   =  textLumpHdr.width;
        picAttr.height  =  textLumpHdr.height;

        if(lumpHdrType == PICTURES)
            picAttr.isFlipped =  0;
        else
            picAttr.isFlipped =  1;

        pic_handler(&picAttr);

    }

    free(ridEntries);
    buffersFree(&picAttr);
}





