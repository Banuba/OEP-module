#include "pixel_buffer.hpp"

#include <iostream>
#include <libyuv.h>

namespace bnb
{

    /* pixel_buffer::pixel_buffer */
    pixel_buffer::pixel_buffer(oep_sptr oep_sptr, uint32_t width, uint32_t height, bnb_image_orientation_alias orientation)
        : m_oep_ptr(oep_sptr)
        , m_width(width)
        , m_height(height)
        , m_orientation(orientation)
    {
    }

    /* pixel_buffer::lock */
    void pixel_buffer::lock()
    {
        ++lock_count;
    }

    /* pixel_buffer::unlock */
    void pixel_buffer::unlock()
    {
        if (lock_count > 0) {
            --lock_count;
            return;
        }

        throw std::runtime_error("pixel_buffer already unlocked");
    }

    /* pixel_buffer::is_locked */
    bool pixel_buffer::is_locked()
    {
        if (lock_count == 0) {
            return false;
        }
        return true;
    }

    /* pixel_buffer::get_rgba */
    void pixel_buffer::get_rgba(oep_image_ready_cb callback)
    {
        if (!is_locked()) {
            std::cout << "[WARNING] The pixel buffer must be locked" << std::endl;
            callback(std::nullopt);
        }

        if (auto oep_sp = m_oep_ptr.lock()) {
            auto convert_callback = [this, callback](data_t data) {
#if C_API_ENABLED
                bnb_image_format_t imfmt{m_width, m_height, m_orientation, false, 0};
                bnb_pixel_format_t pxfmt{BNB_RGBA};
                image_wrapper img(imfmt, pxfmt, data.data.get(), m_width * 4);

                callback(img);
#elif CPP_API_ENABLED
                bnb::image_format frm(m_width, m_height, m_orientation, false, 0, std::nullopt);
                auto bpc8 = bpc8_image_t(color_plane_weak(data.data.get()), interfaces::pixel_format::rgba, frm);
                callback(full_image_t(std::move(bpc8)));
#endif /* CPP_API_ENABLED */
            };

            oep_sp->read_current_buffer(convert_callback);
        } else {
            std::cout << "[ERROR] Offscreen effect player destroyed" << std::endl;
        }
    }

    /* pixel_buffer::get_rgba */
    std::optional<bnb_full_image_alias> pixel_buffer::get_rgba()
    {
        if (auto oep_sp = m_oep_ptr.lock()) {
            data_t data = oep_sp->read_current_buffer();
#if C_API_ENABLED
            bnb_image_format_t imfmt{m_width, m_height, m_orientation, false, 0};
            bnb_pixel_format_t pxfmt{BNB_RGBA};
            image_wrapper img(imfmt, pxfmt, data.data.get(), m_width * 4);
            return img;
#elif CPP_API_ENABLED
            bnb::image_format frm(m_width, m_height, m_orientation, false, 0, std::nullopt);
            auto bpc8 = bpc8_image_t(color_plane_weak(data.data.get()), interfaces::pixel_format::rgba, frm);
            auto img = full_image_t(std::move(bpc8));
            return img;
#endif /* CPP_API_ENABLED */
        } else {
            std::cout << "[ERROR] Offscreen effect player destroyed" << std::endl;
            return std::nullopt;
        }
    }

    /* pixel_buffer::get_nv12 */
    void pixel_buffer::get_nv12(oep_image_ready_cb callback)
    {
        if (!is_locked()) {
            std::cout << "[WARNING] The pixel buffer must be locked" << std::endl;
            callback(std::nullopt);
        }

        if (auto oep_sp = m_oep_ptr.lock()) {
            auto convert_callback = [this, callback](data_t data) {
                std::vector<uint8_t> y_plane(m_width * m_height);
                std::vector<uint8_t> uv_plane((m_width / 2 * m_height / 2) * 2);

                libyuv::ABGRToNV12(data.data.get(), m_width * 4, y_plane.data(), m_width, uv_plane.data(), m_width, m_width, m_height);
#if C_API_ENABLED
                bnb_image_format_t imfmt{m_width, m_height, m_orientation, false, 0};
                image_wrapper img(imfmt, y_plane.data(), m_width, uv_plane.data(), m_width / 2);

                callback(img);
#elif CPP_API_ENABLED
                bnb::image_format frm(m_width, m_height, m_orientation, false, 0, std::nullopt);

                callback(full_image_t(yuv_image_t(color_plane_vector(y_plane), color_plane_vector(uv_plane), frm)));
#endif /* CPP_API_ENABLED */
            };

            oep_sp->read_current_buffer(convert_callback);
        } else {
            std::cout << "[ERROR] Offscreen effect player destroyed" << std::endl;
        }
    }

    /* pixel_buffer::get_texture */
    void pixel_buffer::get_texture(oep_texture_cb callback)
    {
        if (!is_locked()) {
            std::cout << "[WARNING] The pixel buffer must be locked" << std::endl;
            callback(std::nullopt);
        }
        if (auto oep_sp = m_oep_ptr.lock()) {
            oep_sp->get_current_buffer_texture(callback);
        } else {
            std::cout << "[ERROR] Offscreen effect player destroyed" << std::endl;
        }
    }

} /* namespace bnb */
