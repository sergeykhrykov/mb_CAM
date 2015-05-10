# CMAKE Build file
# -------------------------------- 
# Expects passed vars: 
# MB_BIT, 
# MB_BUILD_CONFIG, 
# MB_OPENGL_VERSION,
# MB_INSTALL_DEPENDENCY_DLLS.
# Note that platform (32/64 bit) is actually chosen with cmake -G command line option. 
# MB_BIT won't affect target platform
# --------------------------------

# SET UP VERSION INFORMATION
cmake_minimum_required(VERSION 3.1)

# No idea what it does
if(POLICY CMP0020)
  cmake_policy(SET CMP0020 NEW)
endif()
if(POLICY CMP0043)
  cmake_policy(SET CMP0043 NEW)
endif()

project("mb_test_gui")

# --------------------------------
# Set variables for build configuration
# Restrict possible configs to the chosen one
set(CMAKE_CONFIGURATION_TYPES "${MB_BUILD_CONFIG}" CACHE STRING "Type of configuration" FORCE)

set (CMAKE_BUILD_TYPE ${MB_BUILD_CONFIG} CACHE STRING "Choose the type of build." FORCE)

# Set proper build type for selecting QT libraries
if(CMAKE_BUILD_TYPE EQUAL "Debug")
	set (QT_BUILD_TYPE "DEBUG")
elseif(CMAKE_BUILD_TYPE EQUAL "RelWithDebInfo")
	set (QT_BUILD_TYPE "RELEASE")
endif(CMAKE_BUILD_TYPE EQUAL "Debug")
	
# Set variables for 32 or 64 bit
if(${CMAKE_SIZEOF_VOID_P} MATCHES 4)
	message(STATUS "Configuring for 32 bit")
	set(BIT 32)
	set(BIT_S "32")	
	
else(${CMAKE_SIZEOF_VOID_P} MATCHES 8)
	message(STATUS "Configuring for 64 bit")
	set(BIT 64)
	set(BIT_S "64")		
endif()

# Set variable for OpenGL version
set(OpenGL_VERSION ${MB_OPENGL_VERSION})
message(STATUS "OpenGL version is ${OpenGL_VERSION}")

# --------------------------------
# Set paths to find Qt
set (QT_PATH_32 "C:/dev/Qt/Qt5.4.1_32bit/5.4/msvc2013_opengl")
set (QT_PATH_64 "C:/dev/Qt/Qt5.4.1_64bit/5.4/msvc2013_64_opengl")

# Choose Qt modules to use in the project
set (QT_USED_MODULES "Widgets;Concurrent")

# Set path to find VTK
set(VTK_ROOT_PATH "C:/dev/VTK/_install/${BIT}bit.${OpenGL_VERSION}/${CMAKE_BUILD_TYPE}" CACHE PATH "VTK directory" FORCE)
set(VTK_PATH "${VTK_ROOT_PATH}/lib/cmake/vtk-6.3/")

# --------------------------------

if (BIT MATCHES 32)
	set (QT_PATH ${QT_PATH_32})
else (BIT MATCHES 64)
	set (QT_PATH ${QT_PATH_64})
endif()

set(QT_QMAKE_EXECUTABLE PATH ${QT_PATH}/bin/qmake.exe)
set(QT_BIN_DIR ${QT_PATH}/bin)

set(PROJ_NAME "mb_test_gui${BIT}_${CMAKE_BUILD_TYPE}_${OpenGL_VERSION}")

# For MinGW, I suppose
if (NOT MSVC) 
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
endif(NOT MSVC)


#set(VS_DIR "C:/Program Files (x86)/Microsoft Visual Studio 12.0")
#set(MS_SDK_DIR "C:/Program Files (x86)/Microsoft SDKs")

# --------------------------------
# Set up Qt

#set (CMAKE_PREFIX_PATH "${QT_PATH}/lib/cmake/Qt5;${CMAKE_PREFIX_PATH}")
#set(QT_USE_IMPORTED_TARGETS TRUE)
#find_package(Qt5Widgets CONFIG REQUIRED PATHS QT_PATH)
#find_package(Qt5Concurrent CONFIG REQUIRED PATHS QT_PATH)
message ("")
message (STATUS "--------------------------------")
message (STATUS "Looking for Qt at: ${CMAKE_PREFIX_PATH}")
find_package(Qt5 REQUIRED NO_MODULE COMPONENTS ${QT_USED_MODULES} REQUIRED PATHS "${QT_PATH}/lib/cmake" NO_DEFAULT_PATH)

