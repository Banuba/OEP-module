#pragma once

#include <preprocessor/build_macros.hpp>
#if C_API
    #include <bnb/effect_player.h>
    #include <bnb/utility_manager.h>
#elif CPP_API
    #include <bnb/effect_player/interfaces/all.hpp>
    #include <bnb/effect_player/utility.hpp>
#endif /* CPP_API */

namespace bnb::api
{

class oep_api
{
public:
                oep_api(const std::vector<std::string>& path_to_resources, const std::string& client_token);

protected:
    void        pause();
    void        resume();
    void        enable_audio(bool enable);
    
    void        init(int32_t width, int32_t height, bool manual_audio);
    void        deinit();

    void        surface_created(int32_t width, int32_t height);
    void        surface_changed(int32_t width, int32_t height);
    void        surface_destroyed();

    void        draw_image(std::shared_ptr<image_type_alias> image);

    bool        load_effect(const std::string& effect);

    bool        call_js_method(const std::string& method, const std::string& param);

private:
    IF_C_API(utility_manager_holder_t*)     IF_CPP_API(bnb::utility)                                m_utility;
    IF_C_API(effect_player_holder_t*)       IF_CPP_API(std::shared_ptr<interfaces::effect_player>)  m_ep;
};

} /* bnb::api */

#if C_API
    #define INCLUDE_OEP_C_API_MACRO
    #include "oep_c_api_inline.hpp"
    #undef INCLUDE_OEP_C_API_MACRO
#elif CPP_API
    #define INCLUDE_OEP_CPP_API_MACRO
    #include "oep_cpp_api_inline.hpp"
    #undef INCLUDE_OEP_CPP_API_MACRO
#endif /* CPP_API */