# if option "USE_BNB_OEP_PIXEL_BUFFER" is ON ->             target "bnb_oep_pixel_buffer_target" will be available
# if option "USE_BNB_OEP_IMAGE_PROCESSING_RESULT" is ON ->  target "bnb_oep_image_processing_result_target" will be available
# if option "USE_BNB_OEP_OFFSCREEN_EFFECT_PLAYER" is ON ->  target "bnb_oep_offscreen_effect_player_target" will be available
# if option "USE_BNB_OEP_OFFSCREEN_RENDER_TARGET" is ON ->  target "bnb_oep_offscreen_render_target_target" will be available
# by default all options are ON
option(USE_BNB_OEP_PIXEL_BUFFER "Use bnb pixel_buffer implementation" ON)
option(USE_BNB_OEP_IMAGE_PROCESSING_RESULT "Use bnb image_processing_result implementation" ON)
option(USE_BNB_OEP_OFFSCREEN_EFFECT_PLAYER "Use bnb offscreen_effect_player implementation" ON)
option(USE_BNB_OEP_OFFSCREEN_RENDER_TARGET "Use bnb offscreen_render_target implementation" ON)

set(OEP_SUBMODULE_DIR ${CMAKE_CURRENT_LIST_DIR})

add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/offscreen_effect_player)
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/offscreen_render_target)
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/third)

# only offscreen_render_target used OpenGL
if (USE_BNB_OEP_OFFSCREEN_RENDER_TARGET)
    add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/opengl)
endif()
