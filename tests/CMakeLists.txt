cmake_minimum_required(VERSION 2.6)

PROJECT (tests)

include_directories(
	${PROJECT_SOURCE_DIR}/../externals/gl3w/include
	${PROJECT_SOURCE_DIR}/../gpu/	
	${PROJECT_SOURCE_DIR}/../cpu/		
	${PROJECT_SOURCE_DIR}/../common/			
)

add_executable(klt_gpgpu main.cpp)
target_link_libraries(klt_gpgpu ${ROOT_LIBS})