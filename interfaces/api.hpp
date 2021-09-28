#pragma once

#include <preprocessor/build_macros.hpp>

#if C_API

#include <stddef.h>
#include <stdint.h>
#include <functional>
#include <memory>

#include <bnb/common_types.h>
#include <bnb/effect_player.h>
#include <bnb/utility_manager.h>


class image_wrapper
{
public:
                            image_wrapper();
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

    bnb_image_format_t      get_format();
    uint8_t                 bytes_per_pixel();
    full_image_holder_t*    get();
private:
    std::shared_ptr<full_image_holder_t>    m_image;
    uint8_t                                 m_pxsize{0};
};



/* image_wrapper::image_wrapper */
inline image_wrapper::image_wrapper()
{
}

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
    return std::move(fmt);
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



typedef bnb_image_orientation_t             bnb_image_orientation_alias;
typedef image_wrapper                       bnb_full_image_alias;
typedef utility_manager_holder_t*           bnb_utility_manager_alias;
typedef effect_player_holder_t*             bnb_effect_player_alias;


/* aliases for bnb_image_orientation_t (bnb_image_orientation_alias) */
#define BNB_DEG_0_ALIAS                     BNB_DEG_0
#define BNB_DEG_90_ALIAS                    BNB_DEG_90
#define BNB_DEG_180_ALIAS                   BNB_DEG_180
#define BNB_DEG_270_ALIAS                   BNB_DEG_270

namespace bnb {
    struct data_t
    {
        using type = uint8_t[];
        using pointer = uint8_t*;
        using uptr = std::unique_ptr<type, std::function<void(pointer)>>;
        uptr data;
        size_t size;
    };
} // bnb

#elif CPP_API

#include <bnb/types/base_types.hpp>
#include <bnb/types/full_image.hpp>
#include <bnb/effect_player/interfaces/all.hpp>
#include <bnb/effect_player/utility.hpp>

typedef bnb::camera_orientation             bnb_image_orientation_alias;
typedef bnb::full_image_t                   bnb_full_image_alias;
typedef bnb::utility                        bnb_utility_manager_alias;
typedef std::shared_ptr<bnb::interfaces::effect_player>  bnb_effect_player_alias;

/* aliases for bnb::camera_orientation (bnb_image_orientation_alias) */
#define BNB_DEG_0_ALIAS                     bnb::camera_orientation::deg_0
#define BNB_DEG_90_ALIAS                    bnb::camera_orientation::deg_90
#define BNB_DEG_180_ALIAS                   bnb::camera_orientation::deg_180
#define BNB_DEG_270_ALIAS                   bnb::camera_orientation::deg_270

namespace bnb::render
{
    struct nv12_planes
    {
        color_plane y_plane;
        color_plane uv_plane;
    };
}  /* namespace bnb::render */

#endif /* CPP_API */
