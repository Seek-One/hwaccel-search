#################
# Configuration #
#################

set(LOCAL_PROJECT_NAME        "h264Player")
set(LOCAL_PROJECT_OUTPUT_NAME "h264-player")
set(LOCAL_PROJECT_DESCRIPTION "Simple program to render h264 video")

add_executable(h264_player_target
    local/H264Parser.cc
    main.cc
)

# Also make it accessible via namespace
add_executable(${LOCAL_PROJECT_NAMESPACE}::${LOCAL_PROJECT_NAME} ALIAS h264_player_target)



################
# Dependencies #
################

target_link_libraries(h264_player_target
    PRIVATE
        vw::VdpWrapper
        hb::h264bitstream
)

############
# Building #
############

# Change the output name
set_target_properties(h264_player_target PROPERTIES
    OUTPUT_NAME ${LOCAL_PROJECT_OUTPUT_NAME}
)

# Where to find the header files
target_include_directories(h264_player_target
    PUBLIC
        $<INSTALL_INTERFACE:include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        ${CMAKE_CURRENT_BINARY_DIR}/gen-private-include
)

# Generate a private header "version.h" defining PROJECT_VERSION
# configure_file (
#     "${CMAKE_CURRENT_SOURCE_DIR}/src/version.h.in"
#     "${CMAKE_CURRENT_BINARY_DIR}/gen-private-include/version.h"
# )

# Turn on warnings
target_compile_options(h264_player_target PRIVATE $<$<CXX_COMPILER_ID:GNU>:
    -Wall
    -Wextra
    -g
>)
target_compile_options(h264_player_target PRIVATE $<$<CXX_COMPILER_ID:MSVC>:
    /W4
    /w44265
    /w44061
    /w44062
>)
