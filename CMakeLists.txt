SET (OEP_SUBMODULE_DIR      ${CMAKE_CURRENT_LIST_DIR})

include(${CMAKE_CURRENT_LIST_DIR}/cmake/utils.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/cmake/copy_libs.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/cmake/libyuv.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/cmake/utils.cmake)

add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/libraries)
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/offscreen_effect_player)
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/offscreen_render_target)
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/preprocessor)
