#ifndef UTILS_H
#define UTILS_H

typedef struct picAttr_s *picAttr_p;

typedef unsigned char   BYTE;
typedef unsigned short  WORD;
typedef unsigned int    DWORD;

typedef struct ridHeader_s{
    char magic[4];  // RIDB
    DWORD numEntries;
    DWORD entriesOff; // relative to the header, not absolute
}ridHeader_t;

typedef struct ridEntry_s{
    char name[12];
    DWORD offset;   // relative to the header, not absolute
    DWORD size;
}ridEntry_t;

/* functions declarations */
void getPalette(void);
size_t getBiggestSize(const ridEntry_t ridEntries[], DWORD numEntries);
void buffersAlloc(picAttr_p picAttr, size_t size);
void buffersFree(picAttr_p picAttr);

#endif // UTILS_H
