project(PA1)

cmake_minimum_required(VERSION 2.6.2 FATAL_ERROR)

option(BUILD_DEBUG "Turn on the debug mode" OFF)
#===================================================================
## Compiler
# set compiler flags for debug/release
if ( BUILD_DEBUG )
    add_definitions(-DDEBUG)
    set(CMAKE_BUILD_TYPE Debug)
else ()
    add_definitions(-Wno-deprecated)
    set(CMAKE_BUILD_TYPE Release)
endif ()

# package for opengl and glut
find_package(GLUT REQUIRED)
find_package(OpenGL REQUIRED)

include_directories(${GLUT_INCLUDE_DIR})

add_executable(pa1
    src/main.cpp
    src/trackball.h         src/trackball.cpp
    src/GLScreenCapturer.h  src/GLScreenCapturer.cpp
    src/Texture.h  src/Texture.cpp)
target_link_libraries(pa1
    ${GLUT_LIBRARIES}       ${OPENGL_LIBRARIES})
