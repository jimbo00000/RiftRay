## Build Instructions

The project is built using the cross-platform build system [CMake](http://www.cmake.org/).

If you have it installed, try using cmake-gui instead of the command line cmake to set build configuration variables before generating Makefiles, a Visual Studio solution, or whatever the case may be. Press the "Configure" button until you don't see red, then click "Generate".

### Windows

    Create the directory build/ in project's home(alongside CMakeLists.txt)
    Shift+right-click it in Explorer->"Open command window here"
    build> cmake ..
    Double-click the only .sln file in build to open it in Visual Studio
    Right-click the GLSkeleton project in Solution Explorer, "Set as StartUp Project"
    Press F7 to build, F5 to build and run

### Mac/Linux

    $> cd RiftRay
    $> mkdir build
    $> cd build
    $> cmake .. && make
    $> ./RiftRay