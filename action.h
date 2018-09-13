#pragma once
#include <string>
#include "board.h"

class action {
public:
	action(const action& act) : code(act) {}
	action(const int& op = -1) : code(op) {}
	operator int() const { return code; }

public:
	action& operator =(const action& a) { code = a; return *this; }
	bool operator ==(const action& a) const { return code == int(a); }
	bool operator < (const action& a) const { return code <  int(a); }
	bool operator !=(const action& a) const { return !(*this == a); }
	bool operator > (const action& a) const { return a < *this; }
	bool operator <=(const action& a) const { return !(a < *this); }
	bool operator >=(const action& a) const { return !(*this < a); }

public:

	int apply(board& b) const {
		if ((0b11 & code) == (code)) {
			// player action (slide up, right, down, left)
			return b.slide(code);
		} else if (b(code & 0x0f) == 0) {
			// environment action (place a new tile)
			b(code & 0x0f) = (code >> 4);
			return 0;
		}
		return -1;
	}

	std::string name() const {
		if ((0b11 & code) == (code)) {
			std::string opname[] = { "up", "right", "down", "left" };
			return "slide " + opname[code];
		} else {
			return "place " + std::to_string(code >> 4) + "-index at position " + std::to_string(code & 0x0f);
		}
		return "null";
	}

	static action slide(const int& oper) {
		return action(oper);
	}
	static action place(const int& tile, const int& pos) {
		return action((tile << 4) | (pos));
	}

public:
	friend std::ostream& operator <<(std::ostream& out, const action& a) {
		if ((0b11 & a.code) == (a.code)) {
			return out << '#' << ("URDL")[a.code];
		} else {
			const char* idx = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
			return out << idx[a.code & 0x0f] << idx[(a.code >> 4) % 36];
		}
		return out;
	}
	friend std::istream& operator >>(std::istream& in, action& a) {
		char v;
		in >> v;
		if (v == '#') {
			in >> v;
			const char* idx = "URDL";
			a.code = std::find(idx, idx + 4, v) - idx;
		} else {
			const char* idx = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
			a.code = std::find(idx, idx + 36, v) - idx;
			in >> v;
			a.code |= (std::find(idx, idx + 36, v) - idx) << 4;
		}
		return in;
	}

private:
	int code;
};
