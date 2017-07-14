if(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_BINARY_DIR})
    message(FATAL_ERROR "Prevented in-tree built. Please create a build directory outside of the source code and call cmake from there. Thank you.")
endif()

project(ping_pong CXX C)
cmake_minimum_required(VERSION 3.4)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
if(NOT WIN32)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -pedantic")
endif(NOT WIN32)
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0 -g -DDEBUG")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 -DRELEASE")

set(PROJECT_CMAKE_MODULES cmake/modules)
set(PROJECT_VENDOR_DIR vendor)

set(
    CMAKE_MODULE_PATH
    ${CMAKE_MODULE_PATH}
    ${CMAKE_SOURCE_DIR}/${PROJECT_CMAKE_MODULES}
)

find_package(UV REQUIRED)
find_package(UVW REQUIRED)
find_package(Docopt REQUIRED)
find_package(Date REQUIRED)
find_package(Threads)

include_directories(
    ${UV_INCLUDE_DIR}
    ${UVW_INCLUDE_DIR}
    ${docopt_DIR}
    ${Date_INCLUDE_DIR}
    )

set(SRC_LIST
    main.cpp
)
add_executable(${PROJECT_NAME} ${SRC_LIST})
target_link_libraries(${PROJECT_NAME} uv docopt_s tz ${CMAKE_THREAD_LIBS_INIT})

message(STATUS "CMAKE_CURRENT_BINARY_DIR => ${CMAKE_CURRENT_BINARY_DIR}")
message(STATUS "CMAKE_C_COMPILER => ${CMAKE_C_COMPILER}")
message(STATUS "CMAKE_CXX_COMPILER => ${CMAKE_CXX_COMPILER}")

file(
    GLOB_RECURSE PROJECT_FILES FOLLOW_SYMLINKS
    *.txt *.c *.cpp *.hpp *.h *.in *.dox *.cmake *.sh *.md *.yml *.bat
    AUTHORS LICENSE TODO
)
add_custom_target(
    QTCREATOR_FALLBACK ALL
    COMMENT "Feel free to ignore this target, but please do not remove it."
    SOURCES ${PROJECT_FILES}
)

