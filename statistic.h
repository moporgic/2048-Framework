#pragma once
#include <list>
#include <vector>
#include <algorithm>
#include <iostream>
#include <sstream>
#include <chrono>
#include <numeric>
#include <regex>
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
	class episode {
	friend class statistic;
	public:
		episode() { moves.reserve(10000); }

	protected:
		void open_episode(const std::string& tag) {
			open = { tag, millisec() };
		}
		void close_episode(const std::string& tag) {
			close = { tag, millisec() };
		}
		agent& take_turns(agent& play, agent& evil) {
			temp.time = millisec();
			return (std::max(step() + 1, size_t(2)) % 2) ? play : evil;
		}
		void save_action(action a) {
			temp.time = millisec() - temp.time;
			temp.code = a;
			moves.push_back(temp);
		}

	public:
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
			if (who == 'p') {
				time_t du = 0;
				for (size_t i = 2; i < moves.size(); i += 2) du += (moves[i].time);
				return du;
			}
			if (who == 'e') {
				time_t du = moves[0].time;
				for (size_t i = 1; i < moves.size(); i += 2) du += (moves[i].time);
				return du;
			}
			return close.when - open.when;
		}
		std::string tag(char type = 'o') {
			if (type == 'o') return open.tag;
			if (type == 'c') return close.tag;
			return "";
		}

		friend std::ostream& operator <<(std::ostream& out, const episode& rec) {
			out << rec.open << '|';
			for (const move& mv : rec.moves) out << mv;
			out << '|' << rec.close;
			return out;
		}
		friend std::istream& operator >>(std::istream& in, episode& rec) {
			std::string token;
			std::getline(in, token, '|');
			std::stringstream(token) >> rec.open;
			std::getline(in, token, '|');
			std::regex pattern("[#0-9A-F][0-9A-Z](\\([0-9]+\\))?");
			for (std::smatch match; std::regex_search(token, match, pattern); token = match.suffix()) {
				rec.moves.emplace_back();
				std::stringstream(match.str()) >> rec.moves.back();
			}
			std::getline(in, token, '|');
			std::stringstream(token) >> rec.close;
			return in;
		}

	private:

		time_t millisec() const {
			auto now = std::chrono::system_clock::now().time_since_epoch();
			return std::chrono::duration_cast<std::chrono::milliseconds>(now).count();
		}

		struct move {
			action code;
			time_t time;
			friend std::ostream& operator <<(std::ostream& out, const move& m) {
				if (m.time)
					return out << m.code << '(' << m.time << ')';
				else
					return out << m.code;
			}
			friend std::istream& operator >>(std::istream& in, move& m) {
				in >> m.code;
				m.time = 0;
				if (in.peek() == '(') {
					in.ignore(1);
					in >> m.time;
					in.ignore(1);
				}
				return in;
			}
		};

		struct meta {
			std::string tag;
			time_t when;
			friend std::ostream& operator <<(std::ostream& out, const meta& m) {
				return out << m.tag << "@" << m.when;
			}
			friend std::istream& operator >>(std::istream& in, meta& m) {
				return std::getline(in, m.tag, '@') >> m.when;
			}
		};

		std::vector<move> moves;
		move temp;
		meta open;
		meta close;
	};

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
		unsigned sum = 0, max = 0, stat[64] = { 0 };
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
		std::cout << " (" << pops << "|" << eops << ")";
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
		stat.total = stat.block = stat.limit = stat.count = stat.data.size();
		return in;
	}

private:
	size_t total;
	size_t block;
	size_t limit;
	size_t count;
	std::list<episode> data;
};
