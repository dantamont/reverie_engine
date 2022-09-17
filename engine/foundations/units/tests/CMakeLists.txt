#### Project-level setup
set(BASE_TARGET_NAME "units")
message( STATUS "\n\n*********************** Beginning sub-cmake build of ${BASE_TARGET_NAME} tests ***********************")
set(TARGET_NAME "${BASE_TARGET_NAME}_tests")

message (STATUS "--------- Set target ${TARGET_NAME}---------")

include(common_functions)

check_architecture() # Check if architecture is 64 or 32 bit
set_cpp_version() # Set to C++ 17
set_compiler_options()
# setup_opengl()
# setup_python()

#### Build EXE 
# Get all source files, headers from source directories

# TODO: Make this automatically when files added, see:
# CONFIGURE_DEPENDS https://cmake.org/cmake/help/v3.12/command/file.html#glob
message (STATUS "-------- Finding source, header files relative to  ${CMAKE_CURRENT_SOURCE_DIR} ---------")
file(GLOB_RECURSE MY_TEST_SOURCES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "*.c" "*.cpp"  "*.hpp" "*.h")

message (STATUS "=========== FOUND FILES =============")
message (STATUS "Test sources:\n ${MY_TEST_SOURCES}\n")
message (STATUS "=========== END FILES ===============")

# Add an executable to be built from sources variable
add_executable(${TARGET_NAME} ${MY_TEST_SOURCES})

# Pre-processor Defines ===================================================
add_definitions(-D_UNICODE -D_ENABLE_EXTENDED_ALIGNED_STORAGE -DWIN64 -DQT_DLL -DQT_OPENGL_LIB)
add_definitions(-DDEVELOP_MODE)

# Add solution directory to find in project
set(REVERIE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../../apps/reverie)

# Add test project directory
add_compile_definitions(_MY_TEST_DIR="${CMAKE_CURRENT_SOURCE_DIR}")

# Disable warnings about deprecated things, like using strcpy (sorry, not sorry)
if(MSVC)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
endif()

# Command-line configuration ===================================================
set_target_properties(${TARGET_NAME} PROPERTIES VS_DEBUGGER_COMMAND_ARGUMENTS "${CMAKE_CURRENT_SOURCE_DIR}")


#### Add additional include directories for including using src as root 
set(DEBUG_DEFINITIONS "DEBUG_MODE")
set(RELEASE_DEFINITIONS "QT_NO_DEBUG_OUTPUT")
target_compile_definitions(${TARGET_NAME} PUBLIC
    $<$<CONFIG:Debug>:${DEBUG_DEFINITIONS}>
    $<$<CONFIG:Release>:${RELEASE_DEFINITIONS}>
)

######## Add additional libraries 
# Google test
target_link_libraries(${TARGET_NAME} PRIVATE gtest)

# Base libary
target_link_libraries(${TARGET_NAME} PRIVATE ${BASE_TARGET_NAME})