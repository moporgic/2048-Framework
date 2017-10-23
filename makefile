all:
	g++ -std=c++0x -O3 -g -Wall -fmessage-length=0 -o 2048sol 2048sol.cpp 
clean:
	rm 2048sol