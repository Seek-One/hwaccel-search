# Copyright (c) 2021 Jet1oeil

# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:

# The above copyright notice and this permission notice shall be included in all
# copies or substantial portions of the Software.

# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.

#################
# Configuration #
#################

add_executable(d3d11va_player_target
    local/D3D11Manager.cc
    local/Decoder.cc
    local/DecodedPictureBuffer.cc
    local/FileParser.cc
    local/Filter.cc
    local/VideoTexture.cc
    local/Window.cc
    main.cc
)

# Also make it accessible via namespace
add_executable(${LOCAL_PROJECT_NAMESPACE}::${LOCAL_PROJECT_NAME} ALIAS d3d11va_player_target)

################
# Dependencies #
################

# find_package(DirectX11 REQUIRED)

target_link_libraries(d3d11va_player_target
    PRIVATE
        hb::h264bitstream
)

############
# Building #
############

# Change the output name
set_target_properties(d3d11va_player_target PROPERTIES
    OUTPUT_NAME ${LOCAL_PROJECT_OUTPUT_NAME}
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
)

# Where to find the header files
target_include_directories(d3d11va_player_target
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

# Force Unicode support
target_compile_definitions(d3d11va_player_target
  PRIVATE "UNICODE;_UNICODE"
)

# Turn on warnings
target_compile_options(d3d11va_player_target PRIVATE $<$<CXX_COMPILER_ID:GNU>:
    -Wall
    -Wextra
    -g
>)
target_compile_options(d3d11va_player_target PRIVATE $<$<CXX_COMPILER_ID:MSVC>:
    /W4
    /w44265
    /w44061
    /w44062
>)

include(GNUInstallDirs)
install(TARGETS d3d11va_player_target EXPORT ${LOCAL_PROJECT_NAME}Targets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)
