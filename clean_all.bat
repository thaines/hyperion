cd eos
call clean.bat
rm -R doc
cd ..

cd ares
call clean.bat
cd ..

cd docs/cyclops
call clean.bat
cd ../..

cd dist
call clean.bat
cd ..