#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "rip_sprites.h"
#include "utils.h"
#include "tga_utils.h"

extern FILE *in_fp;
extern char path[];
extern char *currDirPtr;


typedef struct spriteLumpHeader_s{
    BYTE	numRotations;
	BYTE 	numAnims;
	WORD	unkn;	/* seems to be always zero, most likely just a filler/alignment field */
}spriteLumpHeader_t;

typedef struct spriteAttr_s{
	BYTE xOffset;
	BYTE yOffset;
	BYTE isMirrored;
	BYTE unkn;  /* seems to be always zero, most likely just a filler/alignment field */
	DWORD spriteDataOffset;
}spriteAttr_t;


typedef struct spriteHeader_s{
	DWORD height;
	DWORD width;
	DWORD unk; // always 1. Maybe it's 0 for textures and 1 for sprites?
}spriteHeader_t;

void rip_sprites(void){
    ridHeader_t ridHdr;
    ridEntry_t *ridEntries;

    spriteLumpHeader_t *spriteLmpHdr;
    spriteAttr_t *spriteAttr;
    DWORD numSubSprites;

    spriteHeader_t *spriteHdr;

    BYTE *spriteData, *rawDataTemp;
    size_t biggestSize;
    picAttr_t picAttr;

    long hdrPos;

    char *spriteNumPtr;

    unsigned int i, j;


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
    buffersAlloc(&picAttr, biggestSize);

    for(i = 0; i < ridHdr.numEntries; ++i){
        if(ridEntries[i].size == 0)
            continue;

        strcpy(currDirPtr, ridEntries[i].name);
        spriteNumPtr = currDirPtr + strlen(currDirPtr);

        fseek(in_fp, ridEntries[i].offset + hdrPos, SEEK_SET);
        fread(picAttr.rawData, 1, ridEntries[i].size, in_fp);

        spriteLmpHdr = (spriteLumpHeader_t *)picAttr.rawData;
        spriteAttr = (spriteAttr_t *)(picAttr.rawData + sizeof(spriteLumpHeader_t));

        numSubSprites = spriteLmpHdr->numAnims * spriteLmpHdr->numRotations;

        for(j = 0; j < numSubSprites; ++j){
            /* mirrored sprites point to the same data used by their not-mirrored counterpart, then the engine takes care of actually mirroring them; no need to save them twice */
            if(spriteAttr[j].isMirrored)
                continue;

            spriteHdr = (spriteHeader_t *)&picAttr.rawData[spriteAttr[j].spriteDataOffset];
            spriteData = &picAttr.rawData[spriteAttr[j].spriteDataOffset] + sizeof(spriteHeader_t);

            sprintf(spriteNumPtr, "_%.2u.tga", j + 1);

            rawDataTemp = picAttr.rawData;

            picAttr.rawData     = spriteData;
            picAttr.width       = spriteHdr->width;
            picAttr.height      = spriteHdr->height;
            picAttr.isFlipped   = 1;

            pic_handler(&picAttr);

            picAttr.rawData = rawDataTemp;
        }

    }

    free(ridEntries);
    buffersFree(&picAttr);
}
