# CMake build for the Reverie Engine library
# To run cmake, add cmake/bin directory to system path
## TODO:
## Implement source_group, for organizing in IDE, see: https://stackoverflow.com/questions/31966135/cmake-source-group-not-working-correctly-with-hierarchical-project-setup

message( STATUS "\n\n*********************** Beginning sub-cmake build of Reverie Library ***********************")

#### Project-level setup
set(TARGET_NAME "reverie_lib")

message (STATUS "--------- Set target ${TARGET_NAME}---------")

include(common_functions)
include(add_common_libraries)

check_architecture() # Check if architecture is 64 or 32 bit
setup_qt_project() # Set project-level cmake variables for Qt
set_cpp_version() # Set to C++ 17
set_compiler_options()
setup_opengl()
setup_python()

#### Build library
# Get all source files, headers, resources, and UI (forms) from src subdirectory
# See: https://stackoverflow.com/questions/2110795/how-to-use-all-c-files-in-a-directory-with-the-cmake-build-system

# TODO: Group files https://stackoverflow.com/questions/41078807/cmake-and-visualstudio-group-files-in-solution-explorer

# TODO: Make this automatically when files added, see:
# CONFIGURE_DEPENDS https://cmake.org/cmake/help/v3.12/command/file.html#glob
# Note, may cause errors with moc, I tried this already
set(REVERIE_DIR ${CMAKE_CURRENT_SOURCE_DIR})
message (STATUS "-------- Finding source, header, resource, UI files relative to  ${REVERIE_DIR} ---------")
file(GLOB_RECURSE MY_SOURCES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "${REVERIE_DIR}/src/*.c" "${REVERIE_DIR}/src/*.cpp" "${REVERIE_DIR}/include/*.h" "${REVERIE_DIR}/src/*.h"  "${REVERIE_DIR}/src/*.hpp" "${REVERIE_DIR}/resources/*.qrc" "${REVERIE_DIR}/ui/*.ui")

# Ignore main.cpp in any third party libraries, and any tools folders
list(FILTER MY_SOURCES EXCLUDE REGEX "third_party/.*/main.cpp")
list(FILTER MY_SOURCES EXCLUDE REGEX "third_party/.*/tools/")
list(FILTER MY_SOURCES EXCLUDE REGEX "third_party/soloud/.*/vita_homebrew.*")

generate_release_pdb()

set(REVERIE_LIB_DIR "${REVERIE_DIR}" CACHE FILEPATH "Path to the reverie library")

# message (STATUS "=========== FOUND FILES =============")
# message (STATUS "Sources:\n ${MY_SOURCES}\n")
# message (STATUS "=========== END FILES ===============")


# Create library from sources
add_library(${TARGET_NAME} STATIC ${MY_SOURCES}) #TODO: Investigate why shared library results in linker errors

#### Pre-processor Defines 
target_compile_definitions(${TARGET_NAME} 
    PUBLIC
    _UNICODE
    _ENABLE_EXTENDED_ALIGNED_STORAGE
    WIN64
    QT_DLL
    QT_OPENGL_LIB
    DEVELOP_MODE
    WITH_SDL2
    LINALG_USE_EIGEN
)

# Add solution directory to find in project
add_compile_definitions(_SOLUTION_DIR="${REVERIE_DIR}")

# Disable warnings about deprecated things, like using strcpy (sorry, not sorry)
if(MSVC)
    target_compile_definitions(${TARGET_NAME} 
        PUBLIC
        _CRT_SECURE_NO_WARNINGS
    )
endif()

#### Command-line configuration 
set_target_properties(${TARGET_NAME} PROPERTIES VS_DEBUGGER_COMMAND_ARGUMENTS "${REVERIE_DIR}/reverie.json" )

#### Add compile options to the target 
# target_compile_options(${TARGET_NAME} PUBLIC "$<$<CONFIG:Debug>:${MY_DEBUG_OPTIONS}>")
# target_compile_options(${TARGET_NAME} PUBLIC "$<$<CONFIG:Release>:${MY_RELEASE_OPTIONS}>")

#### Add include directories
target_include_directories(${TARGET_NAME} PUBLIC "${REVERIE_DIR}/include")

######## Link libraries used by application

