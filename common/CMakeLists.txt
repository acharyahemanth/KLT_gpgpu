cmake_minimum_required(VERSION 2.6)

PROJECT (common)

include_directories(
	${PROJECT_SOURCE_DIR}/../externals/gl3w/include
)

add_library(common gl_setup.cpp)