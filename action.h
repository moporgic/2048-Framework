#pragma once
#include <algorithm>
#include <string>
#include "board.h"

class action {
public:
	action(unsigned code = -1u) : code(code) {}
	action(const action& a) : code(a.code) {}
	class slide;
	class place;

public:
	int apply(board& b) const;
	std::string name() const;
	std::ostream& operator >>(std::ostream& out) const;
	std::istream& operator <<(std::istream& in);

public:
	operator unsigned() const { return code; }
	unsigned type() const { return code & type_flag(-1u); }
	unsigned event() const { return code & ~type(); }
	template<class alias> alias& cast() const { return reinterpret_cast<alias&>(const_cast<action&>(*this)); }
	friend std::ostream& operator <<(std::ostream& out, const action& a) { return a >> out; }
	friend std::istream& operator >>(std::istream& in, action& a) { return a << in; }

protected:
	static constexpr unsigned type_flag(unsigned v) { return v << 24; }
	unsigned code;
};


/**
 * create a sliding action with opcode
 * 0=UP, 1=RIGHT, 2=DOWN, 3=LEFT
 */
class action::slide : public action {
public:
	static constexpr unsigned type = type_flag('s');
	slide(unsigned oper) : action(slide::type | (oper % 4)) {}
	slide(const action& a) : action(a) {}

	int apply(board& b) const {
		return b.slide(event());
	}
	std::string name() const {
		std::string data[] = { "up", "right", "down", "left", "illegal" };
		return "slide " + data[std::min(event(), 4u)];
	}
	std::ostream& operator >>(std::ostream& out) const {
		return out << '#' << ("URDL?")[std::min(event(), 4u)];
	}
	std::istream& operator <<(std::istream& in) {
		if (in.peek() == '#') {
			const char* opc = "URDL";
			char v;
			in.ignore(1) >> v;
			unsigned op = std::find(opc, opc + 4, v) - opc;
			if (op < 4) {
				operator= (action::slide(op));
				return in;
			}
		}
		in.setstate(std::ios::failbit);
		return in;
	}
};

/**
 * create a placing action with position and tile
 * 0 <= position < 16, tile should be in index form
 */
class action::place : public action {
public:
	static constexpr unsigned type = type_flag('p');
	place(unsigned pos, unsigned tile) : action(place::type | (pos & 0x0f) | (tile % 36 << 4)) {}
	place(const action& a) : action(a) {}
	unsigned position() const { return event() & 0x0f; }
	unsigned tile() const { return event() >> 4; }

	int apply(board& b) const {
		return b.place(position(), tile());
	}
	std::string name() const {
		return "place " + std::to_string(tile()) + "-index at position " + std::to_string(position());
	}
	std::ostream& operator >>(std::ostream& out) const {
		const char* idx = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
		return out << idx[position()] << idx[std::min(tile(), 35u)];
	}
	std::istream& operator <<(std::istream& in) {
		const char* idx = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
		char v = in.peek();
		unsigned pos = std::find(idx, idx + 16, v) - idx;
		if (pos < 16) {
			in.ignore(1) >> v;
			unsigned tile = std::find(idx, idx + 36, v) - idx;
			operator =(action::place(pos, tile));
			return in;
		}
		in.setstate(std::ios_base::failbit);
		return in;
	}
};


int action::apply(board& b) const {
	switch (type()) {
	case slide::type:
		return cast<slide>().apply(b);
	case place::type:
		return cast<place>().apply(b);
	default:
		return -1;
	}
}
std::string action::name() const {
	switch (type()) {
	case slide::type:
		return cast<slide>().name();
	case place::type:
		return cast<place>().name();
	default:
		return "unknown";
	}
}
std::ostream& action::operator >>(std::ostream& out) const {
	switch (type()) {
	case slide::type:
		return cast<slide>() >> out;
	case place::type:
		return cast<place>() >> out;
	default:
		return out << "??";
	}
}
std::istream& action::operator <<(std::istream& in) {
	if (cast<slide>() << in) return in;
	in.clear();
	if (cast<place>() << in) return in;
	in.clear();
	in.ignore(2).setstate(std::ios_base::failbit);
	return in;
}