#### Add compile definitions to the target 
# Set options based on debug or release mode
# Generator expression, see: 
# https://stackoverflow.com/questions/34490294/what-does-configdebugrelease-mean-in-cmake
# https://cmake.org/cmake/help/latest/manual/cmake-generator-expressions.7.html
set(DEBUG_DEFINITIONS "DEBUG_MODE")
set(RELEASE_DEFINITIONS "QT_NO_DEBUG_OUTPUT")
target_compile_definitions(${TARGET_NAME} PUBLIC
    $<$<CONFIG:Debug>:${DEBUG_DEFINITIONS}>
    $<$<CONFIG:Release>:${RELEASE_DEFINITIONS}>
)

#### Add Qt
add_library_qt(${TARGET_NAME})

### For Auto-UI locations to be found by dependent libraries
# See: https://stackoverflow.com/questions/60920473/cant-include-ui-file-from-linked-library-using-cmakes-autouic
target_include_directories(
    ${TARGET_NAME}
    PUBLIC
    $<$<CONFIG:Debug>:${CMAKE_CURRENT_BINARY_DIR}/reverie_lib_autogen/include_debug>
    $<$<CONFIG:Release>:${CMAKE_CURRENT_BINARY_DIR}/reverie_lib_autogen/include_release>
)

######## Add additional libraries 

target_link_libraries(
    ${TARGET_NAME} 
    PUBLIC 
    fortress # Includes python and eigen
    font_lib # font rendering
    heave #physics
    scribe # logging
    ripple # networking
)

# PYBIND
# TODO: Link to pybind11_headers library instead, not sure why this currently works
message (STATUS "=========== Including PYBIND =============")
target_include_directories(${TARGET_NAME} PRIVATE "${PATH_TO_EXTERN}/pybind11/pybind11.submodule/include")

# ASSIMP
message (STATUS "=========== Including ASSIMP =============")
target_include_directories(${TARGET_NAME} PRIVATE 
"${PATH_TO_EXTERN}/assimp/assimp-5.0.0/include" 
"${PATH_TO_EXTERN}/assimp/assimp-5.0.0/BINARIES/Win32/include")

target_link_libraries(${TARGET_NAME} PUBLIC assimp)

# PhysX
message (STATUS "=========== Including PhysX as a static library =============")
target_include_directories(${TARGET_NAME} PRIVATE 
"${PATH_TO_EXTERN}/physx/physx.submodule/physx/include" 
"${PATH_TO_EXTERN}/physx/physx.submodule/pxshared/include")
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

target_link_libraries(${TARGET_NAME} PUBLIC ${PHYSX_LIBRARIES})

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
    add_custom_command(TARGET ${TARGET_NAME} 
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory "${PhysxOutputPath}/$<CONFIG>" "$<TARGET_FILE_DIR:${TARGET_NAME}>/lib"
    )
endif()

######### SoLoud
message (STATUS "=========== Including SoLoud =============")
target_include_directories(${TARGET_NAME} PRIVATE "${REVERIE_DIR}/src/third_party/soloud/include" "${REVERIE_DIR}/src/third_party/SDL2")

# Apparently, SDL has no debug version, weird 
# See: https://wiki.libsdl.org/Installation
# Maybe use vcpkg?
find_library(SDL_LIB NAMES SDL2 HINTS "${REVERIE_DIR}/lib/sdl2/${ARCH_NAME}")
target_link_libraries(${TARGET_NAME} 
    LINK_PUBLIC
    debug ${SDL_LIB} optimized ${SDL_LIB})

    
#### Geppetto
target_link_libraries(${TARGET_NAME} PUBLIC "geppetto")

######## Visual Leak Detector
message(STATUS "=========== Including VLD =============")
target_include_directories(${TARGET_NAME} PRIVATE "${PATH_TO_EXTERN}/Visual Leak Detector/include/")

if( ${BUILD_64})
    # Is 64 bit
    set(VLD_LIB_PATH "${PATH_TO_EXTERN}/Visual Leak Detector/lib/Win64")
else()
    # Is 32 Bit
    set(VLD_LIB_PATH "${PATH_TO_EXTERN}/Visual Leak Detector/lib/Win32")
endif()
find_library(VLD_LIB NAMES vld HINTS ${VLD_LIB_PATH})
target_link_libraries(${TARGET_NAME} PUBLIC ${VLD_LIB})



############### Add tests project
add_subdirectory(tests)