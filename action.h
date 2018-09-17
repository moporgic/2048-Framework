#pragma once
#include <string>
#include "board.h"

class action {
public:
	action(const action& act) : code(act) {}
	action(unsigned op = -1u) : code(op) {}
	operator unsigned() const { return code; }

public:
	enum type : unsigned {
		type_slide = 'p' << 16,
		type_place = 'e' << 16,

		type_mask  = -1u << 16
	};

public:
	/**
	 * apply this action to a given board
	 */
	int apply(board& b) const {
		unsigned label = code & type_mask;
		unsigned value = code & ~type_mask;
		switch (label) {
		case type_slide: // player action (slide up, right, down, left)
			return b.slide(value);
		case type_place: // environment action (place a new tile)
			return b.place(value & 0x0f, value >> 4);
		default:
			return -1;
		}
	}

	/**
	 * get the name of this action
	 */
	std::string name() const {
		std::string opname[] = { "up", "right", "down", "left", "illegal" };
		unsigned label = code & type_mask;
		unsigned value = code & ~type_mask;
		switch (label) {
		case type_slide: // player action (slide up, right, down, left)
			return "slide " + opname[std::min(unsigned(value), 4u)];
		case type_place: // environment action (place a new tile)
			return "place " + std::to_string(value >> 4) + "-index at position " + std::to_string(value & 0x0f);
		default:
			return "null";
		}
	}

	/**
	 * create a sliding action with opcode
	 * 0=UP, 1=RIGHT, 2=DOWN, 3=LEFT
	 */
	static action slide(int oper) {
		return action(type_slide | (oper % 4));
	}
	/**
	 * create a placing action with position and tile
	 * 0 <= position < 16, tile should be in index form
	 */
	static action place(int pos, int tile) {
		return action(type_place | (pos & 0x0f) | (tile % 36 << 4));
	}

public:
	action& operator =(const action& a) { code = a; return *this; }
	bool operator ==(const action& a) const { return code == a.code; }
	bool operator < (const action& a) const { return code <  a.code; }
	bool operator !=(const action& a) const { return !(*this == a); }
	bool operator > (const action& a) const { return a < *this; }
	bool operator <=(const action& a) const { return !(a < *this); }
	bool operator >=(const action& a) const { return !(*this < a); }

public:
	friend std::ostream& operator <<(std::ostream& out, const action& a) {
		const char* idx = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
		unsigned label = unsigned(a) & type_mask;
		unsigned value = unsigned(a) & ~type_mask;
		switch (label) {
		case type_slide: // player action (slide up, right, down, left)
			return out << '#' << ("URDL")[value];
		case type_place: // environment action (place a new tile)
			return out << idx[value & 0x0f] << idx[value >> 4];
		default:
			return out << "??";
		}
	}
	friend std::istream& operator >>(std::istream& in, action& a) {
		char v;
		if (in.peek() == '#') {
			const char* opc = "URDL";
			in.ignore(1) >> v;
			unsigned op = std::find(opc, opc + 4, v) - opc;
			a = action::slide(op);
		} else if (in.peek() != '?') {
			const char* idx = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
			in >> v;
			unsigned pos = std::find(idx, idx + 36, v) - idx;
			in >> v;
			unsigned tile = std::find(idx, idx + 36, v) - idx;
			a = action::place(pos, tile);
		} else {
			in.ignore(2);
			in.setstate(std::ios_base::failbit);
		}
		return in;
	}

private:
	unsigned code;
};
