#include "pixel_buffer.hpp"

#include <iostream>
#include <libyuv.h>

namespace bnb {
    pixel_buffer::pixel_buffer(oep_sptr oep_sptr, uint32_t width, uint32_t height,
                               bnb_image_orientation_t orientation)
            : m_oep_ptr(oep_sptr), m_width(width), m_height(height), m_orientation(orientation) {}

    void pixel_buffer::lock() {
        ++lock_count;
    }

    void pixel_buffer::unlock() {
        if (lock_count > 0) {
            --lock_count;
            return;
        }

        throw std::runtime_error("pixel_buffer already unlocked");
    }

    bool pixel_buffer::is_locked() {
        if (lock_count == 0) {
            return false;
        }
        return true;
    }

    void pixel_buffer::get_rgba_async(oep_image_ready_cb_argb callback) {
        if (!is_locked()) {
            std::cout << "[WARNING] The pixel buffer must be locked" << std::endl;
            callback(std::nullopt);
        }

        if (auto oep_sp = m_oep_ptr.lock()) {
            auto convert_callback = [this, callback](data_t data) {
                color_plane rgb = color_plane_weak(data.data.get());
                bnb_image_format_t frm{m_width, m_height, m_orientation, false, 0};

                callback(rgb_image(rgb, frm, bnb_pixel_format_t::BNB_RGBA));
            };

            oep_sp->read_current_buffer_async(convert_callback);
        } else {
            std::cout << "[ERROR] Offscreen effect player destroyed" << std::endl;
        }
    }

    std::optional<rgb_image> pixel_buffer::get_rgba() {
        if (auto oep_sp = m_oep_ptr.lock()) {
            data_t data = oep_sp->read_current_buffer();
            color_plane rgb = color_plane_weak(data.data.get());
            bnb_image_format_t frm{m_width, m_height, m_orientation, false, 0};
            auto rgb_i = rgb_image(rgb, frm, bnb_pixel_format_t::BNB_RGBA);
            return rgb_i;
        } else {
            std::cout << "[ERROR] Offscreen effect player destroyed" << std::endl;
            return std::nullopt;
        }
    }

    void pixel_buffer::get_nv12(oep_image_ready_cb_nv12 callback)
    {
        if (!is_locked()) {
            std::cout << "[WARNING] The pixel buffer must be locked" << std::endl;
            callback(std::nullopt);
        }

        if (auto oep_sp = m_oep_ptr.lock()) {
            auto convert_callback = [this, callback](data_t data) {
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
            };

            oep_sp->read_current_buffer_async(convert_callback);
        }
        else {
            std::cout << "[ERROR] Offscreen effect player destroyed" << std::endl;
        }
    }

    void pixel_buffer::get_texture(oep_texture_cb callback) {
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

} // bnb
