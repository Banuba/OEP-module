#include "offscreen_effect_player.hpp"
#include "offscreen_render_target.hpp"

#include <iostream>

namespace bnb
{

    /* interfaces::offscreen_effect_player::create */
    ioep_sptr interfaces::offscreen_effect_player::create(
        const std::vector<std::string>& path_to_resources, const std::string& client_token, int32_t width, int32_t height, bool manual_audio, iort_sptr ort)
    {
        if (ort == nullptr) {
            return nullptr;
        }

        // we use "new" instead of "make_shared" because the constructor in "offscreen_effect_player" is private
        return oep_sptr(new bnb::offscreen_effect_player(
            path_to_resources, client_token, width, height, manual_audio, ort));
    }

    /* offscreen_effect_player::offscreen_effect_player     CONSTRUCTOR */
    offscreen_effect_player::offscreen_effect_player(
        const std::vector<std::string>& path_to_resources, const std::string& client_token, int32_t width, int32_t height, bool manual_audio, iort_sptr offscreen_render_target)
        : oep_api(path_to_resources, client_token)
        , m_ort(offscreen_render_target)
        , m_scheduler(1)
    {
        oep_api::init(width, height, manual_audio);

        // MacOS GLFW requires window creation on main thread, so it is assumed that we are on main thread.
        auto task = [this, width, height]() {
            render_thread_id = std::this_thread::get_id();
            m_ort->init();
            m_ort->activate_context();

            oep_api::surface_created(width, height);
            /* Only necessary if we want share context via GLFW on Windows */
            ONLY_WIN32(m_ort->deactivate_context());
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

    /* offscreen_effect_player::~offscreen_effect_player    DESTRUCTOR */
    offscreen_effect_player::~offscreen_effect_player()
    {
        oep_api::surface_destroyed();

        // Deinitialize offscreen render target, should be performed on render thread.
        auto task = [this]() {
            m_ort->deinit();
        };
        m_scheduler.enqueue(task).get();
    }

    /* offscreen_effect_player::process_image_async */
    void offscreen_effect_player::process_image_async(
        std::shared_ptr<bnb_full_image_alias> image, oep_pb_ready_cb callback, std::optional<interfaces::orient_format> target_orient)
    {
        if (m_current_frame == nullptr) {
            m_current_frame = std::make_shared<pixel_buffer>(shared_from_this(), image->get_format().width, image->get_format().height, image->get_format().orientation);
        }

        if (m_current_frame->is_locked()) {
            ONLY_DEBUG(std::cout << "[Warning] The interface for processing the previous frame is lock" << std::endl);
            return;
        }

        if (!target_orient.has_value()) {
            /* set default orientation */
            target_orient = {image->get_format().orientation, true};
        }

        auto task = [this, image, callback, target_orient]() {
            if (m_incoming_frame_queue_task_count == 1) {
                m_current_frame->lock();
                m_ort->activate_context();
                m_ort->prepare_rendering();
                oep_api::draw_image(image);
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

    /* offscreen_effect_player::process_image_rgba */
    ipb_sptr offscreen_effect_player::process_image_rgba(
        std::shared_ptr<bnb_full_image_alias> image,
        std::optional<interfaces::orient_format> target_orient)
    {
        if (m_current_frame == nullptr) {
            m_current_frame = std::make_shared<pixel_buffer>(shared_from_this(), image->get_format().width, image->get_format().height, image->get_format().orientation);
        }

        if (!target_orient.has_value()) {
            target_orient = {image->get_format().orientation, true};
        }

        auto task = [this, image, target_orient]() {
            m_ort->activate_context();
            m_ort->prepare_rendering();
            oep_api::draw_image(image);
            m_ort->orient_image(*target_orient);
            return m_current_frame;
        };
        return m_scheduler.enqueue(task).get();
    }

    /* offscreen_effect_player::surface_changed */
    void offscreen_effect_player::surface_changed(int32_t width, int32_t height)
    {
        auto task = [this, width, height]() {
            m_ort->activate_context();
            oep_api::surface_changed(width, height);
            m_current_frame.reset();
            m_ort->surface_changed(width, height);
        };

        m_scheduler.enqueue(task);
    }

    /* offscreen_effect_player::load_effect */
    void offscreen_effect_player::load_effect(const std::string& effect_path)
    {
        auto task = [this, effect = effect_path]() {
            m_ort->activate_context();
            oep_api::load_effect(effect);
        };
        m_scheduler.enqueue(task);
    }

    /* offscreen_effect_player::call_js_method */
    void offscreen_effect_player::call_js_method(const std::string& method, const std::string& param)
    {
        auto task = [this, method = method, param = param]() {
            m_ort->activate_context();
            oep_api::call_js_method(method, param);
        };
        m_scheduler.enqueue(task);
    }

    /* offscreen_effect_player::read_current_buffer */
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

    /* offscreen_effect_player::read_current_buffer */
    bnb::data_t offscreen_effect_player::read_current_buffer()
    {
        if (std::this_thread::get_id() == render_thread_id) {
            return m_ort->read_current_buffer();
        }
        auto task = [this]() {
            return this->m_ort->read_current_buffer();
        };
        return m_scheduler.enqueue(task).get();
    }

    /* offscreen_effect_player::get_current_buffer_texture */
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

} /* namespace bnb */
