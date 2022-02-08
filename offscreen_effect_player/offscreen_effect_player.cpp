#include "offscreen_effect_player.hpp"

#include <iostream>

#ifdef USE_METAL
#include "../m_interfaces/offscreen_render_target.hpp"
#include "../m_interfaces/effect_player.hpp"
#endif

namespace bnb::oep
{

    /* offscreen_effect_player::create  STATIC INTERFACE */
    offscreen_effect_player_sptr bnb::oep::interfaces::offscreen_effect_player::create(effect_player_sptr ep, offscreen_render_target_sptr ort, int32_t width, int32_t height)
    {
        auto oep = std::make_shared<bnb::oep::offscreen_effect_player>(ep, ort, width, height);
#ifdef USE_METAL
        auto metal_ep = std::dynamic_pointer_cast<bnb::oep::metal_support::effect_player>(ep);
        auto metal_ort = std::dynamic_pointer_cast<bnb::oep::metal_support::offscreen_render_target>(ort);
        if (metal_ep && metal_ort){
            metal_ep->set_render_surface(metal_ort->get_layer());
        } else {
            std::cout << "[ERROR] Effect_player or offscreen_render_target are not suitable for use with METAL" << std::endl;
        }
#endif
        return oep;
    }

    /* offscreen_effect_player::offscreen_effect_player */
    offscreen_effect_player::offscreen_effect_player(effect_player_sptr ep, offscreen_render_target_sptr ort, int32_t width, int32_t height)
        : m_ep(ep)
        , m_ort(ort)
        , m_scheduler(1)
    {
        m_current_frame = bnb::oep::interfaces::image_processing_result::create(m_ort);
        // MacOS GLFW requires window creation on main thread, so it is assumed that we are on main thread.
        auto task = [this, width, height]() {
            render_thread_id = std::this_thread::get_id();
            m_ort->init(width, height);
            m_ort->activate_context();
            m_ep->surface_created(width, height);
            /* Only necessary if we want share context via GLFW on Windows */
            m_ort->deactivate_context();
        };

        auto future = m_scheduler.enqueue(task);
        try {
            // Wait result of task since initialization of glad can cause exceptions if proceed without
            future.get();
        } catch (std::runtime_error& e) {
            std::cout << "[ERROR] Failed to initialize effect player: " << e.what() << std::endl;
            std::string s = "Failed to initialize effect player.\n";
            s.append(e.what());
            throw std::runtime_error(s);
        }
    }

    /* offscreen_effect_player::~offscreen_effect_player */
    offscreen_effect_player::~offscreen_effect_player()
    {
        // Switches effect player to inactive state and deinitializes offscreen render target.
        // Must be performed on render thread.
        auto task = [this]() {
            m_ep->surface_destroyed();
            m_ort->deinit();
        };
        m_scheduler.enqueue(task).get();
    }

    /* offscreen_effect_player::process_image_async */
    void offscreen_effect_player::process_image_async(pixel_buffer_sptr image, bnb::oep::interfaces::rotation input_rotation, oep_image_process_cb callback, std::optional<bnb::oep::interfaces::rotation> target_orientation)
    {
        if (!target_orientation.has_value()) {
            /* set default orientation */
            target_orientation = bnb::oep::interfaces::rotation::deg0;
        }

        auto task = [this, image, callback, input_rotation, target_orientation]() {
            if (m_current_frame->is_locked()) {
                std::cout << "[Warning] The interface for processing the previous frame is lock" << std::endl;
            } else if (m_incoming_frame_queue_task_count == 1) {
                m_current_frame->lock();
                m_ort->activate_context();
                m_ort->prepare_rendering();
                m_ep->push_frame(image, input_rotation);
                m_ep->draw();
                m_ort->orient_image(*target_orientation);
                callback(m_current_frame);
                m_ort->deactivate_context();
                m_current_frame->unlock();
            } else {
                callback(nullptr);
            }
            --m_incoming_frame_queue_task_count;
        };

        ++m_incoming_frame_queue_task_count;
        m_scheduler.enqueue(task);
    }

    /* offscreen_effect_player::surface_changed */
    void offscreen_effect_player::surface_changed(int32_t width, int32_t height)
    {
        auto task = [this, width, height]() {
            m_ort->activate_context();
            m_ep->surface_changed(width, height);
            m_ort->surface_changed(width, height);
            m_ort->deactivate_context();
        };

        m_scheduler.enqueue(task);
    }

    /* offscreen_effect_player::load_effect */
    void offscreen_effect_player::load_effect(const std::string& effect_path)
    {
        auto task = [this, effect = effect_path]() {
            m_ort->activate_context();
            m_ep->load_effect(effect);
            m_ort->deactivate_context();
        };
        m_scheduler.enqueue(task);
    }

    /* offscreen_effect_player::unload_effect */
    void offscreen_effect_player::unload_effect()
    {
        load_effect("");
    }

    /* offscreen_effect_player::pause */
    void offscreen_effect_player::pause()
    {
        m_ep->pause();
    }

    /* offscreen_effect_player::resume */
    void offscreen_effect_player::resume()
    {
        m_ep->resume();
    }

    /* offscreen_effect_player::call_js_method */
    void offscreen_effect_player::call_js_method(const std::string& method, const std::string& param)
    {
        auto task = [this, method = method, param = param]() {
            m_ort->activate_context();
            m_ep->call_js_method(method, param);
            m_ort->deactivate_context();
        };
        m_scheduler.enqueue(task);
    }

} /* namespace bnb::oep */
