find_path(
    UVW_INCLUDE_DIR
    NAMES uvw.hpp
    PATHS ${CMAKE_SOURCE_DIR}/${PROJECT_VENDOR_DIR}/uvw/src
    NO_DEFAULT_PATH
)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(
    UVW
    REQUIRED_VARS
        UVW_INCLUDE_DIR
)
