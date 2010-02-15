#!/bin/bash

cd eos
./clean.sh
rm -R doc
cd ..

cd ares
./clean.sh
cd ..

cd docs/cyclops
./clean.sh
cd ../..

cd dist
./clean.sh
cd ..
