CMAKE_MINIMUM_REQUIRED( VERSION 2.6 )
PROJECT( RiftRay )

INCLUDE(cmake_modules/HardcodeShaders.cmake)

SET( USE_ANTTWEAKBAR TRUE CACHE BOOL "Use AntTweakBar" )
SET( USE_HYDRA TRUE CACHE BOOL "Use Hydra" )

#
# Platform-dependent section
#
IF( WIN32 )
    #
    # Custom Windows include and link dirs for my machine:
    #
    SET( LIBS_HOME "C:/lib" CACHE STRING "Location of support libraries Glut, Glew and others"  )

    SET( GLFW3_ROOT "${LIBS_HOME}/glfw-3.0.1.bin.WIN32" )
    INCLUDE_DIRECTORIES( "${GLFW3_ROOT}/include" )
    LINK_DIRECTORIES   ( "${GLFW3_ROOT}/lib-msvc100" )

    SET( GLEW_ROOT "${LIBS_HOME}/glew" )
    INCLUDE_DIRECTORIES( "${GLEW_ROOT}/include" )
    LINK_DIRECTORIES   ( "${GLEW_ROOT}/lib" )
    #ADD_DEFINITIONS( -DGLEW_STATIC )
    
    SET( OCULUSSDK_ROOT "${LIBS_HOME}/OculusSDK-0.2.2" )
    INCLUDE_DIRECTORIES( "${OCULUSSDK_ROOT}/LibOVR/Include" )
    INCLUDE_DIRECTORIES( "${OCULUSSDK_ROOT}/LibOVR/Src" )
    LINK_DIRECTORIES   ( "${OCULUSSDK_ROOT}/LibOVR/Lib/Win32" )
    SET( OVR_LIBS optimized libovr.lib debug libovrd.lib )

    IF( USE_ANTTWEAKBAR )
        SET( ANTTWEAKBAR_ROOT "${LIBS_HOME}/AntTweakBar_116/AntTweakBar" )
        INCLUDE_DIRECTORIES( "${ANTTWEAKBAR_ROOT}/include" )
        LINK_DIRECTORIES   ( "${ANTTWEAKBAR_ROOT}/lib" )
        SET( ANT_LIBS AntTweakBar.lib )
        ADD_DEFINITIONS( -DUSE_ANTTWEAKBAR )
    ENDIF( USE_ANTTWEAKBAR )

    IF( USE_HYDRA )
        SET( SIXENSE_ROOT "${LIBS_HOME}/SixenseSDK" )
        INCLUDE_DIRECTORIES( "${SIXENSE_ROOT}/include" )
        #LINK_DIRECTORIES   ( "${SIXENSE_ROOT}/lib/win32/debug_static" )
        LINK_DIRECTORIES   ( "${SIXENSE_ROOT}/lib/win32/debug_dll" )
        SET( HYDRA_LIBS sixensed.lib sixense_utilsd.lib )
        ADD_DEFINITIONS( -DUSE_HYDRA )
    ENDIF( USE_HYDRA )

    SET( PLATFORM_LIBS
        glfw3.lib
        opengl32.lib glu32.lib glew32.lib 
        ${OVR_LIBS}
        winmm.lib
        ${ANT_LIBS}
        ${HYDRA_LIBS}
        )

ELSEIF( APPLE )
    #
    # Custom Windows include and link dirs for my machine:
    #

    SET(CMAKE_CXX_FLAGS "-fno-rtti")

    INCLUDE_DIRECTORIES ( /System/Library/Frameworks )
    FIND_LIBRARY(COCOA_LIBRARY Cocoa)
    FIND_LIBRARY(IOKIT_LIBRARY IOKit)
    FIND_LIBRARY(GLUT_LIBRARY GLUT )
    FIND_LIBRARY(OpenGL_LIBRARY OpenGL )
    MARK_AS_ADVANCED (COCOA_LIBRARY
                      GLUT_LIBRARY
                      IOKIT_LIBRARY
                      OpenGL_LIBRARY)
    INCLUDE_DIRECTORIES( "/opt/local/include/" )
    LINK_DIRECTORIES   ( "/opt/local/lib" )

    SET( LIBS_HOME "~/Development" )

    #SET( CUDA_TOOLKIT_ROOT_DIR "/Developer/NVIDIA/CUDA-5.5/" )

    SET( GLFW3_ROOT "${LIBS_HOME}/glfw-3.0.3/" )
    INCLUDE_DIRECTORIES( "${GLFW3_ROOT}/include" )
    LINK_DIRECTORIES   ( "${GLFW3_ROOT}/lib" )

    SET( OCULUSSDK_ROOT "${LIBS_HOME}/OculusSDK/" )
    INCLUDE_DIRECTORIES( "${OCULUSSDK_ROOT}/LibOVR/Include" )
    INCLUDE_DIRECTORIES( "${OCULUSSDK_ROOT}/LibOVR/Src" )
    LINK_DIRECTORIES   ( "${OCULUSSDK_ROOT}/LibOVR/Lib/MacOS/Debug" )

    IF( USE_HYDRA )
        SET( SIXENSE_ROOT "${LIBS_HOME}/SixenseSDK" )
        INCLUDE_DIRECTORIES( "${SIXENSE_ROOT}/include" )
        LINK_DIRECTORIES   ( "${SIXENSE_ROOT}/lib/osx_x64/debug_dll" )
        SET( HYDRA_LIBS -lsixensed_x64 -lsixense_utilsd_x64  )
    ENDIF()

    IF( USE_ANTTWEAKBAR )
        SET( ANT_LIBS -lAntTweakBar )
        ADD_DEFINITIONS( -DUSE_ANTTWEAKBAR )
    ENDIF( USE_ANTTWEAKBAR )

    #SET( GLEW_ROOT "${LIBS_HOME}/glew/" )
    #INCLUDE_DIRECTORIES( "${GLEW_ROOT}/include" )
    #LINK_DIRECTORIES   ( "${GLEW_ROOT}/lib" )
    #ADD_DEFINITIONS( -DGLEW_STATIC )


    # TODO: set 32/64 bit appropriately
    #LINK_DIRECTORIES("${CUDA_TOOLKIT_ROOT_DIR}/lib/")
    #LINK_DIRECTORIES("${CUDA_TOOLKIT_ROOT_DIR}/lib/x64")

    SET(PLATFORM_LIBS ${FRAMEWORK_LIBS} ${COCOA_LIBRARY} ${IOKIT_LIBRARY} ${GLUT_LIBRARY} ${OpenGL_LIBRARY} 
    -lGLEW -lglfw -lovr ${HYDRA_LIBS} ${ANT_LIBS})

