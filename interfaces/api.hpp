#pragma once

#include <preprocessor/build_macros.hpp>

#if C_API_ENABLED /* c-api dependent code */

#include <bnb/common_types.h>
#include <bnb/effect_player.h>
#include <bnb/utility_manager.h>

#include <stddef.h>
#include <stdint.h>
#include <functional>
#include <memory>

namespace bnb
{

struct data_t
{
    using type = uint8_t[];
    using pointer = uint8_t*;
    using uptr = std::unique_ptr<type, std::function<void(pointer)>>;
    uptr data;
    size_t size;
}; /* struct data_t */

} /* namespace bnb */

class image_wrapper
{
public:
                            image_wrapper() = default;
    explicit                image_wrapper(
                                    bnb_image_format_t format,
                                    bnb_pixel_format_t pixel_format,
                                    uint8_t* data,
                                    int32_t stride
                            );
    explicit                image_wrapper(
                                    bnb_image_format_t format,
                                    uint8_t* y_plane,
                                    int32_t y_stride,
                                    uint8_t* uv_plane,
                                    int32_t uv_stride
                            );
                            ~image_wrapper() = default;

    bnb_image_format_t      get_format();
    uint8_t                 bytes_per_pixel();
    full_image_holder_t*    get();
private:
    std::shared_ptr<full_image_holder_t>    m_image;
    uint8_t                                 m_pxsize{0};
}; /* class image_wrapper */



/* image_wrapper::image_wrapper     CONSTRUCTOR create bpc8 an image */
inline image_wrapper::image_wrapper(
        bnb_image_format_t format,
        bnb_pixel_format_t pixel_format,
        uint8_t* data,
        int32_t stride
) 
        : m_image(
                bnb_full_image_from_bpc8_img(format, pixel_format, data, stride, nullptr),
                [](full_image_holder_t* p){ bnb_full_image_release(p, nullptr); }
        )
{
    using fmt_t = bnb_image_format_t;
    switch (pixel_format) {
        case BNB_ARGB:
        case BNB_RGBA:
        case BNB_BGRA:
            m_pxsize = 4;
            return;
        case BNB_RGB:
        case BNB_BGR:
            m_pxsize = 3;
            return;
    }
    throw std::invalid_argument("Unexpected pixel format value");
}

/* image_wrapper::image_wrapper     CONSTRUCTOR create yuv_nv12 an image */
inline image_wrapper::image_wrapper(
        bnb_image_format_t format,
        uint8_t* y_plane,
        int32_t y_stride,
        uint8_t* uv_plane,
        int32_t uv_stride
)
        : m_image(
                bnb_full_image_from_yuv_nv12_img(format, y_plane, y_stride, uv_plane, uv_stride, nullptr),
                [](full_image_holder_t* p){ bnb_full_image_release(p, nullptr); }
        )
{
}

/* image_wrapper::get_format */
inline bnb_image_format_t image_wrapper::get_format()
{
    bnb_image_format_t fmt;
    bnb_full_image_get_format(get(), &fmt, nullptr);
    return fmt;
}

/* image_wrapper::bytes_per_pixel */
inline uint8_t image_wrapper::bytes_per_pixel()
{
    return m_pxsize;
}

/* image_wrapper::get */
inline full_image_holder_t* image_wrapper::get()
{
    return m_image.get();
}

using bnb_image_orientation_alias =         bnb_image_orientation_t;
using bnb_full_image_alias =                image_wrapper;
using bnb_utility_manager_alias =           utility_manager_holder_t*;
using bnb_effect_player_alias =             effect_player_holder_t*;

/* aliases for bnb_image_orientation_t (bnb_image_orientation_alias) */
#define BNB_DEG_0_ALIAS                     BNB_DEG_0
#define BNB_DEG_90_ALIAS                    BNB_DEG_90
#define BNB_DEG_180_ALIAS                   BNB_DEG_180
#define BNB_DEG_270_ALIAS                   BNB_DEG_270

#elif CPP_API_ENABLED /* cpp-api dependent code */

#include <bnb/types/base_types.hpp>
#include <bnb/types/full_image.hpp>
#include <bnb/effect_player/interfaces/all.hpp>
#include <bnb/effect_player/utility.hpp>

using bnb_image_orientation_alias =         bnb::camera_orientation;
using bnb_full_image_alias =                bnb::full_image_t;
using bnb_utility_manager_alias =           bnb::utility;
using bnb_effect_player_alias =             std::shared_ptr<bnb::interfaces::effect_player>;

/* aliases for bnb::camera_orientation (bnb_image_orientation_alias) */
#define BNB_DEG_0_ALIAS                     bnb::camera_orientation::deg_0
#define BNB_DEG_90_ALIAS                    bnb::camera_orientation::deg_90
#define BNB_DEG_180_ALIAS                   bnb::camera_orientation::deg_180
#define BNB_DEG_270_ALIAS                   bnb::camera_orientation::deg_270

#endif /* CPP_API_ENABLED */

/* code that does not depend on c-api or cpp-api, and is too small to put it in a
* separate file */
namespace bnb::interfaces
{

struct orient_format
{
    bnb_image_orientation_alias orientation;
    bool is_y_flip;
}; /* struct orient_format */

} /* namespace bnb::interfaces */
