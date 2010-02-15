#!/bin/bash

# Cyclops itself...
mkdir -p cyclops

cp license.txt cyclops
cp lgpl.txt cyclops


cp -R ../ares/dep/doc cyclops
cp -R ../ares/dep/tut cyclops

cp ../ares/bin/release/cyclops cyclops
cp ../ares/bin/release/libIL.so cyclops
cp ../ares/bin/release/libeos.so.1.0 cyclops
cp ../ares/bin/release/svt cyclops
cp ../ares/bin/release/run.sh cyclops

cp ../docs/cyclops/cyclops.pdf cyclops/doc/manual.pdf

cd cyclops
ln -sf libeos.so.1.0 libeos.so.1
ln -sf libeos.so.1 libeos.so
cd ..

tar -czf cyclops.tar.gz cyclops/



# The re-distributable source code...
mkdir -p redist

cp readme.txt redist
cp lgpl.txt redist

cp ../eos/src/eos/file/dlls.h redist
cp ../eos/src/eos/file/dlls.cpp redist

cp ../eos/src/eos/gui/gtk_funcs.h redist
cp ../eos/src/eos/gui/gtk_funcs.cpp redist

cp ../eos/src/eos/file/devil_funcs.h redist
cp ../eos/src/eos/file/devil_funcs.cpp redist

cp ../eos/src/eos/os/gphoto2_funcs.h redist
cp ../eos/src/eos/os/gphoto2_funcs.cpp redist

tar -czf redist.tar.gz redist/

