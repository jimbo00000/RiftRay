# copyDLLs.py
# Copy the necessary Windows DLLs to the executable directory.
# usage: cd tools/; python copyDLLs.py
# Or just double-click copyDLLs.py .

import os
import shutil

# These directories depend on local setup, where repositories are checked out.
# @todo Maybe grab this directory from CMakeCache
dllHome = "C:/lib/"
projectHome = "../"
dllDest = projectHome + "build/Release/"

# List of DLLs to copy. Paths are listed in components to easily accomodate
# alternate configurations
# @todo Use python's path handling library for correctness
dllList = [
	["AntTweakBar_116/" , "AntTweakBar/lib/"    , "AntTweakBar.dll"],
	["glew/"            , "bin/"                , "glew32.dll"     ],
	["SixenseSDK/"      , "bin/win32/debug_dll/"           , "sixensed.dll"  ],
	["SixenseSDK/"      , "bin/win32/debug_dll/"           , "sixense_utilsd.dll"  ],
	["SixenseSDK/"      , "samples/win32/sixense_simple3d/", "DeviceDLL.dll"  ],
]

for f in dllList:
    src = dllHome + f[0] + f[1] + f[2]
    dst = dllDest + f[2]
    print src,dst
    shutil.copyfile(src, dst)
