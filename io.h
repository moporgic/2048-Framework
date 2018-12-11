#pragma once
#include <string>
#include <sstream>
#include <iostream>

class output {
public:
	output(const std::string& init = "") : out(init) {}
	output(const output&) = delete;
	~output() { std::cout << out.str() << std::flush; }
	template<typename type> output& operator<<(const type& v) { out << v; return *this; }
	output& operator<<(std::ios_base& (*pf)(std::ios_base&)) { out << pf; return *this; }
	output& operator<<(std::ostream& (*pf)(std::ostream&)) { out << pf; return *this; }
	output& operator =(const output&) = delete;
private:
	std::stringstream out;
};

class input {
public:
	input() {}
	input(const input&) = delete;
	operator bool() const { return bool(std::cin); }
	operator std::string() { std::string line; operator>>(line); return line; }
	input& operator>>(std::string& line) {
		if (std::getline(std::cin, line, '\n'))
			if (line.size() && line.back() == '\r') line.pop_back();
		return *this;
	}
	input& operator =(const input&) = delete;
};
