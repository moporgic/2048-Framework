#pragma once
#include <list>
#include <vector>
#include <algorithm>
#include <iostream>
#include <chrono>
#include <numeric>
#include "board.h"
#include "action.h"
#include "agent.h"

class statistic {
public:
	statistic(const size_t& total, const size_t& unit = 0) : total(total), unit(unit ? unit : total) {}

public:
	/**
	 * show the statistic of last unit games
	 *
	 * the format would be
	 * 1000   avg = 273901, max = 382324, ops = 241563
	 *        512     100%   (0.3%)
	 *        1024    99.7%  (0.2%)
	 *        2048    99.5%  (1.1%)
	 *        4096    98.4%  (4.7%)
	 *        8192    93.7%  (22.4%)
	 *        16384   71.3%  (71.3%)
	 *
	 * where (assume that unit = 1000)
	 *  '1000': current index (n)
	 *  'avg = 273901': the average score of saved games is 273901
	 *  'max = 382324': the maximum score of saved games is 382324
	 *  'ops = 241563': the average speed of saved games is 241563
	 *  '93.7%': 93.7% (937 games) reached 8192-tiles in saved games (a.k.a. win rate of 8192-tile)
	 *  '22.4%': 22.4% (224 games) terminated with 8192-tiles (the largest) in saved games
	 */
	void show() const {
		int unit = std::min(data.size(), this->unit);
		size_t sum = 0, max = 0, opc = 0, stat[16] = { 0 };
		auto it = data.end();
		for (int i = 0; i < unit; i++) {
			auto& path = *(--it);
			board game;
			opc += path.size();
			size_t score = 0;
			for (const action& move : path)
				score += move.apply(game);
			sum += score;
			max = std::max(score, max);
			int tile = 0;
			for (int i = 0; i < 16; i++)
				tile = std::max(tile, game(i));
			stat[tile]++;
		}
		float avg = float(sum) / unit;
		float coef = 100.0 / unit;
		float ops = opc * 1000.0 / (data.back().tock_time() - it->tick_time());
		std::cout << data.size() << "\t";
		std::cout << "avg = " << int(avg) << ", ";
		std::cout << "max = " << int(max) << ", ";
		std::cout << "ops = " << int(ops) << std::endl;
		for (int t = 0, c = 0; c < unit; c += stat[t++]) {
			if (stat[t] == 0) continue;
			int accu = std::accumulate(stat + t, stat + 16, 0);
			std::cout << "\t" << ((1 << t) & -2u) << "\t" << (accu * coef) << "%";
			std::cout << "\t(" << (stat[t] * coef) << "%)" << std::endl;
		}
		std::cout << std::endl;
	}

	void summary() const {
		auto unit_temp = unit;
		const_cast<statistic&>(*this).unit = data.size();
		show();
		const_cast<statistic&>(*this).unit = unit_temp;
	}

	bool is_finished() const {
		return data.size() >= total;
	}

	void open_episode() {
		data.emplace_back();
		data.back().tick();
	}

	void close_episode(const bool& disp = true) {
		data.back().tock();
		if (disp && data.size() % unit == 0) show();
	}

	void save_action(const action& move) {
		data.back().push_back(move);
	}

	agent& take_turns(agent& play, agent& evil) {
		// 1:play 0:evil
		return (std::max(data.back().size() + 1, 2ull) % 2) ? play : evil;
	}

	friend std::ostream& operator <<(std::ostream& out, const statistic& stat) {
		auto total = stat.total;
		auto unit = stat.unit;
		auto size = stat.data.size();
		out.write(reinterpret_cast<char*>(&total), sizeof(total));
		out.write(reinterpret_cast<char*>(&unit), sizeof(unit));
		out.write(reinterpret_cast<char*>(&size), sizeof(size));
		for (const record& rec : stat.data) out << rec;
		return out;
	}

	friend std::istream& operator >>(std::istream& in, statistic& stat) {
		auto total = stat.total;
		auto unit = stat.unit;
		auto size = stat.data.size();
		in.read(reinterpret_cast<char*>(&total), sizeof(total));
		in.read(reinterpret_cast<char*>(&unit), sizeof(unit));
		in.read(reinterpret_cast<char*>(&size), sizeof(size));
		stat.total = total;
		stat.unit = unit;
		for (size_t i = 0; i < size; i++) {
			stat.data.emplace_back();
			in >> stat.data.back();
		}
		return in;
	}

private:
	class record : public std::vector<action> {
	public:
		record() { reserve(32768); }
		void tick() { time[0] = milli(); }
		void tock() { time[1] = milli(); }
		uint64_t tick_time() const { return time[0]; }
		uint64_t tock_time() const { return time[1]; }

		friend std::ostream& operator <<(std::ostream& out, const record& rec) {
			auto size = rec.size();
			auto time = rec.time;
			out.write(reinterpret_cast<char*>(&size), sizeof(size));
			for (const action& act : rec) {
				out.write(reinterpret_cast<const char*>(&act), sizeof(act));
			}
			out.write(reinterpret_cast<const char*>(time), sizeof(time[0]) * 2);
			return out;
		}
		friend std::istream& operator >>(std::istream& in, record& rec) {
			auto size = rec.size();
			auto time = rec.time;
			in.read(reinterpret_cast<char*>(&size), sizeof(size));
			rec.reserve(size);
			for (auto i = 0ull; i < size; i++) {
				rec.emplace_back();
				in.read(reinterpret_cast<char*>(&rec.back()), sizeof(rec.back()));
			}
			in.read(reinterpret_cast<char*>(time), sizeof(time[0]) * 2);
			return in;
		}

	private:
		uint64_t milli() const {
			auto now = std::chrono::system_clock::now().time_since_epoch();
			return std::chrono::duration_cast<std::chrono::milliseconds>(now).count();
		}
		uint64_t time[2];
	};

	size_t total;
	size_t unit;
	std::list<record> data;
};
