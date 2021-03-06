# CMake build for the Reverie Engine
# To run cmake, add cmake/bin directory to system path
## TODO:
## Implement source_group, for organizing in IDE, see: https://stackoverflow.com/questions/31966135/cmake-source-group-not-working-correctly-with-hierarchical-project-setup

message( STATUS "---------Beginning sub-cmake build of Reverie ---------")

# Project-level setup ====================================================
set(TARGET_NAME "reverie")
set(TEST_TARGET_NAME "reverie_tests")

message (STATUS "--------- Set target ${TARGET_NAME}---------")

set(BUILD_64 CMAKE_SIZEOF_VOID_P EQUAL 8)
if(${BUILD_64})
	message (STATUS "--------- 64-bit build ---------")
	set(ARCH_NAME "x64")
	set(QT_LIBPATH_SUFFIX "_64")
else()
	message (STATUS "--------- 32-bit build ---------")
	set(ARCH_NAME "x86")
	set(QT_LIBPATH_SUFFIX "")
endif()


# Find includes in corresponding build directories
set(CMAKE_INCLUDE_CURRENT_DIR ON)

# Instruct CMake to run moc automatically when needed.
set(CMAKE_AUTOMOC ON) # Saves having to use QT5_WRAP_CPP
set(CMAKE_AUTORCC ON) # Saves having to use QT5_ADD_RESOURCES
set(CMAKE_AUTOUIC ON) # Saves having to use QT5_WRAP_UI


# Set compiler flags =====================================================
# Use C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# TODO: 64 bit
# https://stackoverflow.com/questions/39258250/how-to-detect-if-64-bit-msvc-with-cmake 
# https://stackoverflow.com/questions/28350214/how-to-build-x86-and-or-x64-on-windows-from-command-line-with-cmake
# See: CMAKE_GENERATOR_PLATFORM
# Multiprocess compiler, disable specific warnings
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP /wd4577 /wd4467 /wd26812")
if (MSVC)
	add_compile_options(/MP /bigobj /wd4577 /wd4467 /wd26812 /wd26812)
else()
endif()

