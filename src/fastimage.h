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

#ifndef FASTIMAGE_H
#define FASTIMAGE_H

#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>
#include <wchar.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifdef _WIN32
#define FASTIMAGE_APIENTRY cdecl
#else
#define FASTIMAGE_APIENTRY
#endif

enum fastimage_image_format {
	fastimage_error,
	fastimage_unknown,
	fastimage_bmp,
	fastimage_tga,
	fastimage_pcx,
	fastimage_png,
	fastimage_gif,
	fastimage_webp,
	fastimage_heic,
	fastimage_jpg,
	fastimage_avif,
	fastimage_miaf, // Not heic and not avif
	fastimage_qoi,
	fastimage_qoy,
	fastimage_ani,
	fastimage_ico
};

typedef struct {
	int format;
	size_t width;
	size_t height;
	unsigned int channels;
	unsigned int bitsperpixel;
	unsigned int palette;
} fastimage_image_t;

typedef size_t (FASTIMAGE_APIENTRY * fastimage_readfunc_t)(void *context, size_t size, void *buf);
typedef bool (FASTIMAGE_APIENTRY * fastimage_seekfunc_t)(void *context, int64_t pos, bool seek_cur);

typedef struct {
	void *context;
	fastimage_readfunc_t read;
	fastimage_seekfunc_t seek;
} fastimage_reader_t;

extern fastimage_image_t fastimageOpen(const fastimage_reader_t *reader);

#ifdef __cplusplus
}
#endif

#endif
