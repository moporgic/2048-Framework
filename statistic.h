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
	/**
	 * the total episodes to run
	 * the block size of statistic
	 * the limit of saving records
	 *
	 * note that total >= limit >= block
	 */
	statistic(const size_t& total, const size_t& block = 0, const size_t& limit = 0)
		: total(total),
		  block(block ? block : this->total),
		  limit(std::max(limit, this->block)),
		  count(0) {}

public:
	/**
	 * show the statistic of last 'block' games
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
	 * where (assume that block = 1000)
	 *  '1000': current index (n)
	 *  'avg = 273901': the average score of saved games is 273901
	 *  'max = 382324': the maximum score of saved games is 382324
	 *  'ops = 241563': the average speed of saved games is 241563
	 *  '93.7%': 93.7% (937 games) reached 8192-tiles in saved games (a.k.a. win rate of 8192-tile)
	 *  '22.4%': 22.4% (224 games) terminated with 8192-tiles (the largest) in saved games
	 */
	void show() const {
		size_t blk = std::min(data.size(), block);
		unsigned sum = 0, max = 0, stat[16] = { 0 };
		size_t sop = 0, pop = 0, eop = 0;
		time_t sdu = 0, pdu = 0, edu = 0;
		auto it = data.end();
		for (size_t i = 0; i < blk; i++) {
			auto& path = *(--it);
			board b;
			unsigned score = path.apply(b);
			sum += score;
			max = std::max(score, max);
			stat[*std::max_element(&b(0), &b(16))]++;
			sop += path.step('*');
			pop += path.step('p');
			eop += path.step('e');
			sdu += path.duration('*');
			pdu += path.duration('p');
			edu += path.duration('e');
		}
		float coef = 100.0 / blk;
		unsigned avg = sum / blk;
		unsigned sops = sop * 1000.0 / sdu;
		unsigned pops = pop * 1000.0 / pdu;
		unsigned eops = eop * 1000.0 / edu;
		std::cout << count << "\t";
		std::cout << "avg = " << avg << ", ";
		std::cout << "max = " << max << ", ";
		std::cout << "ops = " << sops;
		if (high_resolution_timestamp()) std::cout << " (" << pops << "|" << eops << ")";
		std::cout << std::endl;
		for (size_t t = 0, c = 0; c < blk; c += stat[t++]) {
			if (stat[t] == 0) continue;
			unsigned accu = std::accumulate(std::begin(stat) + t, std::end(stat), 0);
			std::cout << "\t" << ((1 << t) & -2u) << "\t" << (accu * coef) << "%";
			std::cout << "\t(" << (stat[t] * coef) << "%)" << std::endl;
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

	board make_empty_board() {
		return {};
	}

	void save_action(action move) {
		data.back().save_action(move);
	}

	agent& take_turns(agent& play, agent& evil) {
		return data.back().take_turns(play, evil);
	}

	agent& last_turns(agent& play, agent& evil) {
		return take_turns(evil, play);
	}

	friend std::ostream& operator <<(std::ostream& out, const statistic& stat) {
		auto size = stat.data.size();
		out.write(reinterpret_cast<char*>(&size), sizeof(size));
		for (const record& rec : stat.data) out << rec;
		return out;
	}

	friend std::istream& operator >>(std::istream& in, statistic& stat) {
		auto size = stat.data.size();
		in.read(reinterpret_cast<char*>(&size), sizeof(size));
		stat.total = stat.block = stat.limit = stat.count = size;
		stat.data.resize(size);
		for (record& rec : stat.data) in >> rec;
		return in;
	}

private:

	class record {
	public:
		record() { moves.reserve(10000); }

		void open_episode(const std::string& tag) {
			open = { tag, millisec() };
		}
		void close_episode(const std::string& tag) {
			close = { tag, millisec() };
		}

		agent& take_turns(agent& play, agent& evil) {
			temp.t0 = high_resolution_timestamp() ? millisec() : 0;
			return (std::max(step() + 1, size_t(2)) % 2) ? play : evil;
		}
		void save_action(action a) {
			temp.t1 = high_resolution_timestamp() ? millisec() : 0;
			temp.code = a;
			moves.push_back(temp);
		}

		std::vector<action> actions() const {
			std::vector<action> a;
			a.reserve(moves.size());
			for (auto mv : moves) a.push_back(mv.code);
			return a;
		}

		size_t step(char who = '*') const {
			size_t size = moves.size();
			if (who == 'p') return (size - (2 - size % 2)) / 2;
			if (who == 'e') return size - (size - (2 - size % 2)) / 2;
			return size;
		}

		int apply(board& b) const {
			int score = 0;
			for (size_t i = 0; i < moves.size(); i++)
				score += action(moves[i].code).apply(b);
			return score;
		}

		time_t duration(char who = '*') const {
			if (who == 'p' && high_resolution_timestamp()) {
				time_t du = 0;
				for (size_t i = 2; i < moves.size(); i += 2) du += (moves[i].t1 - moves[i].t0);
				return du;
			}
			if (who == 'e' && high_resolution_timestamp()) {
				time_t du = moves[0].t1 - moves[0].t0;
				for (size_t i = 1; i < moves.size(); i += 2) du += (moves[i].t1 - moves[i].t0);
				return du;
			}
			return close.when - open.when;
		}

		friend std::ostream& operator <<(std::ostream& out, const record& rec) {
			auto size = rec.moves.size();
			out.write(reinterpret_cast<char*>(&size), sizeof(size));
			for (const move& mv : rec.moves) out << mv;
			out << rec.open << rec.close;
			return out;
		}
		friend std::istream& operator >>(std::istream& in, record& rec) {
			auto size = rec.moves.size();
			in.read(reinterpret_cast<char*>(&size), sizeof(size));
			rec.moves.reserve(size);
			for (size_t i = 0; i < size && in >> rec.temp; i++) rec.moves.push_back(rec.temp);
			in >> rec.open >> rec.close;
			return in;
		}

	private:

		time_t millisec() const {
			auto now = std::chrono::system_clock::now().time_since_epoch();
			return std::chrono::duration_cast<std::chrono::milliseconds>(now).count();
		}

		struct move {
			int code;
			time_t t0;
			time_t t1;
			friend std::ostream& operator <<(std::ostream& out, const move& mv) {
				out.write(reinterpret_cast<const char*>(&mv.code), sizeof(mv.code));
				out.write(reinterpret_cast<const char*>(&mv.t0), sizeof(mv.t0));
				out.write(reinterpret_cast<const char*>(&mv.t1), sizeof(mv.t1));
				return out;
			}
			friend std::istream& operator >>(std::istream& in, move& mv) {
				in.read(reinterpret_cast<char*>(&mv.code), sizeof(mv.code));
				in.read(reinterpret_cast<char*>(&mv.t0), sizeof(mv.t0));
				in.read(reinterpret_cast<char*>(&mv.t1), sizeof(mv.t1));
				return in;
			}
		};

		struct meta {
			std::string tag;
			time_t when;
			friend std::ostream& operator <<(std::ostream& out, const meta& m) {
				out.write(reinterpret_cast<const char*>(m.tag.c_str()), m.tag.size() + 1);
				out.write(reinterpret_cast<const char*>(&m.when), sizeof(m.when));
				return out;
			}
			friend std::istream& operator >>(std::istream& in, meta& m) {
				for (char ch; in.read(&ch, 1) && ch; m.tag.push_back(ch));
				in.read(reinterpret_cast<char*>(&m.when), sizeof(m.when));
				return in;
			}
		};

		std::vector<move> moves;
		move temp;
		meta open;
		meta close;
	};

	static constexpr bool high_resolution_timestamp() { return true; }

	size_t total;
	size_t block;
	size_t limit;
	size_t count;
	std::list<record> data;
};
