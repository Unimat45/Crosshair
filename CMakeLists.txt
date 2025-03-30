cmake_minimum_required(VERSION 3.16.0)

project(Crosshair LANGUAGES C CXX VERSION 2.0.0)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(LUA_ENABLE_SHARED OFF)
set(LUA_BUILD_COMPILER OFF)

if(NOT WIN32)
    message(FATAL_ERROR "This program can only be run on Windows")
endif()

add_subdirectory(${CMAKE_SOURCE_DIR}/lib/SDL2)
add_subdirectory(${CMAKE_SOURCE_DIR}/lib/SDL_ttf)
add_subdirectory(${CMAKE_SOURCE_DIR}/lib/lua)

file(GLOB SRC_FILES "src/*.c*")

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_executable(${PROJECT_NAME} ${SRC_FILES})
else()
    add_executable(${PROJECT_NAME} WIN32 ${SRC_FILES})
endif()

target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/include)
target_compile_definitions(${PROJECT_NAME} PRIVATE WIN32_LEAN_AND_MEAN PRIVATE NOMINMAX)
target_link_libraries(${PROJECT_NAME} PRIVATE SDL2 SDL2main SDL2_ttf Lua::Library)
set_target_properties(${PROJECT_NAME} SDL2 SDL2_ttf PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)
