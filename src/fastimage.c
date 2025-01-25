/*
Zero-Clause BSD

Copyright (c) 2022, Mikhail Morozov
All rights reserved.

Permission to use, copy, modify, and/or distribute this software for
any purpose with or without fee is hereby granted.

THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL
WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES
OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE
FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY
DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN
AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT
OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
*/
// please make all code changes to this file also in (this)(https://github.com/plzombie/fastimage.c) repository

#if defined(_WIN32)
#include <Windows.h>
#else
#define _LARGEFILE_SOURCE
#define _LARGEFILE64_SOURCE
#endif

#include "fastimage.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

typedef struct
{
    short Xmin;
    short Ymin;
    short Xmax;
    short Ymax;
    short HRes;
    short VRes;
    unsigned char Colormap[48];
    unsigned char Reserved;
    unsigned char NPlanes;
} pcx_header_min_t;

#define BMP_FILEHEADER_SIZE 14

typedef struct
{
    int biSize;
    int biWidth;
    int biHeight;
    short biPlanes;
    short biBitCount;
} bmp_infoheader_min_t;

#pragma pack(push, 1)
typedef struct
{
    //char sign[4]; // Already read it
    unsigned int width;
    unsigned int height;
    unsigned char channels;
    unsigned char colorspace;
} qoi_header_t;
#pragma pack(pop)

static void fastimageReadBmp(const fastimage_reader_t *reader, unsigned char *sign, fastimage_image_t *image)
{
    bmp_infoheader_min_t bmp_infoheader;

    (void)sign; // Unused

    if(!reader->seek(reader->context, BMP_FILEHEADER_SIZE, false))
    {
        image->format = fastimage_error;

        return;
    }

    if(reader->read(reader->context, sizeof(bmp_infoheader_min_t), &bmp_infoheader) != sizeof(bmp_infoheader_min_t))
    {
        image->format = fastimage_error;

        return;
    }

    image->width = bmp_infoheader.biWidth;
    image->height = bmp_infoheader.biHeight;
    switch(bmp_infoheader.biBitCount)
    {
    case 16:
    case 24:
    case 32:
        image->bitsperpixel = bmp_infoheader.biBitCount;
        break;
    case 1:
    case 4:
    case 8:
        image->bitsperpixel = 32;
        image->palette = bmp_infoheader.biBitCount;
        break;
    default:
        image->format = fastimage_error;
    }

    if(bmp_infoheader.biBitCount == 16)
        image->channels = 3;
    else
        image->channels = image->bitsperpixel / 8;
}

static void fastimageReadTga(const fastimage_reader_t *reader, unsigned char *sign, fastimage_image_t *image)
{
    unsigned char tga_header[18];

    memset(tga_header, 0, 18);
    memcpy(tga_header, sign, 4);

    if(reader->read(reader->context, 14, tga_header+4) != 14) goto TGA_ERROR;

    image->width = tga_header[12]+256*tga_header[13];
    image->height = tga_header[14]+256*tga_header[15];

    if(tga_header[1] == 1 || tga_header[1] == 9)
    {
        if(tga_header[16] != 8)
            goto TGA_ERROR;

        image->palette = tga_header[16];
        image->bitsperpixel = tga_header[7];
    }
    else
    {
        image->bitsperpixel = tga_header[16];
    }

    if(image->bitsperpixel % 8 == 0 && image->bitsperpixel > 16)
        image->channels = image->bitsperpixel / 8;
    else if(image->bitsperpixel == 8)
        image->channels = 1;
    else
        image->channels = 3;

    return;

TGA_ERROR:
    memset(image, 0, sizeof(fastimage_image_t));
    image->format = fastimage_error;
}

static void fastimageReadPcx(const fastimage_reader_t *reader, unsigned char *sign, fastimage_image_t *image)
{
    pcx_header_min_t pcx_header_min;

    (void)sign; // Unused

    if(reader->read(reader->context, sizeof(pcx_header_min_t), &pcx_header_min) != sizeof(pcx_header_min_t))
    {
        image->format = fastimage_error;

        return;
    }

    image->width = (size_t)(pcx_header_min.Xmax - pcx_header_min.Xmin) + 1;
    image->height = (size_t)(pcx_header_min.Ymax - pcx_header_min.Ymin) + 1;
    image->channels = 3;
    image->bitsperpixel = 24;

    if(pcx_header_min.NPlanes == 1)
        image->palette = 8;
    else if(pcx_header_min.NPlanes != 3)
        image->format = fastimage_error;
}

