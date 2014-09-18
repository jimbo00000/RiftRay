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

# List of DLLs to copy. Paths are listed in components to easily accomodate
# alternate configurations.
# @todo Use python's path handling library for correctness
dllList = [
	["AntTweakBar_116/" , "AntTweakBar/lib/"    , "AntTweakBar.dll"],
	["glew/"            , "bin/"                , "glew32.dll"     ],
	["SDL2-2.0.3/"      , "lib/x86/"            , "SDL2.dll"       ],
	["SixenseSDK/"      , "samples/win32/sixense_simple3d/", "DeviceDLL.dll"  ],
]

debugDllList = [
	["SixenseSDK/", "bin/win32/debug_dll/", "sixensed.dll" ],
	["SixenseSDK/", "bin/win32/debug_dll/", "sixense_utilsd.dll" ],
]

releaseDllList = [
	["SixenseSDK/", "bin/win32/release_dll/", "sixense.dll" ],
	["SixenseSDK/", "bin/win32/release_dll/", "sixense_utils.dll" ],
]

def assembleBuild(buildname, dllHome, dllList):
	""" Copy a list of dlls into a build directory.
	"""
	print "__Assembling: " + buildname
	dllDest = projectHome + "build/" + buildname + "/"
	for f in dllList:
		src = dllHome + f[0] + f[1] + f[2]
		dst = dllDest + f[2]
		print "  copy\n    ",src,"\n    ",dst
		try:
			shutil.copyfile(src, dst)
		except IOError as e:
			print e

assembleBuild("Debug", dllHome, dllList+debugDllList)
assembleBuild("Release", dllHome, dllList+releaseDllList)
