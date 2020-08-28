set(LOCAL_PROJECT_VERSION     "0.2.0")
set(LOCAL_PROJECT_VENDOR      "aizvorski")
set(LOCAL_PROJECT_NAMESPACE   "hb")
set(LOCAL_PROJECT_NAME        "h264bitstream")
set(LOCAL_PROJECT_OUTPUT_NAME "h264bitstream")

add_library(h264bitstream_target STATIC
    h264bitstream/h264_analyze.c
    h264bitstream/h264_sei.c
    h264bitstream/h264_slice_data.c
    h264bitstream/h264_nal.c
    h264bitstream/h264_avcc.c
    h264bitstream/h264_stream.c
)

add_library(${LOCAL_PROJECT_NAMESPACE}::${LOCAL_PROJECT_NAME} ALIAS h264bitstream_target)

################
# Dependencies #
################


############
# Building #
############

# Change the output name from "lib${LOCAL_PROJECT_NAMESPACE}_target.a" to "lib${LOCAL_PROJECT_OUTPUT_NAME}.a"
set_target_properties(h264bitstream_target PROPERTIES
    OUTPUT_NAME ${LOCAL_PROJECT_OUTPUT_NAME}
)

target_include_directories(h264bitstream_target
    PUBLIC
        $<INSTALL_INTERFACE:include>
        $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/lib/vendor/h264bitstream>
)

# Turn on warnings
target_compile_options(h264bitstream_target PRIVATE $<$<C_COMPILER_ID:GNU>:
    -Wall
    -Wextra
    -g
>)
target_compile_options(h264bitstream_target PRIVATE $<$<C_COMPILER_ID:MSVC>:
    /W4
    /w44265
    /w44061
    /w44062
>)

#############
# Exporting #
#############

include(CMakePackageConfigHelpers)
write_basic_package_version_file(
    ${LOCAL_PROJECT_NAME}ConfigVersion.cmake
    VERSION ${PACKAGE_VERSION}
    COMPATIBILITY AnyNewerVersion
)

include(GNUInstallDirs)
install(TARGETS h264bitstream_target EXPORT ${LOCAL_PROJECT_NAME}Targets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)
install(DIRECTORY include DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

export(TARGETS h264bitstream_target NAMESPACE ${LOCAL_PROJECT_NAMESPACE}:: FILE ${LOCAL_PROJECT_NAME}Targets.cmake)
export(PACKAGE ${LOCAL_PROJECT_NAME})
