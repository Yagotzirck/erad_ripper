#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "makedir.h"
#include "utils.h"

#include "rip_texturesSkiesPics.h"
#include "rip_sprites.h"
#include "rip_sounds.h"
#include "rip_palettes.h"

#define NUM_MAIN_DIRS 6
#define PALETTEDIR_IDX 5

extern FILE *in_fp;
extern char path[];
extern char *baseDirPtr;
extern char *currDirPtr;

static const char *fileArg[] =
{
    "MAIN.RID",
    "AMBIENT.RAW",
    "MUSIC.RAW"
};

enum fileArgID
    {
        ID_MAIN_RID,
        ID_AMBIENT_RAW,
        ID_MUSIC_RAW
    };


/* local functions declarations */
static enum fileArgID identify_entry(const char *argStr);
static void init_path(const char *filePath, const char *fileName);



int main(int argc, char **argv){
 void (*ripFunc[NUM_MAIN_DIRS])(void) = {
        rip_texturesSkiesPics,
        rip_texturesSkiesPics,
        rip_texturesSkiesPics,
        rip_sprites,
        rip_sounds,
        rip_palettes
    };

    ridHeader_t ridHdr;
    ridEntry_t *ridEntries;
    size_t biggestSize;

    BYTE *entry_buf;    /* used to rip the files in the root directory */
    FILE *out_fp;       /* used to rip the files in the root directory */

    long hdrPos;

    enum fileArgID fileArgID_val;

    unsigned int i;

    puts("\t\tEradicator resources ripper by Yagotzirck");

    if(argc != 2){
        fputs("Usage: erad_ripper.exe <file>,\n"
              "where <file> is one of the following entries:\n", stderr);

        for(i = 0; i < sizeof(fileArg) / sizeof(fileArg[0]); ++i)
            fprintf(stderr, "\t%s\n", fileArg[i]);

        return 1;
    }

    /* identify the file passed as an argument */
    fileArgID_val = identify_entry(argv[1]);

    /* create main directory */
    init_path(argv[1], fileArg[fileArgID_val]);



    if((in_fp = fopen(argv[1], "rb")) == NULL){
        fprintf(stderr, "Couldn't open %s\n", argv[1]);
        return 1;
    }

    // the file passed as argument is one of the two background music .RAW files
    if(fileArgID_val == ID_AMBIENT_RAW || fileArgID_val == ID_MUSIC_RAW)
        rip_bgMusic(fileArg[fileArgID_val]);

    // the file passed as argument is MAIN.RID
    else{

        /* explore the .RID file */
        hdrPos = ftell(in_fp);
        fread(&ridHdr, sizeof(ridHdr), 1, in_fp);

        fseek(in_fp, ridHdr.entriesOff + hdrPos, SEEK_SET);

        /* allocate space for the rid entries array */
        if((ridEntries = malloc(ridHdr.numEntries * sizeof(ridEntry_t))) == NULL){
            fputs("Couldn't allocate memory for the entries array\n", stderr);
            return 1;
        }

        fread(ridEntries, sizeof(ridEntry_t), ridHdr.numEntries, in_fp);

        /* acquire palette */
        fseek(in_fp, ridEntries[PALETTEDIR_IDX].offset + hdrPos, SEEK_SET);
        getPalette();


        for(i = 0; i < NUM_MAIN_DIRS; ++i){
            printf("Ripping %s\n", ridEntries[i].name);

            sprintf(baseDirPtr, "%s/", ridEntries[i].name);
            makeDir(path);
            currDirPtr = path + strlen(path);

            fseek(in_fp, ridEntries[i].offset + hdrPos, SEEK_SET);

            ripFunc[i]();
        }

        /* rip the files in the root directory */
        putchar('\n');

        /* get the biggest entry size so that we do a single malloc for the entry buffer */
        biggestSize = getBiggestSize(&ridEntries[i], ridHdr.numEntries - i);

        if((entry_buf = malloc(biggestSize)) == NULL){
            fprintf(stderr, "Couldn't allocate %u bytes for the entries in root directory buffer\n", biggestSize);
            return 1;
        }

        for(; i < ridHdr.numEntries; ++i){
            printf("Ripping %s\n", ridEntries[i].name);
            strcpy(baseDirPtr, ridEntries[i].name);

            fseek(in_fp, ridEntries[i].offset + hdrPos, SEEK_SET);
            fread(entry_buf, 1, ridEntries[i].size, in_fp);

            if((out_fp = fopen(path, "wb")) == NULL){
                fprintf(stderr, "Couldn't create %s\n", path);
                return 1;
            }

            fwrite(entry_buf, 1, ridEntries[i].size, out_fp);
            fclose(out_fp);
        }



        free(ridEntries);
        free(entry_buf);
    }

    /* the rip is now complete */
    *baseDirPtr = '\0';
    fclose(in_fp);

    printf("\nResources have been successfully ripped in the following folder:\n%s\n", path);

    return 0;
}


/* local functions definitions */

static enum fileArgID identify_entry(const char *argStr){


    unsigned i;
    const char *argFileName;

    // get the argument's filename without subdirectories(if any)
    argFileName = argStr + strlen(argStr);
    do --argFileName;
    while(argFileName != argStr && *argFileName != '/' && *argFileName != '\\');

    // if argFileName points to a directory separator, we must advance it by one character
    if(argFileName == '/' || *argFileName == '\\')
        ++argFileName;


    for(i = 0; i < sizeof(fileArg) / sizeof(fileArg[0]); ++i)
        if(strcmp(argFileName, fileArg[i]) == 0)
            return i;

    fprintf(stderr, "%s isn't a file supported by this extractor\n", argStr);
    exit(EXIT_FAILURE);
}


static void init_path(const char *filePath, const char *fileName){
    strcpy(path, filePath);

    /* Create a directory named "Eradicator rip" on same path as the input file */

    baseDirPtr = path + strlen(path) - strlen(fileName);

    strcpy(baseDirPtr, "Eradicator rip/");
    makeDir(path);

    /* make baseDirPtr point to the end of "Eradicator rip/" string */
    baseDirPtr += strlen(baseDirPtr);
}