static void fastimageReadPng(const fastimage_reader_t *reader, unsigned char *sign, fastimage_image_t *image)
{
    unsigned char png_bytes[10];
    int64_t png_curr_offt = 4;
    uint32_t png_chunk_size;

    (void)sign; // Unused

    // Read last part of signature
    if(reader->read(reader->context, 4, png_bytes) != 4)
        goto PNG_ERROR;

    png_curr_offt += 4;

    if(memcmp(png_bytes, "\x0d\x0a\x1a\x0a", 4))
        goto PNG_ERROR;

    // Skip to header chunk
    while(1)
    {
        unsigned char png_chunk_head[8];

        if(reader->read(reader->context, 8, png_chunk_head) != 8)
            goto PNG_ERROR;

        png_curr_offt += 8;

        png_chunk_size = (uint32_t)(png_chunk_head[0])*16777216+(uint32_t)(png_chunk_head[1])*65536+(uint32_t)(png_chunk_head[2])*256+png_chunk_head[3];

        if(!memcmp(png_chunk_head+4, "IHDR", 4))
            break;

        png_curr_offt += (int64_t)4 + png_chunk_size;

        if(!reader->seek(reader->context, png_curr_offt, false))
            goto PNG_ERROR;
    }

    if(png_chunk_size != 0xD)
        goto PNG_ERROR;

    if(reader->read(reader->context, 10, png_bytes) != 10)
        goto PNG_ERROR;

    image->width = (uint32_t)(png_bytes[0])*16777216+(uint32_t)(png_bytes[1])*65536+(uint32_t)(png_bytes[2])*256+png_bytes[3];
    image->height = (uint32_t)(png_bytes[4])*16777216+(uint32_t)(png_bytes[5])*65536+(uint32_t)(png_bytes[6])*256+png_bytes[7];

    switch(png_bytes[9])
    {
    case 0:
        image->channels = 1;
        break;
    case 2:
        image->channels = 3;
        break;
    case 3:
        image->channels = 3;
        image->palette = png_bytes[8];
        break;
    case 4:
        image->channels = 2;
        break;
    case 6:
        image->channels = 4;
        break;
    default:
        goto PNG_ERROR;
    }

    if(image->palette)
        image->bitsperpixel = 24;
    else
        image->bitsperpixel = (unsigned int)(png_bytes[8]) * image->channels;

    return;

PNG_ERROR:
    memset(image, 0, sizeof(fastimage_image_t));
    image->format = fastimage_error;
}

static void fastimageReadGif(const fastimage_reader_t *reader, unsigned char *sign, fastimage_image_t *image)
{
    unsigned short gif_header_min[3];

    (void)sign; // Unused

    // GIF87a or GIF89a
    if(reader->read(reader->context, 6, &gif_header_min) != 6)
    {
        image->format = fastimage_error;

        return;
    }

    if( (gif_header_min[0] != '7'+(unsigned short)('a')*256)
            && (gif_header_min[0] != '9'+(unsigned short)('a')*256))
    {
        image->format = fastimage_error;

        return;
    }

    image->width = gif_header_min[1];
    image->height = gif_header_min[2];
    image->bitsperpixel = 24;
    image->channels = 3;
    image->palette = 8;
}

static void fastimageReadWebp(const fastimage_reader_t *reader, unsigned char *sign, fastimage_image_t *image)
{
    size_t riff_size;
    char fourcc[4], vp8fourcc[4];

    (void)sign; // Unused

    if(reader->read(reader->context, 4, &riff_size) != 4) goto WEBP_ERROR;
    if(riff_size < 8) goto WEBP_ERROR;

    if(reader->read(reader->context, 4, fourcc) != 4) goto WEBP_ERROR;

    if(memcmp(fourcc, "WEBP", 4))
    {
        if(!memcmp(fourcc, "ACON", 4))
            image->format = fastimage_ani;
        else
            image->format = fastimage_unknown;

        return;
    }

    if(reader->read(reader->context, 4, vp8fourcc) != 4) goto WEBP_ERROR;

    if(!memcmp(vp8fourcc, "VP8 ", 4))
    {
        image->bitsperpixel = 24;
        image->channels = 3;
    }
    else if(!memcmp(vp8fourcc, "VP8L", 4))     // Lossless?
    {
        image->bitsperpixel = 24;
        image->channels = 3;
    }
    else if(!memcmp(vp8fourcc, "VP8X", 4))
    {
        image->bitsperpixel = 32;
        image->channels = 4;
    }
    else return;

    return;

WEBP_ERROR:
    memset(image, 0, sizeof(fastimage_image_t));
    image->format = fastimage_error;
}

