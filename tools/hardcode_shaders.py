# hardcode_shaders.py

from __future__ import print_function
import sys
import os

header = """/* GENERATED FILE - DO NOT EDIT!
 * Created by hardcode_shaders.py.
 *
 */
"""

def generateSourceFile():
	"""
	Output a hardcoded C++ source file with shaders as strings.
	"""
	shaderPath = "hardcoded_shaders/"
	autogenDir = "autogen/"
	sourceFileOut = autogenDir + "g_shaders.h"

	# Write a small comment if no shaders directory.
	if not os.path.isdir(shaderPath):
		print("Directory", shaderPath, "does not exist.")
		with open(sourceFileOut,'w') as outStream:
			print("/* Directory", shaderPath, "does not exist. */", file=outStream,)
		return

	# Create autogen/ if it's not there.
	if not os.path.isdir(autogenDir):
		os.makedirs(autogenDir)

	print("hardcode_shaders.py writing the following shaders to",autogenDir,":")
	shaderList = os.listdir(shaderPath)
	# filter out some extraneous results: directories, svn files...
	shaderList = [s for s in shaderList if s != '.svn']
	shaderList = [s for s in shaderList if not os.path.isdir(shaderPath + s)]
	for shaderName in shaderList:
		print("    hardcoding shader:", shaderName)

	tab = "    "
	decl = "const char* "
	newline = "\\n"
	quote = "\""

	with open(sourceFileOut,'w') as outStream:
		print(header, file=outStream)
		print("#include <map>", file=outStream)
		
		#shaderList = os.listdir(shaderPath)
		for shaderName in shaderList:
			file = shaderPath + shaderName
			lines = open(file).read().splitlines()
			varname = shaderName.replace(".","_")
			print("\n" + decl + varname + " = ", file=outStream)
			for l in lines:
				if l != "":
					l = l.replace('"', '\\"')
					print(tab + quote + l + newline + quote, file=outStream)
			print(";", file=outStream)

		mapvar = "g_shaderMap"
		print("\n", file=outStream)
		print("std::map<std::string, std::string> " + mapvar + ";", file=outStream)
		print("\n", file=outStream)

		print("void initShaderList() {", file=outStream)
		for fname in shaderList:
			varname = fname.replace(".","_")
			print(tab + mapvar + "[\"" + fname + "\"] = " + varname + ";", file=outStream)
		print("}", file=outStream)


#
# Main: enter here
#
def main(argv=None):
	# TODO: create directory if it doesn't exist
	generateSourceFile()


if __name__ == "__main__":
	sys.exit(main())
