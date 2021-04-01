cmake_minimum_required(VERSION 3.18)

set(CMAKE_CXX_STANDARD 17)
set(PROJECT_NAME vault)
set(OUTPUT_DEBUG bin/Debug)
set(OUTPUT_RELEASE bin/Release)

project(${PROJECT_NAME} CXX)

# Define Release by default.
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release")
    message(STATUS "Build type not specified: Using Release by default.")
endif(NOT CMAKE_BUILD_TYPE)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/${OUTPUT_DEBUG}")
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/${OUTPUT_DEBUG}")
    set(CMAKE_EXECUTABLE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/${OUTPUT_DEBUG}")
else()
    set(CMAKE_BUILD_TYPE "Release")
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/${OUTPUT_RELEASE}")
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/${OUTPUT_RELEASE}")
    set(CMAKE_EXECUTABLE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/${OUTPUT_RELEASE}")
    if(WIN32)
        set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup")
    endif()
endif()

message(STATUS "Building ${CMAKE_BUILD_TYPE}")

################# Flags ################
# Defines Flags for Windows and Linux. #
########################################

# Get packages
find_package(Threads)

# Manual include paths
include_directories(include)

if(MSVC)
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /W3 /MDd /Od /EHsc")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /W3 /GL /Od /Oi /Gy /EHsc /MD")
endif(MSVC)
if(NOT MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++20")
    if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
        set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++")
    endif()
endif(NOT MSVC)

################ Files ################
#   --   Add files to project.   --   #
#######################################
set(INCLUDE_DIR src)
set(APP_SRC src)

file(GLOB_RECURSE FILES
        "${INCLUDE_DIR}/*.h"
        "${APP_SRC}/*.cpp"
        )

add_executable(${PROJECT_NAME} ${FILES})

# COPY RESOURCES
# we don't want to copy if we're building in the source dir
#if (NOT CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_CURRENT_BINARY_DIR)
#    message(STATUS "Copying resources to output directory ${CMAKE_CURRENT_BINARY_DIR}/Resources")
#
#    # list of files for which we add a copy rule
#    set(resources background.png lester.png ProggyClean.ttf)
#
#    foreach(item IN LISTS resources)
#        message(STATUS "Copying Resources/${item}")
#        configure_file("${CMAKE_CURRENT_SOURCE_DIR}/Resources/${item}" "${CMAKE_CURRENT_BINARY_DIR}/Resources/${item}" COPYONLY)
#    endforeach()
#endif()

if(MSVC)
    target_link_libraries(${PROJECT_NAME} raylib)
else()
    target_link_libraries(${PROJECT_NAME}
            ${CMAKE_THREAD_LIBS_INIT}
            raylib)
endif()

#target_include_directories(${PROJECT_NAME} PUBLIC ${LUA_INCLUDE_DIR})
#target_compile_options(${PROJECT_NAME} PUBLIC ${PYTHON3_CFLAGS_OTHER} ${RAYLIB_CFLAGS_OTHER})
