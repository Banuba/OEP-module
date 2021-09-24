#pragma once

#include <interfaces/offscreen_effect_player.hpp>
#include <interfaces/offscreen_render_target.hpp>
#include "thread_pool.h"
#include "pixel_buffer.hpp"
#include "api/oep_api.hpp"

namespace bnb
{
class offscreen_effect_player
        : public api::oep_api
        , public interfaces::offscreen_effect_player
        , public std::enable_shared_from_this<offscreen_effect_player>
{
private:

                    offscreen_effect_player(
                            const std::vector<std::string>& path_to_resources,
                            const std::string& client_token,
                            int32_t width, int32_t height, bool manual_audio, iort_sptr ort
                    );

public:
                        ~offscreen_effect_player();

    void                process_image_async(
                                std::shared_ptr<image_type_alias> image, 
                                oep_pb_ready_cb callback,
                                std::optional<interfaces::orient_format> target_orient
                        ) override;

    void                surface_changed(int32_t width, int32_t height) override;

    void                load_effect(const std::string& effect_path) override;
    void                unload_effect() override;

    void                pause() override;
    void                resume() override;
    void                enable_audio(bool enable) override;

    void                call_js_method(const std::string& method, const std::string& param) override;

private:
    friend class        interfaces::offscreen_effect_player;
    friend class        pixel_buffer;

    void                read_current_buffer(std::function<void(bnb::data_t data)> callback);
    void                get_current_buffer_texture(oep_texture_cb callback);

private:
    iort_sptr           m_ort;                  /* offscreen render target INTERFACE */
    thread_pool         m_scheduler;            /*  */
    std::thread::id     render_thread_id;
    ipb_sptr            m_current_frame;
    std::atomic<uint16_t> m_incoming_frame_queue_task_count = 0;
};



/* offscreen_effect_player::unload_effect */
inline void offscreen_effect_player::unload_effect()
{
    load_effect("");
}

/* offscreen_effect_player::pause */
inline void offscreen_effect_player::pause()
{
    oep_api::pause();
}

/* offscreen_effect_player::resume */
inline void offscreen_effect_player::resume()
{
    oep_api::resume();
}

/* offscreen_effect_player::enable_audio */
inline void offscreen_effect_player::enable_audio(bool enable)
{
    oep_api::enable_audio(enable);
}


} // bnb
