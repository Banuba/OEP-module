cmake_minimum_required(VERSION 3.9)

project(offscreen_effect_player-desktop-c-api)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(${CMAKE_CURRENT_LIST_DIR}/cmake/utils.cmake)

if(WIN32)
    set_variable_if_undefined(DESKTOP_BUILD ON)
elseif(APPLE AND NOT IOS)
    set_variable_if_undefined(NATIVE_MACOS_BUILD ON)
    if(NOT NATIVE_MACOS_BUILD)
        set_variable_if_undefined(DESKTOP_BUILD ON)
    endif()
elseif(ANDROID)
    set_variable_if_undefined(ANDROID_BUILD ON)
endif()

include(${CMAKE_CURRENT_LIST_DIR}/cmake/libyuv.cmake)

add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/libraries)
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/offscreen_effect_player)
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/offscreen_render_target)
