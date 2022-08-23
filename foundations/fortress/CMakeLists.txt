# Project-level setup ====================================================
set(TARGET_NAME "fortress")
message( STATUS "\n\n*********************** Beginning sub-cmake build of ${TARGET_NAME} ***********************")
message (STATUS "--------- Set target ${TARGET_NAME}---------")

include(common_functions)

check_architecture()
set_cpp_version() # Set to C++ 17
set_compiler_options()
setup_python()

######## Build library 
# Get all source files, headers, resources, and UI (forms) from src subdirectory
# TODO: Group files https://stackoverflow.com/questions/41078807/cmake-and-visualstudio-group-files-in-solution-explorer

# TODO: Make this automatically when files added, see:
# CONFIGURE_DEPENDS https://cmake.org/cmake/help/v3.12/command/file.html#glob
# Note, may cause errors with moc, I tried this already
message (STATUS "-------- Finding source, header files relative to ${CMAKE_CURRENT_SOURCE_DIR} ---------")
file(GLOB_RECURSE MY_SOURCES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} 
     "src/*.c" "src/*.cpp" "include/*.c"  "include/*.h" "include/*.hpp")

# message (STATUS "=========== FOUND FILES =============")
# message (STATUS "${MY_SOURCES}")
# message (STATUS "=========== END FILES ===============")

# Generate release PDB
generate_release_pdb()

# Add an library to be built from sources variable
add_library(${TARGET_NAME} STATIC ${MY_SOURCES})

add_definitions(-D_UNICODE -D_ENABLE_EXTENDED_ALIGNED_STORAGE -DWIN64)
add_definitions(-DDEVELOP_MODE)
add_definitions(-DLINALG_USE_EIGEN) # For Eigen

# Disable warnings about deprecated things, like using strcpy (sorry, not sorry)
if(MSVC)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
endif()

#### Command-line configuration 
set_target_properties(${TARGET_NAME} PROPERTIES VS_DEBUGGER_COMMAND_ARGUMENTS "${CMAKE_CURRENT_SOURCE_DIR}")

########### Iterate over all targets to link libraries #################
include(add_common_libraries)

foreach(current_target ${TARGET_NAME})

#### Add additional include directories for including using src as root 
# Public so that applications have access
target_include_directories(${TARGET_NAME} PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")

#### Add compile definitions to the target 
set(DEBUG_DEFINITIONS "DEBUG_MODE")
set(RELEASE_DEFINITIONS "")
target_compile_definitions(${current_target} PUBLIC
    $<$<CONFIG:DEBUG>:${DEBUG_DEFINITIONS}>
    $<$<CONFIG:RELEASE>:${RELEASE_DEFINITIONS}>
)

######## Link required libraries

#### Python
add_library_python(${current_target})

#### Eigen
add_library_eigen(${current_target})

#### Json
target_link_libraries(${current_target} PUBLIC nlohmann_json::nlohmann_json)

endforeach()

######## Fortress tests
add_subdirectory(tests)
