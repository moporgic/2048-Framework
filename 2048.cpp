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
	std::string load, save;
	bool summary = false;
	for (int i = 1; i < argc; i++) {
		std::string arg = argv[i];
		std::string opt = arg.substr(arg.find('=') + 1);
		if (arg.find("--total=") == 0) {
			total = std::stoull(opt);
		} else if (arg.find("--block=") == 0) {
			block = std::stoull(opt);
		} else if (arg.find("--limit=") == 0) {
			limit = std::stoull(opt);
		} else if (arg.find("--play=") == 0) {
			play_args = opt;
		} else if (arg.find("--evil=") == 0) {
			evil_args = opt;
		} else if (arg.find("--load=") == 0) {
			load = opt;
		} else if (arg.find("--save=") == 0) {
			save = opt;
		} else if (arg.find("--summary") == 0) {
			summary = true;
		}
	}

	statistic stat(total, block, limit);

	if (load.size()) {
		std::ifstream in(load, std::ios::in);
		in >> stat;
		in.close();
		summary |= stat.is_finished();
	}

	player play(play_args);
	rndenv evil(evil_args);

	while (!stat.is_finished()) {
		play.open_episode("~:" + evil.name());
		evil.open_episode(play.name() + ":~");

		stat.open_episode(play.name() + ":" + evil.name());
		episode& game = stat.back();
		while (true) {
			agent& who = game.take_turns(play, evil);
			action move = who.take_action(game.state());
			if (game.apply_action(move) != true) break;
			if (who.check_for_win(game.state())) break;
		}
		agent& win = game.last_turns(play, evil);
		stat.close_episode(win.name());

		play.close_episode(win.name());
		evil.close_episode(win.name());
	}

	if (summary) {
		stat.summary();
	}

	if (save.size()) {
		std::ofstream out(save, std::ios::out | std::ios::trunc);
		out << stat;
		out.close();
	}

	return 0;
}
