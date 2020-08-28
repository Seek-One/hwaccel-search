# Copyright (c) 2020 Jet1oeil

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

cmake_minimum_required(VERSION 3.13)
cmake_policy(VERSION 3.13)

#################
# Configuration #
#################

# Variables so we don't repeat ourselves
set(LOCAL_PROJECT_VERSION     "0.0.1")
set(LOCAL_PROJECT_VENDOR      "jet1oeil")
set(LOCAL_PROJECT_NAMESPACE   "vw")
set(LOCAL_PROJECT_NAME        "VdpWrapper")
set(LOCAL_PROJECT_OUTPUT_NAME "vdp_wrapper")
set(LOCAL_PROJECT_DESCRIPTION "Simple c++ VDPAU wrapper")

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake-modules")

project(${LOCAL_PROJECT_NAME}
    VERSION ${LOCAL_PROJECT_VERSION}
    DESCRIPTION ${LOCAL_PROJECT_DESCRIPTION}
)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED "ON")
set(CMAKE_C_EXTENSIONS "OFF")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED "ON")
set(CMAKE_CXX_EXTENSIONS "OFF")

add_subdirectory(src/VdpWrapper)
add_subdirectory(src/ImageViewer)
add_subdirectory(lib/vendor)
add_subdirectory(src/h264Player)
