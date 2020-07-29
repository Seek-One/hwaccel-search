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

add_subdirectory(src/VdpWrapper)
add_subdirectory(src/ImageViewer)
add_subdirectory(lib/vendor)
add_subdirectory(src/h264Player)