static void fastimageReadJpeg(const fastimage_reader_t *reader, unsigned char *sign, fastimage_image_t *image)
{
    int64_t jpg_curr_offt = 4;
    unsigned short jpg_frame;
    int64_t jpg_segment_size;

    jpg_frame = sign[2]+(unsigned short)(sign[3])*256;

    // First of all, skipping segments we don't needed
    // TODO: there are segments without length (with empty or 2-byte data). Need to check them
    while(1)
    {
        unsigned char jpg_bytes[2];

        //printf("jpeg segment %hx\n", jpg_frame);

        // Read segment size
        if(reader->read(reader->context, 2, jpg_bytes) != 2)
        {
            image->format = fastimage_error;

            return;
        }

        jpg_curr_offt += 2;

        jpg_segment_size = (int64_t)(jpg_bytes[0])*256+jpg_bytes[1];

        if(jpg_segment_size < 2)
        {
            image->format = fastimage_error;

            return;
        }

        jpg_segment_size -= 2;

        if(jpg_frame == 0xC0FF || jpg_frame == 0xC1FF || jpg_frame == 0xC2FF)
        {
            break;
        }

        // Skip segment
        jpg_curr_offt += jpg_segment_size;

        if(!reader->seek(reader->context, jpg_curr_offt, false))
        {
            image->format = fastimage_error;

            return;
        }

        // Read next segment signature
        if(reader->read(reader->context, 2, &jpg_frame) != 2)
        {
            image->format = fastimage_error;

            return;
        }

        jpg_curr_offt += 2;
    }

    if(jpg_frame == 0xC0FF || jpg_frame == 0xC1FF || jpg_frame == 0xC2FF)
    {
        // Found segment with header
        unsigned char jpg_bytes[6];

        if(jpg_segment_size < 6)
        {
            image->format = fastimage_error;

            return;
        }

        if(reader->read(reader->context, 6, jpg_bytes) != 6)
        {
            image->format = fastimage_error;

            return;
        }

        image->width = (size_t)(jpg_bytes[3])*256+jpg_bytes[4];
        image->height = (size_t)(jpg_bytes[1])*256+jpg_bytes[2];
        image->channels = jpg_bytes[5];
        image->bitsperpixel = image->channels * (unsigned int)(jpg_bytes[0]);
    }
    else
        image->format = fastimage_error;
}

static void fastimageDetectISOBMFF(const fastimage_reader_t *reader, unsigned char *sign, fastimage_image_t *image)
{
    size_t ftyp_size, i;
    unsigned char *ftyp_body;

    ftyp_size = (size_t)(sign[0])*16777216+(size_t)(sign[1])*65536+(size_t)(sign[2])*256+(size_t)(sign[3]);

    if(ftyp_size < 8) return;

    ftyp_size -= 4;
    if(ftyp_size%4) return;

    ftyp_body = malloc(ftyp_size);
    if(!ftyp_body) return;

    if(reader->read(reader->context, ftyp_size, ftyp_body) != ftyp_size)
    {
        free(ftyp_body);
        return;
    }

    if(memcmp(ftyp_body, "ftyp", 4))
    {
        free(ftyp_body);

        return;
    }

    for(i = 4; i < ftyp_size; i += 4)
    {
        if(!memcmp(ftyp_body+i, "mif1", 4) || !memcmp(ftyp_body + i, "miaf", 4))
        {
            image->format = fastimage_miaf;
        }
        if(!memcmp(ftyp_body+i, "heic", 4) || !memcmp(ftyp_body+i, "hevc", 4))
        {
            image->format = fastimage_heic;
            break;
        }
        if(!memcmp(ftyp_body + i, "avif", 4) || !memcmp(ftyp_body+i, "avis", 4))
        {
            image->format = fastimage_avif;
            break;
        }
    }
    free(ftyp_body);
}

