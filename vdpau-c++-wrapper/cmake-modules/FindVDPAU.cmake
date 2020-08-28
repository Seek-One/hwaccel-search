#[=======================================================================[.rst:
FindVDPAU
-------

Finds the VDPAU library.

Imported Targets
^^^^^^^^^^^^^^^^

This module provides the following imported targets, if found:

``VDPAU::VDPAU``
  The VDPAU library

Result Variables
^^^^^^^^^^^^^^^^

This will define the following variables:

``VDPAU_FOUND``
  True if the system has the VDPAU library.
``VDPAU_VERSION``
  The version of the VDPAU library which was found.
``VDPAU_INCLUDE_DIRS``
  Include directories needed to use VDPAU.
``VDPAU_LIBRARIES``
  Libraries needed to link to VDPAU.

Cache Variables
^^^^^^^^^^^^^^^

The following cache variables may also be set:

``VDPAU_INCLUDE_DIR``
  The directory containing ``VDPAU.h``.
``VDPAU_LIBRARY``
  The path to the VDPAU library.

#]=======================================================================]

find_package(PkgConfig)
pkg_check_modules(PC_VDPAU QUIET vdpau)

find_path(VDPAU_INCLUDE_DIR
    NAMES vdpau.h
    PATHS ${PC_VDPAU_INCLUDE_DIRS} ${PC_VDPAU_INCLUDEDIR}
    PATH_SUFFIXES vdpau
)
find_library(VDPAU_LIBRARY
    NAMES vdpau
    PATHS ${PC_VDPAU_LIBRARY_DIRS}
)

set(VDPAU_VERSION ${PC_VDPAU_VERSION})

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(VDPAU
    FOUND_VAR VDPAU_FOUND
    REQUIRED_VARS
    VDPAU_LIBRARY
    VDPAU_INCLUDE_DIR
    VERSION_VAR VDPAU_VERSION
)

if(VDPAU_FOUND)
    set(VDPAU_LIBRARIES ${VDPAU_LIBRARY})
    set(VDPAU_INCLUDE_DIRS ${VDPAU_INCLUDE_DIR})
    set(VDPAU_DEFINITIONS ${PC_VDPAU_CFLAGS_OTHER})
endif()

if(VDPAU_FOUND AND NOT TARGET VDPAU::VDPAU)
    add_library(VDPAU::VDPAU UNKNOWN IMPORTED)
    set_target_properties(VDPAU::VDPAU PROPERTIES
        IMPORTED_LOCATION "${VDPAU_LIBRARY}"
        INTERFACE_COMPILE_OPTIONS "${PC_VDPAU_CFLAGS_OTHER}"
        INTERFACE_INCLUDE_DIRECTORIES "${VDPAU_INCLUDE_DIR}"
    )
endif()

if(VDPAU_FOUND)
    if (NOT TARGET VDPAU::VDPAU)
        add_library(VDPAU::VDPAU UNKNOWN IMPORTED)
    endif()
    if (VDPAU_LIBRARY_RELEASE)
    set_property(TARGET VDPAU::VDPAU APPEND PROPERTY
        IMPORTED_CONFIGURATIONS RELEASE
    )
    set_target_properties(VDPAU::VDPAU PROPERTIES
        IMPORTED_LOCATION_RELEASE "${VDPAU_LIBRARY_RELEASE}"
    )
    endif()
    if (VDPAU_LIBRARY_DEBUG)
        set_property(TARGET VDPAU::VDPAU APPEND PROPERTY
            IMPORTED_CONFIGURATIONS DEBUG
        )
        set_target_properties(VDPAU::VDPAU PROPERTIES
            IMPORTED_LOCATION_DEBUG "${VDPAU_LIBRARY_DEBUG}"
        )
    endif()
    set_target_properties(VDPAU::VDPAU PROPERTIES
        INTERFACE_COMPILE_OPTIONS "${PC_VDPAU_CFLAGS_OTHER}"
        INTERFACE_INCLUDE_DIRECTORIES "${VDPAU_INCLUDE_DIR}"
    )
endif()

mark_as_advanced(
    VDPAU_INCLUDE_DIR
    VDPAU_LIBRARY
)

# compatibility variables
set(VDPAU_VERSION_STRING ${VDPAU_VERSION})
