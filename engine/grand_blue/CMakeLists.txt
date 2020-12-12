# CMake build for the Reverie Engine
# To run cmake, add cmake/bin directory to system path
## TODO:
## Implement source_group, for organizing in IDE, see: https://stackoverflow.com/questions/31966135/cmake-source-group-not-working-correctly-with-hierarchical-project-setup
## TODO: Remove DLLs from this folder, as they are not needed

message( STATUS "---------Beginning cmake build of Reverie Engine---------")

# MACROS =================================================================
macro(remove_flag_from_target _target _flag)
    get_target_property(_target_cxx_flags ${_target} COMPILE_OPTIONS)
    if(_target_cxx_flags)
        list(REMOVE_ITEM _target_cxx_flags ${_flag})
        set_target_properties(${_target} PROPERTIES COMPILE_OPTIONS "${_target_cxx_flags}")
    endif()
endmacro()

# Project-level setup ====================================================
project( Reverie ) # TODO: Set version
cmake_minimum_required( VERSION 3.16 )
set(TARGET_NAME "reverie") # See: https://stackoverflow.com/questions/31037882/whats-the-cmake-syntax-to-set-and-use-variables

message (STATUS "--------- Set target ${TARGET_NAME}---------")

# Find includes in corresponding build directories
set(CMAKE_INCLUDE_CURRENT_DIR ON)

# Instruct CMake to run moc automatically when needed.
set(CMAKE_AUTOMOC ON) # Saves having to use QT5_WRAP_CPP
set(CMAKE_AUTORCC ON) # Saves having to use QT5_ADD_RESOURCES
set(CMAKE_AUTOUIC ON) # Saves having to use QT5_WRAP_UI


# Set compiler flags =====================================================
# Use C++20
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Multiprocess compiler, disable specific warnings
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP /wd4577 /wd4467")

# Remove unwanted flags
# STRING (REGEX REPLACE "[/|-]RTC(su|[1su])" "" CMAKE_CXX_FLAGS_DEBUG  "${CMAKE_CXX_FLAGS_DEBUG}")
# STRING (REGEX REPLACE "[/|-]Ob0" "" CMAKE_CXX_FLAGS_DEBUG  "${CMAKE_CXX_FLAGS_DEBUG}")
# STRING (REGEX REPLACE "[/|-]GR" "" CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS}")

