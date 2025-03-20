/*
    Scan Tailor - Interactive post-processing tool for scanned pages.
    Copyright (C) 2015  Joseph Artsimovich <joseph.artsimovich@gmail.com>

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

#ifndef IMAGEPROC_GRAYIMAGE_H_
#define IMAGEPROC_GRAYIMAGE_H_

#include <QImage>
#include <QSize>
#include <QRect>
#include <stdint.h>
#include <math.h>
#include "imageproc_config.h"
#include "GridAccessor.h"
#include "IntegralImage.h"
#include "Binarize.h"
#include "GaussBlur.h"

namespace imageproc
{

/**
 * \brief A wrapper class around QImage that is always guaranteed to be 8-bit grayscale.
 */
class IMAGEPROC_EXPORT GrayImage
{
public:
    /**
     * \brief Creates a 8-bit grayscale image with specified dimensions.
     *
     * The image contents won't be initialized.  You can use fill() to initialize them.
     * If size.isEmpty() is true, creates a null image.
     *
     * \throw std::bad_alloc Unlike the underlying QImage, GrayImage reacts to
     *        out-of-memory situations by throwing an exception rather than
     *        constructing a null image.
     */
    explicit GrayImage(QSize size = QSize());

    /**
     * \brief Constructs a 8-bit grayscale image by converting an arbitrary QImage.
     *
     * The QImage may be in any format and may be null.
     */
    explicit GrayImage(QImage const& image);

    GridAccessor<uint8_t const> accessor() const;

    GridAccessor<uint8_t> accessor();

    /**
     * \brief Returns a const reference to the underlying QImage.
     *
     * The underlying QImage is either a null image or a 8-bit indexed
     * image with a grayscale palette.
     */
    QImage const& toQImage() const
    {
        return m_image;
    }

    operator QImage const&() const
    {
        return m_image;
    }

    bool isNull() const
    {
        return m_image.isNull();
    }

    void fill(uint8_t color)
    {
        m_image.fill(color);
    }

    uint8_t* data()
    {
        return m_image.bits();
    }

    uint8_t const* data() const
    {
        return m_image.bits();
    }

    /**
     * \brief Number of bytes per line.
     *
     * This value may be larger than image width.
     * An additional guaranee provided by the underlying QImage
     * is that this value is a multiple of 4.
     */
    int stride() const
    {
        return m_image.bytesPerLine();
    }

    QSize size() const
    {
        return m_image.size();
    }

    QRect rect() const
    {
        return m_image.rect();
    }

    int width() const
    {
        return m_image.width();
    }

    int height() const
    {
        return m_image.height();
    }
private:
    QImage m_image;
};

inline bool operator==(GrayImage const& lhs, GrayImage const& rhs)
{
    return lhs.toQImage() == rhs.toQImage();
}

inline bool operator!=(GrayImage const& lhs, GrayImage const& rhs)
{
    return lhs.toQImage() != rhs.toQImage();
}

/**
 * \brief Statistic Images:
 * mean, deviation, max, contrast
 */
IMAGEPROC_EXPORT GrayImage grayMapMean(
    GrayImage const& src,
    int radius = 100);
IMAGEPROC_EXPORT GrayImage grayMapDeviation(
    GrayImage const& src,
    int radius = 100);
IMAGEPROC_EXPORT GrayImage grayMapMax(
    GrayImage const& src,
    int radius = 100);
IMAGEPROC_EXPORT GrayImage grayMapContrast(
    GrayImage const& src,
    int radius = 100);

/**
 * \brief Threshold Map Images:
 */
IMAGEPROC_EXPORT unsigned int grayBiModalTiledValue(
    GrayImage const& src,
    unsigned int const x1,
    unsigned int const y1,
    unsigned int const x2,
    unsigned int const y2,
    int delta = 0,
    unsigned char bound_lower = 0,
    unsigned char bound_upper = 255);
IMAGEPROC_EXPORT GrayImage grayBiModalTiledMap(
    GrayImage const& src,
    int radius = 50,
    float k = 0.75f,
    int delta = 0,
    unsigned char bound_lower = 0,
    unsigned char bound_upper = 255);
IMAGEPROC_EXPORT GrayImage grayNiblackMap(
    GrayImage const& src,
    int radius = 100,
    float k = 0.20f);
IMAGEPROC_EXPORT void grayBGtoMap(
    GrayImage const& src,
    GrayImage& background,
    float q = 0.6f,
    float p = 0.2f);
IMAGEPROC_EXPORT GrayImage graySauvolaMap(
    GrayImage const& src,
    int radius = 100,
    float k = 0.34f);
