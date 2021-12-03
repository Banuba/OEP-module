#pragma once

#include <interfaces/api.hpp>
#include <optional>

using oep_image_ready_cb = std::function<void(std::optional<bnb_full_image_alias> image)>;
using oep_texture_cb = std::function<void(std::optional<uint32_t> texture_id)>;

namespace bnb::interfaces
{

    class pixel_buffer
    {
    public:
        virtual ~pixel_buffer() = default;

        /**
         * Lock pixel buffer. If you want to keep lock of pixel buffer
         * longer than output image callback scope you should lock pixel buffer.
         *
         * Example lock()
         */
        virtual void lock() = 0;

        /**
         * Unlock pixel_buffer. Must be called if user explicitly called lock()
         * after the work to process output pixel buffer completed.
         *
         * Example unlock()
         */
        virtual void unlock() = 0;

        /**
         * Returns the locking state of pixel_buffer.
         *
         * @return true if pixel_buffer locked else false
         *
         * Example is_locked()
         */
        virtual bool is_locked() = 0;

        /**
         * In thread with active texture get pixel bytes from Offscreen_render_target and
         * convert to bnb_full_image_alias.
         *
         * @param callback calling with bnb_full_image_alias. bnb_full_image_alias keep RGBA
         *
         * Example get_rgba([](std::optional<bnb_full_image_alias> image){})
         */
        virtual void get_rgba(oep_image_ready_cb callback) = 0;

        /**
         * Synchronous method with active texture get pixel bytes from Offscreen_render_target
         * and convert to bnb_full_image_alias.
         *
         * @return std::optional<bnb_full_image_alias> - the resulting image is stored here
         *
         * Example image = get_rgba()
         */
        virtual std::optional<bnb_full_image_alias> get_rgba() = 0;

        /**
         * In thread with active texture get pixel bytes from Offscreen_render_target and
         * convert to bnb_full_image_alias.
         *
         * @param callback calling with bnb_full_image_alias. bnb_full_image_alias keep NV12
         *
         * Example get_nv12([](std::optional<bnb_full_image_alias> image){})
         */
        virtual void get_nv12(oep_image_ready_cb callback) = 0;

        /**
         * Returns texture id of texture used to render frame. Can be used to render with
         * another context if context sharing enabled.
         *
         * @param a callback which accepts texture id
         *
         * Example get_texture([](std::optional<int> testure_id){})
         */
        virtual void get_texture(oep_texture_cb callback) = 0;
    }; /* class pixel_buffer            INTERFACE */

} // namespace bnb::interfaces

using ipb_sptr = std::shared_ptr<bnb::interfaces::pixel_buffer>;
