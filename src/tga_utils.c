#include <stdio.h>

#include "tga_utils.h"
#include "utils.h"
#include "pic_utils.h"

typedef struct _TgaHeader
{
  BYTE IDLength;        /* 00h  Size of Image ID field */
  BYTE ColorMapType;    /* 01h  Color map type */
  BYTE ImageType;       /* 02h  Image type code */
  WORD CMapStart;       /* 03h  Color map origin */
  WORD CMapLength;      /* 05h  Color map length */
  BYTE CMapDepth;       /* 07h  Depth of color map entries */
  WORD XOffset;         /* 08h  X origin of image */
  WORD YOffset;         /* 0Ah  Y origin of image */
  WORD Width;           /* 0Ch  Width of image */
  WORD Height;          /* 0Eh  Height of image */
  BYTE PixelDepth;      /* 10h  Image pixel size */
  BYTE ImageDescriptor; /* 11h  Image descriptor byte */
} TGAHEAD;

struct tga_palette_s{
    BYTE blue, green, red, alpha;
};

/* global variables (file scope only) */
static TGAHEAD tga_header;
static struct tga_palette_s tga_palette[256], tga_shrunk_palette[256];


/* local functions declarations */
static WORD shrink_palette(BYTE imgBuf[], DWORD size, BYTE used_indexes[]);


/* functions definitions */
void eradToTgaPal(const erad_palette_t *erad_palette){
    unsigned int i;

    tga_palette[0].blue = 0;
    tga_palette[0].green = 0;
    tga_palette[0].red = 0;
    tga_palette[0].alpha = 0;

    for(i = 1; i < 256; ++i){
        tga_palette[i].blue = erad_palette[i].blue;
        tga_palette[i].green = erad_palette[i].green;
        tga_palette[i].red = erad_palette[i].red;
        tga_palette[i].alpha = 0xFF;
    }
}


void set_tga_hdr(enum tgaColorMap isCMapped, enum tgaImageType imgType, WORD CMapLen, BYTE CMapDepth, WORD width, WORD height, BYTE PixelDepth, enum tgaImageDescriptor ImageDesc){
    tga_header.IDLength =          0;           /* No image ID field used, size 0 */
    tga_header.ColorMapType =      isCMapped;
    tga_header.ImageType =         imgType;
    tga_header.CMapStart =         0;           /* Color map origin */
    tga_header.CMapLength =        CMapLen;     /* Number of palette entries */
    tga_header.CMapDepth =         CMapDepth;   /* Depth of color map entries */
    tga_header.XOffset =           0;           /* X origin of image */
    tga_header.YOffset =           0;           /* Y origin of image */
    tga_header.Width =             width;       /* Width of image */
    tga_header.Height =            height;      /* Height of image */
    tga_header.PixelDepth =        PixelDepth;  /* Image pixel size */
    tga_header.ImageDescriptor =   ImageDesc;
}

void write_tga_hdr(FILE *stream){
    fwrite(&tga_header.IDLength, sizeof(tga_header.IDLength), 1, stream);
    fwrite(&tga_header.ColorMapType, sizeof(tga_header.ColorMapType), 1, stream);
    fwrite(&tga_header.ImageType, sizeof(tga_header.ImageType), 1, stream);
    fwrite(&tga_header.CMapStart, sizeof(tga_header.CMapStart), 1, stream);
    fwrite(&tga_header.CMapLength, sizeof(tga_header.CMapLength), 1, stream);
    fwrite(&tga_header.CMapDepth, sizeof(tga_header.CMapDepth), 1, stream);
    fwrite(&tga_header.XOffset, sizeof(tga_header.XOffset), 1, stream);
    fwrite(&tga_header.YOffset, sizeof(tga_header.YOffset), 1, stream);
    fwrite(&tga_header.Width , sizeof(tga_header.Width), 1, stream);
    fwrite(&tga_header.Height, sizeof(tga_header.Height), 1, stream);
    fwrite(&tga_header.PixelDepth, sizeof(tga_header.PixelDepth), 1, stream);
    fwrite(&tga_header.ImageDescriptor, sizeof(tga_header.ImageDescriptor), 1, stream);
}

void write_tga_pal(FILE *stream){
    fwrite(tga_palette, sizeof(struct tga_palette_s), tga_header.CMapLength, stream);
}

void write_shrunk_tga_pal(FILE *stream){
    fwrite(tga_shrunk_palette, sizeof(struct tga_palette_s), tga_header.CMapLength, stream);
}


/* shrink_tga(): compress both palette (by deleting unused entries) and data(RLE encoding)*/
int shrink_tga(BYTE imgDest[], BYTE imgBuf[], DWORD size, WORD *CMapLen){
    BYTE used_indexes[256] = {0};
    unsigned int i = 0, j = 0;

    BYTE RLE_byte;

    *CMapLen = shrink_palette(imgBuf, size, used_indexes);

    do{
        RLE_byte = 0;
        while(i + 1 < size && imgBuf[i+1] == imgBuf[i]){
            ++RLE_byte;
            ++i;

            if(RLE_byte == 127)
                break;
        }

        imgDest[j++] =  RLE_byte | 0x80;
        imgDest[j++] =  used_indexes[imgBuf[i++]];
    }while(i < size);

    /* if the RLE compression resulted in increased size keep the data in its raw form
    ** (update the pixel indexes to the new shrunk palette first) */
    if(j >= size){
        for(i = 0; i < size; ++i)
            imgBuf[i] = used_indexes[imgBuf[i]];

        return -1;
    }

    return j;
}


/* local functions definitions */
static WORD shrink_palette(BYTE imgBuf[], DWORD size, BYTE used_indexes[]){
    unsigned int i, j;

    /* scan the whole image to track the palette colors actually used */
    for(i = 0; i < size; ++i)
        used_indexes[imgBuf[i]] = 1;

    /* remap the palette with the used palette colors placed sequentially */
    for(i = 0, j = 0; i < 256; ++i)
        if(used_indexes[i]){
            tga_shrunk_palette[j] = tga_palette[i];
            used_indexes[i] = j++;
        }

    return j;
}
