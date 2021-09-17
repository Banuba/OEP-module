file(GLOB_RECURSE srcs
    ${CMAKE_CURRENT_LIST_DIR}/*.hpp
)

add_library(camera STATIC ${srcs})

target_include_directories(camera PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}/include/
    ${CMAKE_CURRENT_LIST_DIR}
    ${PROJECT_SOURCE_DIR}
)

if (APPLE)
    set(apple_includes
        ${CMAKE_CURRENT_LIST_DIR}/platform/macos/
    )

    file(GLOB_RECURSE apple_srcs
        ${CMAKE_CURRENT_LIST_DIR}/platform/macos/*.mm
    )

    target_sources(camera PUBLIC ${apple_srcs})
    target_include_directories(camera PUBLIC ${apple_includes})
elseif (WIN32)
    file(GLOB_RECURSE win_srcs
        ${CMAKE_CURRENT_LIST_DIR}/platform/windows/*.hpp
        ${CMAKE_CURRENT_LIST_DIR}/platform/windows/*.cpp
    )
    target_sources(camera PUBLIC ${win_srcs})
endif()

target_link_libraries(camera
    bnb_effect_player
)
