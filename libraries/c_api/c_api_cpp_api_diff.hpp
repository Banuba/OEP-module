#pragma once

#include <preprocessor/build_macros.hpp>

#if C_API

#include <stddef.h>
#include <stdint.h>
#include <functional>
#include <memory>

#include <bnb/common_types.h>
#include <interfaces/c_api/nv12_image.hpp>

typedef bnb_image_orientation_t             bnb_image_orientation_alias;
typedef nv12_image                          image_type_alias;

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

typedef bnb::camera_orientation             bnb_image_orientation_alias;
typedef bnb::full_image_t                   image_type_alias;

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


/*
namespace bnb::interfaces
{
    enum class image_format
    {
        // CVPixelBufferRef with image on rgba
        rgba,
        // CVPixelBufferRef with image on nv12
        nv12,
        // CVPixelBufferRef with image on OGL texture
        // It is useful if you want to avoid GPU-CPU data sync for performance.
        // Please take into the account that on first pixel buffer lock sync will take place,
        // so it is assumed that data will not leave OGL context.
        texture
    };
}
*/

#endif /* CPP_API */