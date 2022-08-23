# CMake build for Eigen library
## TODO:
## Implement source_group, for organizing in IDE, see: https://stackoverflow.com/questions/31966135/cmake-source-group-not-working-correctly-with-hierarchical-project-setup

message( STATUS "\n\n*********************** Beginning sub-cmake build of Eigen ***********************")

# Project-level setup ====================================================
set(TARGET_NAME "eigen")

message (STATUS "--------- Set target ${TARGET_NAME}---------")

include(common_functions)

check_architecture()
set_cpp_version() # Set to C++ 17
set_compiler_options()
set_eigen_vectorization()

######## Build library 
# TODO: Make this automatically when files added, see:
# CONFIGURE_DEPENDS https://cmake.org/cmake/help/v3.12/command/file.html#glob
# Note, may cause errors with moc, I tried this already
message (STATUS "-------- Finding source, header files ---------")
file(GLOB_RECURSE MY_SOURCES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "include/*.hpp"  "include/*.h")

# message (STATUS "=========== FOUND FILES =============")
# message (STATUS "${MY_SOURCES}")
# message (STATUS "=========== END FILES ===============")

# Generate release PDB
generate_release_pdb()

#### Add a library to be built from sources variable
# Header only, so make an interface library
add_library(${TARGET_NAME} INTERFACE ${MY_SOURCES})

add_definitions(-D_UNICODE -D_ENABLE_EXTENDED_ALIGNED_STORAGE -DWIN64)

# Disable warnings about deprecated things, like using strcpy (sorry, not sorry)
if(MSVC)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
endif()

#### Command-line configuration 
set_target_properties(${TARGET_NAME} PROPERTIES VS_DEBUGGER_COMMAND_ARGUMENTS "${CMAKE_CURRENT_SOURCE_DIR}")
set_target_properties(${TARGET_NAME} PROPERTIES LINKER_LANGUAGE CXX)


#### Add additional include directories for including using src as root 
# Public so that applications have access
target_include_directories(${TARGET_NAME} INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/include")

#### Add compile definitions to the target 
# Set options based on debug or release mode
set(DEBUG_DEFINITIONS "DEBUG_MODE")
set(RELEASE_DEFINITIONS "")
target_compile_definitions(${TARGET_NAME} INTERFACE
    $<$<CONFIG:DEBUG>:${DEBUG_DEFINITIONS}>
    $<$<CONFIG:RELEASE>:${RELEASE_DEFINITIONS}>
)