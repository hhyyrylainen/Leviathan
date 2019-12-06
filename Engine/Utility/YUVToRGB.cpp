// ------------------------------------ //
#include "YUVToRGB.h"

#include <algorithm>

using namespace Leviathan;
// ------------------------------------ //
// Start of code from: https://github.com/descampsa/yuv2rgb
// Copyright 2016 Adrien Descamps
// Distributed under BSD 3-Clause License
// Modified by Henri Hyyryläinen for inclusion here
// TODO: the original version also has SIMD versions which might also be able to be translated
// similarly to what was done to this version, in order to improve performance

enum YCbCrType { YCBCR_JPEG, YCBCR_601, YCBCR_709 };

uint8_t clamp(int16_t value)
{
    return value < 0 ? 0 : (value > 255 ? 255 : value);
}

#define FIXED_POINT_VALUE(value, precision) ((int)(((value) * (1 << precision)) + 0.5))

struct YUV2RGBParam {
    using ParamT = uint8_t;
    // using ParamT = uint16_t;

    YUV2RGBParam(ParamT cb_factor, ParamT cr_factor, ParamT g_cb_factor, ParamT g_cr_factor,
        ParamT y_factor, ParamT y_offset) :
        cb_factor(cb_factor),
        cr_factor(cr_factor), g_cb_factor(g_cb_factor), g_cr_factor(g_cr_factor),
        y_factor(y_factor), y_offset(y_offset)
    {}

    ParamT cb_factor; // [(255*CbNorm)/CbRange]
    ParamT cr_factor; // [(255*CrNorm)/CrRange]
    ParamT g_cb_factor; // [Bf/Gf*(255*CbNorm)/CbRange]
    ParamT g_cr_factor; // [Rf/Gf*(255*CrNorm)/CrRange]
    ParamT y_factor; // [(YMax-YMin)/255]
    ParamT y_offset; // YMin
};

constexpr auto PARAM_SET_1_SHIFT = 6;
constexpr auto PARAM_SET_2_SHIFT = 6; // was originally 7

#define YUV2RGB_PARAM(Rf, Bf, YMin, YMax, CbCrRange)                                         \
    YUV2RGBParam(FIXED_POINT_VALUE(255.0 * (2.0 * (1 - Bf)) / CbCrRange, PARAM_SET_1_SHIFT), \
        FIXED_POINT_VALUE(255.0 * (2.0 * (1 - Rf)) / CbCrRange, PARAM_SET_1_SHIFT),          \
        FIXED_POINT_VALUE(                                                                   \
            Bf / (1.0 - Bf - Rf) * 255.0 * (2.0 * (1 - Bf)) / CbCrRange, PARAM_SET_2_SHIFT), \
        FIXED_POINT_VALUE(                                                                   \
            Rf / (1.0 - Bf - Rf) * 255.0 * (2.0 * (1 - Rf)) / CbCrRange, PARAM_SET_2_SHIFT), \
        FIXED_POINT_VALUE(255.0 / (YMax - YMin), PARAM_SET_2_SHIFT), YMin)


static const YUV2RGBParam YUV2RGB[] = {
    // ITU-T T.871 (JPEG)
    YUV2RGB_PARAM(0.299, 0.114, 0.0, 255.0, 255.0),
    // ITU-R BT.601-7
    YUV2RGB_PARAM(0.299, 0.114, 16.0, 235.0, 224.0),
    // ITU-R BT.709-6
    YUV2RGB_PARAM(0.2126, 0.0722, 16.0, 235.0, 224.0)};


