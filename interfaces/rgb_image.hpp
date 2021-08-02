#pragma once

#include <bnb/common_types.h>

#include <memory>
#include <string>

using color_plane_data_t = uint8_t;
using color_plane = std::shared_ptr<color_plane_data_t>;

class rgb_image
{
public:
    rgb_image(color_plane data,
               const bnb_image_format_t& i_format,
               const bnb_pixel_format_t& p_format)
        : m_data(std::move(data))
        , m_i_format(i_format)
        , m_p_format(p_format)
    {}

    color_plane_data_t* get_data() const { return m_data.get(); }

    int32_t get_stride() const { return m_i_format.width * bytes_per_pixel(); }

    const bnb_image_format_t& get_i_format() const { return m_i_format; }
    const bnb_pixel_format_t& get_p_format() const { return m_p_format; }

    uint8_t bytes_per_pixel() const
    {
        using fmt_t = bnb_pixel_format_t;
        switch (m_p_format) {
            case fmt_t::BNB_ARGB:
            case fmt_t::BNB_RGBA:
            case fmt_t::BNB_BGRA:
                return 4;
            case fmt_t::BNB_RGB:
            case fmt_t::BNB_BGR:
                return 3;
        }
        throw std::invalid_argument(
            "Unexpected pixel format value: " + std::to_string(static_cast<uint8_t>(m_p_format)));
    }

private:
    color_plane m_data;
    bnb_image_format_t m_i_format;
    bnb_pixel_format_t m_p_format;
};
