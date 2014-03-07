# assembleRelease.py
# Create a release package in the current directory.

import os
import sys
import stat
import shutil

version = "1.0"

# Create directory structure
dirName = "RiftRay-" + version
binDir = os.path.join(dirName, "bin")
docDir = os.path.join(dirName, "doc")
shadersDir = os.path.join(dirName, "shaders")
texturesDir = os.path.join(dirName, "textures")
shadertoyDir = os.path.join(shadersDir, "shadertoy")

dirs = [dirName, binDir, docDir, shadersDir, texturesDir, shadertoyDir]
for d in dirs:
	if not os.path.exists(d):
		os.mkdir(d)

# Assemble file lists
shadertoys = os.listdir(os.path.join("..", "shaders", "shadertoy"))
shadertoys = [s for s in shadertoys if s.find(".glsl") > -1]
print shadertoys

textures = os.listdir(os.path.join("..", "textures"))
textures = [t for t in textures if not t[0] == '.']
print textures

dlls = []
debugDir = os.path.join("..", "build", "Debug")
if os.path.exists(debugDir):
    dlls = os.listdir(debugDir)
dlls = [d for d in dlls if d.find(".dll") > -1]
print dlls

shaders = ["rwwtt.vert", "rwwtt.frag", "rwwtt_header.glsl", "rwwtt_footer.glsl"]

def copyFilesFromDirToDir(fileList, fromDir, toDir):
	"""
	Copy a list of files from one directory to another.
	@param fileList List of filenames(without leading directory)
	@param fromDir Source directory for files in list
	@param toDir Destination directory
	"""
	for l in fileList:
		src = os.path.join(fromDir, l)
		dst = os.path.join(toDir, l)
		if not os.path.exists(src):
			continue
		#print src,dst
		shutil.copyfile(src, dst)


# Copy files in
shadertoysSrc = os.path.join("..", "shaders", "shadertoy")
copyFilesFromDirToDir(shadertoys, shadertoysSrc, shadertoyDir)

shadersSrc = os.path.join("..", "shaders")
copyFilesFromDirToDir(shaders, shadersSrc, shadersDir)

texturesSrc = os.path.join("..", "textures")
copyFilesFromDirToDir(textures, texturesSrc, texturesDir)


# Docs
readmeSrc = os.path.join("..")
readmeName = "README.md"
readmeSrcFull = os.path.join(readmeSrc, readmeName)
readmeDstFull = os.path.join(dirName, readmeName)
shutil.copyfile(readmeSrcFull, readmeDstFull)

docimgSrc = os.path.join("..", "doc")
docimgName = "NvidiaSettings-crop.png"
docimgSrcFull = os.path.join(docimgSrc, docimgName)
docimgDstFull = os.path.join(dirName, "doc", docimgName)
shutil.copyfile(docimgSrcFull, docimgDstFull)


# Windows/Visual Studio-specific
binSrc = os.path.join("..", "build", "Release")
binName = "RiftRay.exe"
print sys.platform
if sys.platform == 'linux2':
	binName = "RiftRay"
	binSrc = os.path.join("..", "build")
binSrcFull = os.path.join(binSrc, binName)
binDstFull = os.path.join(binDir, binName)
shutil.copyfile(binSrcFull, binDstFull)
if sys.platform == 'linux2':
	#os.chmod(binDstFull, stat.S_IXUSR | stat.S_IXGRP | stat.S_IXOTH)
	os.chmod(binDstFull, 0o777)
	# Create the zip archive
	import tarfile
	with tarfile.open(dirName + ".tar.gz", "w:gz") as tar:
		tar.add(dirName, arcname=os.path.basename(dirName))

# DLLs
dllsSrc = binSrc
dllsDir = binDir
copyFilesFromDirToDir(dlls, dllsSrc, dllsDir)

