#Author : Hemanth
#Written on 22/7/2016 
#Base CMakeLists to setup an OpenGL + OpenCV devel environment

cmake_minimum_required(VERSION 2.6)

PROJECT (klt_gpgpu)

FIND_PACKAGE( OpenCV REQUIRED )
FIND_PACKAGE( OpenGL REQUIRED )
FIND_PACKAGE( PkgConfig REQUIRED )
PKG_SEARCH_MODULE( GLFW REQUIRED glfw3 )
PKG_SEARCH_MODULE( ASSIMP REQUIRED assimp )

ADD_DEFINITIONS(
  -DBASE_TEST_DIR="${CMAKE_SOURCE_DIR}"
  -D_CRT_SECURE_NO_WARNINGS
)

LINK_DIRECTORIES (
    /opt/local/lib
)
INCLUDE_DIRECTORIES (
    /opt/local/include
)

SET(GL_LIBS
    gl3w
    ${GLFW_STATIC_LIBRARIES}
    ${OPENGL_LIBRARIES}
)
SET(ROOT_LIBS
  ${GL_LIBS}
  ${OpenCV_LIBS}
  "-framework CoreFoundation"
  assimp
  z
)

SET( CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -std=c++11" )
SET( CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -w -Waddress -Wc++11-compat -Wchar-subscripts -Wformat -Winit-self -Wmaybe-uninitialized -Wnarrowing -Wnonnull -Wparentheses -Wreorder -Wreturn-type -Wsequence-point -Wsign-compare -Wsizeof-pointer-memaccess -Wstrict-aliasing -Wstrict-overflow=1 -Wswitch -Wtrigraphs -Wuninitialized -Wunknown-pragmas -Wunused-function -Wunused-label -Wunused-value -Wunused-variable -Wvolatile-register-var" )

###################################
add_subdirectory(externals)
add_subdirectory(cpu)
add_subdirectory(gpu)
add_subdirectory(tests)
