#pragma once

#include <interfaces/offscreen_effect_player.hpp>
#include <interfaces/offscreen_render_target.hpp>
#include <interfaces/pixel_buffer.hpp>
#include "thread_pool.h"

namespace bnb::oep
{

    class offscreen_effect_player : public interfaces::offscreen_effect_player
    {
    public:
        offscreen_effect_player(effect_player_sptr ep, offscreen_render_target_sptr ort, int32_t width, int32_t height);

        ~offscreen_effect_player();

        void process_image_async(pixel_buffer_sptr image, bnb::oep::interfaces::rotation input_rotation, oep_image_process_cb callback, std::optional<bnb::oep::interfaces::rotation> target_orientation) override;

        void surface_changed(int32_t width, int32_t height) override;

        void load_effect(const std::string& effect_path) override;

        void unload_effect() override;

        void pause() override;

        void resume() override;

        void call_js_method(const std::string& method, const std::string& param) override;

    private:
        effect_player_sptr m_ep;
        offscreen_render_target_sptr m_ort;
        thread_pool m_scheduler;
        std::thread::id render_thread_id;
        image_processing_result_sptr m_current_frame;
        std::atomic<uint16_t> m_incoming_frame_queue_task_count = 0;
    }; /* class offscreen_effect_player */

} /* namespace bnb::oep */
