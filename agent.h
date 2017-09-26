#pragma once
#include <string>
#include <random>
#include <sstream>
#include "board.h"
#include "action.h"

class agent {
public:
	agent() {}
	virtual ~agent() {}
	virtual void open_episode() {}
	virtual void close_episode() {}
	virtual action take_action(const board& b) { return action(); }
	virtual bool check_for_win(const board& b) { return false; }
	virtual std::string name() const { return "null"; }
};

/**
 * add a new random tile on board, or do nothing if the board is full
 * 2-tile: 90%
 * 4-tile: 10%
 */
class random : public agent {
public:
	random(const std::string& args = "") : engine(std::random_device{}()) {
		std::stringstream ss(args);
		for (std::string pair; ss >> pair; ) {
			std::string key = pair.substr(0, pair.find('='));
			std::string value = pair.substr(pair.find('=') + 1);

			if (key == "seed") engine.seed(std::stoull(value));
		}
	}

	virtual action take_action(const board& after) {
		int space[16], num = 0;
		for (int i = 0; i < 16; i++)
			if (after(i) == 0) {
				space[num++] = i;
			}
		if (num) {
			std::uniform_int_distribution<int> popup(0, 9);
			std::uniform_int_distribution<int> empty(0, num - 1);

			int tile = popup(engine) ? 1 : 2;
			int pos = space[empty(engine)];
			return action::place(tile, pos);
		} else {
			return action();
		}
	}

	virtual std::string name() const { return "random"; }

private:
	std::default_random_engine engine;
};

/**
 * select an action by immediately reward
 */
class player : public agent {
public:
	player(const std::string& args = "") : agent() {}

	virtual action take_action(const board& before) {
		int code = -1;
		int reward = -1;
		for (int i = 0; i < 4; i++) {
			board b = before;
			int r = b.move(i);
			if (r > reward) {
				reward = r;
				code = i;
			}
		}
		return action::move(code);
	}

	virtual std::string name() const { return "player"; }

private:
};