if (Qt5_FOUND) 
	message (STATUS "Found Qt, adding modules:")
	foreach (QT_MODULE ${QT_USED_MODULES})		
		set(QT_MODULE_VAR_NAME "Qt5${QT_MODULE}_LIBRARIES")
		set (QT_MODULE_NAME "${${QT_MODULE_VAR_NAME}}")
		message(STATUS "Module: ${QT_MODULE_NAME}")
		list(APPEND QT_MODULE_NAMES "${QT_MODULE_NAME}")
		#set (QT_MODULE_NAMES ${QT_MODULE_NAMES} "\;" ${QT_MODULE_NAME})
		#set (QT_MODULE_NAMES "${QT_MODULE_NAMES};")
		#list(APPEND QT_MODULE_NAMES ";")
	endforeach(QT_MODULE)	
	
elseif(NOT Qt5_FOUND)
	message (FATAL_ERROR "Could not find Qt, please check the path above")
	
endif(Qt5_FOUND)

message (STATUS "Determine dependent Qt modules...")
foreach (QT_MODULE ${QT_MODULE_NAMES})
	get_target_property(DEPS ${QT_MODULE} INTERFACE_LINK_LIBRARIES)	
	message (STATUS "For module ${QT_MODULE}: ${DEPS}")
	list(APPEND QT_MODULES_DEPS ${DEPS})
endforeach(QT_MODULE)

set (QT_MODULES_WITH_DEPS ${QT_MODULES_DEPS})
foreach(QT_MODULE ${QT_MODULE_NAMES})
	#add origonal modules to the list
	list(APPEND QT_MODULES_WITH_DEPS ${QT_MODULE}) 
endforeach(QT_MODULE)

list(APPEND QT_MODULES_WITH_DEPS Qt5::WinMain)

list(REMOVE_DUPLICATES QT_MODULES_WITH_DEPS)
message(STATUS "Summary of Qt with dependencies: ${QT_MODULES_WITH_DEPS}")

# --------------------------------
# Set up VTK
message ("")
message (STATUS "--------------------------------")
set (CMAKE_PREFIX_PATH "${VTK_PATH};${CMAKE_PREFIX_PATH}")
message (STATUS "Looking for VTK at: ${VTK_PATH}")

set (VTK_MODULES vtkGUISupportQt vtkFiltersModeling vtkIOGeometry)

if(${OpenGL_VERSION} STREQUAL "OpenGL")
	list (APPEND VTK_MODULES vtkRenderingOpenGL)
	
elseif(${OpenGL_VERSION} STREQUAL "OpenGL2")
	list (APPEND VTK_MODULES vtkRenderingOpenGL2)	
	
endif(${OpenGL_VERSION} STREQUAL "OpenGL")

find_package(VTK REQUIRED NO_MODULE COMPONENTS ${VTK_MODULES} PATHS ${VTK_PATH} NO_DEFAULT_PATH)

include(${VTK_USE_FILE})

if (VTK_FOUND)
	message (STATUS "Found VTK at: ${VTK_DIR}")
	
elseif (NOT VTK_FOUND)
	message (FATAL_ERROR "Could not find VTK, please check the path above")
	
endif (VTK_FOUND)

# --------------------------------
# Set up Windows Libraries
if(${BIT} MATCHES 64)
#	set(WIN_LIB_DIR ${MS_SDK_DIR}"/Windows/v7.1A/Lib/x64")
else()
#	set(WIN_LIB_DIR ${MS_SDK_DIR}"/Windows/v7.1A/Lib")
endif()
# set(WIN_LIBRARIES ${WIN_LIB_DIR}/IPHlpApi.Lib)

# --------------------------------
# General include directories
include_directories(${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} ${CRYPT_INCLUDE_DIR})


# --------------------------------
set(GLOBAL_SRCS_H
	${CMAKE_CURRENT_SOURCE_DIR}/Phonger.h
)

set(GLOBAL_SRCS_UI
	${CMAKE_CURRENT_SOURCE_DIR}/Phonger.ui
)

set(GLOBAL_SRCS_CXX 
	${CMAKE_CURRENT_SOURCE_DIR}/Phonger.cxx
	
	${CMAKE_CURRENT_SOURCE_DIR}/main.cxx
)


set(MOC_HEADERS ${GLOBAL_SRCS_H})
	
set(UI_FILES ${GLOBAL_SRCS_UI})

set(QT_RES )
	
set(H_FILES ${GLOBAL_SRCS_H})
	
SET(CXX_FILES ${GLOBAL_SRCS_CXX})

# We don't use AUTOMOC and AUTOUIC, because we want to add the according files to the "gen" sourcegroup
qt5_wrap_ui(UISrcs ${UI_FILES})
qt5_wrap_cpp(MOCSrcs ${MOC_HEADERS})
qt5_add_resources(RCCSrcs ${QT_RES})

