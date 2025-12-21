units=$(find . -type f -name *.cpp | tr '\n' ' ')
g++ $units -fdiagnostics-color=always -g -O0 -I include -L lib -std=c++26 -lSDL3 -lvulkan-1 -lgdi32 -o bin/debug/program.exe