static void fastimageReadISOBMFF(const fastimage_reader_t *reader, unsigned char *sign, fastimage_image_t *image)
{
    unsigned char atom_head[8];

    (void)sign;

    while(1)
    {
        size_t ftyp_size;

        if(reader->read(reader->context, 8, atom_head) != 8) goto ISOBMFF_ERROR;

        ftyp_size = (size_t)(atom_head[0])*16777216+(size_t)(atom_head[1])*65536+(size_t)(atom_head[2])*256+(size_t)(atom_head[3]);
        ftyp_size -= 8;

        if(ftyp_size < 8) goto ISOBMFF_ERROR;

        //printf("Container is %hc%hc%hc%hc\n", atom_head[4], atom_head[5], atom_head[6], atom_head[7]);

        if(!memcmp(atom_head+4, "meta", 4))
        {
            unsigned char *atom_data;
            size_t i;

            atom_data = malloc(ftyp_size);
            if(!atom_data) goto ISOBMFF_ERROR;

            if(reader->read(reader->context, ftyp_size, atom_data) != ftyp_size)
            {
                free(atom_data);
                goto ISOBMFF_ERROR;
            }

            // Very dirty implementation (I don't know what should be correct)
            for(i = 0; i < ftyp_size-20; i++)
            {
                // ispe 20 (12 - wid(be), 16 - hei(be))
                if(!memcmp(atom_data+i, "\x00\x00\x00\x14ispe", 8))
                {
                    image->width = (size_t)(atom_data[i+12])*16777216+(size_t)(atom_data[i+13])*65536+(size_t)(atom_data[i+14])*256+(size_t)(atom_data[i+15]);
                    image->height = (size_t)(atom_data[i+16])*16777216+(size_t)(atom_data[i+17])*65536+(size_t)(atom_data[i+18])*256+(size_t)(atom_data[i+19]);
                }
                else if(!memcmp(atom_data+i, "\x00\x00\x00\x10pixi", 8) || !memcmp(atom_data+i, "\x00\x00\x00\x0epixi", 8))
                {
                    size_t j;

                    image->channels += atom_data[i+12];

                    if(atom_data[i+12] > atom_data[i+3]-13) goto ISOBMFF_ERROR;

                    for(j = 0; j < atom_data[i+12]; j++)
                        image->bitsperpixel += atom_data[i+13];
                }
            }

            free(atom_data);
            break;
        }
        else if(!reader->seek(reader->context, ftyp_size, true)) goto ISOBMFF_ERROR;
    }

    return;

ISOBMFF_ERROR:
    memset(image, 0, sizeof(fastimage_image_t));
    image->format = fastimage_error;
}

static void fastimageReadQoi(const fastimage_reader_t *reader, unsigned char *sign, fastimage_image_t *image)
{
    qoi_header_t head;

    (void)sign; // unused

    if(reader->read(reader->context, sizeof(qoi_header_t), &head) != sizeof(qoi_header_t)) goto QOI_ERROR;

    if(head.channels < 3 || head.channels > 4) goto QOI_ERROR;
    if(head.colorspace > 1) goto QOI_ERROR;

    image->width = ((head.width&0xff000000)>>24)+((head.width&0xff0000)>>16)+((head.width&0xff00)<<16)+((head.width&0xff)<<24);
    image->height = ((head.height&0xff000000)>>24)+((head.height&0xff0000)>>16)+((head.height&0xff00)<<16)+((head.height&0xff)<<24);
    image->channels = head.channels;
    image->bitsperpixel = head.channels*8;

    return;
QOI_ERROR:
    memset(image, 0, sizeof(fastimage_image_t));
    image->format = fastimage_error;
}

static void fastimageReadIco(const fastimage_reader_t *reader, unsigned char *sign, fastimage_image_t *image)
{
    char header[14];
    bmp_infoheader_min_t binfoh;

    memcpy(header, sign, 4);

    if(reader->read(reader->context, 10, header+4) != 10) goto ICO_ERROR;

    if((!header[4] && !header[5]) || header[9]) goto ICO_ERROR; // Number of images should be greater than 0, Reserved should be 0

    image->width = header[6];
    if(!image->width) image->width = 256;
    image->height = header[7];
    if(!image->height) image->height = 256;

    image->channels = 4;
    image->bitsperpixel = 32;

#if 0
    if(header[8] > 16) image->palette = 8;
    else if(header[8] > 2) image->palette = 4;
    else if(header[8]) image->palette = 1;
#else
    image->palette = header[12]+256*header[13];
    if(image->palette > 8) image->palette = 0;
#endif
    if(!reader->seek(reader->context, 8, true)) return; // Skip bitmap length

    if(reader->read(reader->context, sizeof(bmp_infoheader_min_t), &binfoh) != sizeof(bmp_infoheader_min_t)) return;

    if(binfoh.biSize == 40)   // If not, it might be PNG
    {
        image->width = binfoh.biWidth;
        image->height = binfoh.biHeight / 2;

        if(binfoh.biBitCount <= 8) image->palette = binfoh.biBitCount;
        else image->palette = 0;
    }

    return;

ICO_ERROR:
    memset(image, 0, sizeof(fastimage_image_t));
    image->format = fastimage_error;
}

