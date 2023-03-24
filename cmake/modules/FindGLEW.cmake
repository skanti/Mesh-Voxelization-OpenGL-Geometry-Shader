# Locate the glew library
#
# This module defines the following variables:
#
# glew_LIBRARY the name of the library;
# glew_INCLUDE_DIR where to find glew include files.
# glew_FOUND true if both the glew_LIBRARY and glew_INCLUDE_DIR have been found.
#
# To help locate the library and include file, you can define a
# variable called glew_ROOT which points to the root of the glew library
# installation.
#
# default search dirs
# 
# Cmake file from: https://github.com/daw42/glslcookbook

set( _glew_HEADER_SEARCH_DIRS
"/usr/include"
"/usr/local/include"
"${CMAKE_SOURCE_DIR}/includes"
"C:/Program Files (x86)/glew/include" )
set( _glew_LIB_SEARCH_DIRS
"/usr/lib"
"/usr/local/lib"
"${CMAKE_SOURCE_DIR}/lib"
"C:/Program Files (x86)/glew/lib" )

# Check environment for root search directory
set( _glew_ENV_ROOT $ENV{glew_ROOT} )
if( NOT glew_ROOT AND _glew_ENV_ROOT )
	set(glew_ROOT ${_glew_ENV_ROOT} )
endif()

# Put user specified location at beginning of search
if( glew_ROOT )
	list( INSERT _glew_HEADER_SEARCH_DIRS 0 "${glew_ROOT}/include" )
	list( INSERT _glew_LIB_SEARCH_DIRS 0 "${glew_ROOT}/lib" )
endif()

# Search for the header
FIND_PATH(glew_INCLUDE_DIR "GL/glew.h"
PATHS ${_glew_HEADER_SEARCH_DIRS} )

# Search for the library
FIND_LIBRARY(glew_LIBRARY NAMES GLEW
PATHS ${_glew_LIB_SEARCH_DIRS} )
INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(GLEW DEFAULT_MSG
glew_LIBRARY glew_INCLUDE_DIR)
