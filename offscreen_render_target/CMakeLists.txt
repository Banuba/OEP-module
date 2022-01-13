# TARGET bnb_oep_offscreen_render_target_target
if (USE_BNB_OEP_OFFSCREEN_RENDER_TARGET)
    # sources
    file(GLOB_RECURSE bnb_oep_offscreen_render_target_target_srcs
        ${CMAKE_CURRENT_SOURCE_DIR}/offscreen_render_target.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/offscreen_render_target.hpp
    )
    # new target bnb_oep_offscreen_render_target_target
    add_library(bnb_oep_offscreen_render_target_target STATIC ${bnb_oep_offscreen_render_target_target_srcs})
    target_include_directories(bnb_oep_offscreen_render_target_target PUBLIC ${OEP_SUBMODULE_DIR})
    target_link_libraries(bnb_oep_offscreen_render_target_target
        bnb_oep_opengl_program_target
        bnb_oep_opengl_yuv_converter_target
    )
endif()
