set(include_dirs
    ${CMAKE_CURRENT_SOURCE_DIR}/
    ${OEP_SUBMODULE_DIR}/
    ${PROJECT_SOURCE_DIR}/
)

file(GLOB_RECURSE srcs
    ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/*.hpp
)

add_library(offscreen_ep STATIC ${srcs})

target_include_directories(offscreen_ep PUBLIC
    ${include_dirs}
)

target_include_directories(offscreen_ep SYSTEM PUBLIC ${LIBYUV_INCLUDE_DIRS})


if(DESKTOP_BUILD)
    target_link_libraries(offscreen_ep
        bnb_effect_player
        offscreen_rt
        utils
        yuv
    )
elseif(ANDROID_BUILD)
    target_link_libraries(offscreen_ep
        offscreen_rt
        utils
        yuv
        banuba
    )
endif()
