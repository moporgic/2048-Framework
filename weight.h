#pragma once
#include <iostream>
#include <vector>
#include <array>
#include <sstream>
#include <iterator>
#include <string>

/**
 * weight table of n-tuple network
 */
class weight {
public:
	weight() : length(0), value(nullptr) {}
	weight(const size_t& len) : length(len), value(alloc(len)) {}
	weight(weight&& f) : length(f.length), value(f.value) { f.value = nullptr; }
	weight(const weight& f) = delete;
	weight& operator =(const weight& f) = delete;
	virtual ~weight() { delete[] value; }

	float& operator[] (const size_t& i) { return value[i]; }
	const float& operator[] (const size_t& i) const { return value[i]; }
	size_t size() const { return length; }

public:
	friend std::ostream& operator <<(std::ostream& out, const weight& w) {
		float* value = w.value;
		size_t size = w.size();
		out.write(reinterpret_cast<char*>(&size), sizeof(size_t));
		out.write(reinterpret_cast<char*>(value), sizeof(float) * size);
		return out;
	}

	friend std::istream& operator >>(std::istream& in, weight& w) {
		float* value = w.value;
		size_t& size = w.length;
		if (in.read(reinterpret_cast<char*>(&size), sizeof(size_t))) {
			if (value) delete[] value;
			value = alloc(size);
			in.read(reinterpret_cast<char*>(value), sizeof(float) * size);
		}
		if (!in) {
			std::cerr << "unexpected end of binary" << std::endl;
			std::exit(1);
		}
		return in;
	}

protected:
	static float* alloc(size_t num) {
		static size_t total = 0;
		static size_t limit = (2 << 30) / sizeof(float); // 2G memory
		try {
			total += num;
			if (total > limit) throw std::bad_alloc();
			return new float[num]();
		} catch (std::bad_alloc&) {
			std::cerr << "memory limit exceeded" << std::endl;
			std::exit(-1);
		}
		return nullptr;
	}

	size_t length;
	float* value;
};

class feature : public weight {
public:
	feature() {}
	feature(const size_t& len) : weight(len) {}
	feature(feature&& f) : weight(std::move(f)) { f.value = nullptr; }
	feature(const feature& f) = delete;
	virtual ~feature() {}

public: // should be implemented

	/**
	 * estimate the value of a given board
	 */
	virtual float estimate(const board& b) const { return 0; };
	/**
	 * update the value of a given board, and return its updated value
	 */
	virtual float update(const board& b, const float& u) { return 0; };
	/**
	 * get the name of this weight
	 */
	virtual std::string name() const { return ""; };

public:
	/**
	 * dump the detail of weight table of a given board
	 */
	virtual void dump(const board& b, std::ostream& out = std::cout) const {
		out << b << "estimate = " << estimate(b) << std::endl;
	}

	friend std::ostream& operator <<(std::ostream& out, const feature& f) {
		std::string name = f.name();
		int len = name.length();
		out.write(reinterpret_cast<char*>(&len), sizeof(int));
		out.write(name.c_str(), len);
		out << (*reinterpret_cast<const weight*>(&f));
		return out;
	}

	friend std::istream& operator >>(std::istream& in, feature& f) {
		std::string name;
		int len = 0;
		in.read(reinterpret_cast<char*>(&len), sizeof(int));
		name.resize(len);
		in.read(&name[0], len);
		if (name != f.name()) {
			std::cerr << "unexpected weight: " << name << " (" << f.name() << " is expected)" << std::endl;
			std::exit(1);
		}
		in >> (*reinterpret_cast<weight*>(&f));
		return in;
	}
};

/**
 * the pattern feature
 * including isomorphic (rotate/mirror)
 *
 * index:
 *  0  1  2  3
 *  4  5  6  7
 *  8  9 10 11
 * 12 13 14 15
 *
 * usage:
 *  pattern({ 0, 1, 2, 3 })
 *  pattern({ 0, 1, 2, 3, 4, 5 })
 */
