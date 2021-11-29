#pragma once

#if !defined(INCLUDE_ORT_ANDROID_INLINE_MACRO)
#error "Do not include this file. Use 'ort_api.hpp' to include the API dependent file."
#endif /* !defined(INCLUDE_ORT_ANDROID_INLINE_MACRO) */

namespace bnb::api
{

/* ort_api::load_functions */
inline void ort_api::load_functions()
{
}

/* ort_api::create_context */
inline void ort_api::create_context(uint32_t width, uint32_t height)
{
    const EGLint attribs[] = {
            EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
            EGL_BLUE_SIZE, 8,
            EGL_GREEN_SIZE, 8,
            EGL_RED_SIZE, 8,
            EGL_NONE
    };
    EGLint contextAttribs[] = {
            EGL_CONTEXT_CLIENT_VERSION, 3,
            EGL_NONE
    };
    EGLDisplay display;
    EGLConfig config;
    EGLint numConfigs;
    EGLint format;
    EGLSurface surface;
    EGLContext context;
    display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    eglInitialize(display, 0, 0);
    eglChooseConfig(display, attribs, &config, 1, &numConfigs);
    eglGetConfigAttrib(display, config, EGL_NATIVE_VISUAL_ID, &format);
    surface = eglCreatePbufferSurface(display, config, 0);
    context = eglCreateContext(display, config, 0, contextAttribs);
    m_display = display;
    m_surface = surface;
    m_context = context;
}

/* ort_api::activate_context */
inline void ort_api::activate_context()
{
    EGLint error = 0;
    if (!eglMakeCurrent(m_display, m_surface, m_surface, m_context)) {
        error = eglGetError();
    }
}

/* ort_api::deactivate_context */
inline void ort_api::deactivate_context()
{
    eglMakeCurrent(m_display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
}

/* ort_api::set_window_size */
inline void ort_api::set_window_size(int32_t width, int32_t height)
{
}

/* ort_api::get_context_raw_ptr */
inline void* ort_api::get_context_raw_ptr()
{
    return reinterpret_cast<void*>(&m_context);
}

} /* namespace bnb::api */
