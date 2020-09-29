cmake_minimum_required(VERSION 3.10)
project(FlashBench)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(CheckCXXCompilerFlag)
CHECK_CXX_COMPILER_FLAG("-march=native" COMPILER_SUPPORTS_MARCH_NATIVE)
if(COMPILER_SUPPORTS_MARCH_NATIVE)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -march=native")
endif()

option(PIBENCH_ENABLE_PCM "Enalbe PCM" ON)

if(PIBENCH_ENABLE_PCM)
  ######################## Intel PCM ########################
  add_custom_command(OUTPUT libpcm.so
                      COMMAND make libpcm.so
                      WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/pcm)
  add_custom_target(pcm DEPENDS libpcm.so)
  ###########################################################

  include_directories("${PROJECT_SOURCE_DIR}/include"
                      "${PROJECT_SOURCE_DIR}/pcm")
else()
  include_directories("${PROJECT_SOURCE_DIR}/include")
endif(PIBENCH_ENABLE_PCM)

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR})

add_subdirectory(src)