fastimage_image_t fastimageOpen(const fastimage_reader_t *reader)
{
    fastimage_image_t image;
    unsigned char sign[4];

    memset(&image, 0, sizeof(fastimage_image_t));

    if(reader->read(reader->context, 4, sign) != 4)
    {
        image.format = fastimage_error;

        return image;
    }

    image.format = fastimage_unknown;

    if(!memcmp(sign, "BM", 2))
        image.format = fastimage_bmp;
    else if(!memcmp(sign, "\x89PNG", 4))
        image.format = fastimage_png;
    else if(!memcmp(sign, "GIF8", 4)) // GIF87a or GIF89a
        image.format = fastimage_gif;
    else if(!memcmp(sign, "RIFF", 4))
        image.format = fastimage_webp;
    else if(!memcmp(sign, "qoif", 4))
        image.format = fastimage_qoi;
    else if(!memcmp(sign, "qoyf", 4))
        image.format = fastimage_qoy;
    else if(sign[0] == 0xFF && sign[1] == 0xD8)
        image.format = fastimage_jpg;
    else if(sign[0] == 10 && sign[1] == 5 && sign[2] == 1 && sign[3] == 8)
        image.format = fastimage_pcx;
    else if(sign[0] == 0 && sign[1] == 0 && sign[2] == 1 && sign[3] == 0)
        image.format = fastimage_ico;

    // Try to detect TGA
    switch(sign[2])   // DataType
    {
    case 1: // Palette, uncompressed
    case 9: // Palette, RLE
        if(sign[1] == 1) // Color Map (availability)
            image.format = fastimage_tga;
        break;
    case 2: // True Color, uncompressed
    case 3: // Grayscale, uncompressed
    case 10: // True Color, RLE
    case 11: // Grayscale, RLE
        if(sign[1] == 0)
            image.format = fastimage_tga;
        break;
    }


    // Try to detect HEIF or AVIF
    if(image.format == fastimage_unknown)
        fastimageDetectISOBMFF(reader, sign, &image); // Should be last, because we read some data here

    // Read BMP meta
    if(image.format == fastimage_bmp)
        fastimageReadBmp(reader, sign, &image);

    // Read TGA meta
    if(image.format == fastimage_tga)
        fastimageReadTga(reader, sign, &image);

    // Read PCX meta
    if(image.format == fastimage_pcx)
        fastimageReadPcx(reader, sign, &image);

    // Read PNG meta
    if(image.format == fastimage_png)
        fastimageReadPng(reader, sign, &image);

    // Read GIF meta
    if(image.format == fastimage_gif)
        fastimageReadGif(reader, sign, &image);

    // Read WEBP meta
    if(image.format == fastimage_webp)
        fastimageReadWebp(reader, sign, &image);

    // Read HEIC or AVIF meta
    if(image.format == fastimage_heic || image.format == fastimage_avif || image.format == fastimage_miaf)
        fastimageReadISOBMFF(reader, sign, &image);

    // Read JPG meta
    if(image.format == fastimage_jpg)
        fastimageReadJpeg(reader, sign, &image);

    if(image.format == fastimage_qoi || image.format == fastimage_qoy)
        fastimageReadQoi(reader, sign, &image);

    if(image.format == fastimage_ico)
        fastimageReadIco(reader, sign, &image);

    return image;
}

#if 0
static size_t FASTIMAGE_APIENTRY fastimageFileRead(void *context, size_t size, void *buf)
{
    return fread(buf, 1, size, context);
}

static bool FASTIMAGE_APIENTRY fastimageFileSeek(void *context, int64_t pos, bool seek_cur)
{
#if defined(_WIN32)
    return _fseeki64(context, pos, seek_cur?(SEEK_CUR):(SEEK_SET)) == 0;
#else
    return fseeko64(context, pos, seek_cur?(SEEK_CUR):(SEEK_SET)) == 0;
#endif
}

fastimage_image_t fastimageOpenFile(FILE *f)
{
    fastimage_reader_t reader;

    reader.context = f;
    reader.read = fastimageFileRead;
    reader.seek = fastimageFileSeek;

    return fastimageOpen(&reader);
}

fastimage_image_t fastimageOpenFileA(const char *filename)
{
    FILE *f = 0;

    f = fopen(filename, "rb");
    if(!f)
    {
        fastimage_image_t image;

        memset(&image, 0, sizeof(fastimage_image_t));
        image.format = fastimage_error;

        return image;
    }

    return fastimageOpenFile(f);
}

#endif
