#pragma once

#include <interfaces/image_processing_result.hpp>

namespace bnb::oep
{

    class image_processing_result : public bnb::oep::interfaces::image_processing_result
    {
    public:
        image_processing_result(offscreen_render_target_sptr ort);

        ~image_processing_result();

        void lock() override;

        void unlock() override;

        bool is_locked() override;

        void get_image(bnb::oep::interfaces::image_format format, oep_pixel_buffer_ready_cb callback) override;

        void get_texture(oep_texture_ready_cb callback) override;

    private:
        pixel_buffer_sptr convert_image_to_bpc8(pixel_buffer_sptr image, bnb::oep::interfaces::image_format bpc8_format);
        pixel_buffer_sptr convert_image_to_nv12(pixel_buffer_sptr image, bnb::oep::interfaces::image_format nv12_format);
        pixel_buffer_sptr convert_image_to_i420(pixel_buffer_sptr image, bnb::oep::interfaces::image_format i420_format);
        const char* image_format_to_cstr(bnb::oep::interfaces::image_format format);

    private:
        offscreen_render_target_sptr m_ort{nullptr};
        int32_t m_lock_count{0};
    }; /* class image_processing_result */

} /* namespace bnb::oep */
