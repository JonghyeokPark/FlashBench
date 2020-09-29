set(Flashbench_SRC
  histogram.cc
  random_generator.cc
  config.cc
  stat.cc
  benchmark.cc

  # test case fuction file list
  test1.cc
  # test2.cc
)

add_library(flashbench ${Flashbench_SRC})

if(PIBENCH_ENABLE_PCM)
  add_dependencies(flashbench pcm)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DENABLE_PCM")
endif(PIBENCH_ENABLE_PCM)

add_executable(flashbench-bin main.cc)

if(PIBENCH_ENABLE_PCM)
  target_link_libraries(flashbench-bin flashbench pthread ${PROJECT_SOURCE_DIR}/pcm/libpcm.so)
else()
  target_link_libraries(flashbench-bin flashbench pthread)
endif(PIBENCH_ENABLE_PCM)

set_target_properties(flashbench-bin PROPERTIES OUTPUT_NAME FlashBench)
