cmake_minimum_required(VERSION 3.9)

project(offscreen_effect_player-desktop-submodule)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(${CMAKE_CURRENT_LIST_DIR}/cmake/utils.cmake)

set(BNB_RESOURCES_FOLDER ${CMAKE_CURRENT_LIST_DIR}/resources)
set(BNB_THIRD_FOLDER ${CMAKE_CURRENT_LIST_DIR}/third)

set_variable_if_undefined(BUILD_C_API ON)

if(WIN32)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /D_ENABLE_EXTENDED_ALIGNED_STORAGE")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /D_ENABLE_EXTENDED_ALIGNED_STORAGE")
endif()

# Set to OFF to disable ffmpeg dependency (SDK should be built with disabled video_player also)
set(BNB_VIDEO_PLAYER ON)

add_definitions(
    -DBNB_RESOURCES_FOLDER="${BNB_RESOURCES_FOLDER}"
    -DBNB_VIDEO_PLAYER=$<BOOL:${BNB_VIDEO_PLAYER}>
)

include(${CMAKE_CURRENT_LIST_DIR}/cmake/copy_libs.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/cmake/libyuv.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/cmake/utils.cmake)

#
# Main
#

if(BUILD_C_API)
    add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/bnb_sdk_c_api)
else() # cpp build
    add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/bnb_sdk)
endif()
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/third)
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/libraries)
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/offscreen_effect_player)
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/offscreen_render_target)
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/preprocessor)

option(DEPLOY_BUILD "Build for deployment" OFF)

###########
# Targets #
###########

if (APPLE)
    # some magic to add these files to copy phase
if(BUILD_C_API)
    set(SDK_FRAMEWORK_PATH "${CMAKE_CURRENT_LIST_DIR}/bnb_sdk_c_api/mac")
    set(FullEPFrameworkPath "${SDK_FRAMEWORK_PATH}/$(CONFIGURATION)/BNBEffectPlayerC.framework")
else()
    set(SDK_FRAMEWORK_PATH "${CMAKE_CURRENT_LIST_DIR}/bnb_sdk/mac")
    set(FullEPFrameworkPath "${SDK_FRAMEWORK_PATH}/$(CONFIGURATION)/BanubaEffectPlayer.framework")
endif()
    add_custom_command(OUTPUT "${FullEPFrameworkPath}" COMMAND "")
    
    add_executable(example main.cpp ${FullEPFrameworkPath})
    
    set_source_files_properties(${FullEPFrameworkPath} PROPERTIES MACOSX_PACKAGE_LOCATION Frameworks)
else (APPLE)
    add_executable(example main.cpp)
endif (APPLE)

target_include_directories(example SYSTEM PUBLIC ${LIBYUV_INCLUDE_DIRS})

if(BUILD_C_API)
    target_link_libraries(example
        glfw
        glfw_utils
        offscreen_ep
        opencv
        camera
        renderer
        yuv
    )
else()
    target_link_libraries(example
        glfw
        glfw_utils
        offscreen_ep
        opencv
        renderer
        yuv
    )
endif()

if (APPLE)
    if(BUILD_C_API)
        set(CMAKE_OSX_DEPLOYMENT_TARGET "10.14")
    else()
        set(CMAKE_OSX_DEPLOYMENT_TARGET "10.12")
    endif()

    set(EXAMPLE_RESOURCES ${BNB_RESOURCES_FOLDER}/mac/bnb_viewer.icns)
    target_sources(example PRIVATE
        ${EXAMPLE_RESOURCES}
    )
    set(EXAMPLE_BUNDLE_NAME "Banuba Offscreen Rendering Example") # This name can contain up to 15 characters according to Apple's documentation
    set(EXAMPLE_COPYRIGHT "© 2021 Banuba. All rights reserved.")
    set(EXAMPLE_INFO "Banuba Offscreen Rendering Example.")

    set_target_properties(example PROPERTIES
        OUTPUT_NAME ${EXAMPLE_BUNDLE_NAME}
        RESOURCE ${EXAMPLE_RESOURCES}
        MACOSX_BUNDLE TRUE

        # The same as stadrard MacOSXBundleInfo.plist.in from CMake but with camera permissions added
        MACOSX_BUNDLE_INFO_PLIST "${BNB_RESOURCES_FOLDER}/mac/Info.plist.in"

        MACOSX_BUNDLE_BUNDLE_NAME           ${EXAMPLE_BUNDLE_NAME}            # Sets CFBundleName
        MACOSX_BUNDLE_COPYRIGHT             ${EXAMPLE_COPYRIGHT}              # Sets NSHumanReadableCopyright
        MACOSX_BUNDLE_GUI_IDENTIFIER        "com.banuba.sdk.offsceen.sample"  # Sets CFBundleIdentifier
        MACOSX_BUNDLE_ICON_FILE             bnb_viewer.icns                   # Sets CFBundleIconFile
        MACOSX_BUNDLE_INFO_STRING           ${EXAMPLE_INFO}                   # Sets CFBundleGetInfoString

        # Explicitly skip code signing (CMake tries to turn it on for application bundles)
        XCODE_ATTRIBUTE_CODE_SIGN_IDENTITY ""
    )
    if(BUILD_C_API)
        target_link_libraries(example
            "-framework AVFoundation"
            "-framework CoreMedia"
        )
    endif()
endif()

copy_sdk(example)
copy_third(example)