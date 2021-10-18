option(DESKTOP_BUILD_PART       "Build for desktop"     ${DESKTOP_BUILD})
option(NATIVE_MACOS_BUILD_PART  "Build for macos"       ${NATIVE_MACOS_BUILD})
option(ANDROID_BUILD_PART       "Build for android"     ${ANDROID_BUILD})
option(C_API_BUILD_PART         "c-api or cpp-api"      ${BUILD_C_API})

configure_file(
    "${CMAKE_CURRENT_LIST_DIR}/config.hpp.in"
    "${CMAKE_CURRENT_LIST_DIR}/config.hpp"
)