IMAGEPROC_EXPORT GrayImage grayWolfMap(
    GrayImage const& src,
    int radius = 100,
    float k = 0.30f);
IMAGEPROC_EXPORT GrayImage grayBradleyMap(
    GrayImage const& src,
    int radius = 100,
    float k = 0.2f);
IMAGEPROC_EXPORT GrayImage grayGradMap(
    GrayImage const& src,
    int radius = 10,
    float coef = 0.75f);
IMAGEPROC_EXPORT GrayImage graySinghMap(
    GrayImage const& src,
    int radius = 100,
    float k = 0.2f);
IMAGEPROC_EXPORT GrayImage grayWANMap(
    GrayImage const& src,
    int radius = 100,
    float k = 0.2f);
IMAGEPROC_EXPORT GrayImage grayMScaleMap(
    GrayImage const& src,
    int radius = 10,
    float coef = 0.5f);
IMAGEPROC_EXPORT GrayImage grayEngravingMap(
    GrayImage const& src,
    int radius = 20,
    float coef = 0.20f);
IMAGEPROC_EXPORT GrayImage grayDotsMap(
    GrayImage const& src,
    int delta = 0);

/**
 * threshold MAPs as filters
 */
void grayMapOverlay(
    GrayImage& src,
    GrayImage& gover,
    float coef = 0.0f);

/**
 * @brief Engraving GrayImage based GaussBlur.
 */
IMAGEPROC_EXPORT GrayImage grayGravure(
    GrayImage const& src,
    int radius = 15,
    float coef = 0.0f);
IMAGEPROC_EXPORT void grayGravureInPlace(
    GrayImage& src,
    int radius = 15,
    float coef = 0.0f);

/**
 * @brief Dots 8x8 GrayImage.
 * Modification: used GaussBlur.
 */
IMAGEPROC_EXPORT GrayImage grayDots8(
    GrayImage const& src,
    int radius = 17,
    float coef = 0.0f);
IMAGEPROC_EXPORT void grayDots8InPlace(
    GrayImage& src,
    int radius = 17,
    float coef = 0.0f);

/**
 * @brief Applies the Wiener filter to a grayscale image.
 *
 * @param src The image to apply the filter to. A null image is allowed.
 * @param window_size The local neighbourhood around a pixel to use.
 * @param noise_sigma The standard deviation of noise in the image.
 * @return The filtered image.
 */
IMAGEPROC_EXPORT GrayImage grayWiener(
    GrayImage const& src,
    int radius = 3,
    float noise_sigma = 0.01f);

/**
 * @brief An in-place version of grayWiener().
 * @see grayWiener()
 */
IMAGEPROC_EXPORT void grayWienerInPlace(
    GrayImage& src,
    int radius = 3,
    float noise_sigma = 0.01f);

/**
 * @brief KNN denoiser GrayImage based BoxBlur.
 */
IMAGEPROC_EXPORT GrayImage grayKnnDenoiser(
    GrayImage const& src,
    int radius = 1,
    float coef = 0.0f);
IMAGEPROC_EXPORT void grayKnnDenoiserInPlace(
    GrayImage& src,
    int radius = 1,
    float coef = 0.0f);

/**
 * @brief Median GrayImage.
 */
IMAGEPROC_EXPORT GrayImage grayMedian(
    GrayImage const& src,
    int radius = 2,
    float coef = 0.0f);
IMAGEPROC_EXPORT void grayMedianInPlace(
    GrayImage& src,
    int radius = 2,
    float coef = 0.0f);

/**
 * @brief Subtract BG GrayImage.
 */
IMAGEPROC_EXPORT GrayImage graySubtractBG(
    GrayImage const& src,
    int radius = 45,
    float coef = 0.0f);
IMAGEPROC_EXPORT void graySubtractBGInPlace(
    GrayImage& src,
    int radius = 45,
    float coef = 0.0f);

/**
 * @brief Despeckle GrayImage.
 */
IMAGEPROC_EXPORT GrayImage grayDespeckle(
    GrayImage const& src,
    int radius = 7,
    float coef = 0.0f);
IMAGEPROC_EXPORT void grayDespeckleInPlace(
    GrayImage& src,
    int radius = 7,
    float coef = 0.0f);

/**
 * @brief Auto Level GrayImage.
 * Modification: used GaussBlur.
 */
IMAGEPROC_EXPORT GrayImage grayAutoLevel(
    GrayImage const& src,
    int radius = 10,
    float coef = 0.0f);
