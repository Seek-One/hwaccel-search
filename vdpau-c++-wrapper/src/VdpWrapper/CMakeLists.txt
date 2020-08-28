add_library(vdp_wrapper_target STATIC
    DecodedPictureBuffer.cc
    DecodedSurface.cc
    Decoder.cc
    Device.cc
    Display.cc
    ImageBuffer.cc
    NalUnit.cc
    PresentationQueue.cc
    RenderSurface.cc
    VdpFunctions.cc
    VideoMixer.cc
)

add_library(${LOCAL_PROJECT_NAMESPACE}::${LOCAL_PROJECT_NAME} ALIAS vdp_wrapper_target)

################
# Dependencies #
################

find_package(X11 REQUIRED)
find_package(VDPAU REQUIRED)
find_package(OpenCV REQUIRED)

target_link_libraries(vdp_wrapper_target
    PRIVATE
        ${X11_LIBRARIES}
        ${VDPAU_LIBRARY}
        ${OpenCV_LIBS}
)

############
# Building #
############

# Change the output name from "lib${LOCAL_PROJECT_NAMESPACE}_target.a" to "lib${LOCAL_PROJECT_OUTPUT_NAME}.a"
set_target_properties(vdp_wrapper_target PROPERTIES
    OUTPUT_NAME ${LOCAL_PROJECT_OUTPUT_NAME}
)

target_include_directories(vdp_wrapper_target
    PUBLIC
        $<INSTALL_INTERFACE:include>
        $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        ${CMAKE_CURRENT_BINARY_DIR}/gen-private-include
)

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
