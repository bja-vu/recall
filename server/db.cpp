#include "db.h"
#include <iostream>
#include <stdexcept>
#include <cstring>

Database::Database(const std::string& path) {
	if (sqlite3_open(path.c_str(), &db_) != SQLITE_OK) {
		printf("Error opening database. %s\n", sqlite3_errmsg(db_));
		db_ = NULL;
	} else {
		const char* create_sql_stmt = R"(
			CREATE TABLE IF NOT EXISTS prompts (
				id INTEGER PRIMARY KEY AUTOINCREMENT,
				prompt TEXT,
				response TEXT,
				type TEXT,
				vec BLOB,
				lang TEXT,
				created_at TEXT DEFAULT (datetime('now'))
			)
		)";
		char* errorMsg = NULL;
		if (sqlite3_exec(db_, create_sql_stmt, NULL, NULL, &errorMsg) != SQLITE_OK) {
		printf("SQL error: %s\n", errorMsg);
		sqlite3_free(errorMsg);
		}
	}
}

Database::~Database() {
	if (db_) sqlite3_close(db_);
}

void Database::savePrompt(const std::string& prompt,
			  const std::string& response,
			  const std::string& type,
			  const std::vector<float>& vec,
			  const std::string& lang) {
	const char* sql = "INSERT INTO prompts (prompt, response, type, vec, lang) VALUES (?, ?, ?, ?, ?)";
	sqlite3_stmt* stmt;

	// Prepare
	if (sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr) != SQLITE_OK) { // NOTE: -1 is a hacky way to get SQLite to infer the str length
		printf("prepare failed: %s\n", sqlite3_errmsg(db_));
		return;
	}

	// Bind
	sqlite3_bind_text(stmt, 1, prompt.c_str(), -1, SQLITE_TRANSIENT);
	sqlite3_bind_text(stmt, 2, response.c_str(), -1, SQLITE_TRANSIENT);
	sqlite3_bind_text(stmt, 3, type.c_str(), -1, SQLITE_TRANSIENT);

	if (!vec.empty()) {
		size_t byteCount = vec.size() * sizeof(float);
		if (byteCount > INT_MAX) {
			throw std::overflow_error("Vector too big for SQLite blob");
		}
		sqlite3_bind_blob(stmt, 4, vec.data(), static_cast<int>(byteCount), SQLITE_TRANSIENT);
	} else {
		sqlite3_bind_null(stmt, 4);
	}
	
	sqlite3_bind_text(stmt, 5, lang.c_str(), -1, SQLITE_TRANSIENT);

	// Step
	if (sqlite3_step(stmt) != SQLITE_DONE) {
		printf("Insertion failed: %s\n", sqlite3_errmsg(db_));
	}

	// Finalize
	sqlite3_finalize(stmt);
}

std::vector<std::pair<std::string,std::string>> Database::chatHistory(int limit) {
    std::vector<std::pair<std::string,std::string>> history;
    sqlite3_stmt* stmt = NULL;

    // Step 1: Find the most recent recall (if any)
    int last_recall_id = -1;
    const char* recall_id_sql = "SELECT id FROM prompts WHERE type='recall' ORDER BY id DESC LIMIT 1";
    
    if (sqlite3_prepare_v2(db_, recall_id_sql, -1, &stmt, NULL) == SQLITE_OK) {
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            last_recall_id = sqlite3_column_int(stmt, 0);
        }
        sqlite3_finalize(stmt);
    }

    // Step 2: get the last *limit* entries from the subsection last_recall-end
const char* messages_sql = 
		// returns the "last" n pairs from the section after the last recall
		"SELECT prompt, response, type FROM (SELECT * FROM prompts WHERE id > ? ORDER BY id DESC LIMIT ?) ORDER BY id ASC";
	if (sqlite3_prepare_v2(db_, messages_sql, -1, &stmt, NULL) != SQLITE_OK) {
		printf("Error: failed to prepare messages query: %s\n", sqlite3_errmsg(db_));
		return history;
	}
	sqlite3_bind_int(stmt, 1, last_recall_id);
	sqlite3_bind_int(stmt, 2, limit);

	while (sqlite3_step(stmt) == SQLITE_ROW) {
		std::string prompt = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
		std::string response = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
		std::string type = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
		if (type == "recall") {
		    // For recall, only include the response as assistant context
		    history.push_back({"assistant", response});
		} else if (type == "chat") {
		    // For chat, include both user prompt and assistant response
		    history.push_back({"user", prompt});
		    history.push_back({"assistant", response});
		}
	}
	sqlite3_finalize(stmt);
	return history;
}


