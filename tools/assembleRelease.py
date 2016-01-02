# assembleRelease.py
# Create a Windows release package in the current directory.

from __future__ import print_function
import os
import shutil
import glob
import datetime

srcDir = ".."
versionFile = os.path.join(srcDir, "src", "version.h")
versionNumber = None
with open(versionFile, "r") as vf:
    vlines = vf.readlines()
    for vl in vlines:
        toks = vl.split('"')
        if len(toks) > 1:
            versionNumber = toks[1]

if not versionNumber:
    print("Could not find version. Exiting.")
    quit()

version = versionNumber + datetime.date.today().strftime('-%Y.%m.%d')
print(version)
dstDir = "RiftRay-" + version
if not os.path.exists(dstDir):
    os.mkdir(dstDir)

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
bins = ["RiftRay3.exe"]
bins.extend(glob.glob(os.path.join(binSrcDir, "*.dll")))
bins = [os.path.basename(b) for b in bins]

binDstDir = os.path.join(dstDir)
if not os.path.exists(binDstDir):
    os.mkdir(binDstDir)
for b in bins:
    shutil.copy(os.path.join(binSrcDir,b), os.path.join(binDstDir, b))