class pattern : public feature {
public:
	pattern(const std::vector<int>& p, const int& iso = 8) : feature(1 << (p.size() * 4)), iso_last(iso) {
		if (p.empty()) {
			std::cerr << "no pattern defined" << std::endl;
			std::exit(1);
		}

		/**
		 * isomorphic patterns can be calculated by board
		 *
		 * take pattern { 0, 1, 2, 3 } as an example
		 * apply the pattern to the original board (left), we will get 0x1372
		 * if we apply the pattern to the clockwise rotated board (right), we will get 0x2131,
		 * which is the same as applying pattern { 12, 8, 4, 0 } to the original board
		 * { 0, 1, 2, 3 } and { 12, 8, 4, 0 } are isomorphic patterns
		 * +------------------------+       +------------------------+
		 * |     2     8   128     4|       |     4     2     8     2|
		 * |     8    32    64   256|       |     2     4    32     8|
		 * |     2     4    32   128| ----> |     8    32    64   128|
		 * |     4     2     8    16|       |    16   128   256     4|
		 * +------------------------+       +------------------------+
		 *
		 * therefore if we make a board whose value is 0xfedcba9876543210ull (the same as index)
		 * we would be able to use the above method to calculate its 8 isomorphisms
		 */
		for (int i = 0; i < 8; i++) {
			board idx;
			for (int t = 0; t < 16; t++) idx(t) = t;
			if (i >= 4) idx.reflect_horizontal();
			idx.rotate(i);
			for (int t : p) {
				isomorphic[i].push_back(idx(t));
			}
		}
	}
	pattern(const pattern& p) = delete;
	virtual ~pattern() {}
	pattern& operator =(const pattern& p) = delete;

public:

	/**
	 * estimate the value of a given board
	 */
	virtual float estimate(const board& b) const {
		float value = 0;
		for (int i = 0; i < iso_last; i++) {
			size_t index = indexof(isomorphic[i], b);
			value += operator[](index);
		}
		return value;
	}

	/**
	 * update the value of a given board, and return its updated value
	 */
	virtual float update(const board& b, const float& u) {
		float u_split = u / iso_last;
		float value = 0;
		for (int i = 0; i < iso_last; i++) {
			size_t index = indexof(isomorphic[i], b);
			operator[](index) += u_split;
			value += operator[](index);
		}
		return value;
	}

	/**
	 * get the name of this feature
	 */
	virtual std::string name() const {
		return std::to_string(isomorphic[0].size()) + "-tuple pattern " + nameof(isomorphic[0]);
	}

public:

	/*
	 * set the isomorphic level of this pattern
	 * 1: no isomorphic
	 * 4: enable rotation
	 * 8: enable rotation and reflection
	 */
	void set_isomorphic(const int& i = 8) { iso_last = i; }

	/**
	 * display the weight information of a given board
	 */
	void dump(const board& b, std::ostream& out = std::cout) const {
		for (int i = 0; i < iso_last; i++) {
			out << "#" << i << ":" << nameof(isomorphic[i]) << "(";
			size_t index = indexof(isomorphic[i], b);
			for (size_t i = 0; i < isomorphic[i].size(); i++) {
				out << std::hex << ((index >> (4 * i)) & 0x0f);
			}
			out << std::dec << ") = " << operator[](index) << std::endl;
		}
	}

protected:

	size_t indexof(const std::vector<int>& patt, const board& b) const {
		size_t index = 0;
		for (size_t i = 0; i < patt.size(); i++)
			index |= b(patt[i]) << (4 * i);
		return index;
	}

	std::string nameof(const std::vector<int>& patt) const {
		std::stringstream ss;
		ss << std::hex;
		std::copy(patt.cbegin(), patt.cend(), std::ostream_iterator<int>(ss, ""));
		return ss.str();
	}

	std::array<std::vector<int>, 8> isomorphic;
	int iso_last;
};
