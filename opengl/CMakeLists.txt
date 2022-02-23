# TARGET bnb_oep_opengl_program_target
file(GLOB_RECURSE bnb_oep_opengl_program_srcs
    "${CMAKE_CURRENT_SOURCE_DIR}/opengl.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/program.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/program.hpp"
)
add_library(bnb_oep_opengl_program_target STATIC ${bnb_oep_opengl_program_srcs})
target_include_directories(bnb_oep_opengl_program_target PUBLIC ${OEP_SUBMODULE_DIR})
if(ANDROID)
    target_link_libraries(bnb_oep_opengl_program_target GLESv3)
else()
    target_link_libraries(bnb_oep_opengl_program_target glad)
endif()

# TARGET bnb_oep_opengl_yuv_converter_target
file(GLOB_RECURSE bnb_oep_opengl_yuv_converter_srcs
    "${CMAKE_CURRENT_SOURCE_DIR}/yuv_converter.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/yuv_converter.hpp"
)
add_library(bnb_oep_opengl_yuv_converter_target STATIC ${bnb_oep_opengl_yuv_converter_srcs})
target_include_directories(bnb_oep_opengl_yuv_converter_target PUBLIC ${OEP_SUBMODULE_DIR})
target_link_libraries(bnb_oep_opengl_yuv_converter_target bnb_oep_opengl_program_target)
