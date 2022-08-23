# Generate release PDB
# https://stackoverflow.com/questions/28178978/how-to-generate-pdb-files-for-release-build-with-cmake-flags/31264946

macro(add_library_eigen current_target)
    target_link_libraries(${current_target} PUBLIC "eigen")
    message (STATUS "=========== Linked ${current_target} with eigen=============")
endmacro()

macro(add_library_python current_target)
    message (STATUS "=========== Including Python =============")
    # Set up include paths and libraries for Python itself
    if(WIN32)
        # for Windows operating system in general
        # if(MSVC OR MSYS OR MINGW) # for detecting Windows compilers
        
        # Convert paths to CMAKE-friendly versions
        set(PYTHON_BASE_PATH "${PATH_TO_EXTERN}/python")
        set(PYTHON_PATH "${PYTHON_BASE_PATH}/${PYTHON_VERSION}/${ARCH_NAME}")
        
        # Link Python
        # Debug DLL fails to import third party modules, so always use release
        target_include_directories(${current_target} PUBLIC "${PYTHON_PATH}/include")
        message(STATUS "Python include path for windows is ${PYTHON_PATH}/include")
        message(STATUS "Python path is ${PYTHON_PATH}")
        find_library(PYTHON_RELEASE_LIB NAMES "python${PYTHON_VERSION}" HINTS ${PYTHON_LIB})
        target_link_libraries(${current_target}
			LINK_PUBLIC
            debug ${PYTHON_RELEASE_LIB} optimized ${PYTHON_RELEASE_LIB}
        )
        
        # \todo: Maybe use add_custom_target instead so that this isn't run on every build
        # Set environmental variables to find site-packages and python lib
        set(PYTHON_LIB_PATH ${PYTHON_PATH}/Lib)
        add_custom_command(TARGET ${current_target} 
            PRE_BUILD
            COMMAND ${PYTHON_BASE_PATH}/set_python_paths.bat ${ARCH_NAME} ${PYTHON_VERSION}
        )
        
    elseif(APPLE)
        # TODO: Set Python path and get DLL
        # for MacOS X or iOS, watchOS, tvOS (since 3.10.3)
        message (STATUS "BUILDING FOR APPLE")
        # Include library directory
        target_include_directories(${current_target} PRIVATE "/System/Library/Frameworks/Python.framework/Headers")
        
        # Find library with name Python, and link
        find_library(PYTHON_LIBRARY_PATH NAMES "Python")
        target_link_libraries(${current_target}
			PUBLIC
            ${PYTHON_LIBRARY_PATH}
        )
    elseif(UNIX AND NOT APPLE)
        # TODO: Set Python path and get DLL
        # For Linux, BSD, Solaris, Minix
        message (STATUS "BUILDING FOR UNIX")
        execute_process(COMMAND python${PYTHON_VERSION}-config --embed --libs
            OUTPUT_VARIABLE PYTHON_LIBRARY_PATH
            RESULT_VARIABLE FAILED_TO_DETECT_PYTHON
        )
        if(FAILED_TO_DETECT_PYTHON)
            message (WARNING "Failed to detect Python")
        else()
            # Set library path
            target_link_libraries(${current_target} PUBLIC ${PYTHON_LIBRARY_PATH})
        endif()
    else()
        message( WARNING "Unrecognized operating system")
    endif()
    
    # Copy DLL to needed directories for use
    file(COPY "${PYTHON_PATH}/python${PYTHON_VERSION}.dll" DESTINATION "${CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG}")
    file(COPY "${PYTHON_PATH}/python${PYTHON_VERSION}.dll" DESTINATION "${CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE}")
endmacro()

macro(add_library_qt current_target)

    #### Link Required Qt modules to target
    # This MUST come after add_executable and find_package for each module
    # Essentially, target_link_libraries links the two specified libraries
    target_link_libraries(${current_target} 
        PUBLIC 
        OpenGL::GL 
        OpenGL::GLU
        Qt5::Core
        #Qt5::Gamepad
        Qt5::Concurrent
        Qt5::Gui
        Qt5::OpenGLExtensions # TODO: Probably unnecessary, remove
        #Qt5::Multimedia
        #Qt5::MultimediaWidgets
        Qt5::Widgets
    )

endmacro()

#### Copy DLLs from Qt to bin location
macro(copy_qt_dlls current_target)
    # Set CMAKE_PREFIX_PATH to include Qt location
    find_qt()

    set(QT_BIN_PATH ${CMAKE_PREFIX_PATH}/bin)
    set(QT_DEBUG_DLLS "${QT_BIN_PATH}/Qt5Widgetsd.dll" "${QT_BIN_PATH}/Qt5Guid.dll" "${QT_BIN_PATH}/Qt5Cored.dll" 
    )
    set(QT_RELEASE_DLLS "${QT_BIN_PATH}/Qt5Widgets.dll" "${QT_BIN_PATH}/Qt5Gui.dll" "${QT_BIN_PATH}/Qt5Core.dll")
    message (STATUS "Qt debug DLL Paths are: ${QT_DEBUG_DLLS}")

    # For custom command examples, see: https://gist.github.com/baiwfg2/39881ba703e9c74e95366ed422641609
    add_custom_command(TARGET ${current_target} 
            PRE_BUILD
            COMMENT "Copy all Qt DLLs to target .exe directory"
            VERBATIM
            COMMAND_EXPAND_LISTS
            COMMAND ${CMAKE_COMMAND} -E copy_if_different 
            "$<$<CONFIG:DEBUG>:${QT_DEBUG_DLLS}>"
            "$<$<CONFIG:RELEASE>:${QT_RELEASE_DLLS}>"
            "$<TARGET_FILE_DIR:${current_target}>"
    )

    #### Copy Qt platform DLLs
    set(QT_PLATFORMS_PATH ${CMAKE_PREFIX_PATH}/plugins/platforms)
    set(QT_DEBUG_PLATFORM_DLLS "${QT_PLATFORMS_PATH}/qdirect2dd.dll" "${QT_PLATFORMS_PATH}/qminimald.dll" "${QT_PLATFORMS_PATH}/qoffscreend.dll" "${QT_PLATFORMS_PATH}/qwindowsd.dll")
    set(QT_RELEASE_PLATFORM_DLLS "${QT_PLATFORMS_PATH}/qdirect2d.dll" "${QT_PLATFORMS_PATH}/qminimal.dll" "${QT_PLATFORMS_PATH}/qoffscreen.dll" "${QT_PLATFORMS_PATH}/qwindows.dll")
    add_custom_command(TARGET ${current_target} 
        PRE_BUILD 
        COMMAND ${CMAKE_COMMAND} -E make_directory 
        "$<TARGET_FILE_DIR:${current_target}>/platforms"
    )
    add_custom_command(TARGET ${current_target} 
        PRE_BUILD
        COMMENT "Copy all Qt platform DLLs to target .exe directory"
        VERBATIM
        COMMAND_EXPAND_LISTS
        COMMAND ${CMAKE_COMMAND} -E copy_if_different 
            "$<$<CONFIG:DEBUG>:${QT_DEBUG_PLATFORM_DLLS}>"
            "$<$<CONFIG:RELEASE>:${QT_RELEASE_PLATFORM_DLLS}>"
            "$<TARGET_FILE_DIR:${current_target}>/platforms"
    )

endmacro()