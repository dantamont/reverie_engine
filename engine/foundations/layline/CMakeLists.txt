# Project-level setup ====================================================
set(TARGET_NAME "layline")
message( STATUS "\n\n*********************** Beginning sub-cmake build of ${TARGET_NAME} ***********************")
message (STATUS "--------- Set target ${TARGET_NAME}---------")

include(common_functions)
include(add_common_libraries)

check_architecture()
set_cpp_version() # Set to C++ 17
set_compiler_options()
setup_python()

#### Build library
# Add an library to be built from sources variable
file(GLOB_RECURSE MY_SOURCES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} 
     "src/*.c" "src/*.cpp" "include/*.c"  "include/*.h" "include/*.hpp")
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

## Add additional include directories for including using src as root 
# Public so that applications have access
target_include_directories(${TARGET_NAME} PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")
target_include_directories(${TARGET_NAME} PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/extern/asio/include")

## Add compile definitions to the target 
set(DEBUG_DEFINITIONS "DEBUG_MODE")
set(RELEASE_DEFINITIONS "")
target_compile_definitions(${TARGET_NAME} PUBLIC
    $<$<CONFIG:Debug>:${DEBUG_DEFINITIONS}>
    $<$<CONFIG:Release>:${RELEASE_DEFINITIONS}>
)

#### Link required libraries

target_link_libraries(
    ${TARGET_NAME} 
    PUBLIC 
       fortress
       scribe
)


######### Add tests subproject
add_subdirectory(tests)