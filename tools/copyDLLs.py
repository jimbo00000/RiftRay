# copyDLLs.py
# Copies the necessary Windows DLLs to the executable directory.
# This script will be invoked by CMakeLists.txt and passed the following parameters:
# usage:
#   copyDLLs.py PROJECT_HOME BUILD_HOME [decls...]
# For manual invocation using default variables:
#   cd tools/; python copyDLLs.py
#   Or just double-click tools/copyDLLs.py

from __future__ import print_function
import os
import sys
import shutil

libsHome = os.path.join('C:', os.sep, 'lib')
# These home directories may be overridden by cmd line args
libraryRootDirs = {
	'ANTTWEAKBAR_ROOT': os.path.join(libsHome, 'AntTweakBar_116', 'AntTweakBar'),
	'SIXENSE_ROOT': os.path.join(libsHome, 'SixenseSDK'),
	'GLEW_ROOT': os.path.join(libsHome, 'glew-1.12.0'),
	'SDL2_ROOT': os.path.join(libsHome, 'SDL2-2.0.3'),
	'SFML_ROOT': os.path.join(libsHome, 'SFML-2.2'),
}

# List of DLLs to copy for this project. Paths are listed in components to easily accomodate
# alternate configurations.
# @todo Use python's path handling library for correctness
commonDllList = [
	['ANTTWEAKBAR_ROOT', ['lib'], 'AntTweakBar.dll' ],
	['GLEW_ROOT', ['bin', 'Release', 'Win32'], 'glew32.dll' ],
	['SDL2_ROOT', ['lib','x86'], 'SDL2.dll' ],
	['SIXENSE_ROOT', ['samples','win32','sixense_simple3d'], 'DeviceDLL.dll' ],
]

debugDllList = [
	['SIXENSE_ROOT', ['bin','win32','debug_dll'], 'sixensed.dll' ],
	['SIXENSE_ROOT', ['bin','win32','debug_dll'], 'sixense_utilsd.dll' ],
	['SFML_ROOT', ['bin'], 'sfml-system-2.dll' ],
	['SFML_ROOT', ['bin'], 'sfml-window-2.dll' ],
]

releaseDllList = [
	['SIXENSE_ROOT', ['bin','win32','release_dll'], 'sixense.dll' ],
	['SIXENSE_ROOT', ['bin','win32','release_dll'], 'sixense_utils.dll' ],
]

def assembleBuild(buildHome, buildname, dllList):
	""" Copy a list of dlls into a build directory."""
	dllDest = os.path.join(buildHome, buildname)
	print("  Assembling: " + dllDest)

	if not os.path.isdir(dllDest):
		os.makedirs(dllDest)

	for f in dllList:
		libHome = libraryRootDirs[f[0]]
		pathList = []
		pathList.append(libHome)
		pathList.extend(f[1])
		pathList.append(f[2])
		src = os.path.join(*pathList)
		dst = os.path.join(dllDest, f[2])
		print("    copying dll [{0}] to [{1}]".format(src, dst))
		try:
			shutil.copyfile(src, dst)
		except IOError as e:
			print("     ", e)


#
# Main: enter here
#
def main(argv=None):
	print("copyDLLs.py: Assembling DLLs in output directories:")
	#print(argv)

	# Default values so we can run the script by double-clicking it in its
	# home location in $PROJECT/tools/.
	projectHome = ".."
	buildHome = ""

	if argv:
		projectHome = argv[0]

	if argv and len(argv) > 1:
		buildHome = argv[1]
	else:
		buildHome = os.path.join(projectHome, "build")

	if len(argv) > 2:
		decls = argv[2:]
		for d in decls:
			#print(d)
			if '=' in d:
				terms = d.split('=')
				if len(terms) == 2:
					if len(terms[1]) > 0:
						libraryRootDirs[terms[0]] = terms[1]
	#print(libraryRootDirs)

	assembleBuild(buildHome, "Debug", commonDllList+debugDllList)
	assembleBuild(buildHome, "Release", commonDllList+releaseDllList)


if __name__ == "__main__":
	sys.exit(main(sys.argv[1:]))
