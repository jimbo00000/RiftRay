# assembleRelease.py
# Create a Windows release package in the current directory.

from __future__ import print_function
import os
import shutil
import glob

version = "2.0.1-2014.11.12"
dstDir = "RiftRay-" + version
if not os.path.exists(dstDir):
    os.mkdir(dstDir)

srcDir = ".."

# data directories
dirs = ["shaders", "textures"]
for d in dirs:
    dd = os.path.join(dstDir, d)
    if not os.path.exists(dd):
        shutil.copytree(os.path.join(srcDir,d), dd)

# documentation
docs = ["LICENSE", "README.md", "RiftRay.cfg"]
for f in docs:
    shutil.copy(os.path.join(srcDir,f), os.path.join(dstDir, f))

# binaries
binSrcDir = os.path.join(srcDir, os.path.join("build", "Release"))
bins = ["RiftRay2.exe"]
bins.extend(glob.glob(os.path.join(binSrcDir, "*.dll")))
bins = [os.path.basename(b) for b in bins]

bin = "bin"
binDstDir = os.path.join(dstDir, bin)
if not os.path.exists(binDstDir):
    os.mkdir(binDstDir)
for b in bins:
    shutil.copy(os.path.join(binSrcDir,b), os.path.join(binDstDir, b))

# simple run script
script = """
@echo off
cd /d bin
start "" RiftRay2.exe
"""
with open(os.path.join(dstDir,"RunRiftRay.bat"), "w") as outfile:
    print(script, file=outfile)
