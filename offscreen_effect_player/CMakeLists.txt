# TARGET bnb_oep_image_processing_result_target
if (USE_BNB_OEP_IMAGE_PROCESSING_RESULT)
    # sources
    file(GLOB_RECURSE bnb_oep_image_processing_result_target_srcs
        ${CMAKE_CURRENT_SOURCE_DIR}/image_processing_result.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/image_processing_result.hpp
    )
    # new target bnb_oep_image_processing_result_target
    add_library(bnb_oep_image_processing_result_target STATIC ${bnb_oep_image_processing_result_target_srcs})
    target_include_directories(bnb_oep_image_processing_result_target PUBLIC ${OEP_SUBMODULE_DIR})
    target_link_libraries(bnb_oep_image_processing_result_target PUBLIC yuv)
endif()


# TARGET bnb_oep_pixel_buffer_target
if (USE_BNB_OEP_PIXEL_BUFFER)
    # sources
    file(GLOB_RECURSE bnb_oep_pixel_buffer_target_srcs
        ${CMAKE_CURRENT_SOURCE_DIR}/pixel_buffer.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/pixel_buffer.hpp
    )
    # new target bnb_oep_pixel_buffer_target
    add_library(bnb_oep_pixel_buffer_target STATIC ${bnb_oep_pixel_buffer_target_srcs})
    target_include_directories(bnb_oep_pixel_buffer_target PUBLIC ${OEP_SUBMODULE_DIR})
endif()


# TARGET bnb_oep_offscreen_effect_player_target
if (USE_BNB_OEP_OFFSCREEN_EFFECT_PLAYER)
    # sources
    file(GLOB_RECURSE bnb_oep_offscreen_effect_player_target_srcs
        ${CMAKE_CURRENT_SOURCE_DIR}/offscreen_effect_player.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/offscreen_effect_player.hpp
    )
    # new target bnb_oep_offscreen_effect_player_target
    add_library(bnb_oep_offscreen_effect_player_target STATIC ${bnb_oep_offscreen_effect_player_target_srcs})
    target_include_directories(bnb_oep_offscreen_effect_player_target PUBLIC ${OEP_SUBMODULE_DIR})
endif()
