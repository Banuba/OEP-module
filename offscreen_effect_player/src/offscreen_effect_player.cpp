#include "offscreen_effect_player.hpp"
#include "offscreen_render_target.hpp"

#include <iostream>

#include <preprocessor/build_macros.hpp>
#if C_API
#define CHECK_ERROR(error)                                  \
    do {                                                    \
        if (error) {                                        \
            std::string msg = bnb_error_get_message(error); \
            bnb_error_destroy(error);                       \
            throw std::runtime_error(msg);                  \
        }                                                   \
    } while (false);
#endif /* C_API */

namespace bnb
{
    ioep_sptr interfaces::offscreen_effect_player::create(
        const std::vector<std::string>& path_to_resources, const std::string& client_token,
        int32_t width, int32_t height, bool manual_audio, iort_sptr ort)
    {
        if (ort == nullptr) {
            return nullptr;
        }

        // we use "new" instead of "make_shared" because the constructor in "offscreen_effect_player" is private
        return oep_sptr(new bnb::offscreen_effect_player(
                path_to_resources, client_token, width, height, manual_audio, ort));
    }

    offscreen_effect_player::offscreen_effect_player(
        const std::vector<std::string>& path_to_resources, const std::string& client_token,
        int32_t width, int32_t height, bool manual_audio,
        iort_sptr offscreen_render_target)

#if C_API
            : m_ort(offscreen_render_target)
            , m_scheduler(1)
#elif CPP_API
            : m_utility(path_to_resources, client_token)
            , m_ep(bnb::interfaces::effect_player::create( {
                width, height,
                bnb::interfaces::nn_mode::automatically,
                bnb::interfaces::face_search_mode::good,
                false, manual_audio }))
            , m_ort(offscreen_render_target)
            , m_scheduler(1)
#endif /* CPP_API */

    {

#if C_API
        std::unique_ptr<const char* []> res_paths = std::make_unique<const char* []>(path_to_resources.size() + 1);
        std::transform(path_to_resources.begin(), path_to_resources.end(), res_paths.get(), [](const auto& s) { return s.c_str(); });
        res_paths.get()[path_to_resources.size()] = nullptr;
        m_utility = bnb_utility_manager_init(res_paths.get(), client_token.c_str(), nullptr);

        bnb_effect_player_configuration_t ep_cfg{width, height, bnb_nn_mode_automatically, bnb_good, false, manual_audio};
        m_ep = bnb_effect_player_create(&ep_cfg, nullptr);
        if (m_ep == nullptr) {
            throw std::runtime_error("Failed to create effect player holder.");
        }
#endif /* C_API */

        // MacOS GLFW requires window creation on main thread, so it is assumed that we are on main thread.
        auto task = [this, width, height]() {
            render_thread_id = std::this_thread::get_id();
            m_ort->init();
            m_ort->activate_context();

            IF_C_API(bnb_effect_player_surface_created(m_ep, width, height, nullptr));     
            IF_CPP_API(m_ep->surface_created(width, height));
#ifdef WIN32 // Only necessary if we want share context via GLFW on Windows
            m_ort->deactivate_context();
#endif
        };

        auto future = m_scheduler.enqueue(task);
        try {
            // Wait result of task since initialization of glad can cause exceptions if proceed without
            future.get();
        }
        catch (std::runtime_error& e) {
            std::cout << "[ERROR] Failed to initialize effect player: " << e.what() << std::endl;
            throw std::runtime_error("Failed to initialize effect player.");
        }
    }

    offscreen_effect_player::~offscreen_effect_player()
    {
        IF_C_API(bnb_effect_player_surface_destroyed(m_ep, nullptr));
        IF_CPP_API(m_ep->surface_destroyed());

        // Deinitialize offscreen render target, should be performed on render thread.
        auto task = [this]() {
            m_ort->deinit();
        };
        m_scheduler.enqueue(task).get();
        
        IF_C_API(bnb_effect_player_destroy(m_ep, nullptr));
        IF_C_API(bnb_utility_manager_release(m_utility, nullptr));
        
    }

    void offscreen_effect_player::process_image_async(std::shared_ptr<image_type_alias> image, oep_pb_ready_cb callback,
                                                      std::optional<interfaces::orient_format> target_orient)
    {
        if (m_current_frame == nullptr) {
            m_current_frame = std::make_shared<pixel_buffer>(shared_from_this(),
                image->get_format().width, image->get_format().height, image->get_format().orientation);
        }

        if (m_current_frame->is_locked()) {
#ifdef DEBUG
            std::cout << "[Warning] The interface for processing the previous frame is lock" << std::endl;
#endif
            return;
        }

        if (!target_orient.has_value()) {
            target_orient = { image->get_format().orientation, true };
        }

        auto task = [this, image, callback, target_orient]() {
            if (m_incoming_frame_queue_task_count == 1) {
                m_current_frame->lock();

                m_ort->activate_context();
                m_ort->prepare_rendering();

#if C_API
                bnb_error* error = nullptr;
                full_image_holder_t* img = bnb_full_image_from_yuv_nv12_img(
                    image->get_format(),
                    image->get_y_data(), image->get_y_stride(),
                    image->get_uv_data(), image->get_uv_stride(),
                    &error);
                CHECK_ERROR(error);
                if (!img) {
                    throw std::runtime_error("no image was created");
                }
                bnb_effect_player_push_frame(m_ep, img, &error);
                CHECK_ERROR(error);
                bnb_full_image_release(img, nullptr);

                while (bnb_effect_player_draw(m_ep, &error) < 0) {
                    std::this_thread::yield();
                }
                CHECK_ERROR(error);
#elif CPP_API
                m_ep->push_frame(std::move(*image));
                while (m_ep->draw() < 0) {
                    std::this_thread::yield();
                }
#endif /* CPP_API */

                m_ort->orient_image(*target_orient);
                callback(m_current_frame);
                m_current_frame->unlock();
            } else {
                callback(std::nullopt);
            }
            --m_incoming_frame_queue_task_count;
        };

        ++m_incoming_frame_queue_task_count;
        m_scheduler.enqueue(task);
    }