# Set paths to find Qt ===================================================
# Find Qt itself
if(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
	# set(QT_MSVC_2017 "C:\\Qt\\5.12\\5.12.2\\msvc2017")
	set(QT_MSVC_2019 "C:/Qt/5.15/5.15.2/msvc2019${QT_LIBPATH_SUFFIX}")
	
    set(CMAKE_PREFIX_PATH ${QT_MSVC_2019})
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
find_package(Qt5 COMPONENTS Multimedia REQUIRED) # Multimedia capabilities
find_package(Qt5 COMPONENTS MultimediaWidgets REQUIRED)
find_package(Qt5 COMPONENTS Widgets REQUIRED) # For widgets

# OpenGL libraries
# See: https://stackoverflow.com/questions/65100749/converting-from-qmake-to-cmake-how-do-i-find-libraries-in-the-same-way/65106458#65106458
find_package(OpenGL REQUIRED) # enforces as a requirement
# find_library(OPENGL_LIB names opengl32) # Also valid
# find_library(GLU_LIB names glu32) # Also valid


# Set directories to find libraries, need to do this before add_executable for desired target
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
message (STATUS "PYTHON VERSION ${PYTHON_VERSION}")

set(PYTHON_LIB "${CMAKE_CURRENT_SOURCE_DIR}/../third_party/python/${PYTHON_VERSION}/${ARCH_NAME}/libs")
# file(TO_CMAKE_PATH "$ENV{PYTHON_LIB}" ENV_PYTHON_LIB)
message(STATUS "Setting link directory ${PYTHON_LIB}")
link_directories(${PYTHON_LIB})

# Build EXE ===============================================================
# Get all source files, headers, resources, and UI (forms) from src subdirectory
# See: https://stackoverflow.com/questions/2110795/how-to-use-all-c-files-in-a-directory-with-the-cmake-build-system

# TODO: Group files https://stackoverflow.com/questions/41078807/cmake-and-visualstudio-group-files-in-solution-explorer

# TODO: Make this automatically when files added, see:
# CONFIGURE_DEPENDS https://cmake.org/cmake/help/v3.12/command/file.html#glob
# Note, may cause errors with moc, I tried this already
message (STATUS "-------- Finding source, header, resource, UI files relative to  ${CMAKE_CURRENT_SOURCE_DIR} ---------")
file(GLOB_RECURSE MY_SOURCES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "src/*.c" "src/*.cpp"  "src/*.h"
"resources/*.qrc" "ui/*.ui")

# Ignore main.cpp in any third party libraries, and any tools folders
list(FILTER MY_SOURCES EXCLUDE REGEX "src/third_party/.*/main.cpp")
list(FILTER MY_SOURCES EXCLUDE REGEX "src/third_party/.*/tools/")
list(FILTER MY_SOURCES EXCLUDE REGEX "src/third_party/soloud/.*/vita_homebrew.*")
# list(FILTER MY_SOURCES EXCLUDE REGEX "src/third_party/soloud/.*/soloud_openmpt.*")

# Get all the source files to build the test suite
file(GLOB_RECURSE MY_TEST_SOURCES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "src/*.c" "src/*.cpp" "src/*.h" "resources/*.qrc" "ui/*.ui" "tests/*.cpp" "tests/*.h")
list(FILTER MY_TEST_SOURCES EXCLUDE REGEX "src/main.cpp")

# Ignore main.cpp in any third party libraries, and any tools folders
list(FILTER MY_TEST_SOURCES EXCLUDE REGEX "src/third_party/.*/main.cpp")
list(FILTER MY_TEST_SOURCES EXCLUDE REGEX "src/third_party/.*/tools/")
list(FILTER MY_TEST_SOURCES EXCLUDE REGEX "src/third_party/soloud/.*/vita_homebrew.*")
# list(FILTER MY_TEST_SOURCES EXCLUDE REGEX "src/third_party/soloud/.*/soloud_openmpt.*")

message (STATUS "=========== FOUND FILES =============")
message (STATUS "${MY_SOURCES}")
message (STATUS "${MY_TEST_SOURCES}")
message (STATUS "=========== END FILES ===============")

# Generate release PDB
# https://stackoverflow.com/questions/28178978/how-to-generate-pdb-files-for-release-build-with-cmake-flags/31264946
add_compile_options("$<$<NOT:$<CONFIG:Debug>>:/Zi>")
add_link_options("$<$<NOT:$<CONFIG:Debug>>:/DEBUG>")
add_link_options("$<$<NOT:$<CONFIG:Debug>>:/OPT:REF>")
add_link_options("$<$<NOT:$<CONFIG:Debug>>:/OPT:ICF>")

# Add an executable to be built from sources variable
# add_executable(${TARGET_NAME} ${MY_SOURCES})
add_executable(${TARGET_NAME} WIN32 ${MY_SOURCES}) # Makes sure that this is a windows application, not a console app
add_executable(${TEST_TARGET_NAME} ${MY_TEST_SOURCES})

# Remove unwanted flags ===================================================
# Doesn't work
# remove_flag_from_target(${TARGET_NAME} -RTC1)

# Pre-processor Defines ===================================================
add_definitions(-D_UNICODE -D_ENABLE_EXTENDED_ALIGNED_STORAGE -DWIN64 -DQT_DLL -DQT_OPENGL_LIB)
# add_definitions(-DQT_NO_DEBUG_OUTPUT) # Thought this would suppress output, it doesn't
add_definitions(-DDEVELOP_MODE)
add_definitions(-DLINALG_USE_EIGEN) # For Eigen
add_definitions(-DWITH_SDL2) # For SoLoud

# Disable warnings about deprecated things, like using strcpy (sorry, not sorry)
if(MSVC)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
endif()

# Command-line configuration ===================================================
set_target_properties(${TARGET_NAME} PROPERTIES VS_DEBUGGER_COMMAND_ARGUMENTS "${CMAKE_CURRENT_SOURCE_DIR}")


# Add compile options to the target =======================================
# target_compile_options(${TARGET_NAME} PUBLIC "$<$<CONFIG:DEBUG>:${MY_DEBUG_OPTIONS}>")
# target_compile_options(${TARGET_NAME} PUBLIC "$<$<CONFIG:RELEASE>:${MY_RELEASE_OPTIONS}>")

########### Iterate over all targets to link libraries #################
foreach(current_target ${TARGET_NAME} ${TEST_TARGET_NAME})

# Add additional include directories for including using src as root =======================================
target_include_directories(${current_target} PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src")

# Set linker properties ===================================================
# By default, cmake makes a console application, not a windows application
# TODO: Do this, and remove /SUBSYSTEM:CONSOLE flag (which is case-insensitive)
# For now, the modified add_executable handles this well
# set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /SUBSYSTEM:WINDOWS")

# Add compile definitions to the target ===================================
# Set options based on debug or release mode
# Generator expression, see: 
# https://stackoverflow.com/questions/34490294/what-does-configdebugrelease-mean-in-cmake
# https://cmake.org/cmake/help/latest/manual/cmake-generator-expressions.7.html
set(DEBUG_DEFINITIONS "DEBUG_MODE")
set(RELEASE_DEFINITIONS "QT_NO_DEBUG_OUTPUT")
target_compile_definitions(${current_target} PUBLIC
    $<$<CONFIG:DEBUG>:${DEBUG_DEFINITIONS}>
    $<$<CONFIG:RELEASE>:${RELEASE_DEFINITIONS}>
)

# Link Required Qt modules to main target =================================
# This MUST come after add_executable and find_package for each module
# Essentially, target_link_libraries links the two specified libraries
# https://stackoverflow.com/questions/25909943/getting-a-cmake-error-cannot-specify-link-libraries-for-target-which-is-not-bui/41262868
target_link_libraries(${current_target} OpenGL::GL OpenGL::GLU)
target_link_libraries(${current_target} Qt5::Core)
target_link_libraries(${current_target} Qt5::Concurrent) 
target_link_libraries(${current_target} Qt5::Gamepad)
target_link_libraries(${current_target} Qt5::Gui)
target_link_libraries(${current_target} Qt5::OpenGLExtensions) # TODO: Probably unnecessary, remove
target_link_libraries(${current_target} Qt5::Multimedia)
target_link_libraries(${current_target} Qt5::MultimediaWidgets)
target_link_libraries(${current_target} Qt5::Widgets)

# Copy DLLs from Qt to bin location
set(QT_BIN_PATH ${CMAKE_PREFIX_PATH}/bin)
set(QT_DEBUG_DLLS "${QT_BIN_PATH}/Qt5Widgetsd.dll" "${QT_BIN_PATH}/Qt5Guid.dll" "${QT_BIN_PATH}/Qt5Cored.dll" 
)
set(QT_RELEASE_DLLS "${QT_BIN_PATH}/Qt5Widgets.dll" "${QT_BIN_PATH}/Qt5Gui.dll" "${QT_BIN_PATH}/Qt5Core.dll")
message (STATUS "Qt debug DLL Paths are: ${QT_DEBUG_DLLS}")
add_custom_command(TARGET ${current_target} PRE_BUILD
		COMMENT "Copy all Qt DLLs to target .exe directory"
		VERBATIM
		COMMAND_EXPAND_LISTS
		COMMAND ${CMAKE_COMMAND} -E copy_if_different 
		"$<$<CONFIG:DEBUG>:${QT_DEBUG_DLLS}>"
		"$<$<CONFIG:RELEASE>:${QT_RELEASE_DLLS}>"
		"$<TARGET_FILE_DIR:${current_target}>")

# Copy Qt platform DLLs
set(QT_PLATFORMS_PATH ${CMAKE_PREFIX_PATH}/plugins/platforms)
set(QT_DEBUG_PLATFORM_DLLS "${QT_PLATFORMS_PATH}/qdirect2dd.dll" "${QT_PLATFORMS_PATH}/qminimald.dll" "${QT_PLATFORMS_PATH}/qoffscreend.dll" "${QT_PLATFORMS_PATH}/qwindowsd.dll")
set(QT_RELEASE_PLATFORM_DLLS "${QT_PLATFORMS_PATH}/qdirect2d.dll" "${QT_PLATFORMS_PATH}/qminimal.dll" "${QT_PLATFORMS_PATH}/qoffscreen.dll" "${QT_PLATFORMS_PATH}/qwindows.dll")
add_custom_command(TARGET ${current_target} PRE_BUILD 
	COMMAND ${CMAKE_COMMAND} -E make_directory 
	"$<TARGET_FILE_DIR:${current_target}>/platforms")
add_custom_command(TARGET ${current_target} PRE_BUILD
		COMMENT "Copy all Qt platform DLLs to target .exe directory"
		VERBATIM
		COMMAND_EXPAND_LISTS
		COMMAND ${CMAKE_COMMAND} -E copy_if_different 
		"$<$<CONFIG:DEBUG>:${QT_DEBUG_PLATFORM_DLLS}>"
		"$<$<CONFIG:RELEASE>:${QT_RELEASE_PLATFORM_DLLS}>"
		"$<TARGET_FILE_DIR:${current_target}>/platforms")

# Add additional libraries ================================================

# Python
message (STATUS "=========== Including Python =============")
# Set up include paths and libraries for Python itself
if(WIN32)
	# for Windows operating system in general
	# if(MSVC OR MSYS OR MINGW) # for detecting Windows compilers
	
	# Convert paths to CMAKE-friendly versions
	set(PYTHON_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../third_party/python/${PYTHON_VERSION}/${ARCH_NAME}")
	# file(TO_CMAKE_PATH "$ENV{PYTHON_PATH}" ENV_PYTHON_PATH)
	
	# Link Python
	# Debug DLL fails to import third party modules, so always use release
	target_include_directories(${current_target} PRIVATE "${PYTHON_PATH}/include")
	message(STATUS "Python include path for windows is ${PYTHON_PATH}/include")
	message(STATUS "Python path is ${PYTHON_PATH}")
	# find_library(PYTHON_DEBUG_LIB NAMES "python${PYTHON_VERSION}_d" HINTS ${PYTHON_LIB})
	find_library(PYTHON_RELEASE_LIB NAMES "python${PYTHON_VERSION}" HINTS ${PYTHON_LIB})
	# message(STATUS "Linking Python with debug path ${PYTHON_DEBUG_LIB} and release path ${PYTHON_RELEASE_LIB}")
	target_link_libraries(${current_target}
		debug ${PYTHON_RELEASE_LIB} optimized ${PYTHON_RELEASE_LIB}
	)
	
	# Set environmental variables to find site-packages and python lib
	set(PYTHON_LIB_PATH ${PYTHON_PATH}/Lib)
	add_custom_target(TARGET${current_target} 
		COMMAND ${CMAKE_COMMAND} -E env PYTHONPATH="${PYTHON_LIB_PATH};${PYTHON_LIB_PATH}/site-packages")
	
elseif(APPLE)
	# TODO: Set Python path and get DLL
	# for MacOS X or iOS, watchOS, tvOS (since 3.10.3)
	message (STATUS "BUILDING FOR APPLE")
	# Include library directory
	target_include_directories(${current_target} PRIVATE "/System/Library/Frameworks/Python.framework/Headers")
	
	# Find library with name Python, and link
	find_library(PYTHON_LIBRARY_PATH NAMES "Python")
	target_link_libraries(${current_target}
		${PYTHON_LIBRARY_PATH}
	)
elseif(UNIX AND NOT APPLE)
	# TODO: Set Python path and get DLL
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
		target_link_libraries(${current_target} ${PYTHON_LIBRARY_PATH})
	endif()
else()
	message( WARNING "Unrecognized operating system")
endif()

# Copy DLL
file(COPY "${PYTHON_PATH}/python${PYTHON_VERSION}.dll" DESTINATION "${CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG}")
file(COPY "${PYTHON_PATH}/python${PYTHON_VERSION}.dll" DESTINATION "${CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE}")

# PYBIND
message (STATUS "=========== Including PYBIND =============")
target_include_directories(${current_target} PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/../third_party/pybind11/include")

# EIGEN 
message (STATUS "=========== Including Eigen at ${CMAKE_CURRENT_SOURCE_DIR}/src/third_party/eigen =============")
target_include_directories(${current_target} PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src/third_party/eigen")


# ASSIMP
message (STATUS "=========== Including ASSIMP =============")
target_include_directories(${current_target} PRIVATE 
"${CMAKE_CURRENT_SOURCE_DIR}/../third_party/assimp/assimp-5.0.0/include" 
"${CMAKE_CURRENT_SOURCE_DIR}/../third_party/assimp/assimp-5.0.0/BINARIES/Win32/include")

target_link_libraries(${current_target} assimp)

# PhysX
message (STATUS "=========== Including PhysX as a static library =============")
target_include_directories(${current_target} PRIVATE 
"${CMAKE_CURRENT_SOURCE_DIR}/../third_party/physx/physx.submodule/physx/include" 
"${CMAKE_CURRENT_SOURCE_DIR}/../third_party/physx/physx.submodule/pxshared/include")
set( PHYSX_LIBRARIES
	PhysXExtensions
	PhysX
	PhysXPvdSDK
	PhysXVehicle
	PhysXCharacterKinematic
	PhysXCooking
	PhysXCommon
	PhysXFoundation
	# SnippetUtils
)

target_link_libraries(${current_target} ${PHYSX_LIBRARIES})

#### Windows only: Copy the Physx dll files to the simulation executable####
if (WIN32)
	# References NvidiaBuildOptions.cmake to figure out if system is 32/64 bit
	IF(${BUILD_64})
		SET(LIBPATH_SUFFIX "64")
	ELSE()
		SET(LIBPATH_SUFFIX "32")
	ENDIF()
	GetPlatformBinName(PLATFORM_BIN_NAME ${LIBPATH_SUFFIX})
	set(PhysxOutputPath ${PX_OUTPUT_LIB_DIR}/bin/${PLATFORM_BIN_NAME}/)
	message("Physx Output Path: " ${PhysxOutputPath})

	# copy PhysX dll's to build dir. Happens on every build.
	# See: https://stackoverflow.com/questions/13429656/how-to-copy-contents-of-a-directory-into-build-directory-after-make-with-cmake
	add_custom_command(TARGET ${current_target} POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy_directory "${PhysxOutputPath}/$<CONFIG>" "$<TARGET_FILE_DIR:${current_target}>/lib")
endif()

######## FreeType
message (STATUS "=========== Including FreeType =============")
target_include_directories(${current_target} PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/../third_party/freetype/freetype.submodule/include")
target_link_libraries(${current_target} freetype)

######### SoLoud
message (STATUS "=========== Including SoLoud =============")
target_include_directories(${current_target} PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src/third_party/soloud/include" "${CMAKE_CURRENT_SOURCE_DIR}/src/third_party/SDL2")

# Apparently, SDL has no debug version, weird 
# See: https://wiki.libsdl.org/Installation
find_library(SDL_LIB NAMES SDL2 HINTS "${CMAKE_CURRENT_SOURCE_DIR}/lib/sdl2/${ARCH_NAME}")
target_link_libraries(${current_target} 
	debug ${SDL_LIB} optimized ${SDL_LIB})
	
######## Color widgets
message (STATUS "=========== Including Color Widgets =============")
target_include_directories(${current_target} PRIVATE "../third_party/color_widgets/include")
target_link_libraries(${current_target} QtColorWidgets)


######## Visual Leak Detector
message(STATUS "=========== Including VLD =============")
target_include_directories(${current_target} PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/../third_party/Visual Leak Detector/include/")

if( ${BUILD_64})
	# Is 64 bit
	set(VLD_LIB_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../third_party/Visual Leak Detector/lib/Win64")
else()
	# Is 32 Bit
	set(VLD_LIB_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../third_party/Visual Leak Detector/lib/Win32")
endif()
find_library(VLD_LIB NAMES vld HINTS ${VLD_LIB_PATH})
target_link_libraries(${current_target} ${VLD_LIB})


endforeach()