#### Houses commonly used macros

macro(set_cpp_version)
    # Set compiler flags
    # Use C++17
    set(CMAKE_CXX_STANDARD 17)
    set(CMAKE_CXX_STANDARD_REQUIRED ON)
endmacro()


macro(check_architecture)
    set(BUILD_64 CMAKE_SIZEOF_VOID_P EQUAL 8)
    if(${BUILD_64})
        message (STATUS "--------- 64-bit build ---------")
        set(ARCH_NAME "x64")
        set(QT_LIBPATH_SUFFIX "_64")
    else()
        message (STATUS "--------- 32-bit build ---------")
        set(ARCH_NAME "x86")
        set(QT_LIBPATH_SUFFIX "")
    endif()
endmacro()


macro(setup_python)
    #### Setup python for use in project
    # Set directories to find libraries, need to do this before add_executable for desired target   
    # For system versions, see: 
    # https://stackoverflow.com/questions/9160335/os-specific-instructions-in-cmake-how-to
    if(DEFINED ENV{PYTHON_VERSION})
        set(PYTHON_VERSION $ENV{PYTHON_VERSION} CACHE INTERNAL "")
    else()
        if(WIN32)
            # for Windows operating system in general
            # if(MSVC OR MSYS OR MINGW) # for detecting Windows compilers
            set(PYTHON_VERSION 38)
        elseif(APPLE)
            # for MacOS X or iOS, watchOS, tvOS (since 3.10.3)
        elseif(UNIX AND NOT APPLE)
            # For Linux, BSD, Solaris, Minix
            message(STATUS "Setting UNIX python version")
            set(PYTHON_VERSION 3.8)
        else()
            message( WARNING "Unrecognized operating system")
        endif()
    endif()
    message (STATUS "PYTHON VERSION ${PYTHON_VERSION}")

    set(PYTHON_LIB "${PATH_TO_EXTERN}/python/${PYTHON_VERSION}/${ARCH_NAME}/libs" CACHE INTERNAL "")
    # file(TO_CMAKE_PATH "$ENV{PYTHON_LIB}" ENV_PYTHON_LIB)
    message(STATUS "Setting link directory ${PYTHON_LIB}")
    link_directories(${PYTHON_LIB})
endmacro()

