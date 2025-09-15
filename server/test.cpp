#include "db.h"
#include <iostream> 
#include <vector>
#include <cstdio>
#include <string>

int main() {
	std::string TEST_DIR = "data/test.db";
	Database db(TEST_DIR);

	// insert with vec
	std::vector<float> v1 = {1.0f, 2.0f, 3.0f};
	db.savePrompt("hello?", "world!", "recall", v1, "en");

	// insert without vec
	db.savePrompt("ping", "pong", "chat", {}, "en");

	// add more entries
	db.savePrompt("ping", "135ms", "recall", {}, "en");
	db.savePrompt("ping", "150ms", "recall", {}, "en");
	db.savePrompt("foo", "bar", "chat", {}, "en");
	// history
	auto rows = db.chatHistory(3);
	std::cout << "got " << rows.size() << " rows\n";
	for (auto& row : rows) {
		std::cout << "Q: " << row.first << "\nA: " << row.second << "\n---\n";
	}
	// search
	std::string query = "ping";
	auto searchRows = db.historySearch(query, 2);
	std:: cout << "got " << rows.size() << " rows \n";

	for (auto& row : searchRows) {
		std::cout << "Q: " << row.first << "\nA: " << row.second << "\n---\n";
	}

	db.close();

	// cleanup
	if (std::remove(TEST_DIR.c_str()) == 0) {
		std::cout << "test db removed.\n";
	} else {
		std::cout << "test db not removed.\n";
	}
	return 0;
}