std::vector<std::pair<std::string,std::string>> Database::historySearch(std::optional<std::string> search_opt, std::optional<int> limit_opt) {
	std::string search = "%";
	int limit = 10;
	if (search_opt.has_value()) {
		search = *search_opt;
	}
	if (limit_opt.has_value()) {
		limit = *limit_opt;
	}

	std::vector<std::pair<std::string,std::string>> results;
	const char* sql = "SELECT prompt, response FROM prompts "
	"WHERE prompt LIKE ? OR response LIKE ? "
	"ORDER BY id DESC LIMIT ?";

	sqlite3_stmt* stmt;
	if (sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr) != SQLITE_OK) {
		printf("Prepare failed: %s\n", sqlite3_errmsg(db_));
		return results;
	}

	sqlite3_bind_text(stmt, 1, search.c_str(), -1, SQLITE_TRANSIENT);
	sqlite3_bind_text(stmt, 2, search.c_str(), -1, SQLITE_TRANSIENT);
	sqlite3_bind_int(stmt, 3, limit);

	while (sqlite3_step(stmt) == SQLITE_ROW) {
		std::string p = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
		std::string r = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
		results.emplace_back(p, r);
	}
	
	sqlite3_finalize(stmt);
	return results;
}

std::string Database::chatHistoryStr(int limit) {
	auto history = chatHistory(limit);
	std::string res;
	    printf("\nCHAT HISTORY (from DB)\n");
	for (const auto& [role, text] : history) {
		if (role == "user") {
			res += "User: " + text + "\n";
		} else if (role == "assistant") {
			res += "Assistant: " + text + "\n";
		} else {
			res += role + ": " + text + "\n";
		}
		printf("%s: %s\n", role.c_str(), text.c_str());
	}
	return res;
}

std::vector<std::vector<float>> Database::get_embeddings() const {
	// returns all embeddings in a 2d list
	// unopt - idk if it can be
	std::vector<std::vector<float>> embeddings;
	sqlite3_stmt* stmt = NULL;

	const char* sql = "SELECT vec FROM prompts WHERE vec IS NOT NULL";
	if (sqlite3_prepare_v2(db_, sql, -1, &stmt, NULL) != SQLITE_OK) {
		printf("error: failed to prepare statement (vec. embeddings).\n");
	}
	while (sqlite3_step(stmt) == SQLITE_ROW) {
		const void* blob = sqlite3_column_blob(stmt, 0);
		int bytes = sqlite3_column_bytes(stmt, 0);

		if (blob && bytes > 0) {
			int n = bytes / sizeof(float);
			std::vector<float> vec(n);
			std::memcpy(vec.data(), blob, bytes);
			embeddings.push_back(vec);
		}
	}
	sqlite3_finalize(stmt);
	return embeddings;
}

std::pair<std::string, std::string> Database::get_entry(int idx) {
	std::string prompt;
	std::string resp;
	sqlite3_stmt* stmt = NULL;

	const char* sql = "SELECT prompt, response FROM prompts LIMIT 1 OFFSET ?";

	if (sqlite3_prepare_v2(db_, sql, -1, &stmt, NULL) != SQLITE_OK) {
		printf("error: failed to prepare statement (entry retrieval.\n");
	}

	if (sqlite3_bind_int(stmt, 1, idx) != SQLITE_OK) {
		printf("error: failed to bind id for entry retrieval.\n");
		sqlite3_finalize(stmt);
	}
	if (sqlite3_step(stmt) == SQLITE_ROW) {
		prompt = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
		resp = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
	}
	sqlite3_finalize(stmt);
	return {prompt, resp};
}

void Database::close() {
	if (db_) {
		sqlite3_close(db_);
		db_ = NULL;
	}
}
