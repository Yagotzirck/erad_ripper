#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "rip_sounds.h"
#include "utils.h"

extern FILE *in_fp;
extern char path[];
extern char *currDirPtr;
extern char *baseDirPtr;


typedef struct wavHeader_s{
        char chunkID[4];
        DWORD chunkSize;
        char format[4];

        char subChunk1ID[4];
        DWORD subChunk1Size;
        WORD audioFormat;
        WORD numChannels;
        DWORD sampleRate;
        DWORD byteRate;
        WORD blockAlign;
        WORD bitsPerSample;

        char subChunk2ID[4];
        DWORD subChunk2Size;
    }wavHeader_t;

static wavHeader_t wavHdr =
{
    {'R', 'I', 'F', 'F'},
    0, /* to be defined later */
    {'W', 'A', 'V', 'E'},

    {'f', 'm', 't', ' '},
    16,
    1, /* pcm */
    1, /* mono */
    11025,
    (11025 * 1 * 8) / 8, /* (SampleRate * NumChannels * BitsPerSample) / 8 */
    1 * 8 / 8, /* NumChannels * BitsPerSample / 8 */
    8,

    {'d', 'a', 't', 'a'},
    0 /* to be defined later */
};

void rip_sounds(void){
    FILE *wav_fp;

    ridHeader_t ridHdr;
    ridEntry_t *ridEntries;

    BYTE *entry_buf;
    size_t biggestSize;

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

    /* get the biggest entry size so that we do a single malloc for the entry buffer */
    biggestSize = getBiggestSize(ridEntries, ridHdr.numEntries);

    /* allocate the entry buffer */
    if((entry_buf = malloc(biggestSize)) == NULL){
        fputs("Couldn't allocate memory for the entry buffer\n", stderr);
        exit(EXIT_FAILURE);
    }

    for(i = 0; i < ridHdr.numEntries; ++i){
        if(ridEntries[i].size == 0)
            continue;

        sprintf(currDirPtr, "%s.wav", ridEntries[i].name);

        fseek(in_fp, ridEntries[i].offset + hdrPos, SEEK_SET);

        fread(entry_buf, 1, ridEntries[i].size, in_fp);

        if((wav_fp = fopen(path, "wb")) == NULL){
            fprintf(stderr, "Couldn't create %s\n", path);
            exit(EXIT_FAILURE);
        }

        /* set chunkSize and subChunk2Size header fields */
        wavHdr.chunkSize = 36 + ridEntries[i].size;
        wavHdr.subChunk2Size = ridEntries[i].size;

        /* write the wav header */
        fwrite(&wavHdr, sizeof(wavHdr), 1, wav_fp);

        /* write the pcm data */
        fwrite(entry_buf, 1, ridEntries[i].size, wav_fp);

        fclose(wav_fp);

    }

    free(ridEntries);
    free(entry_buf);
}


void rip_bgMusic(const char *fileNameArg){
    char fileName[FILENAME_MAX];
    FILE *wav_fp;
    size_t pcmDataSize;
    BYTE *pcmData;



    // change file extension from .RAW to .wav
    strcpy(fileName, fileNameArg);
    strcpy(fileName + strlen(fileName) - 3, "wav");

    // open the output file
    strcpy(baseDirPtr, fileName);
    if((wav_fp = fopen(path, "wb")) == NULL){
        fprintf(stderr, "Couldn't create file %s\n", path);
        exit(EXIT_FAILURE);
    }

    // get the file(pcm data) size
    fseek(in_fp, 0L, SEEK_END);
    pcmDataSize = ftell(in_fp);
    rewind(in_fp);

    /* set the wav header fields according to background music's attributes; since the extractor is supposed to enter this function once then terminate
    ** without extracting other files such as MAIN.RID, there's no need to reset the fields to their default initialized values once it finishes
    */
    wavHdr.sampleRate = 44100;
    wavHdr.byteRate = (44100 * 1 * 16) / 8, /* (SampleRate * NumChannels * BitsPerSample) / 8 */
    wavHdr.blockAlign = 1 * 16 / 8, /* NumChannels * BitsPerSample / 8 */
    wavHdr.bitsPerSample = 16;
    wavHdr.chunkSize = 36 + pcmDataSize;
    wavHdr.subChunk2Size = pcmDataSize;


    // read the pcm data
    if((pcmData = malloc(pcmDataSize)) == NULL){
        fprintf(stderr, "Couldn't allocate %u bytes for %s's PCM data\n", pcmDataSize, fileNameArg);
        exit(EXIT_FAILURE);
    }

    fread(pcmData, 1, pcmDataSize, in_fp);

    // save the wav header and the pcm data in the output file
    fwrite(&wavHdr, sizeof(wavHdr), 1, wav_fp);
    fwrite(pcmData, 1, pcmDataSize, wav_fp);

    fclose(wav_fp);
    free(pcmData);
}
