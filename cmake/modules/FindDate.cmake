find_path(
    Date_DIR
    NAMES date.h
    PATHS ${CMAKE_SOURCE_DIR}/${PROJECT_VENDOR_DIR}/date
    NO_DEFAULT_PATH
)

if (NOT Date_DIR)
    message(FATAL_ERROR "Date not found!")
endif()

set(Date_INCLUDE_DIR ${Date_DIR})

add_library(tz STATIC ${Date_DIR}/tz.cpp)
set_target_properties(tz PROPERTIES COMPILE_DEFINITIONS "USE_OS_TZDB=1")
target_include_directories(tz PUBLIC ${Date_INCLUDE_DIR})

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(
    Date
    REQUIRED_VARS
        Date_INCLUDE_DIR
)
