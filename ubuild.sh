rm -rf build/
mkdir build
cd build
cmake ..
make -j8

echo " BUILD FINISHED AT : $(date)"
