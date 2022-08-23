# CMake build for the Reverie Engine executable

message( STATUS "\n\n*********************** Beginning sub-cmake build of Reverie ***********************")

#### Project-level setup
set(TARGET_NAME "reverie")
check_architecture() # Check if architecture is 64 or 32 bit
set_cpp_version() # Set to C++ 17
set_compiler_options()

#### Build EXE
add_executable(${TARGET_NAME} WIN32 "src/main.cpp") # Makes sure that this is a windows application, not a console app

#### Command-line configuration 
message(STATUS "Configuring to start VS Debugger with argument: ${REVERIE_LIB_DIR}")
set_target_properties(${TARGET_NAME} PROPERTIES VS_DEBUGGER_COMMAND_ARGUMENTS "${REVERIE_LIB_DIR}" )

#### Link libraries
target_link_libraries(
    ${TARGET_NAME}
    PUBLIC reverie_lib
)

#### Copy DLLs to built executable location
include(common_functions)
copy_qt_dlls(${TARGET_NAME})

# Copy SDL dll to build dir for SoLoud. Happens on every build.
# \todo: Need to also copy lib for deployment. Not sure how to link to a lib when it will be moved for deployment, maybe pre_build or pre_link will work
add_custom_command(TARGET ${TARGET_NAME} 
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different "${REVERIE_LIB_DIR}/lib/sdl2/${ARCH_NAME}/SDL2.dll" "$<TARGET_FILE_DIR:${TARGET_NAME}>"
)
