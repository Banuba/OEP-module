add_subdirectory(utils)

if(DESKTOP_BUILD)
    add_subdirectory(camera)
    add_subdirectory(renderer)
endif()