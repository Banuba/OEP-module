#pragma once

#if !defined(INCLUDE_OEP_C_API_INLINE_MACRO)
#error "Do not include this file. Use 'oep_api.hpp' to include the API dependent file."
#endif /* !defined(INCLUDE_OEP_C_API_INLINE_MACRO) */

#include <iostream>

namespace bnb::api
{

/* oep_api::oep_api     CONSTRUCTOR */
inline oep_api::oep_api(const std::vector<std::string>& path_to_resources, const std::string& client_token)
{
    std::unique_ptr<const char* []> res_paths = std::make_unique<const char* []>(path_to_resources.size() + 1);
    std::transform(path_to_resources.begin(), path_to_resources.end(), res_paths.get(), [](const auto& s) { return s.c_str(); });
    res_paths.get()[path_to_resources.size()] = nullptr;
    m_utility = bnb_utility_manager_init(res_paths.get(), client_token.c_str(), nullptr);
}

/* oep_api::~oep_api    DESTRUCTOR */
inline oep_api::~oep_api()
{
    bnb_effect_player_destroy(m_ep, nullptr);
    bnb_utility_manager_release(m_utility, nullptr);
}

/* oep_api::pause */
inline void oep_api::pause()
{
    bnb_effect_player_playback_pause(m_ep, nullptr);
}

/* oep_api::resume */
inline void oep_api::resume()
{
    bnb_effect_player_playback_play(m_ep, nullptr);
}

/* oep_api::enable_audio */
inline void oep_api::enable_audio(bool enable)
{
    bnb_effect_player_enable_audio(m_ep, enable, nullptr);
}

/* oep_api::init */
inline void oep_api::init(int32_t width, int32_t height, bool manual_audio)
{
    bnb_effect_player_configuration_t ep_cfg{width, height, bnb_nn_mode_automatically, bnb_good, false, manual_audio};
    m_ep = bnb_effect_player_create(&ep_cfg, nullptr);
    if (m_ep == nullptr) {
        throw std::runtime_error("Failed to create effect player holder.");
    }
}

/* oep_api::surface_created */
inline void oep_api::surface_created(int32_t width, int32_t height)
{
    bnb_effect_player_surface_created(m_ep, width, height, nullptr);
}

/* oep_api::surface_changed */
inline void oep_api::surface_changed(int32_t width, int32_t height)
{
    bnb_effect_player_surface_changed(m_ep, width, height, nullptr);
    effect_manager_holder_t* em = bnb_effect_player_get_effect_manager(m_ep, nullptr);
    bnb_effect_manager_set_effect_size(em, width, height, nullptr);
}

/* oep_api::surface_destroyed */
inline void oep_api::surface_destroyed()
{
    bnb_effect_player_surface_destroyed(m_ep, nullptr);
}

#define CHECK_ERROR(error)                                  \
    do {                                                    \
        if (error) {                                        \
            std::string msg = bnb_error_get_message(error); \
            bnb_error_destroy(error);                       \
            throw std::runtime_error(msg);                  \
        }                                                   \
    } while (false);


/* oep_api::draw_image */
inline void oep_api::draw_image(std::shared_ptr<bnb_full_image_alias> image)
{
    bnb_error* error{nullptr};
    if (!image) {
        throw std::runtime_error("no image was created");
    }
    bnb_effect_player_push_frame(m_ep, image.get()->get(), &error);
    CHECK_ERROR(error);

    while (bnb_effect_player_draw(m_ep, &error) < 0) {
        std::this_thread::yield();
    }
    CHECK_ERROR(error);
}

/* oep_api::load_effect */
inline bool oep_api::load_effect(const std::string& effect)
{
    if (auto e_manager = bnb_effect_player_get_effect_manager(m_ep, nullptr)) {
        bnb_effect_manager_load_effect(e_manager, effect.c_str(), nullptr);
        std::cout << "[Error] effect manager not initialized" << std::endl;
    } else {
        return true;
    }
    return true;
}

/* oep_api::call_js_method */
inline bool oep_api::call_js_method(const std::string& method, const std::string& param)
{
    if (auto e_manager = bnb_effect_player_get_effect_manager(m_ep, nullptr)) {
        if (auto effect = bnb_effect_manager_get_current_effect(e_manager, nullptr)) {
            bnb_effect_call_js_method(effect, method.c_str(), param.c_str(), nullptr);
        } else {
            std::cout << "[Error] effect not loaded" << std::endl;
            return false;
        }
    } else {
        std::cout << "[Error] effect manager not initialized" << std::endl;
        return false;
    }
    return true;
}

} /* bnb::api */
