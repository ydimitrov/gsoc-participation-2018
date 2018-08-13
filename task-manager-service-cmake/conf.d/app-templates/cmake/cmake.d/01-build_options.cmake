###########################################################################
# Copyright 2015, 2016, 2017 IoT.bzh
#
# author: Fulup Ar Foll <fulup@iot.bzh>
# contrib: Romain Forlot <romain.forlot@iot.bzh>
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
###########################################################################


#--------------------------------------------------------------------------
#  WARNING:
#     Do not change this cmake template
#     Customise your preferences in "./conf.d/cmake/config.cmake"
#--------------------------------------------------------------------------

# (BUG!!!) as PKG_CONFIG_PATH does not work [should be en env variable]
set(PKG_CONFIG_USE_CMAKE_PREFIX_PATH ON CACHE BOOLEAN "Flag for using prefix path")

INCLUDE(FindPkgConfig)
INCLUDE(CheckIncludeFiles)
INCLUDE(CheckLibraryExists)
INCLUDE(GNUInstallDirs)

if(NOT CMAKE_BUILD_TYPE)
	if(BUILD_TYPE)
		set(CMAKE_BUILD_TYPE ${BUILD_TYPE} CACHE STRING "the type of build" FORCE)
	else()
		set(CMAKE_BUILD_TYPE DEBUG CACHE STRING "the type of build" FORCE)
	endif()
endif()

set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMP0048 1)

# Default compilation options
############################################################################
link_libraries(-Wl,--as-needed -Wl,--gc-sections)
set(COMPILE_OPTIONS -Wall
 -Wextra
 -Wconversion
 -Wno-unused-parameter
 -Wno-sign-compare
 -Wno-sign-conversion
 -Werror=implicit-function-declaration
 -ffunction-sections
 -fdata-sections
 -fPIC CACHE STRING "Compilation flags")

set(COMPILE_OPTIONS_GNU -Werror=maybe-uninitialized CACHE STRING "GNU Compile specific options")
set(COMPILE_OPTIONS_CLANG -Werror=uninitialized CACHE STRING "CLang compile specific options")

# Compilation OPTIONS depending on language
#########################################
foreach(option ${COMPILE_OPTIONS})
	add_compile_options(${option})
endforeach()

if (${CMAKE_C_COMPILER_ID} STREQUAL "GNU" OR ${CMAKE_CXX_COMPILER_ID} STREQUAL "GNU")
	foreach(option ${COMPILE_OPTIONS_GNU})
		add_compile_options(${option})
	endforeach()
elseif(${CMAKE_C_COMPILER_ID} STREQUAL "Clang" OR ${CMAKE_CXX_COMPILER_ID} STREQUAL "Clang")
	foreach(option ${COMPILE_OPTIONS_CLANG})
		add_compile_options(${option})
	endforeach()
endif()

foreach(option ${C_COMPILE_OPTIONS})
	add_compile_options($<$<COMPILE_LANGUAGE:C>:${option}>)
endforeach()
foreach(option ${CXX_COMPILE_OPTIONS})
	add_compile_options($<$<COMPILE_LANGUAGE:CXX>:${option}>)
endforeach()

# Compilation option depending on CMAKE_BUILD_TYPE
##################################################
set(PROFILING_COMPILE_OPTIONS -g -O0 -pg -Wp,-U_FORTIFY_SOURCE CACHE STRING "Compilation flags for PROFILING build type.")
set(DEBUG_COMPILE_OPTIONS -g -ggdb -Og -D_FORTIFY_SOURCE=2 CACHE STRING "Compilation flags for DEBUG build type.")
set(COVERAGE_COMPILE_OPTIONS -g --coverage CACHE STRING "Compilation flags for COVERAGE build type.")
set(RELEASE_COMPILE_OPTIONS -O2 -D_FORTIFY_SOURCE=2 CACHE STRING "Compilation flags for RELEASE build type.")
foreach(option ${PROFILING_COMPILE_OPTIONS})
	add_compile_options($<$<CONFIG:PROFILING>:${option}>)
endforeach()
foreach(option ${DEBUG_COMPILE_OPTIONS})
	add_compile_options($<$<CONFIG:DEBUG>:${option}>)
endforeach()
foreach(option ${COVERAGE_COMPILE_OPTIONS})
	add_compile_options($<$<CONFIG:COVERAGE>:${option}>)
endforeach()
foreach(option ${RELEASE_COMPILE_OPTIONS})
	add_compile_options($<$<CONFIG:RELEASE>:${option}>)
endforeach()

# Loop on required package and add options
foreach (PKG_CONFIG ${PKG_REQUIRED_LIST})
	string(REGEX REPLACE "[<>]?=.*$" "" XPREFIX ${PKG_CONFIG})
	PKG_CHECK_MODULES(${XPREFIX} REQUIRED ${PKG_CONFIG})

	INCLUDE_DIRECTORIES(${${XPREFIX}_INCLUDE_DIRS})
	list(APPEND link_libraries ${${XPREFIX}_LDFLAGS})
	add_compile_options (${${XPREFIX}_CFLAGS})
endforeach(PKG_CONFIG)

# Optional LibEfence Malloc debug library
IF(CMAKE_BUILD_TYPE MATCHES DEBUG AND USE_EFENCE)
CHECK_LIBRARY_EXISTS(efence malloc "" HAVE_LIBEFENCE)
IF(HAVE_LIBEFENCE)
	MESSAGE(STATUS "Linking with ElectricFence for debugging purposes...")
	SET(libefence_LIBRARIES "-lefence")
	list (APPEND link_libraries ${libefence_LIBRARIES})
ENDIF(HAVE_LIBEFENCE)
ENDIF(CMAKE_BUILD_TYPE MATCHES DEBUG AND USE_EFENCE)

# set default include directories
INCLUDE_DIRECTORIES(${EXTRA_INCLUDE_DIRS})

# Default Linkflag
set(PKG_TEMPLATE_PREFIX "\"${CMAKE_SOURCE_DIR}/${PROJECT_APP_TEMPLATES_DIR}\"" CACHE PATH "Default Package Templates Directory")
set(BARE_PKG_TEMPLATE_PREFIX "${CMAKE_SOURCE_DIR}/${PROJECT_APP_TEMPLATES_DIR}" CACHE PATH "Default Package Templates Directory")

if(NOT BINDINGS_LINK_FLAG)
	set(BINDINGS_LINK_FLAG "-Wl,--version-script=${PKG_TEMPLATE_PREFIX}/cmake/export.map")
endif()
