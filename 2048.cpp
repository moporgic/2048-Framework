/**
 * Basic Environment for Game 2048
 * use 'g++ -std=c++11 -O3 -g -o 2048 2048.cpp' to compile the source
 *
 * Computer Games and Intelligence (CGI) Lab, NCTU, Taiwan
 * http://www.aigames.nctu.edu.tw
 */

#include <iostream>
#include <fstream>
#include <iterator>
#include <string>
#include <regex>
#include <memory>
#include "board.h"
#include "action.h"
#include "agent.h"
#include "episode.h"
#include "statistic.h"
#include "arena.h"
#include "io.h"

int shell(int argc, const char* argv[]) {
	arena host("anonymous");

	for (int i = 1; i < argc; i++) {
		std::string para(argv[i]);
		if (para.find("--name=") == 0 || para.find("--account=") == 0) {
			host.set_account(para.substr(para.find("=") + 1));
		} else if (para.find("--login=") == 0) {
			host.set_login(para.substr(para.find("=") + 1));
		} else if (para.find("--save=") == 0 || para.find("--dump=") == 0) {
			host.set_dump_file(para.substr(para.find("=") + 1));
		} else if (para.find("--play") == 0) {
			std::shared_ptr<agent> play(new player(para.substr(para.find("=") + 1)));
			host.register_agent(play);
		} else if (para.find("--evil") == 0) {
			std::shared_ptr<agent> evil(new rndenv(para.substr(para.find("=") + 1)));
			host.register_agent(evil);
		}
	}

	std::regex match_move("^#\\S+ \\S+$"); // e.g. "#M0001 ?", "#M0001 #U"
	std::regex match_ctrl("^#\\S+ \\S+ \\S+$"); // e.g. "#M0001 open Slider:Placer", "#M0001 close score=15424"
	std::regex arena_ctrl("^[@$].+$"); // e.g. "@ login", "@ error the account "Name" has already been taken"
	std::regex arena_info("^[?%].+$"); // e.g. "? message from anonymous: 2048!!!"

	for (std::string command; input() >> command; ) {
		try {
			if (std::regex_match(command, match_move)) {
				std::string id, move;
				std::stringstream(command) >> id >> move;

				if (move == "?") {
					// your agent need to take an action
					action a = host.at(id).take_action();
					host.at(id).apply_action(a);
					output() << id << ' ' << a << std::endl;
				} else {
					// perform your opponent's action
					action a;
					std::stringstream(move) >> a;
					host.at(id).apply_action(a);
				}

			} else if (std::regex_match(command, match_ctrl)) {
				std::string id, ctrl, tag;
				std::stringstream(command) >> id >> ctrl >> tag;

				if (ctrl == "open") {
					// a new match is pending
					if (host.open(id, tag)) {
						output() << id << " open accept" << std::endl;
					} else {
						output() << id << " open reject" << std::endl;
					}
				} else if (ctrl == "close") {
					// a match is finished
					host.close(id, tag);
				}

			} else if (std::regex_match(command, arena_ctrl)) {
				std::string ctrl;
				std::stringstream(command).ignore(1) >> ctrl;

				if (ctrl == "login") {
					// register yourself and your agents
					std::stringstream agents;
					for (auto who : host.list_agents()) {
						agents << " " << who->name() << "(" << who->role() << ")";
					}
					output("@ ") << "login " << host.login() << agents.str() << std::endl;

				} else if (ctrl == "status") {
					// display current local status
					info() << "+++++ status +++++" << std::endl;
					info() << "login: " << host.account();
					for (auto who : host.list_agents()) {
						info() << " " << who->name() << "(" << who->role() << ")";
					}
					info() << std::endl;
					info() << "match: " << host.list_matches().size() << std::endl;
					for (auto ep : host.list_matches()) {
						info() << ep->name() << " " << (*ep) << std::endl;
					}
					info() << "----- status -----" << std::endl;

				} else if (ctrl == "error" || ctrl == "exit") {
					// some error messages or exit command
					std::string message = command.substr(command.find_first_not_of("@$ "));
					info() << message << std::endl;
					break;
				}

			} else if (std::regex_match(command, arena_info)) {
				// message from arena server
			}
		} catch (std::exception& ex) {
			std::string message = std::string(typeid(ex).name()) + ": " + ex.what();
			message = message.substr(0, message.find_first_of("\r\n"));
			output("? ") << "exception " << message << " at \"" << command << "\"" << std::endl;
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
