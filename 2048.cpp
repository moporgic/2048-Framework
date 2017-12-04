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
#include <map>
#include <regex>
#include <memory>
#include "board.h"
#include "action.h"
#include "agent.h"
#include "statistic.h"

int shell(int argc, const char* argv[]) {

	struct match {
		std::string id;
		std::shared_ptr<agent> play;
		std::shared_ptr<agent> evil;
		board b;
		std::shared_ptr<statistic> stat;
		match(const std::string& id, const std::string& info) : id(id) {
			play = new agent("name=" + info.substr(0, info.find(':')) + " role=dummy");
			evil = new agent("name=" + info.substr(info.find(':') + 1) + " role=dummy");
			stat = new statistic(1);
		}
	};
	// match $id $role new
	// match $id $role move request
	// match $id $role move $action
	// match $id $role win

	// match $id $role move $action

	std::regex match_open  ("^match \\S+ (play|evil) open \\S+:\\S+$");
	std::regex match_take  ("^match \\S+ (play|evil) take turn$");
	std::regex match_move  ("^match \\S+ (play|evil) move \\d+$");
	std::regex match_close ("^match \\S+ (play|evil) close \\S+$");

	std::map<std::string, std::shared_ptr<agent>> lounge;
	std::map<std::string, std::shared_ptr<match>> arena;

	std::string command;
	while (std::getline(std::cin, command)) {

		if (std::regex_match(command, match_take)) {
			std::string id, role, buf;
			std::stringstream(command) >> buf >> id >> role;

			match& m = *arena.at(id);
			agent& who = *((role == "player") ? m.play : m.evil);
			if (who.role() != "dummy") {
				action a = who.take_action(m.b);
				std::cout << "match " << id << " " << role << " move " << int(a) << std::endl;
			}

		} else if (std::regex_match(command, match_move)) {
			std::string id, role, buf; int code;
			std::stringstream(command) >> buf >> id >> role >> buf >> code;

			match& m = *arena.at(id);
			statistic& stat = *m.stat;
			int r = action(code).apply(m.b);
			if (r != -1) {
				stat.save_action(code);
			}

		} else if (std::regex_match(command, match_open)) {
			std::string id, role, info, buf;
			std::stringstream(command) >> buf >> id >> role >> buf >> info;

			if (arena.find(id) == arena.end()) {
				arena[id] = new match(id, info);
				match& m = *arena.at(id);
				m.stat->open_episode(info);
				m.b = m.stat->make_empty_board();
			}
			match& m = *arena.at(id);
			auto& who = role == "player" ? m.play : m.evil;
			auto it = std::find_if(lounge.begin(), lounge.end(), [=](std::shared_ptr<agent> p) {
				return p->name() == who->name() && p->role() == who->role();
			});
			if (it != lounge.end()) {
				who = *it;
				who->open_episode(role == "player" ? "~:" + m.evil->name() : m.play->name() + ":~");
			} else {
				arena.erase(id);
			}

		} else if (std::regex_match(command, match_close)) {
			std::string id, role, buf;
			std::stringstream(command) >> buf >> id >> role;

			match& m = *arena.at(id);
			auto& win = *(role == "player" ? m.play : m.evil);
			auto& stat = *m.stat;
			stat.close_episode(win.name());
			m.play->close_episode(win.name());
			m.evil->close_episode(win.name());

			std::string save = id + ".stat";
			std::ofstream out;
			out.open(save.c_str(), std::ios::out | std::ios::binary | std::ios::trunc);
			if (out.is_open()) {
				out << stat;
				out.close();
			}

			arena.erase(id);

		}

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
