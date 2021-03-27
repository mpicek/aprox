all: aprox

.PHONY: all clean valgrind format

aprox: main.cpp distribution.hpp expression.hpp
	g++ main.cpp -o aprox -std=c++17 -Wall -Wextra

valgrind:
	valgrind ./aprox --leak-check=full < inp

format:
	clang-format -style=llvm main.cpp > main_format.cpp

clean:
	rm aprox