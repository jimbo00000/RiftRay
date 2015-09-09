CMAKE_MINIMUM_REQUIRED( VERSION 2.6 )
PROJECT( RiftRay2 )

INCLUDE(cmake_modules/InvokePython.cmake)
INVOKEPYTHON( "tools/hardcode_shaders.py" )
# Python script will dump generated headers to autogen/
INCLUDE_DIRECTORIES("autogen/")

# GLFW, SDL
SET( GL_FRAMEWORK "GLFW" CACHE STRING "Which OpenGL windowing framework to use" )

# Control linking to AntTweakBar and Sixense libraries by toggling a config flag.
# @todo Write or find a "FindXXX.cmake" script to search typical install locations
# on the build machine and link the libraries in.
SET( USE_ANTTWEAKBAR TRUE CACHE BOOL "Use AntTweakBar" )
SET( USE_SIXENSE TRUE CACHE BOOL "Use Sixense" )

SET( USE_CORE_CONTEXT TRUE CACHE BOOL "Use Core Context" )
IF( USE_CORE_CONTEXT )
    ADD_DEFINITIONS(-DUSE_CORE_CONTEXT)
ENDIF()

#
# Platform-dependent section
#
IF( WIN32 )
    MESSAGE(STATUS "MSVC_VERSION=${MSVC_VERSION}")
    #
    # Custom Windows include and link dirs for my machine:
    #
    SET( LIBS_HOME "C:/lib" CACHE STRING "Location of support libraries Glut, Glew and others"  )

    SET( GLM_ROOT "${LIBS_HOME}/glm-0.9.5.4/glm" CACHE STRING "glm root" )
    INCLUDE_DIRECTORIES( "${GLM_ROOT}" )
    ADD_DEFINITIONS( -DGLM_FORCE_RADIANS )

    IF( GL_FRAMEWORK STREQUAL "GLFW" )
        MESSAGE("Using GLFW Framework.")
        IF( MSVC_VERSION STREQUAL "1600" ) # VS 2010
            SET( GLFW3_ROOT "${LIBS_HOME}/glfw-3.1.bin.WIN32" CACHE STRING "glfw root" )
            LINK_DIRECTORIES( "${GLFW3_ROOT}/lib-vc2010" )
        ELSEIF( MSVC_VERSION STREQUAL "1700" )
            SET( GLFW3_ROOT "${LIBS_HOME}/glfw-3.1.bin.WIN32" CACHE STRING "glfw root" )
            LINK_DIRECTORIES( "${GLFW3_ROOT}/lib-vc2012" )
        ELSEIF( MSVC_VERSION STREQUAL "1800" )
            SET( GLFW3_ROOT "${LIBS_HOME}/glfw-3.1.bin.WIN32" CACHE STRING "glfw root" )
            LINK_DIRECTORIES( "${GLFW3_ROOT}/lib-vc2013" )
        ENDIF()
        INCLUDE_DIRECTORIES( "${GLFW3_ROOT}/include" )
        SET( FRAMEWORK_LIBS glfw3.lib )
    ELSEIF( GL_FRAMEWORK STREQUAL "SDL" )
        MESSAGE("Using SDL2 Framework.")
        SET( SDL2_ROOT "${LIBS_HOME}/SDL2-2.0.3" CACHE STRING "SDL2 root" )
        INCLUDE_DIRECTORIES( "${SDL2_ROOT}/include" )
        LINK_DIRECTORIES   ( "${SDL2_ROOT}/lib/x86" )
        SET( FRAMEWORK_LIBS SDL2.lib )
    ELSE()
        MESSAGE("Unknown GL_FRAMEWORK: choose GLFW or SDL.")
    ENDIF()

    SET( GLEW_ROOT "${LIBS_HOME}/glew-1.12.0" CACHE STRING "glew root" )
    INCLUDE_DIRECTORIES( "${GLEW_ROOT}/include" )
    LINK_DIRECTORIES   ( "${GLEW_ROOT}/lib/Release/Win32" )
    ADD_DEFINITIONS( -DGLEW_STATIC )

    SET( OCULUSSDK_ROOT "${LIBS_HOME}/OculusSDK" CACHE STRING "OculusSDK root" )
    INCLUDE_DIRECTORIES( "${OCULUSSDK_ROOT}/LibOVR/Include" )
    INCLUDE_DIRECTORIES( "${OCULUSSDK_ROOT}/LibOVR/Src" )
    INCLUDE_DIRECTORIES( "${OCULUSSDK_ROOT}/LibOVRKernel/Src" )
    IF( MSVC_VERSION STREQUAL "1600" )
        IF( DEBUG )
            LINK_DIRECTORIES( "${OCULUSSDK_ROOT}/LibOVR/Lib/Windows/Win32/Debug/VS2010" )
        ELSE()
            LINK_DIRECTORIES( "${OCULUSSDK_ROOT}/LibOVR/Lib/Windows/Win32/Release/VS2010" )
        ENDIF()
    ELSEIF( MSVC_VERSION STREQUAL "1700" )
        IF( DEBUG )
            LINK_DIRECTORIES( "${OCULUSSDK_ROOT}/LibOVR/Lib/Windows/Win32/Debug/VS2012" )
        ELSE()
            LINK_DIRECTORIES( "${OCULUSSDK_ROOT}/LibOVR/Lib/Windows/Win32/Release/VS2012" )
        ENDIF()
    ELSEIF( MSVC_VERSION STREQUAL "1800" )
        IF( DEBUG )
            LINK_DIRECTORIES( "${OCULUSSDK_ROOT}/LibOVR/Lib/Windows/Win32/Debug/VS2013" )
        ELSE()
            LINK_DIRECTORIES( "${OCULUSSDK_ROOT}/LibOVR/Lib/Windows/Win32/Release/VS2013" )
        ENDIF()
    ENDIF()
    SET( OVR_LIBS LibOVR.lib )
    ADD_DEFINITIONS( -DUSE_OCULUSSDK )

    IF( USE_SIXENSE )
        MESSAGE("Using Sixense SDK.")
        SET( SIXENSE_ROOT "${LIBS_HOME}/SixenseSDK" CACHE STRING "SixenseSDK root" )
        INCLUDE_DIRECTORIES( "${SIXENSE_ROOT}/include" )
        #LINK_DIRECTORIES   ( "${SIXENSE_ROOT}/lib/win32/debug_static" )
        LINK_DIRECTORIES   ( "${SIXENSE_ROOT}/lib/win32/debug_dll"
                             "${SIXENSE_ROOT}/lib/win32/release_dll" )
        SET( HYDRA_LIBS
            optimized sixense.lib
            optimized sixense_utils.lib
            debug sixensed.lib
            debug sixense_utilsd.lib )
        ADD_DEFINITIONS( -DUSE_SIXENSE )
    ENDIF( USE_SIXENSE )

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
        Winmm.lib
        ws2_32.lib
        ${HYDRA_LIBS}
        ${ANT_LIBS}
        )

    ADD_DEFINITIONS( /W4 )

    # These lines will silence some linker warnings on VS2010,
    # but also remove debug info from the debug build.
    #SET( CMAKE_EXE_LINKER_FLAGS /NODEFAULTLIB:\"libcmt.lib\" )
    #SET( CMAKE_EXE_LINKER_FLAGS_DEBUG /NODEFAULTLIB:\"msvcrt.lib\" )

    # Copy DLLs of the included libraries into output directories.
    INVOKEPYTHON( "tools/copyDLLs.py"
        ${PROJECT_SOURCE_DIR}
        ${PROJECT_BINARY_DIR}
        "ANTTWEAKBAR_ROOT=${ANTTWEAKBAR_ROOT}"
        "OCULUSSDK_ROOT=${OCULUSSDK_ROOT}"
        "SIXENSE_ROOT=${SIXENSE_ROOT}"
        "GLEW_ROOT=${GLEW_ROOT}"
        "SDL2_ROOT=${SDL2_ROOT}"
        "SFML_ROOT=${SFML_ROOT}"
        )
