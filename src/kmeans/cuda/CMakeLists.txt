cmake_minimum_required(VERSION 3.2)

set(FOLDER_NAME kmeans_cuda)
set(SUBDIRECTORIES)
set(SRC_FILES 
  main.cc
  kmeans_cuda_benchmark.cu
)
set(HEADER_FILES
  kmeans_cuda_benchmark.h
)
set(TEST_FILES)
set(EXTRA_FILES)

if (NOT COMPILE_HCC)
if (CUDA_FOUND)
  cuda_add_executable(${FOLDER_NAME} ${HEADER_FILES} ${SRC_FILES})
  target_link_libraries(${FOLDER_NAME}
      kmeans time_measurement command_line_option benchmark)
endif (CUDA_FOUND)
endif (NOT COMPILE_HCC)
