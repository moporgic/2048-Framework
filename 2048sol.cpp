/**
 * Basic Environment for Game 2048
 * use 'g++ -std=c++0x -O3 -g -o 2048 2048.cpp' to compile the source
 *
 * Computer Games and Intelligence (CGI) Lab, NCTU, Taiwan
 * http://www.aigames.nctu.edu.tw
 */

#include <iostream>
#include <algorithm>
#include <functional>
#include <iterator>
#include <string>
#include <random>
#include <sstream>
#include <fstream>
#include <cmath>
#include <chrono>

#include "action2x3.h"
#include "board2x3.h"
#include "type.h"
#include "solver.h"

int main(int argc, const char* argv[]) {
	std::cout << "2048-Solver: ";
	std::copy(argv, argv + argc, std::ostream_iterator<const char*>(std::cout, " "));
	std::cout << std::endl << std::endl;

	std::string solve_args;
	for (int i = 1; i < argc; i++) {
		std::string para(argv[i]);
		if (para.find("--solve=") == 0) {
			solve_args = para.substr(para.find("=") + 1);
		}
	}

	solver solve(solve_args);
	state_type type;
	board2x3 test;
	while (std::cin >> type >> test) {
		auto ans = solve.solve2x3(type, test);
		std::cout << "= " << ans << std::endl;
	}

	return 0;
}
