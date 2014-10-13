# copyDLLs.py
# Copies the necessary Windows DLLs to the executable directory.
# This script will be invoked by the CopyDLLsToBuild.cmake module
# and passed the following parameters:
# usage:
#   copyDLLs.py PROJECT_HOME BUILD_HOME LIBS_HOME
# For manual invocation using default variables:
#   cd tools/; python copyDLLs.py
#   Or just double-click tools/copyDLLs.py

import os
import sys
import shutil

# List of DLLs to copy for this project. Paths are listed in components to easily accomodate
# alternate configurations.
# @todo Use python's path handling library for correctness
commonDllList = [
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

def assembleBuild(buildHome, buildname, libsHome, dllList):
	""" Copy a list of dlls into a build directory."""
	#print buildHome
	dllDest = os.path.join(buildHome, buildname)
	print "  Assembling: " + dllDest

	if not os.path.isdir(dllDest):
		os.makedirs(dllDest)

	for f in dllList:
		src = os.path.join(libsHome, f[0], f[1], f[2])
		dst = os.path.join(dllDest, f[2])
		#print "  copy\n    ",src,"\n    ",dst
		print "    copying dll:", f[2]
		try:
			shutil.copyfile(src, dst)
		except IOError as e:
			print e


#
# Main: enter here
#
def main(argv=None):
	# Default values so we can run the script by double-clicking it in its
	# home location in $PROJECT/tools/.
	projectHome = ".."
	buildHome = ""
	libsHome = "C:/lib/"

	if argv:
		projectHome = argv[0]

	if argv and len(argv) > 1:
		buildHome = argv[1]
	else:
		buildHome = os.path.join(projectHome, "build")

	if argv and len(argv) > 2:
		libsHome = argv[2]

	assembleBuild(buildHome, "Debug", libsHome, commonDllList+debugDllList)
	assembleBuild(buildHome, "Release", libsHome, commonDllList+releaseDllList)


if __name__ == "__main__":
	sys.exit(main(sys.argv[1:]))