ELSEIF( UNIX )
    #
    # Custom Windows include and link dirs for my machine:
    #
    ADD_DEFINITIONS( -D_LINUX )
    SET( LIBS_HOME "~/lib" )

    SET( OCULUSSDK_ROOT "${LIBS_HOME}/OculusSDK" )
    INCLUDE_DIRECTORIES( "${OCULUSSDK_ROOT}/LibOVR/Include" )
    INCLUDE_DIRECTORIES( "${OCULUSSDK_ROOT}/LibOVR/Src" )
    LINK_DIRECTORIES   ( "${OCULUSSDK_ROOT}/LibOVR/Lib/Linux/Release/x86_64/" )

    IF( USE_ANTTWEAKBAR )
        SET( ANTTWEAKBAR_ROOT "${LIBS_HOME}/AntTweakBar" )
        INCLUDE_DIRECTORIES( "${ANTTWEAKBAR_ROOT}/include" )
        LINK_DIRECTORIES   ( "${ANTTWEAKBAR_ROOT}/lib" )
        SET( ANT_LIBS libAntTweakBar.a )
        ADD_DEFINITIONS( -DUSE_ANTTWEAKBAR )
    ENDIF( USE_ANTTWEAKBAR )

    SET( PLATFORM_LIBS
        -lovr
        -lGLEW
        -lGLU -lglfw3
        -pthread
        -ludev
        -lrt
        -lXinerama
        -lXrandr
        -lXxf86vm
        -lXi
        ${ANT_LIBS}
        )


ENDIF()


#
# Platform-independent section
#
FILE( GLOB_RECURSE UTIL_SOURCE_FILES
    src/utils/*.cpp
    src/utils/*.h
    )

FILE( GLOB_RECURSE VECTORMATH_SOURCE_FILES
    src/vectormath/*.cpp
    src/vectormath/*.h
    )

FILE( GLOB_RECURSE OVRKILL_SOURCE_FILES
    src/OVRkill/*.cpp
    src/OVRkill/*.h
    )

FILE( GLOB_RECURSE FLYINGMOUSE_SOURCE_FILES
        src/FlyingMouse/*.cpp
        src/FlyingMouse/*.h
    )

FILE( GLOB_RECURSE APPSKELETON_SOURCE_FILES
    src/appskeleton/*.cpp
    src/appskeleton/*.h
    )

INCLUDE_DIRECTORIES("src")
INCLUDE_DIRECTORIES("src/utils")
INCLUDE_DIRECTORIES("src/utils/GL")
INCLUDE_DIRECTORIES("src/vectormath")
INCLUDE_DIRECTORIES("src/OVRkill")
INCLUDE_DIRECTORIES("src/FlyingMouse")
INCLUDE_DIRECTORIES("src/appskeleton")

ADD_LIBRARY( Util        ${UTIL_SOURCE_FILES} )
ADD_LIBRARY( VectorMath  ${VECTORMATH_SOURCE_FILES} )
ADD_LIBRARY( OVRkill     ${OVRKILL_SOURCE_FILES} )
ADD_LIBRARY( FlyingMouse ${FLYINGMOUSE_SOURCE_FILES} )
ADD_LIBRARY( AppSkeleton ${APPSKELETON_SOURCE_FILES} )

ADD_EXECUTABLE( ${PROJECT_NAME}
    src/main/simple_glfw_main.cpp
    )

TARGET_LINK_LIBRARIES( ${PROJECT_NAME}
    AppSkeleton
    FlyingMouse
    OVRkill
    Util
    VectorMath
    ${PLATFORM_LIBS}
    )
