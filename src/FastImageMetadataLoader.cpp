/*
    Scan Tailor - Interactive post-processing tool for scanned pages.
	Copyright (C) 2007-2009  Joseph Artsimovich <joseph_a@mail.ru>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/


#include "FastImageMetadataLoader.h"
#include "ImageMetadata.h"
#include "fastimage.h"
#include <QIODevice>
#include <QSize>
#include <limits.h>

static size_t FASTIMAGE_APIENTRY fastimageQIODeviceRead(void* context, size_t size, void *buf)
{
    QIODevice* io_device = (QIODevice*)context;
    char* read_ptr = (char*)buf;
    size_t read_size = 0;

    while (size > read_size)
    {
        qint64 const read = io_device->read(read_ptr, size-read_size);

        if (read <= 0) break;

        read_size += (size_t)read;
        read_ptr += read;
    }

    return read_size;
}

static bool FASTIMAGE_APIENTRY fastimageQIODeviceSeek(void* context, int64_t pos, bool seek_cur)
{
    QIODevice* io_device = (QIODevice*)context;

    if(seek_cur)
    {
        qint64 cur_pos = io_device->pos();

        if(cur_pos < 0) return false;
        if(INT64_MAX-cur_pos < pos) return false;
        if(cur_pos+pos < 0) return false;

        return io_device->seek(cur_pos+pos);
    }
    else
        return io_device->seek(pos);
}

void
FastImageMetadataLoader::registerMyself()
{
    static bool registered = false;
    if (!registered)
    {
        ImageMetadataLoader::registerLoader(
            IntrusivePtr<ImageMetadataLoader>(new FastImageMetadataLoader)
        );
        registered = true;
    }
}

ImageMetadataLoader::Status
FastImageMetadataLoader::loadMetadata(
    QIODevice& io_device,
    VirtualFunction1<void, ImageMetadata const&>& out)
{
    fastimage_reader_t fastimage_reader = { (void*)(&io_device), fastimageQIODeviceRead, fastimageQIODeviceSeek };

    fastimage_image_t fastimage_image = fastimageOpen(&fastimage_reader);

    if(fastimage_image.format == fastimage_error)
        return GENERIC_ERROR;
    else if(fastimage_image.format == fastimage_unknown)
        return FORMAT_NOT_RECOGNIZED;
    else if(fastimage_image.width >= INT_MAX || fastimage_image.height >= INT_MAX)
        return GENERIC_ERROR;

    QSize size;
    size.setWidth(fastimage_image.width);
    size.setHeight(fastimage_image.height);

    out(ImageMetadata(size));

    return LOADED;
}


