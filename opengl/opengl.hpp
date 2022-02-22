#pragma once

#define BNB_GL_UNSUPPORTED (-1)

#if defined(ANDROID_PLATFORM) /* android platform */
    #include <GLES3/gl3.h>
    #define BNB_GLSL_VERSION "#version 300 es \n"
    #if !defined(GL_BGR)
        #define GL_BGR BNB_GL_UNSUPPORTED
    #endif /* !defined(GL_BGR) */
    #if !defined(GL_BGRA)
        #define GL_BGRA BNB_GL_UNSUPPORTED
    #endif  /* !defined(GL_BGRA) */
#else /* other platform */
    #include <glad/glad.h>
    #define BNB_GLSL_VERSION "#version 330 core \n"
#endif /* defined(ANDROID_PLATFORM) */

#define GL_CHECK_ERROR() ((void) 0)
#define GL_CALL(FUNC) [&]() {FUNC; GL_CHECK_ERROR(); }()
