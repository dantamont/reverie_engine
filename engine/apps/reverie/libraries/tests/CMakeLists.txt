message( STATUS "\n\n*********************** Beginning sub-cmake build of Reverie tests ***********************")

#### Project-level setup
set(TARGET_NAME "reverie_tests")

message (STATUS "--------- Set target ${TARGET_NAME}---------")

check_architecture() # Check if architecture is 64 or 32 bit
set_cpp_version() # Set to C++ 17
set_compiler_options()

# TODO: Make this automatically when files added, see:
# CONFIGURE_DEPENDS https://cmake.org/cmake/help/v3.12/command/file.html#glob
message (STATUS "-------- Finding source, header files relative to  ${CMAKE_CURRENT_SOURCE_DIR} ---------")
file(GLOB_RECURSE MY_TEST_SOURCES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "*.c" "*.cpp"  "*.hpp" "*.h")

message (STATUS "=========== FOUND FILES =============")
message (STATUS "Reverie Test sources:\n ${MY_TEST_SOURCES}\n")
message (STATUS "=========== END FILES ===============")


# Add an executable to be built from sources variable
add_executable(
    ${TARGET_NAME} 
    ${MY_TEST_SOURCES}
)

# Add solution directory to find in project
add_compile_definitions(_SOLUTION_DIR="${REVERIE_DIR}")

# Add test project directory
add_compile_definitions(_REVERIE_TEST_DIR="${CMAKE_CURRENT_SOURCE_DIR}")

# Disable warnings about deprecated things, like using strcpy (sorry, not sorry)
if(MSVC)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
endif()

# Command-line configuration ===================================================
set_target_properties(${TARGET_NAME} PROPERTIES VS_DEBUGGER_COMMAND_ARGUMENTS "${REVERIE_DIR}")


######## Link libraries
target_link_libraries(
    ${TARGET_NAME}
    PUBLIC reverie_lib
    PRIVATE gtest # Google test
)