template<typename T>
void YUV420ToRGBA(uint32_t width, uint32_t height, const T* RESTRICT Y, const T* RESTRICT U,
    const T* RESTRICT V, uint32_t Y_stride, uint32_t UV_stride, uint8_t* RESTRICT RGB,
    uint32_t RGB_stride, YCbCrType yuv_type)
{
    const YUV2RGBParam* const param = &(YUV2RGB[yuv_type]);
    for(uint32_t y = 0; y < (height - 1); y += 2) {
        const T* y_ptr1 = Y + y * Y_stride;
        const T* y_ptr2 = Y + (y + 1) * Y_stride;
        const T* u_ptr = U + (y / 2) * UV_stride;
        const T* v_ptr = V + (y / 2) * UV_stride;

        uint8_t* rgb_ptr1 = RGB + y * RGB_stride;
        uint8_t* rgb_ptr2 = RGB + (y + 1) * RGB_stride;

        for(uint32_t x = 0; x < (width - 1); x += 2) {
            int16_t u_tmp = u_ptr[0] - 128;
            int16_t v_tmp = v_ptr[0] - 128;

            // compute Cb Cr color offsets, common to four pixels
            int16_t b_cb_offset, r_cr_offset, g_cbcr_offset;
            b_cb_offset = (param->cb_factor * u_tmp) >> PARAM_SET_1_SHIFT;
            r_cr_offset = (param->cr_factor * v_tmp) >> PARAM_SET_1_SHIFT;
            g_cbcr_offset =
                (param->g_cb_factor * u_tmp + param->g_cr_factor * v_tmp) >> PARAM_SET_2_SHIFT;

            int16_t y_tmp;
            y_tmp = (param->y_factor * (y_ptr1[0] - param->y_offset)) >> PARAM_SET_2_SHIFT;
            rgb_ptr1[0] = clamp(y_tmp + r_cr_offset);
            rgb_ptr1[1] = clamp(y_tmp - g_cbcr_offset);
            rgb_ptr1[2] = clamp(y_tmp + b_cb_offset);
            rgb_ptr1[3] = 255;

            y_tmp = (param->y_factor * (y_ptr1[1] - param->y_offset)) >> PARAM_SET_2_SHIFT;
            rgb_ptr1[4] = clamp(y_tmp + r_cr_offset);
            rgb_ptr1[5] = clamp(y_tmp - g_cbcr_offset);
            rgb_ptr1[6] = clamp(y_tmp + b_cb_offset);
            rgb_ptr1[7] = 255;

            y_tmp = (param->y_factor * (y_ptr2[0] - param->y_offset)) >> PARAM_SET_2_SHIFT;
            rgb_ptr2[0] = clamp(y_tmp + r_cr_offset);
            rgb_ptr2[1] = clamp(y_tmp - g_cbcr_offset);
            rgb_ptr2[2] = clamp(y_tmp + b_cb_offset);
            rgb_ptr2[3] = 255;

            y_tmp = (param->y_factor * (y_ptr2[1] - param->y_offset)) >> PARAM_SET_2_SHIFT;
            rgb_ptr2[4] = clamp(y_tmp + r_cr_offset);
            rgb_ptr2[5] = clamp(y_tmp - g_cbcr_offset);
            rgb_ptr2[6] = clamp(y_tmp + b_cb_offset);
            rgb_ptr2[7] = 255;

            rgb_ptr1 += 8;
            rgb_ptr2 += 8;
            y_ptr1 += 2;
            y_ptr2 += 2;
            u_ptr += 1;
            v_ptr += 1;
        }
    }
}

// End of code from https://github.com/descampsa/yuv2rgb
// ------------------------------------ //
DLLEXPORT bool Leviathan::YUVToRGBA(const std::array<const uint8_t*, 3>& planes,
    const std::array<int, 3>& strides, const std::array<std::tuple<int, int>, 3>& planesizes,
    bool bytesare16bit, uint8_t* target, size_t width, size_t height,
    YUV_COLOUR_SPACE colourtype /*= YUV_COLOUR_SPACE::BT_601*/)
{
    if(strides[1] != strides[2]) {
        LOG_ERROR("YUVToRGBA: U and V planes must be the same size");
        return false;
    }

    if(std::get<0>(planesizes[0]) != static_cast<int>(width) ||
        std::get<1>(planesizes[0]) != static_cast<int>(height)) {
        LOG_ERROR("YUVToRGBA: Y (luma) plane is not the same size as target");
        return false;
    }

    // Only cases where the luma plane is 2 times the width of the other planes is handled
    if(std::get<0>(planesizes[0]) != std::get<0>(planesizes[1]) * 2) {
        LOG_ERROR("YUVToRGBA: only the case where Y is the twice the size of U is handled");
        return false;
    }

    if(!bytesare16bit) {
        YUV420ToRGBA(width, height, planes[0], planes[1], planes[2], strides[0], strides[1],
            target, width * 4, YCBCR_601);
    } else {
        using T = int16_t;

        YUV420ToRGBA(width, height, reinterpret_cast<const T*>(planes[0]),
            reinterpret_cast<const T*>(planes[1]), reinterpret_cast<const T*>(planes[2]),
            strides[0] / static_cast<int>(sizeof(T)), strides[1] / static_cast<int>(sizeof(T)),
            target, width * 4, YCBCR_601);
    }

    return true;
}
