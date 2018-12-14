#pragma once
#include <fstream>
#include <memory>
#include <unordered_map>
#include <string>
#include "agent.h"
#include "episode.h"

class arena {
public:
	class match : public episode {
	public:
		match(const std::string& id, std::shared_ptr<agent> play, std::shared_ptr<agent> evil) : id(id), play(play), evil(evil) {}
		std::string name() const { return id; }

		action take_action() {
			agent& who = take_turns(*play, *evil);
			return who.take_action(state());
		}

	private:
		std::string id;
		std::shared_ptr<agent> play;
		std::shared_ptr<agent> evil;
	};

public:
	arena() {}

	arena::match& at(const std::string& id) {
		return *(ongoing.at(id));
	}
	bool open(const std::string& id, const std::string& tag) {
		if (ongoing.find(id) != ongoing.end()) return false;

		auto play = find_agent(tag.substr(0, tag.find(':')));
		auto evil = find_agent(tag.substr(tag.find(':') + 1));
		if (play->role() == "dummy" && evil->role() == "dummy") return false;

		std::shared_ptr<match> m(new match(id, play, evil));
		m->open_episode(tag);
		ongoing[id] = m;
		return true;
	}
	bool close(const std::string& id, const std::string& tag) {
		auto it = ongoing.find(id);
		if (it != ongoing.end()) {
			auto m = it->second;
			m->close_episode(tag);
			dump << (*m) << std::endl << std::flush;
			ongoing.erase(it);
			return true;
		}
		return false;
	}

public:
	bool register_agent(std::shared_ptr<agent> a) {
		if (lounge.find(a->name()) != lounge.end()) return false;
		lounge[a->name()] = a;
		return true;
	}
	bool remove_agent(std::shared_ptr<agent> a) {
		return lounge.erase(a->name());
	}

private:
	std::shared_ptr<agent> find_agent(const std::string& name) {
		auto it = lounge.find(name);
		if (it != lounge.end()) return it->second;
		return std::shared_ptr<agent>(new agent("name=" + name + " role=dummy"));
	}

public:
	std::vector<std::shared_ptr<match>> list_matches() {
		std::vector<std::shared_ptr<match>> res;
		for (auto ep : ongoing) res.push_back(ep.second);
		return res;
	}
	std::vector<std::shared_ptr<agent>> list_agents() {
		std::vector<std::shared_ptr<agent>> res;
		for (auto who : lounge) res.push_back(who.second);
		return res;
	}

public:
	void set_dump_file(const std::string& path) {
		if (dump.is_open()) dump.close();
		dump.clear();
		dump.open(path, std::ios::out | std::ios::app);
	}

private:
	std::unordered_map<std::string, std::shared_ptr<agent>> lounge;
	std::unordered_map<std::string, std::shared_ptr<match>> ongoing;
	std::ofstream dump;
};