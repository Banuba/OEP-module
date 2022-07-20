
if (APPLE)
    set(include_dirs
        ${CMAKE_CURRENT_SOURCE_DIR}/apple/
    )

    file(GLOB_RECURSE srcs
        ${CMAKE_CURRENT_SOURCE_DIR}/apple/*.mm
        ${CMAKE_CURRENT_SOURCE_DIR}/apple/*.h
    )

    add_library(oep_utils STATIC ${srcs})

    target_include_directories(oep_utils PUBLIC
        ${include_dirs}
    )
endif (APPLE)