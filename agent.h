#pragma once
#include <string>
#include <random>
#include <sstream>
#include <map>
#include <type_traits>
#include <algorithm>
#include "board.h"
#include "action.h"
#include "weight.h"

class agent {
public:
	agent(const std::string& args = "") {
		std::stringstream ss(args);
		for (std::string pair; ss >> pair; ) {
			std::string key = pair.substr(0, pair.find('='));
			std::string value = pair.substr(pair.find('=') + 1);
			property[key] = { value };
		}
	}
	virtual ~agent() {}
	virtual void open_episode(const std::string& flag = "") {}
	virtual void close_episode(const std::string& flag = "") {}
	virtual action take_action(const board& b) { return action(); }
	virtual bool check_for_win(const board& b) { return false; }

public:
	virtual std::string name() const {
		auto it = property.find("name");
		return it != property.end() ? std::string(it->second) : "unknown";
	}
protected:
	typedef std::string key;
	struct value {
		std::string value;
		operator std::string() const { return value; }
		template<typename numeric, typename = typename std::enable_if<std::is_arithmetic<numeric>::value, numeric>::type>
		operator numeric() const { return numeric(std::stod(value)); }
	};
	std::map<key, value> property;
};

/**
 * evil (environment agent)
 * add a new random tile on board, or do nothing if the board is full
 * 2-tile: 90%
 * 4-tile: 10%
 */
class rndenv : public agent {
public:
	rndenv(const std::string& args = "") : agent("name=rndenv " + args) {
		if (property.find("seed") != property.end())
			engine.seed(int(property["seed"]));
	}

	virtual action take_action(const board& after) {
		int space[] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 };
		std::shuffle(space, space + 16, engine);
		for (int pos : space) {
			if (after(pos) != 0) continue;
			std::uniform_int_distribution<int> popup(0, 9);
			int tile = popup(engine) ? 1 : 2;
			return action::place(tile, pos);
		}
		return action();
	}

private:
	std::default_random_engine engine;
};

/**
 * player (dummy)
 * select an action randomly
 */
class player : public agent {
public:
	player(const std::string& args = "") : agent("name=player " + args) {
		episode.reserve(32768);
		if (property.find("seed") != property.end())
			engine.seed(int(property["seed"]));
		if (property.find("load") != property.end())
			load_weights(property["load"]);

		patt.emplace_back(new pattern({ 0, 1, 2, 3, 4, 5 }));
		patt.emplace_back(new pattern({ 4, 5, 6, 7, 8, 9 }));
		patt.emplace_back(new pattern({ 0, 1, 2, 4, 5, 6 }));
		patt.emplace_back(new pattern({ 4, 5, 6, 8, 9, 10 }));
	}
	~player() {
		if (property.find("save") != property.end())
			save_weights(property["save"]);
	}
	std::vector<pattern*> patt;

	virtual action take_action(const board& before) {
		int best = -1;
		// TODO: select a proper action
		// TODO: push the step into episode

		float best_value = -10000;
		board best_after = before;
		int best_reward = -1;

		for (int i = 0; i < 4; i++) {
			int op(i);
			board after = before;
			int reward = after.move(op);
			if (reward == -1) continue;
			float value = reward;
			for (pattern* p : patt) {
				value += p->estimate(after);
			}
			if (value > best_value) {
				best_value = value;
				best_after = after;
				best = op;
				best_reward = reward;
			}
		}

		episode.emplace_back(before, best_after, action(best), best_reward);
		return best;
	}

	virtual void open_episode(const std::string& flag = "") {
		episode.clear();
		episode.reserve(32768);
	}

	virtual void close_episode(const std::string& flag = "") {
		// TODO: train your agent by TD(0)

		float exact = 0;

		episode.pop_back();
		while (episode.size()) {
			auto& last = episode.back();

			float current = 0;
			for (pattern* p : patt) {
				current += p->estimate(last.after);
			}
			float error = exact - current;
			float update = error * 0.1;
			float updated = 0;
			for (pattern* p : patt) {
				updated += p->update(last.after, update);
			}

			exact = updated + last.reward;
			episode.pop_back();
		}
	}

	virtual void load_weights(const std::string& path) {
		std::ifstream in;
		in.open(path.c_str(), std::ios::in | std::ios::binary);
		if (!in.is_open()) std::exit(-1);
		size_t size;
		in.read(reinterpret_cast<char*>(&size), sizeof(size));
		weights.resize(size);
		for (weight& w : weights)
			in >> w;
		in.close();
	}

	virtual void save_weights(const std::string& path) {
		std::ofstream out;
		out.open(path.c_str(), std::ios::out | std::ios::binary | std::ios::trunc);
		if (!out.is_open()) std::exit(-1);
		size_t size = weights.size();
		out.write(reinterpret_cast<char*>(&size), sizeof(size));
		for (weight& w : weights)
			out << w;
		out.flush();
		out.close();

	}

private:
	std::default_random_engine engine;

	std::vector<weight> weights;

	struct state {
		state(const board& before, const board& after, const action& move, const int& reward)
		: before(before), after(after), move(move), reward(reward) {}
		board before;
		board after;
		action move;
		int reward;
	};

	std::vector<state> episode;
};
