/**
 * Framework for 2048 & 2048-Like Games (C++ 11)
 * 2048.cpp: Main file for the 2048 framework
 *
 * Author: Hung Guei
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
#include "statistics.h"

int main(int argc, const char* argv[]) {
	std::cout << "2048-Demo: ";
	std::copy(argv, argv + argc, std::ostream_iterator<const char*>(std::cout, " "));
	std::cout << std::endl << std::endl;

	size_t total = 1000, block = 0, limit = 0;
	std::string slide_args, place_args;
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
		} else if (match("slide") || match("play")) {
			slide_args = next_opt();
		} else if (match("place") || match("env")) {
			place_args = next_opt();
		} else if (match("load")) {
			load_path = next_opt();
		} else if (match("save")) {
			save_path = next_opt();
		}
	}

	statistics stats(total, block, limit);

	if (load_path.size()) {
		std::ifstream in(load_path, std::ios::in);
		in >> stats;
		in.close();
		if (stats.is_finished()) stats.summary();
	}

	random_slider slide(slide_args);
	random_placer place(place_args);

	while (!stats.is_finished()) {
//		std::cerr << "======== Game " << stats.step() << " ========" << std::endl;
		slide.open_episode("~:" + place.name());
		place.open_episode(slide.name() + ":~");

		stats.open_episode(slide.name() + ":" + place.name());
		episode& game = stats.back();
		while (true) {
			agent& who = game.take_turns(slide, place);
			action move = who.take_action(game.state());
//			std::cerr << game.state() << "#" << game.step() << " " << who.name() << ": " << move << std::endl;
			if (game.apply_action(move) != true) break;
			if (who.check_for_win(game.state())) break;
		}
		agent& win = game.last_turns(slide, place);
		stats.close_episode(win.name());

		slide.close_episode(win.name());
		place.close_episode(win.name());
	}

	if (save_path.size()) {
		std::ofstream out(save_path, std::ios::out | std::ios::trunc);
		out << stats;
		out.close();
	}

	return 0;
}
