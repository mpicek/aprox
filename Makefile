all: main

.PHONY: all clean valgrind format

main: main.cpp distribution.hpp expression.hpp
	g++ main.cpp -o main -std=c++17 -Wall -Wextra

valgrind:
	valgrind ./main --leak-check=full < inp

format:
	clang-format -style=llvm main.cpp > main_format.cpp

clean:
	rm main