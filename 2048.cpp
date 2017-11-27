/**
 * Basic Environment for Game 2048
 * use 'g++ -std=c++0x -O3 -g -o 2048 2048.cpp' to compile the source
 *
 * Computer Games and Intelligence (CGI) Lab, NCTU, Taiwan
 * http://www.aigames.nctu.edu.tw
 */

#include <iostream>
#include <algorithm>
#include <functional>
#include <iterator>
#include <string>
#include <random>
#include <sstream>
#include <fstream>
#include <cmath>
#include <chrono>
#include <vector>
#include <queue>
#include <map>
#include <regex>
#include "board.h"
#include "action.h"
#include "agent.h"
#include "statistic.h"

int shell(int argc, const char* argv[]) {

	struct match {
		std::string id;
		agent* actor;
		board state;
		std::queue<int> drop;
		statistic stat;

		match(const std::string& id, agent* actor, const std::string& info) : id(id), actor(actor) {
			stat.open_episode(info);
		}
	};

	std::vector<agent*> lounge;
	std::map<std::string, match> arena;

	std::string command;
	while (std::getline(std::cin, command)) {
		std::stringstream tokenizer(command);
		std::vector<std::string> tokens;
		for (std::string token; tokenizer >> token; tokens.push_back(token));

		try {
			if (tokens.at(0) == "match") {
				auto id = tokens.at(1);
				if (tokens.at(2) != "set") {
					auto role = tokens.at(2);
					auto oper = tokens.at(3);
					if (oper == "move") {
						auto code = tokens.at(4);
						if (code == "request") {
							if (arena.at(id).actor->role() == role) {
								// TODO: I need to output an action
							}
						} else {
							action a(std::stol(code));
							int reward = a.apply(arena.at(id).state);
							// TODO: check reward
							arena.at(id).stat.save_action(a);
						}
					} else if (oper == "new") {
						if (arena.find(id) == arena.end()) {
							auto info = tokens.at(4);
							if (tokens.at(5) == "request") {
								auto name = (role == "player") ?
									info.substr(0, info.find(':')) :
									info.substr(info.find(':') + 1);
								auto actor = std::find_if(lounge.begin(), lounge.end(), [=](agent* a) {
									return a->role() == role && a->name() == name;
								});
								if (actor == lounge.end()) {
									actor = std::find_if(lounge.begin(), lounge.end(), [=](agent* a) {
										return a->role() == role;
									});
								}
								if (actor) {
									arena.emplace();
									match m(id, actor, info);
								}
							}
						}
					} else if (oper == "win") {

					}
				} else if (tokens.at(2) == "set") {
					auto opt = tokens.at(3);
					if (opt == "sequence") {
						match& m = arena.at(id);
						for (size_t i = 4; i < tokens.size(); i++)
							m.drop.push(std::stol(tokens.at(i)));
					} else if (opt == "timeout") {

					}
				}
			}
		} catch (std::out_of_range&) {

		} catch (int&) {

		}

		std::regex match_new_request  ("^match (\\S+) (player/environment) new ([^ :]+):(\\S+) request$");
		std::regex match_move_request ("^match (\\S+) (player/environment) move request$");
		std::regex match_move_action  ("^match (\\S+) (player/environment) move (\\S+)$");
		std::regex match_win          ("^match (\\S+) (player/environment) win$");
		std::regex match_set_sequence ("^match (\\S+) set sequence( \\d)+$");

		// match $id $role new $name:$name request
		// match $id $role move request
		// match $id $role move $action
		// match $id $role win
		// match $id set sequence $sequence
		// match $id set timeout $time
		// set debug [on/off]
		// set statistic [on/off]
		// set $role $name [on/off] [with $arguments]
		// set timeout $time
		// set label $label
		// status

		// match $id accept
		// match $id reject
		// match $id $role move $action
		// match $id $role win
	}

	return 0;
}

int main(int argc, const char* argv[]) {
	std::cout << "2048-Demo: ";
	std::copy(argv, argv + argc, std::ostream_iterator<const char*>(std::cout, " "));
	std::cout << std::endl << std::endl;

	size_t total = 1000, block = 0, limit = 0;
	std::string play_args, evil_args;
	std::string load, save;
	bool summary = false;
	for (int i = 1; i < argc; i++) {
		std::string para(argv[i]);
		if (para.find("--total=") == 0) {
			total = std::stoull(para.substr(para.find("=") + 1));
		} else if (para.find("--block=") == 0) {
			block = std::stoull(para.substr(para.find("=") + 1));
		} else if (para.find("--limit=") == 0) {
			limit = std::stoull(para.substr(para.find("=") + 1));
		} else if (para.find("--play=") == 0) {
			play_args = para.substr(para.find("=") + 1);
		} else if (para.find("--evil=") == 0) {
			evil_args = para.substr(para.find("=") + 1);
		} else if (para.find("--load=") == 0) {
			load = para.substr(para.find("=") + 1);
		} else if (para.find("--save=") == 0) {
			save = para.substr(para.find("=") + 1);
		} else if (para.find("--summary") == 0) {
			summary = true;
		} else if (para.find("--shell") == 0) {
			return shell(argc, argv);
		}
	}

	statistic stat(total, block, limit);

	if (load.size()) {
		std::ifstream in;
		in.open(load.c_str(), std::ios::in | std::ios::binary);
		if (!in.is_open()) return -1;
		in >> stat;
		in.close();
	}

	player play(play_args);
	rndenv evil(evil_args);

	while (!stat.is_finished()) {
		play.open_episode("~:" + evil.name());
		evil.open_episode(play.name() + ":~");

		stat.open_episode(play.name() + ":" + evil.name());
		board game = stat.make_empty_board();
		while (true) {
			agent& who = stat.take_turns(play, evil);
			action move = who.take_action(game);
			if (move.apply(game) == -1) break;
			stat.save_action(move);
			if (who.check_for_win(game)) break;
		}
		agent& win = stat.last_turns(play, evil);
		stat.close_episode(win.name());

		play.close_episode(win.name());
		evil.close_episode(win.name());
	}

	if (summary) {
		stat.summary();
	}

	if (save.size()) {
		std::ofstream out;
		out.open(save.c_str(), std::ios::out | std::ios::binary | std::ios::trunc);
		if (!out.is_open()) return -1;
		out << stat;
		out.flush();
		out.close();
	}

	return 0;
}
