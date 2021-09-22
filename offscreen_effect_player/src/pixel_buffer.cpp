#include "pixel_buffer.hpp"

#include <iostream>
#include <libyuv.h>

namespace bnb
{
    pixel_buffer::pixel_buffer(oep_sptr oep_sptr, uint32_t width, uint32_t height, bnb_image_orientation_alias orientation)
        : m_oep_ptr(oep_sptr)
        , m_width(width)
        , m_height(height)
        , m_orientation(orientation) {}

    void pixel_buffer::lock()
    {
        ++lock_count;
    }

    void pixel_buffer::unlock()
    {
        if (lock_count > 0) {
            --lock_count;
            return;
        }

        throw std::runtime_error("pixel_buffer already unlocked");
    }

    bool pixel_buffer::is_locked()
    {
        if (lock_count == 0) {
            return false;
        }
        return true;
    }

    void pixel_buffer::get_rgba(oep_image_ready_cb callback)
    {
        if (!is_locked()) {
            std::cout << "[WARNING] The pixel buffer must be locked" << std::endl;
            callback(std::nullopt);
        }

        if (auto oep_sp = m_oep_ptr.lock()) {
            auto convert_callback = [this, callback](data_t data) {
#if C_API
                /* bnb::image_format frm(m_width, m_height, m_orientation, false, 0, std::nullopt);
                auto bpc8 = bpc8_image_t(color_plane_weak(data.data.get()), interfaces::pixel_format::rgba, frm);
                callback(nv12_image(y_plane, m_width, uv_plane, m_width, frm)); */
                // TODO for c-api
#elif CPP_API
                bnb::image_format frm(m_width, m_height, m_orientation, false, 0, std::nullopt);
                auto bpc8 = bpc8_image_t(color_plane_weak(data.data.get()), interfaces::pixel_format::rgba, frm);
                callback(full_image_t(std::move(bpc8)));
#endif /* CPP_API */
            };

            oep_sp->read_current_buffer(convert_callback);
        } else {
            std::cout << "[ERROR] Offscreen effect player destroyed" << std::endl;
        }
    }

    void pixel_buffer::get_nv12(oep_image_ready_cb callback)
    {
        if (!is_locked()) {
            std::cout << "[WARNING] The pixel buffer must be locked" << std::endl;
            callback(std::nullopt);
        }

        if (auto oep_sp = m_oep_ptr.lock()) {
            auto convert_callback = [this, callback](data_t data) {
#if C_API
                color_plane y_plane = color_plane_alloc(m_width * m_height);
                color_plane uv_plane = color_plane_alloc((m_width / 2 * m_height / 2) * 2);

                bnb_image_format_t frm{m_width, m_height, m_orientation, false, 0};

                libyuv::ABGRToNV12(data.data.get(),
                    m_width * 4,
                    y_plane.get(),
                    m_width,
                    uv_plane.get(),
                    m_width,
                    m_width,
                    m_height);

                callback(nv12_image(y_plane, m_width, uv_plane, m_width, frm));
#elif CPP_API
                std::vector<uint8_t> y_plane(m_width * m_height);
                std::vector<uint8_t> uv_plane((m_width / 2 * m_height / 2) * 2);

                bnb::image_format frm(m_width, m_height, m_orientation, false, 0, std::nullopt);

                libyuv::ABGRToNV12(data.data.get(),
                    m_width * 4,
                    y_plane.data(),
                    m_width,
                    uv_plane.data(),
                    m_width,
                    m_width,
                    m_height);

                callback(full_image_t(yuv_image_t(color_plane_vector(y_plane), color_plane_vector(uv_plane), frm)));
#endif /* CPP_API */
            };

            oep_sp->read_current_buffer(convert_callback);
        }
        else {
            std::cout << "[ERROR] Offscreen effect player destroyed" << std::endl;
        }
    }

    void pixel_buffer::get_texture(oep_texture_cb callback)
    {
        if (!is_locked()) {
            std::cout << "[WARNING] The pixel buffer must be locked" << std::endl;
            callback(std::nullopt);
        }
        if (auto oep_sp = m_oep_ptr.lock()) {
            oep_sp->get_current_buffer_texture(callback);
        }
        else {
            std::cout << "[ERROR] Offscreen effect player destroyed" << std::endl;
        }
    }

} // bnb
