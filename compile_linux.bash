
rm -rf build/
mkdir build/
gcc -O0 -Isrc -Ilib src/linux.cpp -o game -lX11 -lGL -lGLU -lGLEW -lm -std=c++11 -w
