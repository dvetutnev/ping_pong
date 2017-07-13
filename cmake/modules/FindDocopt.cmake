find_path(
    docopt_DIR
    NAMES CMakeLists.txt
    PATHS ${CMAKE_SOURCE_DIR}/${PROJECT_VENDOR_DIR}/docopt
    NO_DEFAULT_PATH
)

if(NOT IS_DIRECTORY ${docopt_DIR})
    return()
endif()

add_subdirectory(${docopt_DIR})
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(
    Docopt
    REQUIRED_VARS
        docopt_DIR
)
