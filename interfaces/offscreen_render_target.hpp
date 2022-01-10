#pragma once

#include <interfaces/image_format.hpp>
#include <interfaces/pixel_buffer.hpp>
#include <interfaces/render_context.hpp>

namespace bnb::oep::interfaces
{
    class offscreen_render_target;
}

using rendered_texture_t = void*;
using offscreen_render_target_sptr = std::shared_ptr<bnb::oep::interfaces::offscreen_render_target>;

namespace bnb::oep::interfaces
{

    class offscreen_render_target
    {
    public:
        /**
         * Create the offscreen render target.
         *
         * @param rc - shared pointer to rendering context
         *
         * @return - shared pointer to the offscreen render target
         *
         * @example bnb::oep::interfaces::offscreen_render_target::create(my_rc)
         */
        static offscreen_render_target_sptr create(render_context_sptr rc);

        virtual ~offscreen_render_target() = default;

        /**
         * Offscreen Render Target initialization. Includes initialization of gl context's
         * buffers and support objects.
         * Called by offscreen effect player.
         *
         * @param width Initial width of the offscreen render target
         * @param width Initial height of the offscreen render target
         *
         * @example init(1280, 720)
         */
        virtual void init(int32_t width, int32_t height) = 0;

        /**
         * Offscreen Render Target deinitialization. Should be called within the same thread as init();
         * Called by offscreen effect player.
         *
         * @example deinit()
         */
        virtual void deinit() = 0;

        /**
         * Notify about rendering surface being resized.
         * Called by offscreen effect player.
         *
         * @param width New width for the rendering surface
         * @param height New height for the rendering surface
         *
         * @example surface_changed(1280, 720)
         */
        virtual void surface_changed(int32_t width, int32_t height) = 0;

        /**
         * Activate context for current thread
         *
         * @example activate_context()
         */
        virtual void activate_context() = 0;

        /**
         * Deactivate context in the corresponding thread
         * In the certain cases (GLFW on Windows) when it is intended to make OGL resource sharing
         * it is required that sharing context is not active in any thread.
         *
         * @example deactivate_context()
         */
        virtual void deactivate_context() = 0;

        /**
         * Preparing texture for effect_player
         * Called by offscreen effect player.
         *
         * @example prepare_rendering()
         */
        virtual void prepare_rendering() = 0;

        /**
         * Orientates the image
         * Called by offscreen effect player.
         *
         * @param orient output image orientation
         *
         * @example orient_image(rotation::deg180)
         */
        virtual void orient_image(rotation orient) = 0;

        /**
         * Reading current buffer of active texture.
         * The implementation must definitely support for reading format image_format::bpc8_rgba
         * Called by image_processing_result
         *
         * @param format requested output image format
         *
         * @return pixel_buffer_sptr - the smart pointer to the instance of the pixel_buffer interface,
         * providing access to the output image byte data, or nullptr if the specified format is not supported.
         * Must have support for format image_format::bpc8_rgba
         *
         * @example read_current_buffer(image_format::bpc8_rgba)
         */
        virtual pixel_buffer_sptr read_current_buffer(image_format format) = 0;

        /**
         * Get texture id used for rendering of frame
         * Called by offscreen effect player.
         *
         * @return texture id
         *
         * @example get_current_buffer_texture()
         */
        virtual rendered_texture_t get_current_buffer_texture() = 0;
    }; /* class offscreen_render_target         INTERFACE */

} /* namespace bnb::oep::interfaces */
