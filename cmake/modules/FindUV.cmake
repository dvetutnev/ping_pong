find_path(
    UV_DIR
    NAMES autogen.sh
    PATHS ${CMAKE_SOURCE_DIR}/${PROJECT_VENDOR_DIR}/libuv
    NO_DEFAULT_PATH
)

if (NOT UV_DIR)
    message(FATAL_ERROR "libuv not found!")
endif()

set(UV_DEPS_DIR "${CMAKE_CURRENT_BINARY_DIR}/vendor/libuv")

include(ExternalProject)

if(WIN32)
    ExternalProject_Add(
        libuv
        TMP_DIR ${UV_DEPS_DIR}/tmp
        STAMP_DIR ${UV_DEPS_DIR}/stamp
        SOURCE_DIR ${UV_DIR}
        BUILD_IN_SOURCE 1
        CONFIGURE_COMMAND <SOURCE_DIR>/vcbuild.bat release x86 shared
        BUILD_COMMAND ""
        INSTALL_COMMAND ""
        TEST_COMMAND ""
    )
else(WIN32)
    ExternalProject_Add(
        libuv
        TMP_DIR ${UV_DEPS_DIR}/tmp
        STAMP_DIR ${UV_DEPS_DIR}/stamp
        SOURCE_DIR ${UV_DIR}
        BUILD_IN_SOURCE 1
        CONFIGURE_COMMAND sh <SOURCE_DIR>/autogen.sh && ./configure --enable-static --disable-shared CFLAGS=-fPIC CC=${CMAKE_C_COMPILER}
        BUILD_COMMAND make -j4
        INSTALL_COMMAND ""
        TEST_COMMAND ""
    )
endif(WIN32)

set(UV_INCLUDE_DIR "${UV_DIR}/include")

add_library(uv STATIC IMPORTED)
find_library(UV_LIBRARY NAMES libuv.a libuv PATHS ${UV_DIR} PATH_SUFFIXES .libs NO_DEFAULT_PATH)
set_target_properties(uv PROPERTIES IMPORTED_LOCATION ${UV_LIBRARY})
set_target_properties(uv PROPERTIES INTERFACE_INCLUDE_DIRECTORIES ${UV_INCLUDE_DIR})
add_dependencies(uv libuv)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(
    UV
    REQUIRED_VARS
        UV_DIR
)

