set(include_dirs
    ${CMAKE_CURRENT_SOURCE_DIR}/
    ${OEP_SUBMODULE_DIR}/
    ${PROJECT_SOURCE_DIR}/
)

file(GLOB_RECURSE srcs
    ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/*.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/api/*.hpp
)

add_library(offscreen_rt STATIC ${srcs})

target_include_directories(offscreen_rt PUBLIC
    ${include_dirs}
)

target_include_directories(offscreen_rt SYSTEM PUBLIC ${LIBYUV_INCLUDE_DIRS})

if (APPLE)
    set(apple_includes
        ${CMAKE_CURRENT_SOURCE_DIR}/macos/include/
    )

    file(GLOB_RECURSE apple_srcs
        ${CMAKE_CURRENT_SOURCE_DIR}/macos/src/*.mm
    )

    target_sources(offscreen_rt PUBLIC ${apple_srcs})
    target_include_directories(offscreen_rt PUBLIC ${apple_includes})
endif()

if(DESKTOP_BUILD)
    target_link_libraries(offscreen_rt
        glad
        glfw
        ogl_utils
        yuv
    )
elseif(ANDROID_BUILD)
    target_link_libraries(offscreen_rt
        GLESv3
        EGL
        ogl_utils
        banuba
    )
endif()
