#pragma once

#include <interfaces/api.hpp>
#include <interfaces/offscreen_render_target.hpp>
#include "opengl.hpp"

#if defined(DESKTOP_BUILD_PART)
    /* only for desktop */
    #include <glad/glad.h>
    #define GLFW_INCLUDE_NONE
    #include <GLFW/glfw3.h>

    class GLFWwindow;
    struct DestroyglfwWin{
        void operator()(GLFWwindow* ptr){
            glfwDestroyWindow(ptr);
        }
    };
    using smart_GLFWwindow = std::unique_ptr<GLFWwindow, DestroyglfwWin>;
    typedef smart_GLFWwindow            gl_context_alias;

#elif defined(ANDROID_BUILD_PART)
    /* only for android */
    #include <GLES3/gl3.h>
    #include <EGL/egl.h>

    typedef EGLContext                  gl_context_alias;
#endif /* defined(ANDROID_BUILD_PART) */



/* class bnb::api::ort_api */
namespace bnb::api
{

class ort_api
{
protected:
    void        load_functions();
    void        create_context(uint32_t width, uint32_t height);
    void        activate_context();
    void        deactivate_context();
    void        set_window_size(int32_t width, int32_t height);
    void*       get_context_raw_ptr();
    
private:
    #if defined(ANDROID_BUILD_PART)
        /* only for android */
        EGLDisplay          m_display;
        EGLSurface          m_surface;
    #endif /* defined(ANDROID_BUILD_PART) */
    gl_context_alias        m_context;
}; /* class ort_api */

} /* bnb::api */


#if defined(DESKTOP_BUILD_PART)
    #define INCLUDE_ORT_DESKTOP_INLINE_MACRO
    #include "ort_desktop_inline.hpp"
    #undef INCLUDE_ORT_DESKTOP_INLINE_MACRO
#elif defined(ANDROID_BUILD_PART)
    #define INCLUDE_ORT_ANDROID_INLINE_MACRO
    #include "ort_android_inline.hpp"
    #undef INCLUDE_ORT_ANDROID_INLINE_MACRO
#endif /* defined(ANDROID_BUILD_PART) */