ELSEIF( APPLE )
    ADD_DEFINITIONS( -D_MACOS )

    SET(CMAKE_CXX_FLAGS "-fno-rtti")

    SET( LIBS_HOME "~/Development/" )
    SET( OCULUSSDK_ROOT "${LIBS_HOME}/OculusSDK" )
    

    INCLUDE_DIRECTORIES ( /System/Library/Frameworks )
    FIND_LIBRARY(COCOA_LIBRARY Cocoa)
    FIND_LIBRARY(IOKIT_LIBRARY IOKit)
    FIND_LIBRARY(GLUT_LIBRARY GLUT )
    FIND_LIBRARY(OVR_LIBRARY NAMES LibOVR PATHS ${OCULUSSDK_ROOT}/LibOVR/Lib/Mac/Release NO_DEFAULT_PATH)
    FIND_LIBRARY(OpenGL_LIBRARY OpenGL )
    MARK_AS_ADVANCED (COCOA_LIBRARY
                      GLUT_LIBRARY
                      IOKIT_LIBRARY
                      OpenGL_LIBRARY
                      OVR_LIBRARY)
    INCLUDE_DIRECTORIES( "/opt/local/include/" )
    LINK_DIRECTORIES   ( "/opt/local/lib" )

    

    SET( GLM_ROOT "${LIBS_HOME}/glm-0.9.5.4/glm" CACHE STRING "glm root" )
    INCLUDE_DIRECTORIES( "${GLM_ROOT}" )
    ADD_DEFINITIONS( -DGLM_FORCE_RADIANS )

    SET( GLFW3_ROOT "${LIBS_HOME}/glfw-3.0.3/" )
    INCLUDE_DIRECTORIES( "${GLFW3_ROOT}/include" )
    LINK_DIRECTORIES   ( "${GLFW3_ROOT}/lib" )

   #MESSAGE("Building against OVR SDK 0.3.2 - Mac version not available as of release." )
    
    INCLUDE_DIRECTORIES( "${OCULUSSDK_ROOT}/LibOVR/Include" )
    INCLUDE_DIRECTORIES( "${OCULUSSDK_ROOT}/LibOVR/Src" )
    INCLUDE_DIRECTORIES( "${OCULUSSDK_ROOT}/LibOVRKernel/Src" )
    ADD_DEFINITIONS( -DUSE_OCULUSSDK )

    # Todo - build OVR with RTTI suppo
    SET(CMAKE_CXX_FLAGS "-fno-rtti")

    IF( USE_SIXENSE )
        MESSAGE("Using Sixense SDK.")
        SET( SIXENSE_ROOT "${LIBS_HOME}/sixenseSDK" CACHE STRING "SixenseSDK root" )
        INCLUDE_DIRECTORIES( "${SIXENSE_ROOT}/include" )
        LINK_DIRECTORIES   ( "${SIXENSE_ROOT}/lib/osx_x64/debug" )
        SET( HYDRA_LIBS -lsixensed_x64 -lsixense_utilsd_x64 )
        ADD_DEFINITIONS( -DUSE_SIXENSE )
    ENDIF( USE_SIXENSE )

    IF( USE_ANTTWEAKBAR )
        MESSAGE("Using AntTweakBar.")
        SET( ANT_LIBS -lAntTweakBar )
        ADD_DEFINITIONS( -DUSE_ANTTWEAKBAR )
    ENDIF( USE_ANTTWEAKBAR )

    #SET( PLATFORM_LIBS libovr.a ${COCOA_LIBRARY} ${IOKIT_LIBRARY} )
    SET( PLATFORM_LIBS
        ${FRAMEWORK_LIBS}
        ${COCOA_LIBRARY}
        ${IOKIT_LIBRARY}
        ${GLUT_LIBRARY}
        ${OpenGL_LIBRARY}
        ${OVR_LIBRARY}
        -lGLEW
        -lglfw
        ${HYDRA_LIBS}
        ${ANT_LIBS}
        )
        # TODO: SDL2 or glfw