# Set paths to find Qt ===================================================
# Find Qt itself
if(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
    set(CMAKE_PREFIX_PATH "C:\\Qt\\5.12.2\\msvc2017")
else()
    message (WARNING "-------- System not supported ---------")
endif()

# Find UI files
# See: https://stackoverflow.com/questions/40630820/how-to-place-header-and-ui-file-in-different-folders-using-autouic-in-cmake
set(CMAKE_AUTOUIC_SEARCH_PATHS "${CMAKE_CURRENT_SOURCE_DIR}/ui")

# Find Required Qt Libraries =============================================
find_package(Qt5 COMPONENTS Core REQUIRED)# Core stuff
find_package(Qt5 COMPONENTS Concurrent REQUIRED) # For threading
find_package(Qt5 COMPONENTS Gamepad REQUIRED) # Controller support
find_package(Qt5 COMPONENTS Gui REQUIRED) # For OpenGL integration
find_package(Qt5 COMPONENTS OpenGLExtensions REQUIRED)
# find_package(Qt5OpenGL) # OpenGL should come from GUI
find_package(Qt5 COMPONENTS Multimedia REQUIRED) # Multimedia capabilities
find_package(Qt5 COMPONENTS MultimediaWidgets REQUIRED)
find_package(Qt5 COMPONENTS Widgets REQUIRED) # For widgets

# OpenGL libraries
# See: https://stackoverflow.com/questions/65100749/converting-from-qmake-to-cmake-how-do-i-find-libraries-in-the-same-way/65106458#65106458
find_package(OpenGL REQUIRED) # enforces as a requirement
# find_library(OPENGL_LIB names opengl32) # Also valid
# find_library(GLU_LIB names glu32) # Also valid

# Set build mode directory options ========================================
# add_definitions(-DAPPLETTUTORIAL1_LIBRARY)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/debug)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/release)

# Set directories to find libraries, need to do this before add_executable for desired target
# link_directories(${CMAKE_SOURCE_DIR} "${CMAKE_SOURCE_DIR}/lib")
file(TO_CMAKE_PATH "$ENV{PYTHON_LIB}" ENV_PYTHON_LIB)
message(STATUS "Setting link directory ${ENV_PYTHON_LIB}")
link_directories(${ENV_PYTHON_LIB})

# Works as alternative, but does not solve any of my runtime problems
# set(PYTHON_QT_PATH "${CMAKE_SOURCE_DIR}/../third_party/PythonQt")
# set(PYTHON_QT_LIB_PATH "${PYTHON_QT_PATH}/lib")
# link_directories(${PYTHON_QT_LIB_PATH})

# Build EXE ===============================================================
# Get all source files, headers, resources, and UI (forms) from src subdirectory
# See: https://stackoverflow.com/questions/2110795/how-to-use-all-c-files-in-a-directory-with-the-cmake-build-system
# TODO: Group files https://stackoverflow.com/questions/41078807/cmake-and-visualstudio-group-files-in-solution-explorer
message (STATUS "-------- Finding source, header, resource, UI files relative to  ${CMAKE_SOURCE_DIR} ---------")
file(GLOB_RECURSE MY_SOURCES RELATIVE ${CMAKE_SOURCE_DIR} "src/*.cpp"  "src/*.h"
"resources/*.qrc" "ui/*.ui")

message (STATUS "=========== FOUND FILES =============")
message (STATUS "${MY_SOURCES}")
message (STATUS "=========== END FILES ===============")

# Generate release PDB
# https://stackoverflow.com/questions/28178978/how-to-generate-pdb-files-for-release-build-with-cmake-flags/31264946
add_compile_options("$<$<NOT:$<CONFIG:Debug>>:/Zi>")
add_link_options("$<$<NOT:$<CONFIG:Debug>>:/DEBUG>")
add_link_options("$<$<NOT:$<CONFIG:Debug>>:/OPT:REF>")
add_link_options("$<$<NOT:$<CONFIG:Debug>>:/OPT:ICF>")

# Add an executable to be built from sources variable
add_executable(${TARGET_NAME} ${MY_SOURCES})

# Add compile definitions to the target ===================================
# Set options based on debug or release mode
# Generator expression, see: 
# https://stackoverflow.com/questions/34490294/what-does-configdebugrelease-mean-in-cmake
# https://cmake.org/cmake/help/latest/manual/cmake-generator-expressions.7.html
set(DEBUG_DEFINITIONS "DEBUG_MODE")
set(RELEASE_DEFINITIONS "QT_NO_DEBUG_OUTPUT")
target_compile_definitions(${TARGET_NAME} PUBLIC
    $<$<CONFIG:DEBUG>:${DEBUG_DEFINITIONS}>
    $<$<CONFIG:RELEASE>:${RELEASE_DEFINITIONS}>
)

# Remove unwanted flags ===================================================
# Doesn't work
# remove_flag_from_target(${TARGET_NAME} -RTC1)

# Pre-processor Defines ===================================================
add_definitions(-D_UNICODE -D_ENABLE_EXTENDED_ALIGNED_STORAGE -DWIN64 -DQT_DLL -DQT_OPENGL_LIB)
add_definitions(-DDEVELOP_MODE)
add_definitions(-DLINALG_USE_EIGEN)

# Disable warnings about deprecated things, like using strcpy (sorry, not sorry)
if(MSVC)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
endif()

# Add compile options to the target =======================================
# target_compile_options(${TARGET_NAME} PUBLIC "$<$<CONFIG:DEBUG>:${MY_DEBUG_OPTIONS}>")
# target_compile_options(${TARGET_NAME} PUBLIC "$<$<CONFIG:RELEASE>:${MY_RELEASE_OPTIONS}>")

# Link Required Qt modules to main target =================================
# This MUST come after add_executable and find_package for each module
# Essentially, target_link_libraries links the two specified libraries
# https://stackoverflow.com/questions/25909943/getting-a-cmake-error-cannot-specify-link-libraries-for-target-which-is-not-bui/41262868
target_link_libraries(${TARGET_NAME} OpenGL::GL OpenGL::GLU)
target_link_libraries(${TARGET_NAME} Qt5::Core)
target_link_libraries(${TARGET_NAME} Qt5::Concurrent) 
target_link_libraries(${TARGET_NAME} Qt5::Gamepad)
target_link_libraries(${TARGET_NAME} Qt5::Gui)
target_link_libraries(${TARGET_NAME} Qt5::OpenGLExtensions) # TODO: Probably unnecessary, remove
target_link_libraries(${TARGET_NAME} Qt5::Multimedia)
target_link_libraries(${TARGET_NAME} Qt5::MultimediaWidgets)
target_link_libraries(${TARGET_NAME} Qt5::Widgets)

# Add additional libraries ================================================
# Add a default search locations
# TODO: link_directories takes generators, so maybe split into debug and release that way
message(STATUS "Default library search directories: ${CMAKE_SOURCE_DIR}/lib ${CMAKE_SOURCE_DIR}/lib/assimp")

# Include directories:
# https://stackoverflow.com/questions/13703647/how-to-properly-add-include-directories-with-cmake
# Linking libraries:
# **** https://stackoverflow.com/questions/48273378/qmake-to-cmake-transition-syntax-for-external-librairies
# https://stackoverflow.com/questions/60061299/convert-qmake-into-cmake

# Python
message (STATUS "=========== Including Python =============")
# To eliminate conflict with "slots" keyword: PyType_Slot *slots; /* terminated by slot==0. */
# Equivalent to CONFIG += no_keywords in qmake
# add_definitions(-DQT_NO_KEYWORDS)

# For system versions, see: 
# https://stackoverflow.com/questions/9160335/os-specific-instructions-in-cmake-how-to
if(DEFINED ENV{PYTHON_VERSION})
	set(PYTHON_VERSION $ENV{PYTHON_VERSION})
else()
	if(WIN32)
		# for Windows operating system in general
		# if(MSVC OR MSYS OR MINGW) # for detecting Windows compilers
		set(PYTHON_VERSION 38)
	elseif(APPLE)
		# for MacOS X or iOS, watchOS, tvOS (since 3.10.3)
	elseif(UNIX AND NOT APPLE)
		# For Linux, BSD, Solaris, Minix
		message(STATUS "Setting UNIX python version")
		set(PYTHON_VERSION 3.8)
	else()
		message( WARNING "Unrecognized operating system")
	endif()
endif()
message (STATUS "PYTHON VERSION $ENV{PYTHON_VERSION}")

# Set up include paths and libraries for Python itself
if(WIN32)
	# for Windows operating system in general
	# if(MSVC OR MSYS OR MINGW) # for detecting Windows compilers
	
	# Convert paths to CMAKE-friendly versions
	file(TO_CMAKE_PATH "$ENV{PYTHON_PATH}" ENV_PYTHON_PATH)
	
	# Link Python
	# Debug DLL fails to import third party modules, so always use release
	target_include_directories(${TARGET_NAME} PRIVATE "${ENV_PYTHON_PATH}/include")
	message(STATUS "Python include path for windows is ${ENV_PYTHON_PATH}/include")
	message(STATUS "Python path is ${ENV_PYTHON_PATH}")
	# find_library(PYTHON_DEBUG_LIB NAMES "python${PYTHON_VERSION}_d" HINTS ${ENV_PYTHON_LIB})
	find_library(PYTHON_RELEASE_LIB NAMES "python${PYTHON_VERSION}" HINTS ${ENV_PYTHON_LIB})
	# message(STATUS "Linking Python with debug path ${PYTHON_DEBUG_LIB} and release path ${PYTHON_RELEASE_LIB}")
	target_link_libraries(${TARGET_NAME}
		debug ${PYTHON_RELEASE_LIB} optimized ${PYTHON_RELEASE_LIB}
	)
elseif(APPLE)
	# for MacOS X or iOS, watchOS, tvOS (since 3.10.3)
	message (STATUS "BUILDING FOR APPLE")
	# Include library directory
	target_include_directories(${TARGET_NAME} PRIVATE "/System/Library/Frameworks/Python.framework/Headers")
	
	# Find library with name Python, and link
	find_library(PYTHON_LIBRARY_PATH NAMES "Python")
	target_link_libraries(${TARGET_NAME}
		${PYTHON_LIBRARY_PATH}
	)
elseif(UNIX AND NOT APPLE)
	# For Linux, BSD, Solaris, Minix
	message (STATUS "BUILDING FOR UNIX")
	execute_process(COMMAND python${PYTHON_VERSION}-config --embed --libs
		OUTPUT_VARIABLE PYTHON_LIBRARY_PATH
		RESULT_VARIABLE FAILED_TO_DETECT_PYTHON
	)
	if(FAILED_TO_DETECT_PYTHON)
		message (WARNING "Failed to detect Python")
	else()
		# Set library path
		target_link_libraries(${TARGET_NAME} ${PYTHON_LIBRARY_PATH})
	endif()
else()
	message( WARNING "Unrecognized operating system")
endif()


# PythonQt
# message (STATUS "=========== Including PythonQt =============")
# add_definitions(-DPYTHONQT_USE_RELEASE_PYTHON_FALLBACK)

# Include and link PythonQt
# set(PYTHON_QT_PATH "${CMAKE_SOURCE_DIR}/../third_party/PythonQt")
# set(PYTHON_QT_LIB_PATH "${PYTHON_QT_PATH}/lib")
# find_library(PYTHON_QT_DEBUG_LIB NAMES "PythonQt-Qt5-Python${PYTHON_VERSION}_d" HINTS "${PYTHON_QT_LIB_PATH}")
# find_library(PYTHON_QT_RELEASE_LIB NAMES "PythonQt-Qt5-Python${PYTHON_VERSION}" HINTS "${PYTHON_QT_LIB_PATH}")

# message(STATUS "Linking PythonQt with debug path ${PYTHON_QT_DEBUG_LIB} and release path ${PYTHON_QT_RELEASE_LIB}") 
# target_include_directories(${TARGET_NAME} PRIVATE "${PYTHON_QT_PATH}/src")
# target_link_libraries(${TARGET_NAME} 
	# debug "${PYTHON_QT_DEBUG_LIB}" optimized "${PYTHON_QT_RELEASE_LIB}")

# PYBIND
message (STATUS "=========== Including PYBIND =============")
target_include_directories(${TARGET_NAME} PRIVATE "${CMAKE_SOURCE_DIR}/../third_party/pybind11/include")
# find_package(pybind11 REQUIRED)
# target_link_libraries(${TARGET_NAME} pybind11::embed)
# set(PYBIND11_PYTHON_VERSION 3.8)
# add_subdirectory("../third_party/pybind11" "./bin/pybind11")


# EIGEN 
message (STATUS "=========== Including Eigen at ${CMAKE_SOURCE_DIR}/src/third_party/eigen =============")
target_include_directories(${TARGET_NAME} PRIVATE "${CMAKE_SOURCE_DIR}/src/third_party/eigen")


# ASSIMP
message (STATUS "=========== Including ASSIMP =============")
target_include_directories(${TARGET_NAME} PRIVATE 
"${CMAKE_SOURCE_DIR}/../third_party/assimp/assimp-5.0.0/include" 
"${CMAKE_SOURCE_DIR}/../third_party/assimp/assimp-5.0.0/build/include")
find_library(ASSIMP_DEBUG_LIB NAMES assimp_d HINTS "${CMAKE_SOURCE_DIR}/lib/assimp")
find_library(ASSIMP_LIB NAMES assimp HINTS "${CMAKE_SOURCE_DIR}/lib/assimp")
target_link_libraries(${TARGET_NAME} 
	debug ${ASSIMP_DEBUG_LIB} optimized ${ASSIMP_LIB})

# PhysX
message (STATUS "=========== Including PhysX as a static library =============")
add_definitions(-DPX_PHYSX_STATIC_LIB)
target_include_directories(${TARGET_NAME} PRIVATE 
"${CMAKE_SOURCE_DIR}/../third_party/physx/physx/include" 
"${CMAKE_SOURCE_DIR}/../third_party/physx/pxshared/include")
set( PHYSX_LIBRARY_DEBUG_DIR "${CMAKE_SOURCE_DIR}/lib/physx/debug")
set( PHYSX_LIBRARY_RELEASE_DIR "${CMAKE_SOURCE_DIR}/lib/physx/release")
message (STATUS "Debug directory is ${PHYSX_LIBRARY_DEBUG_DIR}")
target_link_libraries(${TARGET_NAME} 
	debug "${PHYSX_LIBRARY_DEBUG_DIR}/PhysX_static_32.lib" optimized "${PHYSX_LIBRARY_RELEASE_DIR}/PhysX_static_32.lib"
	debug "${PHYSX_LIBRARY_DEBUG_DIR}/PhysXCharacterKinematic_static_32.lib" optimized "${PHYSX_LIBRARY_RELEASE_DIR}/PhysXCharacterKinematic_static_32.lib" 
	debug "${PHYSX_LIBRARY_DEBUG_DIR}/PhysXCommon_static_32.lib" optimized "${PHYSX_LIBRARY_RELEASE_DIR}/PhysXCommon_static_32.lib"
	debug "${PHYSX_LIBRARY_DEBUG_DIR}/PhysXCooking_static_32.lib" optimized "${PHYSX_LIBRARY_RELEASE_DIR}/PhysXCooking_static_32.lib"
	debug "${PHYSX_LIBRARY_DEBUG_DIR}/PhysXExtensions_static_32.lib" optimized "${PHYSX_LIBRARY_RELEASE_DIR}/PhysXExtensions_static_32.lib"
	debug "${PHYSX_LIBRARY_DEBUG_DIR}/PhysXFoundation_static_32.lib" optimized "${PHYSX_LIBRARY_RELEASE_DIR}/PhysXFoundation_static_32.lib"
	debug "${PHYSX_LIBRARY_DEBUG_DIR}/PhysXPvdSDK_static_32.lib" optimized "${PHYSX_LIBRARY_RELEASE_DIR}/PhysXPvdSDK_static_32.lib"
	debug "${PHYSX_LIBRARY_DEBUG_DIR}/PhysXVehicle_static_32.lib" optimized "${PHYSX_LIBRARY_RELEASE_DIR}/PhysXVehicle_static_32.lib"
	)

# FreeType
message (STATUS "=========== Including FreeType =============")
target_include_directories(${TARGET_NAME} PRIVATE "${CMAKE_SOURCE_DIR}/../third_party/freetype-2.10.1/include")
find_library(FREETYPE_DEBUG_LIB NAMES freetype HINTS "${CMAKE_SOURCE_DIR}/lib/freetype/debug")
find_library(FREETYPE_RELEASE_LIB NAMES freetype HINTS "${CMAKE_SOURCE_DIR}/lib/freetype/release")
target_link_libraries(${TARGET_NAME} 
	debug ${FREETYPE_DEBUG_LIB} optimized ${FREETYPE_RELEASE_LIB})

# SoLoud
message (STATUS "=========== Including SoLoud =============")
target_include_directories(${TARGET_NAME} PRIVATE "${CMAKE_SOURCE_DIR}/../third_party/soloud/include")
find_library(SOLOUD_DEBUG_LIB NAMES soloud_x86_d HINTS "${CMAKE_SOURCE_DIR}/lib/soloud/debug")
find_library(SOLOUD_RELEASE_LIB NAMES soloud_x86 HINTS "${CMAKE_SOURCE_DIR}/lib/soloud/release")
find_library(SOLOUD_STATIC_DEBUG_LIB NAMES soloud_static_x86_d HINTS "${CMAKE_SOURCE_DIR}/lib/soloud/debug")
find_library(SOLOUD_STATIC_RELEASE_LIB NAMES soloud_static_x86 HINTS "${CMAKE_SOURCE_DIR}/lib/soloud/release")
target_link_libraries(${TARGET_NAME} 
	debug ${SOLOUD_DEBUG_LIB} optimized ${SOLOUD_RELEASE_LIB}
	debug ${SOLOUD_STATIC_DEBUG_LIB} optimized ${SOLOUD_STATIC_RELEASE_LIB}
	)
	
# Visual Leak Detector
# See: https://stackoverflow.com/questions/4029587/compiling-32-bit-vs-64-bit-project-using-cmake/15986042
target_include_directories(${TARGET_NAME} PRIVATE "${CMAKE_SOURCE_DIR}/../third_party/Visual Leak Detector/include/")
if( CMAKE_SIZEOF_VOID_P EQUAL 8 )
	message(STATUS "=========== Including 64 Bit VLD =============")
	# Is 64 bit
	find_library(VLD_LIB NAMES vld HINTS "${CMAKE_SOURCE_DIR}/../third_party/Visual Leak Detector/lib/Win64")
	target_link_libraries(${TARGET_NAME} ${VLD_LIB})
else()
	# Is 32 Bit
	message(STATUS "=========== Including 32 Bit VLD =============")
	find_library(VLD_LIB NAMES vld HINTS "${CMAKE_SOURCE_DIR}/../third_party/Visual Leak Detector/lib/Win32")
	target_link_libraries(${TARGET_NAME} ${VLD_LIB})
endif()
