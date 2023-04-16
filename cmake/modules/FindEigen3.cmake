# Locate the Eigen3 library
#
# This module defines the following variables:
#
# Eigen3_LIBRARY the name of the library;
# Eigen3_INCLUDE_DIR where to find Eigen3 include files.
# Eigen3_FOUND true if both the Eigen3_LIBRARY and Eigen3_INCLUDE_DIR have been found.
#
# To help locate the library and include file, you can define a
# variable called Eigen3_ROOT which points to the root of the Eigen3 library
# installation.
#
# default search dirs
# 

set( _Eigen3_HEADER_SEARCH_DIRS
"/usr/include"
"/usr/local/include"
"${CMAKE_SOURCE_DIR}/includes"
"C:/Program Files (x86)/eigen3/include/eigen3" )

# Check environment for root search directory
set( _Eigen3_ENV_ROOT $ENV{Eigen3_ROOT} )
if( NOT Eigen3_ROOT AND _Eigen3_ENV_ROOT )
	set(Eigen3_ROOT ${_Eigen3_ENV_ROOT} )
endif()

# Put user specified location at beginning of search
if( Eigen3_ROOT )
	list( INSERT _Eigen3_HEADER_SEARCH_DIRS 0 "${Eigen3_ROOT}/include" )
endif()

# Search for the header
FIND_PATH(Eigen3_INCLUDE_DIR "Eigen/Eigen"
PATHS ${_Eigen3_HEADER_SEARCH_DIRS} )