IMAGEPROC_EXPORT void grayAutoLevelInPlace(
    GrayImage& src,
    int radius = 10,
    float coef = 0.0f);

/**
 * @brief Balance GrayImage based GaussBlur.
 */
IMAGEPROC_EXPORT GrayImage grayBalance(
    GrayImage const& src,
    int radius = 23,
    float coef = 0.0f);
IMAGEPROC_EXPORT void grayBalanceInPlace(
    GrayImage& src,
    int radius = 23,
    float coef = 0.0f);

/**
 * @brief OverBlur GrayImage based GaussBlur.
 */
IMAGEPROC_EXPORT GrayImage grayOverBlur(
    GrayImage const& src,
    int radius = 49,
    float coef = 0.0f);
IMAGEPROC_EXPORT void grayOverBlurInPlace(
    GrayImage& src,
    int radius = 49,
    float coef = 0.0f);

/**
 * @brief Retinex GrayImage based GaussBlur.
 */
IMAGEPROC_EXPORT GrayImage grayRetinex(
    GrayImage const& src,
    int radius = 31,
    float coef = 0.0f);
IMAGEPROC_EXPORT void grayRetinexInPlace(
    GrayImage& src,
    int radius = 31,
    float coef = 0.0f);

/**
 * @brief Equalize GrayImage.
 * Modification: used GaussBlur.
 */
IMAGEPROC_EXPORT GrayImage grayEqualize(
    GrayImage const& src,
    int radius = 1,
    float coef = 0.0f,
    bool flg_blur = false);
IMAGEPROC_EXPORT void grayEqualizeInPlace(
    GrayImage& src,
    int radius = 1,
    float coef = 0.0f,
    bool flg_blur = false);

/**
 * @brief Sigma (blur manipulation) GrayImage based GaussBlur.
 */
IMAGEPROC_EXPORT GrayImage graySigma(
    GrayImage const& src,
    int radius = 29,
    float coef = 0.0f);
IMAGEPROC_EXPORT void graySigmaInPlace(
    GrayImage& src,
    int radius = 29,
    float coef = 0.0f);

/**
 * @brief Blur GrayImage based GaussBlur.
 */
IMAGEPROC_EXPORT GrayImage grayBlur(
    GrayImage const& src,
    int radius = 1,
    float coef = 0.0f);
IMAGEPROC_EXPORT void grayBlurInPlace(
    GrayImage& src,
    int radius = 1,
    float coef = 0.0f);

/**
 * @brief Comix filter GrayImage based GaussBlur.
 */
IMAGEPROC_EXPORT GrayImage grayComix(
    GrayImage const& src,
    int radius = 6,
    float coef = 0.0f);
IMAGEPROC_EXPORT void grayComixInPlace(
    GrayImage& src,
    int radius = 6,
    float coef = 0.0f);

/**
 * @brief Screen GrayImage based GaussBlur.
 */
IMAGEPROC_EXPORT GrayImage grayScreen(
    GrayImage const& src,
    int radius = 5,
    float coef = 0.0f);
IMAGEPROC_EXPORT void grayScreenInPlace(
    GrayImage& src,
    int radius = 5,
    float coef = 0.0f);

/**
 * \brief Image prefilter EdgeDiv (EdgePlus & BlurDiv) for local/global thresholding method.
 *
 * EdgeDiv, zvezdochiot 2023. "Adaptive/global document image binarization".
 */
IMAGEPROC_EXPORT GrayImage grayEdgeDiv(
    GrayImage const& src,
    int radius = 10,
    double kep = 0.5,
    double kdb = 0.5);
IMAGEPROC_EXPORT void grayEdgeDivInPlace(
    GrayImage& src,
    int radius = 10,
    double kep = 0.5,
    double kdb = 0.5);
/**
 * \brief Image prefilter Robust's for local thresholding method.
 *
 * Tom Liao
 * "Robust document binarization with OFF center-surround cells", 2017-08-09.
 * https://www.researchgate.net/publication/226333284_Robust_document_binarization_with_OFF_center-surround_cells
 */
IMAGEPROC_EXPORT GrayImage grayRobust(
    GrayImage const& src,
    int radius = 10,
    float coef = 0.2f);
IMAGEPROC_EXPORT void grayRobustInPlace(
    GrayImage& src,
    int radius = 10,
    float coef = 0.2f);

/**
 * @brief Dominanta GrayImage.
 */
IMAGEPROC_EXPORT unsigned int grayDominantaValue(
    GrayImage const& src);

} // namespace imageproc

#endif