    void offscreen_effect_player::surface_changed(int32_t width, int32_t height)
    {
        auto task = [this, width, height]() {
            m_ort->activate_context();

#if C_API
            bnb_effect_player_surface_changed(m_ep, width, height, nullptr);
            effect_manager_holder_t* em = bnb_effect_player_get_effect_manager(m_ep, nullptr);
            bnb_effect_manager_set_effect_size(em, width, height, nullptr);
#elif CPP_API
            m_ep->surface_changed(width, height);
            m_ep->effect_manager()->set_effect_size(width, height);
#endif /* CPP_API */

            m_current_frame.reset();
            m_ort->surface_changed(width, height);
        };

        m_scheduler.enqueue(task);
    }

    void offscreen_effect_player::load_effect(const std::string& effect_path)
    {
        auto task = [this, effect = effect_path]() {
            m_ort->activate_context();

#if C_API
            if (auto e_manager = bnb_effect_player_get_effect_manager(m_ep, nullptr)) {
                bnb_effect_manager_load_effect(e_manager, effect.c_str(), nullptr);
            } else {
                std::cout << "[Error] effect manager not initialized" << std::endl;
            }
#elif CPP_API
            if (auto e_manager = m_ep->effect_manager()) {
                e_manager->load(effect);
            } else {
                std::cout << "[Error] effect manager not initialized" << std::endl;
            }
#endif /* CPP_API */

        };

        m_scheduler.enqueue(task);
    }

    void offscreen_effect_player::unload_effect()
    {
        load_effect("");
    }

    void offscreen_effect_player::pause()
    {
        IF_C_API( bnb_effect_player_playback_pause(m_ep, nullptr) );
        IF_CPP_API( m_ep->playback_pause() );
    }

    void offscreen_effect_player::resume()
    {
        IF_C_API( bnb_effect_player_playback_play(m_ep, nullptr) );
        IF_CPP_API( m_ep->playback_play() );
    }

    void offscreen_effect_player::enable_audio(bool enable)
    {
        IF_C_API( bnb_effect_player_enable_audio(m_ep, enable, nullptr) );
        IF_CPP_API( m_ep->enable_audio(enable) );
    }

    void offscreen_effect_player::call_js_method(const std::string& method, const std::string& param)
    {
        auto task = [this, method = method, param = param]() {
            m_ort->activate_context();

#if C_API
            if (auto e_manager = bnb_effect_player_get_effect_manager(m_ep, nullptr)) {
                if (auto effect = bnb_effect_manager_get_current_effect(e_manager, nullptr)) {
                    bnb_effect_call_js_method(effect, method.c_str(), param.c_str(), nullptr);
                }
                else {
                    std::cout << "[Error] effect not loaded" << std::endl;
                }
            }
            else {
                std::cout << "[Error] effect manager not initialized" << std::endl;
            }
#elif CPP_API
            if (auto e_manager = m_ep->effect_manager()) {
                if (auto effect = e_manager->current()) {
                    effect->call_js_method(method, param);
                }
                else {
                    std::cout << "[Error] effect not loaded" << std::endl;
                }
            }
            else {
                std::cout << "[Error] effect manager not initialized" << std::endl;
            }
#endif /* CPP_API */
        };

        m_scheduler.enqueue(task);
    }

    void offscreen_effect_player::read_current_buffer(std::function<void(bnb::data_t data)> callback)
    {
        if (std::this_thread::get_id() == render_thread_id) {
            callback(m_ort->read_current_buffer());
            return;
        }

        oep_wptr this_ = shared_from_this();
        auto task = [this_, callback]() {
            if (auto this_sp = this_.lock()) {
                callback(this_sp->m_ort->read_current_buffer());
            }
        };
        m_scheduler.enqueue(task);
    }

    void offscreen_effect_player::get_current_buffer_texture(oep_texture_cb callback)
    {
        if (std::this_thread::get_id() == render_thread_id) {
            callback(m_ort->get_current_buffer_texture());
            return;
        }

        oep_wptr this_ = shared_from_this();
        auto task = [this_, callback]() {
            if (auto this_sp = this_.lock()) {
                callback(this_sp->m_ort->get_current_buffer_texture());
            }
        };
        m_scheduler.enqueue(task);
    }


} // bnb
