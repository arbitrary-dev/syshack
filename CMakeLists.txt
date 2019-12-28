cmake_minimum_required(VERSION 3.9)

project(
  SysHack
  LANGUAGES C
  VERSION 0.1
)

set(CMAKE_C_COMPILER "gcc")
set(CMAKE_C_FLAGS_DEBUG "-Og -g -Wall")
set(CMAKE_C_FLAGS_RELEASE "-O3")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

file(GLOB SOURCES src/*.c)

add_executable(syshack ${SOURCES})

configure_file(config.h.in config.h)

target_include_directories(
  syshack
  PUBLIC "${PROJECT_BINARY_DIR}" "${PROJECT_SOURCE_DIR}/include"
)

target_link_libraries(syshack ncursesw tinfow)
