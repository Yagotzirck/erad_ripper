#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "rip_palettes.h"
#include "utils.h"
#include "makedir.h"

#define SIZE_64K        65536
#define PAL_NUM_DIRS    4
#define PAL_NUM_SUBDIR_ENTRIES 5


extern FILE *in_fp;
extern char path[];
extern char *currDirPtr;

/* local functions declarations */
static void ripPalSubDir(BYTE entry_buf[]);

void rip_palettes(void){
    FILE *pal_fp;

    ridHeader_t ridHdr;
    ridEntry_t *ridEntries;

    BYTE entry_buf[SIZE_64K];

    long hdrPos;


    unsigned int i;


    hdrPos = ftell(in_fp);

    fread(&ridHdr, sizeof(ridHdr), 1, in_fp);

    fseek(in_fp, ridHdr.entriesOff + hdrPos, SEEK_SET);

    /* allocate space for the rid entries array */
    if((ridEntries = malloc(ridHdr.numEntries * sizeof(ridEntry_t))) == NULL){
        fputs("Couldn't allocate memory for the entries array\n", stderr);
        exit(EXIT_FAILURE);
    }

    fread(ridEntries, sizeof(ridEntry_t), ridHdr.numEntries, in_fp);

    /* rip the subdirectories and the palette files contained in them */
    for(i = 0; i < PAL_NUM_DIRS; ++i){
        sprintf(currDirPtr, "%s/", ridEntries[i].name);
        makeDir(path);
        fseek(in_fp, ridEntries[i].offset + hdrPos, SEEK_SET);
        ripPalSubDir(entry_buf);
    }

    /* rip the only file contained in the PALETTE main directory(MENUPAL) */
    sprintf(currDirPtr, "%s.pal", ridEntries[i].name);
    fseek(in_fp, ridEntries[i].offset + hdrPos, SEEK_SET);
    fread(entry_buf, 1, ridEntries[i].size, in_fp);

    if((pal_fp = fopen(path, "wb")) == NULL){
            fprintf(stderr, "Couldn't create %s\n", path);
            exit(EXIT_FAILURE);
        }
    fwrite(entry_buf, 1, ridEntries[i].size, pal_fp);
    fclose(pal_fp);

    free(ridEntries);
}

/* local functions definitions */
static void ripPalSubDir(BYTE entry_buf[]){
    FILE *pal_fp;
    ridHeader_t ridHdr;
    ridEntry_t ridEntries[PAL_NUM_SUBDIR_ENTRIES];

    char *palSubDirPtr = currDirPtr + strlen(currDirPtr);

    long hdrPos;
    unsigned int i;

    hdrPos = ftell(in_fp);

    fread(&ridHdr, sizeof(ridHdr), 1, in_fp);

    fseek(in_fp, ridHdr.entriesOff + hdrPos, SEEK_SET);

    fread(ridEntries, sizeof(ridEntry_t), ridHdr.numEntries, in_fp);

    for(i = 0; i < ridHdr.numEntries; ++i){
        sprintf(palSubDirPtr, "%s.pal", ridEntries[i].name);
        fseek(in_fp, ridEntries[i].offset + hdrPos, SEEK_SET);
        fread(entry_buf, 1, ridEntries[i].size, in_fp);

        if((pal_fp = fopen(path, "wb")) == NULL){
            fprintf(stderr, "Couldn't create %s\n", path);
            exit(EXIT_FAILURE);
        }

        fwrite(entry_buf, 1, ridEntries[i].size, pal_fp);
        fclose(pal_fp);
    }
}
