/**
 * Framework for 2048 & 2048-like Games (C++ 11)
 * 2048.cpp: Main file for the 2048 framework
 *
 * Author: Hung Guei (moporgic)
 *         Computer Games and Intelligence (CGI) Lab, NYCU, Taiwan
 *         https://cgilab.nctu.edu.tw/
 */

#include <iostream>
#include <fstream>
#include <iterator>
#include <string>
#include "board.h"
#include "action.h"
#include "agent.h"
#include "episode.h"
#include "statistic.h"

int main(int argc, const char* argv[]) {
	std::cout << "2048-Demo: ";
	std::copy(argv, argv + argc, std::ostream_iterator<const char*>(std::cout, " "));
	std::cout << std::endl << std::endl;

	size_t total = 1000, block = 0, limit = 0;
	std::string play_args, evil_args;
	std::string load_path, save_path;
	for (int i = 1; i < argc; i++) {
		std::string arg = argv[i];
		auto match = [&](std::string flag) -> bool {
			auto it = arg.find_first_not_of('-');
			return arg.find(flag, it) == it;
		};
		auto next_opt = [&]() -> std::string {
			auto it = arg.find('=') + 1;
			return it ? arg.substr(it) : argv[++i];
		};
		if (match("total")) {
			total = std::stoull(next_opt());
		} else if (match("block")) {
			block = std::stoull(next_opt());
		} else if (match("limit")) {
			limit = std::stoull(next_opt());
		} else if (match("play")) {
			play_args = next_opt();
		} else if (match("evil")) {
			evil_args = next_opt();
		} else if (match("load")) {
			load_path = next_opt();
		} else if (match("save")) {
			save_path = next_opt();
		}
	}

	statistic stat(total, block, limit);

	if (load_path.size()) {
		std::ifstream in(load_path, std::ios::in);
		in >> stat;
		in.close();
		if (stat.is_finished()) stat.summary();
	}

	player play(play_args);
	rndenv evil(evil_args);

	while (!stat.is_finished()) {
//		std::cerr << "======== Game " << stat.step() << " ========" << std::endl;
		play.open_episode("~:" + evil.name());
		evil.open_episode(play.name() + ":~");

		stat.open_episode(play.name() + ":" + evil.name());
		episode& game = stat.back();
		while (true) {
			agent& who = game.take_turns(play, evil);
			action move = who.take_action(game.state());
//			std::cerr << game.state() << "#" << game.step() << " " << who.name() << ": " << move << std::endl;
			if (game.apply_action(move) != true) break;
			if (who.check_for_win(game.state())) break;
		}
		agent& win = game.last_turns(play, evil);
		stat.close_episode(win.name());

		play.close_episode(win.name());
		evil.close_episode(win.name());
	}

	if (save_path.size()) {
		std::ofstream out(save_path, std::ios::out | std::ios::trunc);
		out << stat;
		out.close();
	}

	return 0;
}
