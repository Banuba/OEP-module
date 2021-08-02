#pragma once

#include <bnb/common_types.h>

#include <memory>

using color_plane_data_t = uint8_t;
using color_plane = std::shared_ptr<color_plane_data_t>;

inline color_plane color_plane_alloc(size_t size)
{
    return color_plane(new color_plane_data_t [size], [](color_plane_data_t* p) { delete[] p; });
}

class nv12_image
{
public:
    nv12_image(color_plane y_plane, int32_t y_stride,
               color_plane uv_plane, int32_t uv_stride,
               const bnb_image_format_t& format)
        : m_y_plane(std::move(y_plane))
        , m_uv_plane(std::move(uv_plane))
        , m_y_stride(y_stride)
        , m_uv_stride(uv_stride)
        , m_format(format)
    {}

    color_plane_data_t* get_y_data() const { return m_y_plane.get(); }
    color_plane_data_t* get_uv_data() const { return m_uv_plane.get(); }

    int32_t get_y_stride() const { return m_y_stride; }
    int32_t get_uv_stride() const { return m_uv_stride; }

    const bnb_image_format_t& get_format() const { return m_format; }

private:
    color_plane m_y_plane;
    color_plane m_uv_plane;
    int32_t m_y_stride;
    int32_t m_uv_stride;
    bnb_image_format_t m_format;
};
