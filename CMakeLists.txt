CMAKE_MINIMUM_REQUIRED( VERSION 2.6 )
PROJECT( RiftRay3 )

INCLUDE(cmake_modules/InvokePython.cmake)
INVOKEPYTHON( "tools/hardcode_shaders.py" )
# Python script will dump generated headers to autogen/
INCLUDE_DIRECTORIES("autogen/")

ADD_DEFINITIONS( -DPROJECT_NAME="${PROJECT_NAME}" )
MESSAGE( "CMAKE_SYSTEM_NAME: ${CMAKE_SYSTEM_NAME}" )

SET( USE_ANTTWEAKBAR TRUE CACHE BOOL "Use AntTweakBar" )

#
# Platform-dependent section
#
IF( WIN32 )
    MESSAGE(STATUS "MSVC_VERSION=${MSVC_VERSION}")

    #
    # Windows include and lib dirs: I drop everything into C:\lib\.
    #
    SET( LIBS_HOME "C:/lib" CACHE STRING "Location of support libraries Glut, Glew and others"  )

    SET( GLM_ROOT "${LIBS_HOME}/glm-0.9.5.4/glm" CACHE STRING "glm root" )
    INCLUDE_DIRECTORIES( "${GLM_ROOT}" )
    ADD_DEFINITIONS( -DGLM_FORCE_RADIANS )

    MESSAGE("Using GLFW Framework.")
    SET( GLFW3_ROOT "${LIBS_HOME}/glfw-3.1.2.bin.WIN32" CACHE STRING "glfw root" )
    IF( MSVC_VERSION STREQUAL "1600" ) # VS 2010
        LINK_DIRECTORIES( "${GLFW3_ROOT}/lib-vc2010" )
    ELSEIF( MSVC_VERSION STREQUAL "1700" )
        LINK_DIRECTORIES( "${GLFW3_ROOT}/lib-vc2012" )
    ELSEIF( MSVC_VERSION STREQUAL "1800" )
        LINK_DIRECTORIES( "${GLFW3_ROOT}/lib-vc2013" )
    ELSEIF( MSVC_VERSION STREQUAL "1900" )
        LINK_DIRECTORIES( "${GLFW3_ROOT}/lib-vc2015" )
    ENDIF()
    INCLUDE_DIRECTORIES( "${GLFW3_ROOT}/include" )
    SET( FRAMEWORK_LIBS glfw3.lib )

    SET( GLEW_ROOT "${LIBS_HOME}/glew-1.12.0" CACHE STRING "glew root" )
    INCLUDE_DIRECTORIES( "${GLEW_ROOT}/include" )
    LINK_DIRECTORIES   ( "${GLEW_ROOT}/lib/Release/Win32" )
    ADD_DEFINITIONS( -DGLEW_STATIC )

    SET( OCULUSSDK_ROOT "${LIBS_HOME}/OculusSDK" CACHE STRING "OculusSDK root" ) # OVR SDK 0.8
    INCLUDE_DIRECTORIES( "${OCULUSSDK_ROOT}/LibOVR/Include" )
    INCLUDE_DIRECTORIES( "${OCULUSSDK_ROOT}/LibOVR/Src" )
    INCLUDE_DIRECTORIES( "${OCULUSSDK_ROOT}/LibOVRKernel/Src" )
    IF( MSVC_VERSION STREQUAL "1600" )
        SET( VS_DIR "VS2010" )
    ELSEIF( MSVC_VERSION STREQUAL "1700" )
        SET( VS_DIR "VS2012" )
    ELSEIF( MSVC_VERSION STREQUAL "1800" )
        SET( VS_DIR "VS2013" )
    ELSEIF( MSVC_VERSION STREQUAL "1900" )
        SET( VS_DIR "VS2015" )
    ENDIF()
    IF( DEBUG )
        LINK_DIRECTORIES( "${OCULUSSDK_ROOT}/LibOVR/Lib/Windows/Win32/Debug/${VS_DIR}" )
    ELSE()
        LINK_DIRECTORIES( "${OCULUSSDK_ROOT}/LibOVR/Lib/Windows/Win32/Release/${VS_DIR}" )
    ENDIF()
    SET( OVR_LIBS LibOVR.lib )
    ADD_DEFINITIONS( -DUSE_OCULUSSDK )

    IF( USE_ANTTWEAKBAR )
        MESSAGE("Using AntTweakBar.")
        SET( ANTTWEAKBAR_ROOT "${LIBS_HOME}/AntTweakBar_116/AntTweakBar" CACHE STRING "AntTweakBar root" )
        INCLUDE_DIRECTORIES( "${ANTTWEAKBAR_ROOT}/include" )
        LINK_DIRECTORIES   ( "${ANTTWEAKBAR_ROOT}/lib" )
        SET( ANT_LIBS AntTweakBar.lib )
        ADD_DEFINITIONS( -DUSE_ANTTWEAKBAR )
    ENDIF( USE_ANTTWEAKBAR )

    SET( PLATFORM_LIBS
        ${FRAMEWORK_LIBS}
        opengl32.lib glu32.lib glew32s.lib
        ${OVR_LIBS}
        ${ANT_LIBS}
        Winmm.lib
        )

    ADD_DEFINITIONS( /W4 -D_CRT_SECURE_NO_WARNINGS )

    # Copy DLLs of the included libraries into output directories.
    INVOKEPYTHON( "tools/copyDLLs.py"
        ${PROJECT_SOURCE_DIR}
        ${PROJECT_BINARY_DIR}
        "OCULUSSDK_ROOT=${OCULUSSDK_ROOT}"
        "GLEW_ROOT=${GLEW_ROOT}"
        "ANTTWEAKBAR_ROOT=${ANTTWEAKBAR_ROOT}"
        )
ENDIF()

#
# Platform-independent section
#
FILE( GLOB_RECURSE UTIL_SOURCE_FILES
    src/Util/*.cpp
    src/Util/*.h
    )

FILE( GLOB_RECURSE SCENE_SOURCE_FILES
    src/Scene/*.cpp
    src/Scene/*.h
    )

INCLUDE_DIRECTORIES("src/Util")
INCLUDE_DIRECTORIES("src/Util/GL")
INCLUDE_DIRECTORIES("src/Util/third_party")
INCLUDE_DIRECTORIES("src/Scene")

ADD_LIBRARY( Util        ${UTIL_SOURCE_FILES} )
ADD_LIBRARY( Scene       ${SCENE_SOURCE_FILES} )
ADD_EXECUTABLE( ${PROJECT_NAME} src/main_glfw_ovrsdk08.cpp )

TARGET_LINK_LIBRARIES( ${PROJECT_NAME}
    Scene
    Util
    ${PLATFORM_LIBS}
    )