add_executable(${PROJ_NAME} ${H_FILES} ${CXX_FILES} ${UISrcs} ${MOCSrcs} ${RCCSrcs})


if (MB_BUILD_CONFIG MATCHES Debug)
	target_link_libraries(${PROJ_NAME} debug ${VTK_LIBRARIES} debug ${QT_MODULES_WITH_DEPS})
	message(STATUS "Libraries of dependency: ${VTK_LIBRARIES} ${QT_MODULES_WITH_DEPS}")	
	#add_custom_command(TARGET ${PROJ_NAME} POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE_DIR:${PROJ_NAME}> /bin VERBATIM)
	
elseif(MB_BUILD_CONFIG MATCHES RelWithDebInfo)
	target_link_libraries(${PROJ_NAME} optimized ${VTK_LIBRARIES} optimized ${QT_MODULES_WITH_DEPS})
	
endif()

message (STATUS "MB_INSTALL_DEPENDENCY_DLLS=${MB_INSTALL_DEPENDENCY_DLLS}")

if (MB_INSTALL_DEPENDENCY_DLLS EQUAL 1)
	
	# --------------------------------
	# INSTALL VTK DLLS
	
	# get list of VTK files with paths
	set(VTK_DLLS_PATH "${VTK_ROOT_PATH}/bin")

	foreach (VTK_LIBRARY ${VTK_LIBRARIES})
		set(FILENAME_WITH_PATH "${VTK_DLLS_PATH}/${VTK_LIBRARY}-${VTK_MAJOR_VERSION}.${VTK_MINOR_VERSION}.dll")
	#	message(STATUS "File to copy: ${FILENAME_WITH_PATH}")
		list(APPEND VTK_DLLS_TO_INSTALL ${FILENAME_WITH_PATH})
	endforeach (VTK_LIBRARY)

	#message(STATUS "DLLs to copy: ${VTK_DLLS_TO_INSTALL}")
	# Determine where to copy
	set (VTK_DLLS_INSTALL_DIR "${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_BUILD_TYPE}")
	message(STATUS "Installing VTK libraries to: ${VTK_DLLS_INSTALL_DIR}")
	file(INSTALL ${VTK_DLLS_TO_INSTALL} DESTINATION ${VTK_DLLS_INSTALL_DIR})	

	# --------------------------------
	# INSTALL QT DLLS
	
	set (QT_DLLS_INSTALL_DIR ${VTK_DLLS_INSTALL_DIR})
	
	# Get libraries names to install
	foreach(QT_MODULE ${QT_MODULES_WITH_DEPS})
		get_target_property(FILENAME_WITH_PATH ${QT_MODULE} LOCATION_${CMAKE_BUILD_TYPE})
		list(APPEND QT_DLLS_TO_INSTALL ${FILENAME_WITH_PATH}) 
	endforeach(QT_MODULE)
	
	#message(STATUS "Qt dlls to be installed: ${QT_DLLS_TO_INSTALL}")
	message(STATUS "Installing QT libraries to: ${QT_DLLS_INSTALL_DIR}")
	
	file(INSTALL ${QT_DLLS_TO_INSTALL} DESTINATION ${QT_DLLS_INSTALL_DIR})	
	
	#install additional Qt unicode libraries for Windows
	
	if(WIN32)
		message (STATUS "Installing additional Qt dlls for Windows...")
		set (QT_ADDITIONAL_DLLS icudt53.dll	icuin53.dll	icuuc53.dll	)
		foreach(QT_ADDITIONAL_DLL_NAME ${QT_ADDITIONAL_DLLS})
			set(FILENAME_WITH_PATH "${QT_BIN_DIR}/${QT_ADDITIONAL_DLL_NAME}")
			file(INSTALL ${FILENAME_WITH_PATH} DESTINATION ${QT_DLLS_INSTALL_DIR})	
		endforeach(QT_ADDITIONAL_DLL_NAME)
	endif(WIN32)
	
else(MB_INSTALL_DEPENDENCY_DLLS EQUAL 1)
	message ("Not installing dependency dlls since user did not specify --install-dependency-dlls key")

endif (MB_INSTALL_DEPENDENCY_DLLS EQUAL 1)



# --------------------------------
# SET UP FOR VISUAL STUDIO

# Configure the template file that allows debugging
set(USER_FILE ${PROJ_NAME}.vcxproj.user) 	
set(OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR}/${USER_FILE})
configure_file(UserTemplate.user ${USER_FILE} @ONLY)

source_group(gen FILES ${UISrcs} ${MOCSrcs} ${RCCSrcs})
source_group(global FILES ${GLOBAL_SRCS_H} ${GLOBAL_SRCS_UI} ${GLOBAL_SRCS_CXX} ${GLOBAL_SRCS_MAIN})


