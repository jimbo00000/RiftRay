# hardcode_shaders.py

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
		print "Directory", shaderPath, "does not exist."
		with open(sourceFileOut,'w') as outStream:
			print >>outStream, "/* Directory", shaderPath, "does not exist. */"
		return

	# Create autogen/ if it's not there.
	if not os.path.isdir(autogenDir):
		os.makedirs(autogenDir)

	print "hardcode_shaders.py writing the following shaders to",autogenDir,":"
	shaderList = os.listdir(shaderPath)
	# filter out some extraneous results: directories, svn files...
	shaderList = [s for s in shaderList if s != '.svn']
	shaderList = [s for s in shaderList if not os.path.isdir(shaderPath + s)]
	for shaderName in shaderList:
		print "    hardcoding shader:", shaderName

	tab = "    "
	decl = "const char* "
	newline = "\\n"
	quote = "\""

	with open(sourceFileOut,'w') as outStream:
		print >>outStream, header
		print >>outStream, "#include <map>"
		
		#shaderList = os.listdir(shaderPath)
		for shaderName in shaderList:
			file = shaderPath + shaderName
			lines = open(file).read().splitlines()
			varname = shaderName.replace(".","_")
			print >>outStream, "\n" + decl + varname + " = "
			for l in lines:
				if l != "":
					l = l.replace('"', '\\"')
					print >>outStream, tab + quote + l + newline + quote
			print >>outStream, ";"

		mapvar = "g_shaderMap"
		print >>outStream, "\n"
		print >>outStream, "std::map<std::string, std::string> " + mapvar + ";"
		print >>outStream, "\n"

		print >>outStream, "void initShaderList() {"
		for fname in shaderList:
			varname = fname.replace(".","_")
			print >>outStream, tab + mapvar + "[\"" + fname + "\"] = " + varname + ";"
		print >>outStream, "}"


#
# Main: enter here
#
def main(argv=None):
	# TODO: create directory if it doesn't exist
	generateSourceFile()


if __name__ == "__main__":
	sys.exit(main())
