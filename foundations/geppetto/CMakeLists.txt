# Project-level setup ====================================================
set(TARGET_NAME "geppetto")
message( STATUS "\n\n*********************** Beginning sub-cmake build of ${TARGET_NAME} ***********************")
message (STATUS "--------- Set target ${TARGET_NAME}---------")

include(common_functions)
include(add_common_libraries)

check_architecture()
setup_qt_project() # Set project-level cmake variables for Qt
set_cpp_version() # Set to C++ 17
set_compiler_options()
setup_opengl()
generate_release_pdb() # Nice for debugging in Visual Studio

#### Build library 
# Add an library to be built from sources variable
file(GLOB_RECURSE MY_SOURCES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} 
     "src/*.c" "src/*.cpp" "include/*.c"  "include/*.h" "include/*.hpp")
    
add_library(${TARGET_NAME} STATIC ${MY_SOURCES})

add_definitions(-D_UNICODE -D_ENABLE_EXTENDED_ALIGNED_STORAGE -DWIN64 -DQT_DLL -DQT_OPENGL_LIB)
add_definitions(-DDEVELOP_MODE)

# Disable warnings about deprecated things, like using strcpy (sorry, not sorry)
if(MSVC)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
endif()

#### Command-line configuration 
set_target_properties(${TARGET_NAME} PROPERTIES VS_DEBUGGER_COMMAND_ARGUMENTS "${CMAKE_CURRENT_SOURCE_DIR}")

## Add additional include directories for including using src as root 
# Public so that applications have access
target_include_directories(${TARGET_NAME} PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")

## Add compile definitions to the target 
set(COMMON_DEFINITIONS "LINALG_USE_EIGEN") # Use Eigen
set(DEBUG_DEFINITIONS "DEBUG_MODE" ${COMMON_DEFINITIONS})
set(RELEASE_DEFINITIONS ${COMMON_DEFINITIONS})
target_compile_definitions(${TARGET_NAME} PUBLIC
    $<$<CONFIG:Debug>:${DEBUG_DEFINITIONS}>
    $<$<CONFIG:Release>:${RELEASE_DEFINITIONS}>
)

#### Link required libraries

## Add Qt
add_library_qt(${TARGET_NAME})

# Physics library for transform object
target_link_libraries(
    ${TARGET_NAME} 
    PRIVATE 
    heave
    scribe
    PUBLIC 
    QtColorWidgets
    fortress
    ripple
    font_lib
)

######### Add tests subproject
add_subdirectory(tests)