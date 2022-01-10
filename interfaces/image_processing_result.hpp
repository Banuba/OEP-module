#pragma once

#include <optional>
#include <interfaces/image_format.hpp>
#include <interfaces/pixel_buffer.hpp>
#include <interfaces/offscreen_render_target.hpp>

namespace bnb::oep::interfaces
{
    class image_processing_result;
}

using image_processing_result_sptr = std::shared_ptr<bnb::oep::interfaces::image_processing_result>;
using oep_pixel_buffer_ready_cb = std::function<void(pixel_buffer_sptr)>;
using oep_texture_ready_cb = std::function<void(std::optional<rendered_texture_t>)>;

namespace bnb::oep::interfaces
{

    class image_processing_result
    {
    public:
        /**
         * Create the image processing result. Called in offsceen effect player.
         *
         * @param ort shared pointer to the offscreen render target
         *
         * @return shared pointer to the image processing result
         *
         * @example bnb::oep::interfaces::image_processing_result::create(my_ep, my_ort, width, height)
         */
        static image_processing_result_sptr create(offscreen_render_target_sptr ort);

        virtual ~image_processing_result() = default;

        /**
         * Lock pixel buffer. If you want to keep lock of pixel buffer
         * longer than output image callback scope you should lock pixel buffer.
         *
         * @example lock()
         */
        virtual void lock() = 0;

        /**
         * Unlock image_processing_result. Must be called if user explicitly called lock()
         * after the work to process output pixel buffer completed.
         *
         * @example unlock()
         */
        virtual void unlock() = 0;

        /**
         * Returns the locking state of image_processing_result.
         *
         * @return true if pixel_buffer locked else false
         *
         * @example is_locked()
         */
        virtual bool is_locked() = 0;

        /**
         * In tread with active texture get pixel bytes from Offscreen_render_target,
         * convert to 'format' and write data to the pixel_buffer.
         *
         * @param format specifies the output image format
         * @param callback calling with pixel_buffer_sptr
         *
         * @example get_image(image_format::bpc8_rgba, [](pixel_buffer_sptr image){})
         */
        virtual void get_image(image_format format, oep_pixel_buffer_ready_cb callback) = 0;

        /**
         * Returns the texture id of the texture used to render a frame. Can be used
         * to render with another context, if the OGL context sharing is enabled.
         *
         * @param callback which accepts texture id
         *
         * @example get_texture([](std::optional<void*> texture_id){})
         */
        virtual void get_texture(oep_texture_ready_cb callback) = 0;
    }; /* class image_processing_result   INTERFACE */

} /* namespace bnb::oep::interfaces */
