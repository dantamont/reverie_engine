# CMake build for the Reverie Engine
# To run cmake, add cmake/bin directory to system path
## TODO:
## Implement source_group, for organizing in IDE, see: https://stackoverflow.com/questions/31966135/cmake-source-group-not-working-correctly-with-hierarchical-project-setup
## Also see: https://stackoverflow.com/questions/53772946/single-cmakelists-txt-enough-for-my-project

message( STATUS "---------Beginning cmake build of Reverie Engine---------")

# Project-level setup =====================================================
project( Reverie ) # TODO: Set version
cmake_minimum_required( VERSION 3.16 )

# Set build mode directory options ========================================
# Make it such that all DLLs/exes from all subdirectories are put in the same folder location
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/Debug)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/Release)

# Make it such that all .libs from all subdirectories are put in the same folder location
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_DEBUG "${CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG}/lib")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_RELEASE "${CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE}/lib")

# Make it such that all static libs from all subdirectories are put in the same folder location
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_DEBUG "${CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG}/lib")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELEASE "${CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE}/lib")

# Build all libraries and sub-projects ====================================
# Set folders
set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set(DEPENDENCY_FOLDER Dependencies)

# Make sure there is NO CMAKE_DEBUG_POSTFIX
SET(CMAKE_DEBUG_POSTFIX "" CACHE STRING "Debug Postfix for lib, samples and tools") # Important to avoid having a d at the end of all libraries. See assimp top-level CMakeLists.txt, line 353

#### Build color widgets
add_subdirectory(third_party/color_widgets)

set_target_properties (QtColorWidgets PROPERTIES
    FOLDER ${DEPENDENCY_FOLDER}/ColorWidgets
)


#### Build physx
# This is not an intended PhysX use-case, but the road has been paved. 
# See: https://github.com/NVIDIAGameWorks/PhysX/issues/249
set(PHYSX_VS_FOLDER_NAME ${DEPENDENCY_FOLDER}/PhysX)
set(PHYSX_ROOT_DIR ${CMAKE_SOURCE_DIR}/third_party/physx/physx.submodule/physx)
set(PHYSX_INCLUDE_DIRS ${PHYSX_ROOT_DIR}/include/ ${PHYSX_ROOT_DIR}/../pxshared/include/ )
set(TARGET_BUILD_PLATFORM "windows") # has to match the TARGET_BUILD_PLATFORM in PHYSX_ROOT_DIR/physx/buildtools/cmake_generate_projects.py
set(PX_BUILDSNIPPETS OFF CACHE BOOL "Generate the snippets")
set(PX_BUILDPUBLICSAMPLES OFF CACHE BOOL "Generate the samples projects")
set(PX_GENERATE_STATIC_LIBRARIES ON CACHE BOOL "Generate static libraries")
set(PX_FLOAT_POINT_PRECISE_MATH OFF CACHE BOOL "Float point precise math. If off, prefers speed")
set(NV_USE_STATIC_WINCRT OFF CACHE BOOL "Use the statically linked windows CRT")
set(NV_USE_DEBUG_WINCRT ON CACHE BOOL "Use the debug version of the CRT")
set(PXSHARED_PATH ${PHYSX_ROOT_DIR}/../pxshared)
set(PXSHARED_INSTALL_PREFIX ${CMAKE_INSTALL_PREFIX})
set(CMAKEMODULES_VERSION "1.27")
set(CMAKEMODULES_PATH ${PHYSX_ROOT_DIR}/../externals/cmakemodules)
set(PX_OUTPUT_LIB_DIR ${CMAKE_CURRENT_BINARY_DIR}/third_party/physx)
set(PX_OUTPUT_BIN_DIR ${CMAKE_CURRENT_BINARY_DIR}/third_party/physx)
add_subdirectory(${PHYSX_ROOT_DIR}/compiler/public third_party/physx)

#### Build ASSIMP
set(ZLIB_LIBRARIES zlibstatic)
set(ENABLE_BOOST_WORKAROUND ON)
set(BUILD_STATIC_LIB ON)
set(ASSIMP_BUILD_ASSIMP_TOOLS OFF CACHE BOOL "")
set(ASSIMP_BUILD_TESTS OFF CACHE BOOL "")
set(ASSIMP_BUILD_STATIC_LIB ON) 
set(ASSIMP_INJECT_DEBUG_POSTFIX OFF CACHE BOOL "Whether or not to enable debug postfix") # Important to avoid having a d at the end of all libraries. See assimp top-level CMakeLists.txt, line 353
add_subdirectory(third_party/assimp/assimp.submodule)
set_target_properties (assimp PROPERTIES
    FOLDER ${DEPENDENCY_FOLDER}/assimp
)
set_target_properties (zlibstatic PROPERTIES
    FOLDER ${DEPENDENCY_FOLDER}/assimp/zlibstatic
)
set_target_properties (UpdateAssimpLibsDebugSymbolsAndDLLs PROPERTIES
    FOLDER ${DEPENDENCY_FOLDER}/assimp
)
set_target_properties (uninstall PROPERTIES
    FOLDER ${DEPENDENCY_FOLDER}/assimp
)

#### Build Freetype
set(DISABLE_FORCE_DEBUG_POSTFIX ON CACHE BOOL "")
add_subdirectory(third_party/freetype/freetype.submodule)
set_target_properties (freetype PROPERTIES
    FOLDER ${DEPENDENCY_FOLDER}/freetype
)

#### Run main reverie cmake
add_subdirectory(reverie)
