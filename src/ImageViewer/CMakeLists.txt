#################
# Configuration #
#################

set(LOCAL_PROJECT_NAME        "ImageViewer")
set(LOCAL_PROJECT_OUTPUT_NAME "image-viewer")
set(LOCAL_PROJECT_DESCRIPTION "Simple program to render YUV or RGB images")

add_executable(image_viewer_target
    main.cc
)

# Also make it accessible via namespace
add_executable(${LOCAL_PROJECT_NAMESPACE}::${LOCAL_PROJECT_NAME} ALIAS image_viewer_target)



################
# Dependencies #
################

target_link_libraries(image_viewer_target vw::VdpWrapper)

############
# Building #
############

# Change the output name
set_target_properties(image_viewer_target PROPERTIES
    OUTPUT_NAME ${LOCAL_PROJECT_OUTPUT_NAME}
)

# Where to find the header files
target_include_directories(image_viewer_target
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
target_compile_options(image_viewer_target PRIVATE $<$<CXX_COMPILER_ID:GNU>:
    -Wall
    -Wextra
    -g
>)
target_compile_options(image_viewer_target PRIVATE $<$<CXX_COMPILER_ID:MSVC>:
    /W4
    /w44265
    /w44061
    /w44062
>)
