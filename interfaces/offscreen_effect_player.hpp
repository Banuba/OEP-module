#pragma once

#include <optional>
#include <interfaces/effect_player.hpp>
#include <interfaces/offscreen_render_target.hpp>
#include <interfaces/image_processing_result.hpp>

namespace bnb::oep::interfaces
{
    class offscreen_effect_player;
}

using offscreen_effect_player_sptr = std::shared_ptr<bnb::oep::interfaces::offscreen_effect_player>;
using oep_image_process_cb = std::function<void(image_processing_result_sptr)>;
using result_bool_cb = std::function<void( bool )>;

namespace bnb::oep::interfaces
{

    class offscreen_effect_player
    {
    public:
        /**
         * Create the offscreen effect player
         *
         * @param ep - shared pointer to the effect player
         * @param ort - shared pointer to the offscreen render target
         * @param width - initial width for offscreen render target
         * @param height - initial height for offscreen render target
         *
         * @return - shared pointer to the offscreen effect player
         *
         * @example bnb::oep::interfaces::offscreen_effect_player::create(my_ep, my_ort, width, height)
         */
        static offscreen_effect_player_sptr create(effect_player_sptr ep, offscreen_render_target_sptr ort, int32_t width, int32_t height);

        virtual ~offscreen_effect_player() = default;

        /**
         * An asynchronous method for passing a frame to effect player,
         * and calling callback as a frame will be processed
         *
         * @param image the passed instance of the pixel_buffer class provides the access to the image byte data
         * @param input_rotation image orientation for effect player
         * @param require_mirroring require mirroring for effect player
         * @param callback calling when frame will be processed, containing pointer of pixel_buffer for get bytes
         * @param target_orientation image orientation for postprocessing
         *
         * @example process_image_async(my_input_image, rotation::deg0, true, [](image_processing_result_sptr sptr){}, rotation::deg180)
         * @return false if the frame is rejected because of too many items in the internal queue of frames, otherwise true
         */
        virtual bool process_image_async(pixel_buffer_sptr image, rotation input_rotation, bool require_mirroring, oep_image_process_cb callback, std::optional<rotation> target_orientation) = 0;

        /**
         * Notify about rendering surface being resized.
         * Must be called from the render thread.
         *
         * @param width New width for the rendering surface
         * @param height New height for the rendering surface
         *
         * @example surface_changed(1280, 720)
         */
        virtual void surface_changed(int32_t width, int32_t height) = 0;

        /**
         * Load and activate effect async. May be called from any thread
         *
         * @param effect_path Path to directory of effect
         *
         * @example load_effect("effects/Afro")
         */
        virtual void load_effect(const std::string& effect_path, result_bool_cb result_callback) = 0;

        /**
         * Unload effect from cache.
         *
         * @example unload_effect()
         */
        virtual void unload_effect(result_bool_cb result_callback) = 0;

        /**
         * Pause in effect player
         *
         * @example pause();
         */
        virtual void pause() = 0;

        /**
         * Resume in effect player
         *
         * @example resume()
         */
        virtual void resume() = 0;

        /**
         * Stop in effect player
         *
         * @example stop()
         */
        virtual void stop() = 0;

        /**
         * Call js method defined in config.js file of active effect
         *
         * @param method JS function name. Member functions are not supported.
         * @param param function arguments as JSON string.
         *
         * @example call_js_method("just_bg", "{ \"recordDuration\": 15, \"rotation_vector\": true }")
         */
        virtual void call_js_method(const std::string& method, const std::string& param, result_bool_cb result_callback) = 0;

        /**
         * * Evaluate the `script` in effect. This method is thread safe.
         * @param script JS string to execute
         * @param result_callback Callback for result. Must be called from the render thread
         *
         * @example eval_js("Skin.softening(1)", [](const std::string&){ DO SOMETHING })
         */
        virtual void eval_js(const std::string& script, oep_eval_js_result_cb result_callback) = 0;
    }; /* class offscreen_effect_player     INTERFACE */

} /* namespace bnb::oep::interfaces */