macro(find_qt)
    if(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
        # set(QT_MSVC_2017 "C:\\Qt\\5.12\\5.12.2\\msvc2017")
        set(QT_MSVC_2019 "C:/Qt/5.15/5.15.2/msvc2019${QT_LIBPATH_SUFFIX}")
        if(NOT EXISTS ${QT_MSVC_2019})
			message(FATAL_ERROR "Qt 5.15 install not found. Please install!")
		endif()
        set(CMAKE_PREFIX_PATH ${QT_MSVC_2019})
    else()
        message (WARNING "-------- System not supported ---------")
    endif()
endmacro()

macro(setup_qt_project)
    ######## Commands recommended for setting up a project using Qt

    # Find includes in corresponding build directories
    set(CMAKE_INCLUDE_CURRENT_DIR ON)

    # Instruct CMake to run moc automatically when needed.
    set(CMAKE_AUTOMOC ON) # Saves having to use QT5_WRAP_CPP
    set(CMAKE_AUTORCC ON) # Saves having to use QT5_ADD_RESOURCES
    set(CMAKE_AUTOUIC ON) # Saves having to use QT5_WRAP_UI

    #### Set paths to find Qt ===================================================
    # Find Qt itself
    find_qt()

    #### Find UI files
    # See: https://stackoverflow.com/questions/40630820/how-to-place-header-and-ui-file-in-different-folders-using-autouic-in-cmake
    set(CMAKE_AUTOUIC_SEARCH_PATHS "${CMAKE_CURRENT_SOURCE_DIR}/ui")

    #### Find Required Qt Libraries
    find_package(Qt5 COMPONENTS Core REQUIRED)# Core stuff
    find_package(Qt5 COMPONENTS Concurrent REQUIRED) # For threading
    find_package(Qt5 COMPONENTS Gamepad REQUIRED) # Controller support
    find_package(Qt5 COMPONENTS Gui REQUIRED) # For OpenGL integration
    find_package(Qt5 COMPONENTS OpenGLExtensions REQUIRED)
    find_package(Qt5 COMPONENTS Multimedia REQUIRED) # Multimedia capabilities
    find_package(Qt5 COMPONENTS MultimediaWidgets REQUIRED)
    find_package(Qt5 COMPONENTS Widgets REQUIRED) # For widgets
endmacro()


macro(set_eigen_vectorization)
    # Eigen vectorization
    # \todo Figure out why this doesn't enable FMA or AVX, both should be supported by my CPU:
    # https://en.wikichip.org/wiki/amd/ryzen_5/1600
    # MIGHT be because flags are wrong, try: https://docs.microsoft.com/en-us/cpp/build/reference/arch-x64?view=msvc-160
    # https://stackoverflow.com/questions/49446615/vectorization-flags-with-eigen-and-ipopt
    set(_CXX_FLAGS)
    unset(_avx_works CACHE) # Force checking to always occur
    unset(_avx_512_works CACHE) # Force checking to always occur
    unset(_fma_works CACHE) # Force checking to always occur
    unset(_sse_works CACHE) # Force checking to always occur
    if(MSVC)
        set(AVX_FLAG /arch:AVX) # Putting the flag in quotes breaks everything :)
        set(FMA_FLAG /arch:AVX2)
        set(AVX_FLAG_512 /arch:AVX512)
        set(SSE_FLAG /arch:SSE2)
    else()
        set(AVX_FLAG -mavx)
        set(FMA_FLAG -fma)
        set(SSE_FLAG -msse2)
        set(SSE_FLAG3 -msse3)
        set(SSE_FLAG4 -msse4)
        
        unset(_sse3_works CACHE) # Force checking to always occur
        unset(_sse4_works CACHE) # Force checking to always occur

        check_cxx_compiler_flag(${SSE_FLAG3} _sse3_works)
        check_cxx_compiler_flag(${SSE_FLAG4} _sse4_works)
        
        if(_sse3_works)
          message(STATUS "Using SSE3 with Eigen")
          set(_CXX_FLAGS "${_CXX_FLAGS} ${SSE_FLAG3}")
        else()
          message(STATUS "SSE3 unsupported for Eigen")
        endif()
        
        if(_sse4_works)
          message(STATUS "Using SSE4 with Eigen")
          set(_CXX_FLAGS "${_CXX_FLAGS} ${SSE_FLAG4}")
        else()
          message(STATUS "SSE4 unsupported for Eigen")
        endif()
    endif()

    check_cxx_compiler_flag(${AVX_FLAG} _avx_works)
    check_cxx_compiler_flag(${FMA_FLAG} _fma_works)
    check_cxx_compiler_flag(${SSE_FLAG} _sse_works)

    
    if(_avx_works)
      message(STATUS "Using AVX with Eigen")
      set(_CXX_FLAGS "${_CXX_FLAGS} ${AVX_FLAG}")
    else()
      message(STATUS "AVX unsupported for Eigen")
    endif()

    if(_fma_works)
      message(STATUS "Using FMA with eigen")
      set(_CXX_FLAGS "${_CXX_FLAGS} ${FMA_FLAG}")
    else()
      message(STATUS "FMA unsupported for Eigen")
    endif()
    
    if(_sse_works)
      message(STATUS "Using SSE with eigen")
      set(_CXX_FLAGS "${_CXX_FLAGS} ${SSE_FLAG}")
    else()
      message(STATUS "SSE unsupported for Eigen")
    endif()

    unset(_march_native_works CACHE) # Force checking to always occur
    unset(_xhost_works CACHE) # Force checking to always occur
    check_cxx_compiler_flag("-march=native" _march_native_works)
    check_cxx_compiler_flag("-xHost" _xhost_works)

    if(_march_native_works)
      message(STATUS "Using processor's vector instructions (-march=native compiler flag set)")
      set(_CXX_FLAGS "${_CXX_FLAGS} -march=native")
    elseif(_xhost_works)
      message(STATUS "Using processor's vector instructions (-xHost compiler flag set)")
      set(_CXX_FLAGS "${_CXX_FLAGS} -xHost")
    else()
      message(STATUS "No suitable compiler flag found for march native or xhost vectorization")
    endif()
    
    add_compile_options(${_CXX_FLAGS})
    
    message(STATUS "Final vectorization flags used are: ${_CXX_FLAGS}")
endmacro()


macro(set_compiler_options)
    #### Sets compiler-specific options
    # See: CMAKE_GENERATOR_PLATFORM
    # Multiprocess compiler, disable specific warnings
    # UTF-8 specifies source code and execution character sets
    # Zc:__cplusplus ensures that __cplusplus macro actually works properly
    if (MSVC)
        add_compile_options(/MP /bigobj /utf-8 /Zc:__cplusplus /wd4577 /wd4467 /wd26812 /wd26812 /wd26495 )
    else()
    endif()
endmacro()

macro(setup_opengl)
    #### Find and include OpenGL libraries
    # OpenGL libraries
    # See: https://stackoverflow.com/questions/65100749/converting-from-qmake-to-cmake-how-do-i-find-libraries-in-the-same-way/65106458#65106458
    find_package(OpenGL REQUIRED) # enforces as a requirement
    # find_library(OPENGL_LIB names opengl32) # Also valid
    # find_library(GLU_LIB names glu32) # Also valid
endmacro()

macro(generate_release_pdb)
    add_compile_options("$<$<NOT:$<CONFIG:Debug>>:/Zi>")
    add_link_options("$<$<NOT:$<CONFIG:Debug>>:/DEBUG>")
    add_link_options("$<$<NOT:$<CONFIG:Debug>>:/OPT:REF>")
    add_link_options("$<$<NOT:$<CONFIG:Debug>>:/OPT:ICF>")
endmacro()