ELSEIF( UNIX )
    ADD_DEFINITIONS( -D_LINUX )
    SET( LIBS_HOME "~/lib" )
    
    ADD_DEFINITIONS( -D_DEBUG )

    SET( GLM_ROOT "${LIBS_HOME}/glm-0.9.5.4/glm" CACHE STRING "glm root" )
    INCLUDE_DIRECTORIES( "${GLM_ROOT}" )
    ADD_DEFINITIONS( -DGLM_FORCE_RADIANS )

    IF( GL_FRAMEWORK STREQUAL "GLFW" )
        MESSAGE("Using GLFW Framework.")
        SET( FRAMEWORK_LIBS -lglfw3 )
    ELSEIF( GL_FRAMEWORK STREQUAL "SDL" )
        MESSAGE("Using SDL2 Framework.")
        INCLUDE_DIRECTORIES( "/usr/local/include/SDL2" )
        LINK_DIRECTORIES   ( "/usr/local/lib/x86" )
        SET( FRAMEWORK_LIBS -lSDL2 )
    ELSE()
        MESSAGE("Unknown GL_FRAMEWORK: choose GLFW or SDL.")
    ENDIF()

    SET( OCULUSSDK_ROOT "${LIBS_HOME}/ovr_sdk_linux_0.5.0.1" CACHE STRING "OculusSDK root" )
    INCLUDE_DIRECTORIES( "${OCULUSSDK_ROOT}/LibOVR/Include" )
    INCLUDE_DIRECTORIES( "${OCULUSSDK_ROOT}/LibOVR/Src" )
    INCLUDE_DIRECTORIES( "${OCULUSSDK_ROOT}/LibOVRKernel/Src" )
    LINK_DIRECTORIES   ( "${OCULUSSDK_ROOT}/LibOVR/Lib/Linux/Release/x86_64" )
    SET( OVR_LIBS -lOVR -ldl -ludev -lXinerama )
    ADD_DEFINITIONS( -DUSE_OCULUSSDK )

    IF( USE_SIXENSE )
        MESSAGE("Using Sixense SDK.")
        SET( SIXENSE_ROOT "${LIBS_HOME}/sixenseSDK_linux_OSX" CACHE STRING "SixenseSDK root" )
        INCLUDE_DIRECTORIES( "${SIXENSE_ROOT}/include" )
        LINK_DIRECTORIES   ( "${SIXENSE_ROOT}/lib/linux_x64/debug"
                             "${SIXENSE_ROOT}/lib/linux_x64/release" )
        SET( HYDRA_LIBS
            optimized -lsixense_x64
            optimized -lsixense_utils_x64
            debug -lsixensed_x64
            debug -lsixense_utilsd_x64 )
        ADD_DEFINITIONS( -DUSE_SIXENSE )
    ENDIF( USE_SIXENSE )

    IF( USE_ANTTWEAKBAR )
        MESSAGE("Using AntTweakBar.")
        SET( ANTTWEAKBAR_ROOT "${LIBS_HOME}/AntTweakBar" CACHE STRING "AntTweakBar root" )
        INCLUDE_DIRECTORIES( "${ANTTWEAKBAR_ROOT}/include" )
        LINK_DIRECTORIES   ( "${ANTTWEAKBAR_ROOT}/lib" )
        SET( ANT_LIBS libAntTweakBar.a )
        ADD_DEFINITIONS( -DUSE_ANTTWEAKBAR )
    ENDIF( USE_ANTTWEAKBAR )

    find_package(OpenGL REQUIRED)
    include_directories( ${OPENGL_INCLUDE_DIRS} )

    SET( PLATFORM_LIBS
        -lGLEW
        -lGL
        -lGLU
        ${FRAMEWORK_LIBS}
        -pthread
        -lrt
        -lXrandr
        -lXxf86vm
        -lXi
        -lX11
        -lXcursor # GLFW 3.1
        ${OVR_LIBS}
        ${HYDRA_LIBS}
        ${ANT_LIBS}
        )
