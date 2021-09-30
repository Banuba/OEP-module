set(include_dirs
    ${CMAKE_CURRENT_SOURCE_DIR}/include/
    ${OEP_SUBMODULE_DIR}/libraries/
    ${OEP_SUBMODULE_DIR}/
    ${PROJECT_SOURCE_DIR}/
)

file(GLOB_RECURSE srcs
    ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/include/*.hpp
)

add_library(ogl_utils STATIC ${srcs})

target_include_directories(ogl_utils PUBLIC
    ${include_dirs}
)

if(DESKTOP_BUILD)
    target_link_libraries(ogl_utils
        bnb_effect_player
        glad
    )
elseif(ANDROID_BUILD)
    target_link_libraries(ogl_utils
        banuba
    )
endif()