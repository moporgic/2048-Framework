#pragma once
#include <list>
#include <algorithm>
#include <iostream>
#include <sstream>
#include "board.h"
#include "action.h"
#include "agent.h"
#include "episode.h"

class statistic {
public:
	/**
	 * the total episodes to run
	 * the block size of statistic
	 * the limit of saving records
	 *
	 * note that total >= limit >= block
	 */
	statistic(size_t total, size_t block = 0, size_t limit = 0)
		: total(total),
		  block(block ? block : total),
		  limit(limit ? limit : total),
		  count(0) {}

public:
	/**
	 * show the statistic of last 'block' games
	 *
	 * the format would be
	 * 1000   avg = 273901, max = 382324, ops = 241563 (170543|896715)
	 *        512     100%   (0.3%)
	 *        1024    99.7%  (0.2%)
	 *        2048    99.5%  (1.1%)
	 *        4096    98.4%  (4.7%)
	 *        8192    93.7%  (22.4%)
	 *        16384   71.3%  (71.3%)
	 *
	 * where (block = 1000 by default)
	 *  '1000': current index (n)
	 *  'avg = 273901': the average score is 273901
	 *  'max = 382324': the maximum score is 382324
	 *  'ops = 241563 (170543|896715)': the average speed is 241563
	 *                                  the average speed of player is 170543
	 *                                  the average speed of environment is 896715
	 *  '93.7%': 93.7% (937 games) reached 8192-tiles (a.k.a. win rate of 8192-tile)
	 *  '22.4%': 22.4% (224 games) terminated with 8192-tiles (the largest)
	 */
	void show(bool tstat = true) const {
		size_t blk = std::min(data.size(), block);
		size_t stat[64] = { 0 };
		size_t sop = 0, pop = 0, eop = 0;
		time_t sdu = 0, pdu = 0, edu = 0;
		board::reward sum = 0, max = 0;
		auto it = data.end();
		for (size_t i = 0; i < blk; i++) {
			auto& ep = *(--it);
			sum += ep.score();
			max = std::max(ep.score(), max);
			stat[*std::max_element(&(ep.state()(0)), &(ep.state()(16)))]++;
			sop += ep.step();
			pop += ep.step(action::slide::type);
			eop += ep.step(action::place::type);
			sdu += ep.time();
			pdu += ep.time(action::slide::type);
			edu += ep.time(action::place::type);
		}

		std::ios ff(nullptr);
		ff.copyfmt(std::cout);
		std::cout << std::fixed << std::setprecision(0);
		std::cout << count << "\t";
		std::cout << "avg = " << (sum / blk) << ", ";
		std::cout << "max = " << (max) << ", ";
		std::cout << "ops = " << (sop * 1000.0 / sdu);
		std::cout <<     " (" << (pop * 1000.0 / pdu);
		std::cout <<      "|" << (eop * 1000.0 / edu) << ")";
		std::cout << std::endl;
		std::cout.copyfmt(ff);

		if (!tstat) return;
		for (size_t t = 0, c = 0; c < blk; c += stat[t++]) {
			if (stat[t] == 0) continue;
			unsigned accu = std::accumulate(std::begin(stat) + t, std::end(stat), 0);
			std::cout << "\t" << ((1 << t) & -2u); // type
			std::cout << "\t" << (accu * 100.0 / blk) << "%"; // win rate
			std::cout << "\t" "(" << (stat[t] * 100.0 / blk) << "%" ")"; // percentage of ending
			std::cout << std::endl;
		}
		std::cout << std::endl;
	}

	void summary() const {
		auto block_temp = block;
		const_cast<statistic&>(*this).block = data.size();
		show();
		const_cast<statistic&>(*this).block = block_temp;
	}

	bool is_finished() const {
		return count >= total;
	}

	void open_episode(const std::string& flag = "") {
		if (count++ >= limit) data.pop_front();
		data.emplace_back();
		data.back().open_episode(flag);
	}

	void close_episode(const std::string& flag = "") {
		data.back().close_episode(flag);
		if (count % block == 0) show();
	}

	episode& at(size_t i) {
		auto it = data.begin();
		while (i--) it++;
		return *it;
	}
	episode& front() {
		return data.front();
	}
	episode& back() {
		return data.back();
	}

	friend std::ostream& operator <<(std::ostream& out, const statistic& stat) {
		for (const episode& rec : stat.data) out << rec << std::endl;
		return out;
	}
	friend std::istream& operator >>(std::istream& in, statistic& stat) {
		for (std::string line; std::getline(in, line) && line.size(); ) {
			stat.data.emplace_back();
			std::stringstream(line) >> stat.data.back();
		}
		stat.total = std::max(stat.total, stat.data.size());
		stat.count = stat.data.size();
		return in;
	}

private:
	size_t total;
	size_t block;
	size_t limit;
	size_t count;
	std::list<episode> data;
};
