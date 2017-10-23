#pragma once
#include <string>
#include <cmath>
#include "board2x3.h"
#include "type.h"

class solver {
public:
	typedef float value_t;

public:
	class answer {
	public:
		answer(value_t value) : value(value) {}
	    friend std::ostream& operator <<(std::ostream& out, const answer& a) {
	    	return out << (std::isnan(a.value) ? "-1" : std::to_string(a.value));
		}
	public:
		const value_t value;
	};

public:
	solver(const std::string& args) {
		// TODO: explore the tree and save the result
		std::cout << "feel free to display some messages..." << std::endl;
		std::cout << "solver is initialized." << std::endl << std::endl;
	}

	answer solve2x3(state_type type, const board2x3& b) {
		// TODO: find the answer in the lookup table and return it
		return -1;
	}

private:
	// TODO: place your transposition table here
};
