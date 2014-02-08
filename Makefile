main: main.cpp utils.h gf256.h aes.h
	g++ -std=c++11 -O2 main.cpp -o main -Wall -Wextra
