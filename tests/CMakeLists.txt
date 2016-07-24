cmake_minimum_required(VERSION 2.6)

PROJECT (tests)

include_directories(
	${PROJECT_SOURCE_DIR}/../externals/gl3w/include
	${PROJECT_SOURCE_DIR}/../gpu/	
	${PROJECT_SOURCE_DIR}/../cpu/		
	${PROJECT_SOURCE_DIR}/../common/			
)

add_executable(test_klt_image test_image.cpp)
target_link_libraries(test_klt_image cpu ${ROOT_LIBS})

add_executable(test_klt_video test_video.cpp)
target_link_libraries(test_klt_video cpu ${ROOT_LIBS})