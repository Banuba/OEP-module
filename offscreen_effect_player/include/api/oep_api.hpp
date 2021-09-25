#pragma once

#include <interfaces/api.hpp>

namespace bnb::api
{

class oep_api
{
protected:
                oep_api(const std::vector<std::string>& path_to_resources, const std::string& client_token);
                ~oep_api();

    void        pause();
    void        resume();
    void        enable_audio(bool enable);
    
    void        init(int32_t width, int32_t height, bool manual_audio);
    void        deinit();

    void        surface_created(int32_t width, int32_t height);
    void        surface_changed(int32_t width, int32_t height);
    void        surface_destroyed();

    void        draw_image(std::shared_ptr<bnb_full_image_alias> image);

    bool        load_effect(const std::string& effect);

    bool        call_js_method(const std::string& method, const std::string& param);

private:
    bnb_utility_manager_alias   m_utility;
    bnb_effect_player_alias     m_ep;
}; /* class oep_api */

} /* bnb::api */

#if C_API
    #define INCLUDE_OEP_C_API_INLINE_MACRO
    #include "oep_c_api_inline.hpp"
    #undef INCLUDE_OEP_C_API_INLINE_MACRO
#elif CPP_API
    #define INCLUDE_OEP_CPP_API_INLINE_MACRO
    #include "oep_cpp_api_inline.hpp"
    #undef INCLUDE_OEP_CPP_API_INLINE_MACRO
#endif /* CPP_API */