######## CMake build for extern libraries
# Include dependency manager 
# @todo Only used by color widgets. Use this for everything
include(${CMAKE_SOURCE_DIR}/cmake/cpm/cmake/CPM.cmake)

# for find_qt()
include(${CMAKE_SOURCE_DIR}/cmake/common_functions.cmake)

# Set folders
set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set(DEPENDENCY_FOLDER Dependencies)

# Make sure there is NO CMAKE_DEBUG_POSTFIX
SET(CMAKE_DEBUG_POSTFIX "" CACHE STRING "Debug Postfix for lib, samples and tools") # Important to avoid having a d at the end of all libraries. See assimp top-level CMakeLists.txt, line 353

#### Build color widgets
# add_subdirectory(color_widgets)
# set_target_properties (QtColorWidgets PROPERTIES
    # FOLDER ${DEPENDENCY_FOLDER}/ColorWidgets
# )
check_architecture()
find_qt()
CPMAddPackage("gl:daintybubainty/Qt-Color-Widgets#master")
set_target_properties(QtColorWidgets PROPERTIES FOLDER ${DEPENDENCY_FOLDER}/ColorWidgets)

#### Build physx
# This is not an intended PhysX use-case, but the road has been paved. 
# See: https://github.com/NVIDIAGameWorks/PhysX/issues/249
set(PHYSX_VS_FOLDER_NAME ${DEPENDENCY_FOLDER}/PhysX)
set(PHYSX_ROOT_DIR ${CMAKE_SOURCE_DIR}/extern/physx/physx.submodule/physx)
set(PHYSX_INCLUDE_DIRS ${PHYSX_ROOT_DIR}/include/ ${PHYSX_ROOT_DIR}/../pxshared/include/ )
set(TARGET_BUILD_PLATFORM "windows" CACHE INTERNAL "") # has to match the TARGET_BUILD_PLATFORM in PHYSX_ROOT_DIR/physx/buildtools/cmake_generate_projects.py
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
set(PX_OUTPUT_LIB_DIR ${CMAKE_CURRENT_BINARY_DIR}/physx CACHE INTERNAL "")
set(PX_OUTPUT_BIN_DIR ${CMAKE_CURRENT_BINARY_DIR}/physx CACHE INTERNAL "")
add_subdirectory(${PHYSX_ROOT_DIR}/compiler/public extern/physx)

#### Build ASSIMP
# set(ZLIB_LIBRARIES zlibstatic) # Remove, probably didn't do anything
# set(BUILD_STATIC_LIB ON) # Remove, probably didn't do anything
# set(ASSIMP_BUILD_STATIC_LIB ON) # Remove, probably didn't do anything
set(ENABLE_BOOST_WORKAROUND ON)
set(ASSIMP_BUILD_ASSIMP_TOOLS OFF CACHE BOOL "")
set(ASSIMP_BUILD_TESTS OFF CACHE BOOL "")
set(ASSIMP_BUILD_ZLIB ON)
set(BUILD_SHARED_LIBS OFF) # Make static
set(ASSIMP_INSTALL OFF) # Disabled when using assimp as a submodule
set(ASSIMP_NO_EXPORT ON) # Not using any export functionality
set(ASSIMP_INJECT_DEBUG_POSTFIX OFF CACHE BOOL "Whether or not to enable debug postfix") # Important to avoid having a d at the end of all libraries. See assimp top-level CMakeLists.txt, line 353
add_subdirectory(assimp/assimp.submodule)

# Targets may not be present depending on the version of assimp being used
if(TARGET assimp)
    set_target_properties (assimp PROPERTIES
        FOLDER ${DEPENDENCY_FOLDER}/assimp
    )
endif()
if(TARGET zlibstatic)
    set_target_properties (zlibstatic PROPERTIES
        FOLDER ${DEPENDENCY_FOLDER}/assimp/zlibstatic
    )
endif()
if(TARGET zlib)
    set_target_properties (zlib PROPERTIES
        FOLDER ${DEPENDENCY_FOLDER}/assimp/zlib
    )
endif()
if(TARGET IrrXML)
    set_target_properties (IrrXML PROPERTIES
        FOLDER ${DEPENDENCY_FOLDER}/assimp/IrrXML
    )
endif()
if(TARGET UpdateAssimpLibsDebugSymbolsAndDLLs)
    set_target_properties (UpdateAssimpLibsDebugSymbolsAndDLLs PROPERTIES
        FOLDER ${DEPENDENCY_FOLDER}/assimp
    )
endif()
if(TARGET uninstall)
    set_target_properties (uninstall PROPERTIES
        FOLDER ${DEPENDENCY_FOLDER}/assimp
    )
endif()


#### Build Freetype
set(DISABLE_FORCE_DEBUG_POSTFIX ON CACHE BOOL "")
add_subdirectory(freetype/freetype.submodule)
set_target_properties (freetype PROPERTIES
    FOLDER ${DEPENDENCY_FOLDER}/freetype
)

#### Build eigen
add_subdirectory(eigen)
set_target_properties (eigen PROPERTIES
    FOLDER ${DEPENDENCY_FOLDER}/eigen
)

#### Build JSON
set(JSON_BuildTests OFF CACHE INTERNAL "") # Don't build Json's third party tests
add_subdirectory(json/json.submodule)

### Build google tests
set(GOOGLE_TEST_DIR "googletest/googletest.submodule")
add_subdirectory(${GOOGLE_TEST_DIR})
enable_testing()
include_directories("${GOOGLE_TEST_DIR}/googletest/include")
if(TARGET gtest)
    set_target_properties (gtest PROPERTIES
        FOLDER ${DEPENDENCY_FOLDER}/gtest
    )
endif()
if(TARGET gmock)
    set_target_properties (gmock PROPERTIES
        FOLDER ${DEPENDENCY_FOLDER}/gmock
    )
endif()
if(TARGET gmock_main)
    set_target_properties (gmock_main PROPERTIES
        FOLDER ${DEPENDENCY_FOLDER}/gmock_main
    )
endif()
if(TARGET gtest_main)
    set_target_properties (gtest_main PROPERTIES
        FOLDER ${DEPENDENCY_FOLDER}/gtest_main
    )
endif()
