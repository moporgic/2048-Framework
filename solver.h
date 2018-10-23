#pragma once
#include <iostream>
#include <algorithm>
#include <cmath>
#include "board.h"

class state_type {
public:
	enum type : char {
		before = 'b',
		after = 'a',
		illegal = 'i'
	};

public:
	state_type() : t(illegal) {}
	state_type(const state_type& st) = default;
	state_type(state_type::type code) : t(code) {}

	friend std::istream& operator >>(std::istream& in, state_type& type) {
		std::string s;
		if (in >> s) type.t = static_cast<state_type::type>((s + " ").front());
		return in;
	}

	friend std::ostream& operator <<(std::ostream& out, const state_type& type) {
		switch (type.t) {
		case state_type::before:  return out << "before";
		case state_type::after:   return out << "after";
		case state_type::illegal: return out << "illegal";
		default:                  return out << "unknown";
		}
	}

	bool is_before()  const { return t == before; }
	bool is_after()   const { return t == after; }
	bool is_illegal() const { return t == illegal; }

private:
	type t;
};


class solver {
public:
	typedef float value_t;

public:
	class answer {
	public:
		answer(value_t value) : value(value) {}
	    friend std::ostream& operator <<(std::ostream& out, const answer& ans) {
	    	return out << (std::isnan(ans.value) ? -1 : ans.value);
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

	answer solve2x3(const board& state, state_type type = state_type::before) {
		// TODO: find the answer in the lookup table and return it
		return -1;
	}

private:
	// TODO: place your transposition table here
};
