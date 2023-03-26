cmake_minimum_required(VERSION 3.0)
project(SD)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/modules/")

# dependencies
find_package(GLFW3 REQUIRED)
include_directories(${GLFW3_INCLUDE_DIR})
message(STATUS "GLFW3 included at: ${GLFW3_INCLUDE_DIR}")
message(STATUS "GLFW3 library dir: ${GLFW3_LIBRARY}")

#find_package(OpenGL REQUIRED)
#include_directories(${OpenGL_INCLUDE_DIRS})

find_package(GLEW REQUIRED)
include_directories(${GLEW_INCLUDE_DIRS})
message(STATUS "GLEW included at: ${GLEW_INCLUDE_DIRS}")
message(STATUS "GLEW library dir: ${GLEW_LIBRARY}")

find_package(Eigen3 REQUIRED)
include_directories(${Eigen3_INCLUDE_DIR})
message(STATUS "Eigen3 included at: ${Eigen3_INCLUDE_DIR}")

find_package(GLM REQUIRED)
include_directories(${GLM_INCLUDE_DIR})
message(STATUS "GLM included at: ${GLM_INCLUDE_DIR}")

#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wno-int-in-bool-context -fopenmp -DGLEW_NO_GLU")

add_definitions(-DHOMEDIR="${CMAKE_CURRENT_SOURCE_DIR}")

set(SOURCE_FILES_CPP src/tinyply.cpp)
set(SOURCE_FILES_H src/OpenGLHelper.h src/CameraHelper.h src/tinyply.h)

add_executable(main src/main.cpp ${SOURCE_FILES_CPP} ${SOURCE_FILES_H})
#target_link_libraries(main opengl32 glfw3 glew)
target_link_libraries(main opengl32 ${GLEW_LIBRARY} ${GLFW3_LIBRARY})

