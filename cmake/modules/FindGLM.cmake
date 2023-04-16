# Locate the GLM library
#
# This module defines the following variables:
#
# GLM_LIBRARY the name of the library;
# GLM_INCLUDE_DIR where to find GLM include files.
# GLM_FOUND true if both the GLM_LIBRARY and GLM_INCLUDE_DIR have been found.
#
# To help locate the library and include file, you can define a
# variable called GLM_ROOT which points to the root of the GLM library
# installation.
#
# default search dirs
# 

set( _GLM_HEADER_SEARCH_DIRS
"/usr/include"
"/usr/local/include"
"${CMAKE_SOURCE_DIR}/includes"
"C:/Program Files (x86)/glm/include" )

# Check environment for root search directory
set( _GLM_ENV_ROOT $ENV{GLM_ROOT} )
if( NOT GLM_ROOT AND _GLM_ENV_ROOT )
	set(GLM_ROOT ${_GLM_ENV_ROOT} )
endif()

# Put user specified location at beginning of search
if( GLM_ROOT )
	list( INSERT _GLM_HEADER_SEARCH_DIRS 0 "${GLM_ROOT}/include" )
endif()

# Search for the header
FIND_PATH(GLM_INCLUDE_DIR "glm/glm.hpp"
PATHS ${_GLM_HEADER_SEARCH_DIRS} )
