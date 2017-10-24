#pragma once
#include <iostream>
#include <vector>
#include <array>
#include <sstream>
#include <iterator>
#include <string>
#include <cstdint>

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
		uint64_t size = w.length;
		float* value = w.value;
		out.write(reinterpret_cast<char*>(&size), sizeof(uint64_t));
		out.write(reinterpret_cast<char*>(value), sizeof(float) * size);
		if (!out) {
			std::cerr << "weight may not have been stored successfully" << std::endl;
		}
		return out;
	}

	friend std::istream& operator >>(std::istream& in, weight& w) {
		uint64_t size = w.length;
		float* value = w.value;
		if (in.read(reinterpret_cast<char*>(&size), sizeof(uint64_t))) {
			if (value) {
				std::cerr << "reading to a non-empty weight" << std::endl;
				allocated_memory() -= w.length;
				delete[] w.value;
			}
			w.length = size;
			w.value = value = alloc(size);
			in.read(reinterpret_cast<char*>(value), sizeof(float) * size);
		}
		if (!in) {
			std::cerr << "unexpected end of weight" << std::endl;
			std::exit(1);
		}
		return in;
	}

protected:

	static size_t& allocated_memory() {
		static size_t total = 0;
		return total;
	}

	static float* alloc(size_t num) {
		try {
			size_t& total = allocated_memory();
			const size_t limit = (2 << 30) / sizeof(float); // 2G memory, feel free to remove the limitation
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
