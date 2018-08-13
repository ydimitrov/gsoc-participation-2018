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

#Generate a cmake cache file usable by cmake script.
set(CacheForScript ${CMAKE_BINARY_DIR}/CMakeCacheForScript.cmake)
#Create a tmp cmake file.
file(WRITE ${CacheForScript} "")

get_cmake_property(Vars VARIABLES)
foreach(Var ${Vars})
	if(${Var})
		#Replace unwanted char.
		string(REPLACE "\\" "\\\\" VALUE ${${Var}})
		string(REPLACE "\n" "\\n" VALUE ${VALUE})
		string(REPLACE "\r" "\\n" VALUE ${VALUE})
		string(REPLACE "\"" "\\\"" VALUE ${VALUE})
	endif()
	file(APPEND ${CacheForScript} "set(${Var} \"${VALUE}\")\n")
endforeach()

# ----------------------------------------------------------------------------
#                                Autobuild target
# ----------------------------------------------------------------------------

add_custom_command(OUTPUT ${PROJECT_TEMPLATE_AGL_AUTOBUILD_DIR}/autobuild ${PROJECT_TEMPLATE_LINUX_AUTOBUILD_DIR}/autobuild
	DEPENDS ${CMAKE_SOURCE_DIR}/${PROJECT_APP_TEMPLATES_DIR}/template.d/autobuild/agl/autobuild.in
	${CMAKE_SOURCE_DIR}/${PROJECT_APP_TEMPLATES_DIR}/template.d/autobuild/linux/autobuild.in

	COMMAND [ ! -f "${PROJECT_TEMPLATE_AGL_AUTOBUILD_DIR}/autobuild" ] && ${CMAKE_COMMAND} -DINFILE=${CMAKE_SOURCE_DIR}/${PROJECT_APP_TEMPLATES_DIR}/template.d/autobuild/agl/autobuild.in -DOUTFILE=${PROJECT_TEMPLATE_AGL_AUTOBUILD_DIR}/autobuild -DPROJECT_BINARY_DIR=${CMAKE_CURRENT_BINARY_DIR} -P ${CMAKE_SOURCE_DIR}/${PROJECT_APP_TEMPLATES_DIR}/cmake/configure_file.cmake || exit 0
	COMMAND [ ! -f "${PROJECT_TEMPLATE_LINUX_AUTOBUILD_DIR}/autobuild" ] && ${CMAKE_COMMAND} -DINFILE=${CMAKE_SOURCE_DIR}/${PROJECT_APP_TEMPLATES_DIR}/template.d/autobuild/agl/autobuild.in -DOUTFILE=${PROJECT_TEMPLATE_LINUX_AUTOBUILD_DIR}/autobuild -DPROJECT_BINARY_DIR=${CMAKE_CURRENT_BINARY_DIR} -P ${CMAKE_SOURCE_DIR}/${PROJECT_APP_TEMPLATES_DIR}/cmake/configure_file.cmake || exit 0
)

add_custom_target(autobuild ALL DEPENDS ${PROJECT_TEMPLATE_AGL_AUTOBUILD_DIR}/autobuild ${PROJECT_TEMPLATE_LINUX_AUTOBUILD_DIR}/autobuild)
