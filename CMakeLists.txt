cmake_minimum_required(VERSION 3.0.0)

project(Crosshair VERSION 2.0.0)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


if (NOT MSVC) 
    message(FATAL_ERROR "This program can only be run on Windows")
endif()

include_directories(${CMAKE_SOURCE_DIR}/include)

add_subdirectory(${CMAKE_SOURCE_DIR}/lib/SDL2)
add_subdirectory(${CMAKE_SOURCE_DIR}/lib/SDL_ttf)


file(GLOB SRC_FILES "src/*.cpp")

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
add_executable(Crosshair ${SRC_FILES})
else()
add_executable(Crosshair WIN32 ${SRC_FILES})
endif()

target_compile_definitions(Crosshair PRIVATE WIN32_LEAN_AND_MEAN PRIVATE NOMINMAX)

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    configure_file(Arial.ttf ${CMAKE_SOURCE_DIR}/bin/Arial.ttf COPYONLY)
endif()

target_link_libraries(Crosshair PRIVATE SDL2)
target_link_libraries(Crosshair PRIVATE SDL2main)
target_link_libraries(Crosshair PRIVATE SDL2_ttf)

set_target_properties(SDL2 PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)
set_target_properties(SDL2_ttf PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)
set_target_properties(Crosshair PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)
