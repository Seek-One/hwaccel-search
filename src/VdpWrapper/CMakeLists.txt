add_library(vdp_wrapper_target STATIC
    Sample.cc
)

# Also make it accessible via namespace
add_library(${LOCAL_PROJECT_NAMESPACE}::${LOCAL_PROJECT_NAME} ALIAS vdp_wrapper_target)

################
# Dependencies #
################

# Subdir with CMake support
# add_subdirectory(some/subdir)

############
# Building #
############

# Change the output name from "lib${LOCAL_PROJECT_NAMESPACE}_target.a" to "lib${LOCAL_PROJECT_OUTPUT_NAME}.a"
set_target_properties(vdp_wrapper_target PROPERTIES
    OUTPUT_NAME ${LOCAL_PROJECT_OUTPUT_NAME}
)

# Where to find the header files
target_include_directories(vdp_wrapper_target
    PUBLIC
        $<INSTALL_INTERFACE:include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        ${CMAKE_CURRENT_BINARY_DIR}/gen-private-include
)

# # Generate a private header "version.h" defining PROJECT_VERSION
# configure_file (
#     "${CMAKE_CURRENT_SOURCE_DIR}/src/version.h.in"
#     "${CMAKE_CURRENT_BINARY_DIR}/gen-private-include/version.h"
# )

# Turn on warnings
target_compile_options(vdp_wrapper_target PRIVATE $<$<CXX_COMPILER_ID:GNU>:
    -Wall
    -Wextra
    -g
>)
target_compile_options(vdp_wrapper_target PRIVATE $<$<CXX_COMPILER_ID:MSVC>:
    /W4
    /w44265
    /w44061
    /w44062
>)

#############
# Exporting #
#############

# Write a version file so that CMake's find_package() can check for a specific
# version of this package.
include(CMakePackageConfigHelpers)
write_basic_package_version_file(
    ${LOCAL_PROJECT_NAME}ConfigVersion.cmake
    VERSION ${PACKAGE_VERSION}
    COMPATIBILITY AnyNewerVersion
)

include(GNUInstallDirs)
install(TARGETS vdp_wrapper_target EXPORT ${LOCAL_PROJECT_NAME}Targets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)
install(DIRECTORY include DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

export(TARGETS vdp_wrapper_target NAMESPACE ${LOCAL_PROJECT_NAMESPACE}:: FILE ${LOCAL_PROJECT_NAME}Targets.cmake)
export(PACKAGE ${LOCAL_PROJECT_NAME})
