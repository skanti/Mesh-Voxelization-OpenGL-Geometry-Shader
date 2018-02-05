cmake_minimum_required(VERSION 3.0)
project(SD)

# dependencies
find_package(glfw3 3.2 REQUIRED)
include_directories(${GLFW_INCLUDE_DIRS})

find_package(OpenGL REQUIRED)
include_directories(${OpenGL_INCLUDE_DIRS})

find_package(GLEW REQUIRED)
include_directories(${GLEW_INCLUDE_DIRS})

find_package(Eigen3 3.3 REQUIRED)
include_directories(${EIGEN3_INCLUDE_DIR})
# <-

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wno-int-in-bool-context -fopenmp -DGLEW_NO_GLU")

add_definitions(-DHOMEDIR="${CMAKE_CURRENT_SOURCE_DIR}")

set(SOURCE_FILES_CPP src/tinyply.cpp)
set(SOURCE_FILES_H src/OpenGLHelper.h src/CameraHelper.h src/tinyply.h)

add_executable(main src/main.cpp ${SOURCE_FILES_CPP} ${SOURCE_FILES_H})
target_link_libraries(main glfw GL GLEW)

