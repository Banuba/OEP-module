#pragma once

#if !defined(INCLUDE_ORT_DESKTOP_INLINE_MACRO)
#error "Do not include this file. Use 'ort_api.hpp' to include the API dependent file."
#endif /* !defined(INCLUDE_ORT_DESKTOP_INLINE_MACRO) */

#ifdef __APPLE__
    extern void run_on_main_queue(std::function<void()> f);
#endif

namespace bnb::api
{

/* ort_api::load_functions */
inline void ort_api::load_functions()
{
#if C_API_ENABLED
    bnb_error* error = nullptr;
    bnb_effect_player_load_glad_functions((void*)glfwGetProcAddress, &error);
    if (error) {
        bnb_error_destroy(error);
        throw std::runtime_error("gladLoadGLLoader error");
    }
#elif CPP_API_ENABLED
    #if BNB_OS_WINDOWS || BNB_OS_MACOS
        // it's only need for use while working with dynamic libs
        utility::load_glad_functions((GLADloadproc) glfwGetProcAddress);
    #endif
#endif

    if (0 == gladLoadGLLoader((GLADloadproc) glfwGetProcAddress)) {
        throw std::runtime_error("gladLoadGLLoader error");
    }
}

/* ort_api::create_context */
inline void ort_api::create_context(uint32_t width, uint32_t height)
{
    glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
    #ifdef __APPLE__
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    #endif

    m_context.reset();
    m_context = gl_context_alias(glfwCreateWindow(width, height, "", nullptr, nullptr));
}

/* ort_api::activate_context */
inline void ort_api::activate_context()
{
    if (m_context) {
        glfwMakeContextCurrent(m_context.get());
    }
}

/* ort_api::deactivate_context */
inline void ort_api::deactivate_context()
{
    glfwMakeContextCurrent(nullptr);
}

/* ort_api::set_window_size */
inline void ort_api::set_window_size(int32_t width, int32_t height)
{
    auto set_window_size = [this, width, height]() {
        glfwSetWindowSize(m_context.get(), width, height);
    };
    #ifdef __APPLE__
        run_on_main_queue(set_window_size);
    #else
        set_window_size();
    #endif
}

/* ort_api::get_context_raw_ptr */
inline void* ort_api::get_context_raw_ptr()
{
    return m_context.get();
}

} /* namespace bnb::api */
