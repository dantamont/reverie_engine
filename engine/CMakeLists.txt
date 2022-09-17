# CMake build for the Reverie Engine
## TODO:
## Implement source_group, for organizing in IDE, see: https://stackoverflow.com/questions/31966135/cmake-source-group-not-working-correctly-with-hierarchical-project-setup
## Also see: https://stackoverflow.com/questions/53772946/single-cmakelists-txt-enough-for-my-project

#### Project-level setup 
cmake_minimum_required( VERSION 3.20 )

# Load in cmake utilities 
set(
    CMAKE_MODULE_PATH 
    "${CMAKE_SOURCE_DIR}/cmake" 
    "${CMAKE_SOURCE_DIR}/cmake/VersionFromGit" 
    ${CMAKE_MODULE_PATH})
include( MunkeiVersionFromGit )

message( STATUS "Configuring preset: ${PRESET_NAME}")
message(STATUS "Building in ${CMAKE_CONFIGURATION_TYPES} Modes")

# Get project version
version_from_git(
  LOG       ON
  TIMESTAMP "%Y%m%d%H%M%S"
)
message( STATUS "============================= Version Info =============================")
message( STATUS "Git Tag: ${GIT_TAG}")
message( STATUS "Full semantic version: ${SEMVER}")
message( STATUS "Version: ${VERSION}")
message( STATUS "Major Version: ${VERSION_MAJOR}")
message( STATUS "Minor Version: ${VERSION_MINOR}")
message( STATUS "Patch Version: ${VERSION_PATCH}")
message( STATUS "============================= End Version Info =============================")

# Create project
# C language is set for GLFW
project( 
    Reverie
    VERSION ${VERSION}
) 
message( STATUS "---------Beginning cmake build of Reverie Engine---------")

#### Set root path for auto-generated code for sub-builds
set(AUTOGEN_ROOT_PATH ${CMAKE_BINARY_DIR}/autogen CACHE INTERNAL "")

#### Set path for kindler (auto-generation) tool
set(KINDLER_PATH ${CMAKE_SOURCE_DIR}/tools/kindler CACHE INTERNAL "")

#### Set extern path for sub-builds
set(PATH_TO_EXTERN "${PROJECT_SOURCE_DIR}/extern")

#### Set build mode directory options 
# Make it such that all DLLs/exes from all subdirectories are put in the same folder location
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/Debug)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/Release)

# Make it such that all .libs from all subdirectories are put in the same folder location
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_DEBUG "${CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG}/lib")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_RELEASE "${CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE}/lib")

# Make it such that all static libs from all subdirectories are put in the same folder location
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_DEBUG "${CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG}/lib")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELEASE "${CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE}/lib")

#### Build all external libraries and sub-projects 
add_subdirectory(extern)

######## Logic for my code

#### Set options for all projects
add_compile_definitions(
    WIN32_LEAN_AND_MEAN # To fix compatibility issues with Qt and asio
)

#### Run cmake for common code
add_subdirectory(foundations)

#### Run cmake for all applications
add_subdirectory(apps)