ENDIF()


#
# Platform-independent section
#
FILE( GLOB_RECURSE UTIL_SOURCE_FILES
    src/Util/*.cpp
    src/Util/*.h
    )

FILE( GLOB_RECURSE APPSKELETON_SOURCE_FILES
    src/AppSkeleton/*.cpp
    src/AppSkeleton/*.h
    )

FILE( GLOB_RECURSE SCENE_SOURCE_FILES
    src/Scene/*.cpp
    src/Scene/*.h
    )

FILE( GLOB_RECURSE FLYINGMOUSE_SOURCE_FILES
        src/FlyingMouse/*.cpp
        src/FlyingMouse/*.h
    )

IF(APPLE)

file(GLOB SHADER_FILES 
        shaders/*
    ) 
file(GLOB TEXTURES_FILES 
        textures/*
    ) 
file(GLOB HARDCODED_SHADERS 
        hardcoded_shaders/*
    ) 

# set how it shows up in the Info.plist file
SET(MACOSX_BUNDLE_ICON_FILE RiftRay.icns) 
# set where in the bundle to put the icns file
SET_SOURCE_FILES_PROPERTIES(RiftRay.icns PROPERTIES MACOSX_PACKAGE_LOCATION Resources)
SET_SOURCE_FILES_PROPERTIES(${SHADER_FILES} PROPERTIES MACOSX_PACKAGE_LOCATION Resources/shaders)
SET_SOURCE_FILES_PROPERTIES(${TEXTURES_FILES} PROPERTIES MACOSX_PACKAGE_LOCATION Resources/textures)
SET_SOURCE_FILES_PROPERTIES(${HARDCODED_SHADERS} PROPERTIES MACOSX_PACKAGE_LOCATION Resources/hardcoded_shaders)
ENDIF(APPLE)

INCLUDE_DIRECTORIES("src")
INCLUDE_DIRECTORIES("src/Util")
INCLUDE_DIRECTORIES("src/Util/GL")
INCLUDE_DIRECTORIES("src/Util/third_party")
INCLUDE_DIRECTORIES("src/AppSkeleton")
INCLUDE_DIRECTORIES("src/Scene")
INCLUDE_DIRECTORIES("src/FlyingMouse")

ADD_LIBRARY( Util        ${UTIL_SOURCE_FILES} )
ADD_LIBRARY( AppSkeleton ${APPSKELETON_SOURCE_FILES} )
ADD_LIBRARY( Scene       ${SCENE_SOURCE_FILES} )
ADD_LIBRARY( FlyingMouse ${FLYINGMOUSE_SOURCE_FILES} )

IF (APPLE)
    SET( APP_ICON RiftRay.icns ${SHADER_FILES} ${TEXTURES_FILES} ${HARDCODED_SHADERS})
ELSE()
    SET( APP_ICON "" )
ENDIF()

IF( GL_FRAMEWORK STREQUAL "GLFW" )
    ADD_EXECUTABLE( ${PROJECT_NAME} MACOSX_BUNDLE src/glfw_main.cpp  ${APP_ICON} )
ELSEIF( GL_FRAMEWORK STREQUAL "SDL" )
    ADD_EXECUTABLE( ${PROJECT_NAME} MACOSX_BUNDLE src/sdl_main.cpp ${APP_ICON})
ENDIF()

TARGET_LINK_LIBRARIES( ${PROJECT_NAME}
    AppSkeleton
    Scene
    Util
    FlyingMouse
    ${PLATFORM_LIBS}
    )

IF ( APPLE )
    SET(APPS "\${CMAKE_INSTALL_PREFIX}/RiftRay2.app")

    INSTALL(TARGETS RiftRay2 
        BUNDLE DESTINATION . COMPONENT Runtime
        RUNTIME DESTINATION bin COMPONENT Runtime
        )

    INSTALL(CODE "
        include(BundleUtilities)
        fixup_bundle(\"${APPS}\" \"\" \"\")
        " COMPONENT Runtime)

    # To Create a package, one can run "cpack -G DragNDrop CPackConfig.cmake" on Mac OS X
    set(CPACK_BINARY_DRAGNDROP ON)
    include(CPack)
ENDIF()
