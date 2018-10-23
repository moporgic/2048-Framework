/**
 * Basic Environment for Game 2048
 * use 'g++ -std=c++11 -O3 -g -o 2048 2048.cpp' to compile the source
 *
 * Computer Games and Intelligence (CGI) Lab, NCTU, Taiwan
 * http://www.aigames.nctu.edu.tw
 */

#include <iostream>
#include <fstream>
#include <iterator>
#include <string>
#include "board.h"
#include "solver.h"

int main(int argc, const char* argv[]) {
	std::cout << "2048-Demo: ";
	std::copy(argv, argv + argc, std::ostream_iterator<const char*>(std::cout, " "));
	std::cout << std::endl << std::endl;

	std::string solve_args;
	int precision = 10;
	for (int i = 1; i < argc; i++) {
		std::string para(argv[i]);
		if (para.find("--solve=") == 0) {
			solve_args = para.substr(para.find("=") + 1);
		} else if (para.find("--precision=") == 0) {
			precision = std::stol(para.substr(para.find("=") + 1));
		}
	}

	solver solve(solve_args);
	board state;
	state_type type;
	state_hint hint(state);
	std::cout << std::setprecision(precision);
	while (std::cin >> type >> state >> hint) {
		auto value = solve.solve(state, type);
		std::cout << type << " " << state << " " << hint;
		std::cout << " = " << value << std::endl;
	}

	return 0;
}
