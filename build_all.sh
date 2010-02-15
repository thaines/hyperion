#!/bin/bash

cd eos
make lin_debug 1> out.txt 2>&1
make lin_release 1>> out.txt 2>&1
make lin_docs 1>> out.txt 2>&1
gedit out.txt &
cd ..

cd ares
make lin_debug 1> out.txt 2>&1
make lin_release 1>> out.txt 2>&1
gedit out.txt &
cd ..

cd docs/cyclops
./build.sh
cd ../..

cd dist
./build.sh
cd ..
