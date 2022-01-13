#include "image_processing_result.hpp"

#include <iostream>
#include <libyuv.h>
#include <vector>

namespace bnb::oep
{

    /* interfaces::image_process::create */
    image_processing_result_sptr bnb::oep::interfaces::image_processing_result::create(offscreen_render_target_sptr ort)
    {
        return image_processing_result_sptr(new bnb::oep::image_processing_result(ort));
    }

    /* image_processing_result::image_processing_result */
    image_processing_result::image_processing_result(offscreen_render_target_sptr ort)
        : m_ort(ort)
    {
    }

    /* image_processing_result::~image_processing_result */
    image_processing_result::~image_processing_result()
    {
    }

    /* image_processing_result::lock */
    void image_processing_result::lock()
    {
        m_lock_count++;
    }

    /* image_processing_result::unlock */
    void image_processing_result::unlock()
    {
        if (m_lock_count > 0) {
            m_lock_count--;
            return;
        }
        throw std::runtime_error("image_processing_result already unlocked");
    }

    /* image_processing_result::is_locked */
    bool image_processing_result::is_locked()
    {
        return m_lock_count != 0;
    }

    /* image_processing_result::get_image */
    void image_processing_result::get_image(bnb::oep::interfaces::image_format format, oep_pixel_buffer_ready_cb callback)
    {
        if (!is_locked()) {
            std::cout << "[WARNING] The 'image processing result' must be locked" << std::endl;
            callback(nullptr);
            return;
        }

        using ns = bnb::oep::interfaces::image_format;

        /* Since offscreen_render_target may not be able to read some implementations of
        formats, we first read the format that is needed */
        /* In the current implementation of the offscreen_render_target - has hardware support for
        converting to i420. It's better because works faster. */
        pixel_buffer_sptr image = m_ort->read_current_buffer(format);

        /* If image != nullptr then we got the image with needed image_format and returns it */
        if (image != nullptr) {
            callback(image);
            return;
        }

        /* Code below assumes software conversion to needed format of the image */

        /* In the current implementation offscreen_render_target provide convertation to i420,
        but not nv12. i420 and nv12 have similar conversion alhorithms, differs only in
        the method of writing the pixel bytes. Code below converts from i420 to nv12 */
        switch (format) {
            case ns::nv12_bt601_full:
                image = m_ort->read_current_buffer(ns::i420_bt601_full);
                break;
            case ns::nv12_bt601_video:
                image = m_ort->read_current_buffer(ns::i420_bt601_video);
                break;
            case ns::nv12_bt709_full:
                image = m_ort->read_current_buffer(ns::i420_bt709_full);
                break;
            case ns::nv12_bt709_video:
                image = m_ort->read_current_buffer(ns::i420_bt709_video);
                break;
            default:
                break;
        }

        if (image != nullptr) {
            switch (format) {
                case ns::nv12_bt601_full:
                case ns::nv12_bt601_video:
                case ns::nv12_bt709_full:
                case ns::nv12_bt709_video:
                    callback(convert_image_to_nv12(image, format));
                    return;
                default:
                    break;
            }
        }

        std::cout << "[WARNING] Conversion to '" << image_format_to_cstr(format) << "' format is not implemented." << std::endl;
        callback(nullptr);
    }

    /* image_processing_result::get_texture */
    void image_processing_result::get_texture(oep_texture_ready_cb callback)
    {
        callback(m_ort->get_current_buffer_texture());
    }

    /* image_processing_result::convert_image_to_nv12 */
    pixel_buffer_sptr image_processing_result::convert_image_to_nv12(pixel_buffer_sptr image, bnb::oep::interfaces::image_format nv12_format)
    {
        /* The code below converts from i420 to nv12. If nv12_format is not format i420 then this code
        will be 'undefined behaviour' */
        using ns_pb = bnb::oep::interfaces::pixel_buffer;
        int32_t stride = image->get_width();
        size_t size = stride * image->get_height() + stride * image->get_height() / 2;
        ns_pb::plane_sptr y_plane_data(new uint8_t[size]);
        size_t y_plane_size = stride * image->get_height();
        ns_pb::plane_sptr uv_plane_data(y_plane_data.get() + y_plane_size, [](uint8_t*) {});
        size_t uv_plane_size = stride * image->get_height() / 2;

        ns_pb::plane_data y_plane{y_plane_data, y_plane_size, stride};
        ns_pb::plane_data uv_plane{uv_plane_data, uv_plane_size, stride};
        std::vector<ns_pb::plane_data> planes{y_plane, uv_plane};

        libyuv::I420ToNV12(
            image->get_base_sptr_of_plane(0).get(),
            image->get_bytes_per_row_of_plane(0),
            image->get_base_sptr_of_plane(1).get(),
            image->get_bytes_per_row_of_plane(1),
            image->get_base_sptr_of_plane(2).get(),
            image->get_bytes_per_row_of_plane(2),
            y_plane_data.get(),
            stride,
            uv_plane_data.get(),
            stride,
            image->get_width(),
            image->get_height());

        return bnb::oep::interfaces::pixel_buffer::create(planes, nv12_format, image->get_width(), image->get_height());
    }

    /* image_processing_result::image_format_to_cstr */
    const char* image_processing_result::image_format_to_cstr(bnb::oep::interfaces::image_format format)
    {
        using ns = bnb::oep::interfaces::image_format;
        switch (format) {
            case ns::bpc8_rgb:
                return "bpc8_rgb";
            case ns::bpc8_bgr:
                return "bpc8_bgr";
            case ns::bpc8_rgba:
                return "bpc8_rgba";
            case ns::bpc8_bgra:
                return "bpc8_bgra";
            case ns::bpc8_argb:
                return "bpc8_argb";
            case ns::nv12_bt601_full:
                return "nv12_bt601_full";
            case ns::nv12_bt601_video:
                return "nv12_bt601_video";
            case ns::nv12_bt709_full:
                return "nv12_bt709_full";
            case ns::nv12_bt709_video:
                return "nv12_bt709_video";
            case ns::i420_bt601_full:
                return "i420_bt601_full";
            case ns::i420_bt601_video:
                return "i420_bt601_video";
            case ns::i420_bt709_full:
                return "i420_bt709_full";
            case ns::i420_bt709_video:
                return "i420_bt709_video";
            default:
                return "UNKNOWN IMAGE FORMAT";
        }
    }

} /* namespace bnb::oep */
