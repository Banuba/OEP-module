if (USE_BNB_OEP_CAMERA)
    add_library(bnb_oep_camera_target STATIC)

    target_include_directories(bnb_oep_camera_target PUBLIC ${OEP_SUBMODULE_DIR})
    target_include_directories(bnb_oep_camera_target PUBLIC ${CMAKE_CURRENT_LIST_DIR})

    if (APPLE)
        file(GLOB_RECURSE apple_srcs
            ${CMAKE_CURRENT_LIST_DIR}/*.hpp
            ${CMAKE_CURRENT_LIST_DIR}/macos/*.hpp
            ${CMAKE_CURRENT_LIST_DIR}/macos/*.mm
        )
        target_sources(bnb_oep_camera_target PUBLIC ${apple_srcs})
        target_link_libraries(bnb_oep_camera_target
            "-framework AVFoundation"
            "-framework CoreMedia"
        )
    elseif (WIN32)
        file(GLOB_RECURSE win_srcs
            ${CMAKE_CURRENT_LIST_DIR}/*.hpp
            ${CMAKE_CURRENT_LIST_DIR}/windows/*.hpp
            ${CMAKE_CURRENT_LIST_DIR}/windows/*.cpp
        )
        target_sources(bnb_oep_camera_target PUBLIC ${win_srcs})
    else ()
        message(FATAL_ERROR "Unsupported platform for 'bnb_oep_camera_target', CMake will exit.")
    endif ()
endif()
