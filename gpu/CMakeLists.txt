cmake_minimum_required(VERSION 2.6)

PROJECT (gpu)

include_directories(
	${PROJECT_SOURCE_DIR}/../common/
	${PROJECT_SOURCE_DIR}/../externals/gl3w/include	
)

add_library(gpu gl_apis.cpp klt_gpu.cpp)