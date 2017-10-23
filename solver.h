#pragma once
#include <string>
#include "board2x3.h"
#include "type.h"

class solver {
public:
	typedef float value_t;

public:
	class answer {
	public:
		answer(size_t occur, value_t value) : occur(occur), value(value) {}
	    friend std::ostream& operator <<(std::ostream& out, const answer& a) {
	    	out << a.occur;
	    	return a.occur ? out << " " << std::to_string(a.value) : out;
		}
	public:
		const size_t occur;
		const value_t value;
	};

public:
	solver(const std::string& args) {
		// TODO: expand the tree and save the result
		std::cout << "solver is initializing..." << std::endl;
	}

	answer solve2x3(state_type type, const board2x3& b) {
		// TODO: find the answer in the lookup table and return it
		return { 0, 0 };
	}

private:
	// TODO: place your transposition table here
};
