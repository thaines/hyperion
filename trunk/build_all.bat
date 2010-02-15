cd eos
redir -eo mingw32-make win_debug > out.txt
redir -eo mingw32-make win_release >> out.txt
redir -eo mingw32-make win_docs >> out.txt
start notepad out.txt
cd ..

cd ares
redir -eo mingw32-make win_debug > out.txt
redir -eo mingw32-make win_release >> out.txt
start notepad out.txt
cd ..

cd docs/cyclops
call build.bat
cd ../..

cd dist
call build.bat
cd ..
