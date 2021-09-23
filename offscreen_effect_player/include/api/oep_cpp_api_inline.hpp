#pragma once

#if !defined(INCLUDE_OEP_CPP_API_MACRO)
#error "Do not include this file. Use 'oep_api.hpp' to include the API dependent file."
#endif /* !defined(INCLUDE_OEP_CPP_API_MACRO) */

#include <iostream>

namespace bnb::api
{

/* oep_api::oep_api     CONSTRUCTOR */
inline oep_api::oep_api(const std::vector<std::string>& path_to_resources, const std::string& client_token)
        : m_utilit(path_to_resources, client_token)
{
}

/* oep_api::pause */
inline void oep_api::pause()
{
    m_ep->playback_pause();
}

/* oep_api::resume */
inline void oep_api::resume()
{
    m_ep->playback_play();
}

/* oep_api::enable_audio */
inline void oep_api::enable_audio(bool enable)
{
    m_ep->enable_audio(enable);
}

/* oep_api::init */
inline void oep_api::init(int32_t width, int32_t height, bool manual_audio)
{
    m_ep = std::move(bnb::interfaces::effect_player::create( {
            width, height, bnb::interfaces::nn_mode::automatically,
            bnb::interfaces::face_search_mode::good, false, manual_audio } )
    );
}

/* oep_api::deinit */
inline void oep_api::deinit()
{
}

/* oep_api::surface_created */
inline void oep_api::surface_created(int32_t width, int32_t height)
{
    m_ep->surface_created(width, height);
}

/* oep_api::surface_changed */
inline void oep_api::surface_changed(int32_t width, int32_t height)
{
    m_ep->surface_changed(width, height);
    m_ep->effect_manager()->set_effect_size(width, height);
}

/* oep_api::surface_destroyed */
inline void oep_api::surface_destroyed()
{
    m_ep->surface_destroyed();
}

/* oep_api::draw_image */
inline void oep_api::draw_image(std::shared_ptr<image_type_alias> image)
{
    m_ep->push_frame(std::move(*image));
    while (m_ep->draw() < 0) {
        std::this_thread::yield();
    }
}

/* oep_api::load_effect */
inline bool oep_api::load_effect(const std::string& effect)
{
    if (auto e_manager = m_ep->effect_manager()) {
        e_manager->load(effect);
        return true;
    }
    return false;
}

/* oep_api::call_js_method */
inline bool oep_api::call_js_method(const std::string& method, const std::string& param)
{
    if (auto e_manager = m_ep->effect_manager()) {
        if (auto effect = e_manager->current()) {
            effect->call_js_method(method, param);
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
