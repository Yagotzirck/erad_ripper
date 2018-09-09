#include <stdio.h>
#include <stdlib.h>

#include "utils.h"
#include "pic_utils.h"
#include "tga_utils.h"

#define PAL_NUM_DIRS    4
#define PAL_NUM_SUBDIR_ENTRIES 5


/* global data */
FILE *in_fp;

char path[FILENAME_MAX];
char *baseDirPtr;
char *currDirPtr;


/* functions definitions */
void getPalette(void){
    erad_palette_t eradPal[256];

    ridHeader_t ridHdr, subPalDirRidHdr;
    ridEntry_t *ridEntries;
    ridEntry_t subPalDirRidEntries[PAL_NUM_SUBDIR_ENTRIES];


    long hdrPos, palHdrPos;
    unsigned int i, palChoice = 20;


    hdrPos = ftell(in_fp);

    fread(&ridHdr, sizeof(ridHdr), 1, in_fp);

    fseek(in_fp, ridHdr.entriesOff + hdrPos, SEEK_SET);

    /* allocate space for the rid entries array */
    if((ridEntries = malloc(ridHdr.numEntries * sizeof(ridEntry_t))) == NULL){
        fputs("Couldn't allocate memory for the entries array\n", stderr);
        exit(EXIT_FAILURE);
    }

    fread(ridEntries, sizeof(ridEntry_t), ridHdr.numEntries, in_fp);

        do{
            puts("Which palette do you want to use?");

            for(i = 0; i < ridHdr.numEntries; ++i)
                printf("\t%u: %s\n", i, ridEntries[i].name);

        scanf(" %u", &palChoice);
        while(getchar() != '\n')    /* clear stdin buffer */
            ;
        if(palChoice > ridHdr.numEntries - 1)
            puts("Invalid choice");
    } while(palChoice > ridHdr.numEntries - 1);

    /* get the offsets of the palettes in subdirectories */

    /* the chosen palette is inside a subdirectory */
    if(ridEntries[palChoice].size != 768){
        fseek(in_fp, ridEntries[palChoice].offset + hdrPos, SEEK_SET);

        palHdrPos = ftell(in_fp);

        fread(&subPalDirRidHdr, sizeof(subPalDirRidHdr), 1, in_fp);

        fseek(in_fp, subPalDirRidHdr.entriesOff + palHdrPos, SEEK_SET);

        fread(subPalDirRidEntries, sizeof(ridEntry_t), subPalDirRidHdr.numEntries, in_fp);

        /* the main PALETTE file descriptor is(should be) located in the first position of the array, so I ain't bothering to search for it */
        fseek(in_fp, subPalDirRidEntries[0].offset + palHdrPos, SEEK_SET);
    }

    /* the chosen palette is the only file contained in the PALETTE main directory(MENUPAL) */
    else
        fseek(in_fp, ridEntries[palChoice].offset + hdrPos, SEEK_SET);

    free(ridEntries);

    fread(&eradPal, sizeof(eradPal[0]), sizeof(eradPal) / sizeof(eradPal[0]), in_fp);
    eradToTgaPal(&eradPal);
}

size_t getBiggestSize(const ridEntry_t ridEntries[], DWORD numEntries){
    unsigned int i;
    size_t biggestSize = 0;

    for(i = 0; i < numEntries; ++i)
        if(ridEntries[i].size > biggestSize)
            biggestSize = ridEntries[i].size;

    return biggestSize;
}

void buffersAlloc(picAttr_p picAttr, size_t size){
    if((picAttr->rawData = malloc(size)) == NULL){
        fputs("Couldn't allocate memory for the entry buffer\n", stderr);
        exit(EXIT_FAILURE);
    }

    if((picAttr->flippedBuf = malloc(size)) == NULL){
        fputs("Couldn't allocate memory for the flipped image buffer\n", stderr);
        exit(EXIT_FAILURE);
    }

     /* in the worst case scenario RLE encoding will occupy twice the size of unencoded data */
    if((picAttr->shrunkBuf = malloc(size * 2)) == NULL){
        fputs("Couldn't allocate memory for the shrunk image buffer\n", stderr);
        exit(EXIT_FAILURE);
    }
}

void buffersFree(picAttr_t *picAttr){
    free(picAttr->rawData);
    free(picAttr->flippedBuf);
    free(picAttr->shrunkBuf);
}




