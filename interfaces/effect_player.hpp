#pragma once

#include <vector>
#include <interfaces/render_context.hpp>
#include <interfaces/pixel_buffer.hpp>

namespace bnb::oep::interfaces
{
    class effect_player;
}

using effect_player_sptr = std::shared_ptr<bnb::oep::interfaces::effect_player>;

namespace bnb::oep::interfaces
{

    class effect_player
    {
    public:
        /**
         * Create the effect player
         *
         * @param path_to_resources the path where the effect player will find for resources
         * @param client_token - banuba client token
         *
         * @return shared pointer to the effect player
         *
         * @example bnb::oep::interfaces::effect_player::create(path_to_resources, klient_token)
         */
        static effect_player_sptr create(const std::vector<std::string>& path_to_resources, const std::string& client_token);

        virtual ~effect_player() = default;

        /**
         * Notify about rendering surface created. Called from offscreen effect player
         * Must be called from the render thread
         *
         * @param width Initial width for the rendering surface
         * @param height Initial height for the rendering surface
         *
         * @example surface_created(1280, 720)
         */
        virtual void surface_created(int32_t width, int32_t height) = 0;

        /**
         * Notify about rendering surface being resized. Called from offscreen effect player
         * Must be called from the render thread
         *
         * @param width New width for the rendering surface
         * @param height New height for the rendering surface
         *
         * @example surface_changed(1280, 720)
         */
        virtual void surface_changed(int32_t width, int32_t height) = 0;

        /**
         * Notify about rendering surface destroying. Called from offscreen effect player
         * Must be called from the render thread
         *
         * @example surface_destroyed()
         */
        virtual void surface_destroyed() = 0;

        /**
         * Load and activate effect async. Called from offscreen effect player
         * Must be called from the render thread
         *
         * @param effect Path to directory of effect
         *
         * @example load_effect("effects/Afro")
         */
        virtual bool load_effect(const std::string& effect) = 0;

        /**
         * Call js method defined in config.js file of active effect. Called from offscreen effect player
         * Must be called from the render thread
         *
         * @param method JS function name. Member functions are not supported.
         * @param param function arguments as JSON string.
         *
         * @example call_js_method("just_bg", "{ 'recordDuration': 15, 'rotation_vector': true }")
         */
        virtual bool call_js_method(const std::string& method, const std::string& param) = 0;

        /**
         * Pause playing. Called from offscreen effect player
         * Must be called from the render thread
         *
         * @example pause();
         */
        virtual void pause() = 0;

        /**
         * Resume playing. Called from offscreen effect player
         * Must be called from the render thread
         *
         * @example resume();
         */
        virtual void resume() = 0;

        /**
         * Push image to the effect player before calling draw() method. Called from offscreen effect player
         * Must be called from the render thread
         *
         * @param image input image for processing by the effect player
         * @param image_orientation input image orientation
         *
         * @example push_frame(my_image, rotation::deg0);
         */
        virtual void push_frame(pixel_buffer_sptr image, rotation image_orientation) = 0;

        /**
         * Start processing the image with the effect player and draw the image into the prepared texture.
         * The image must be passed before the draw() method.
         * Must be called from the render thread
         *
         * @example draw()
         */
        virtual void draw() = 0;
    }; /* class effect_player   INTERFACE */

} /* namespace bnb::oep::interfaces */
