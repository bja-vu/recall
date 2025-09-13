#include "db.h"
#include <iostream>
#include <stdexcept>

Database::Database(const std::string& path) {
	if (sqlite3_open(path.c_str(), &db_)) {
		printf("Error: Cannot open database.\n");
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
// Fetch last N prompts/responses
std::vector<std::pair<std::string,std::string>> Database::chatHistory(int limit) {
	std::vector<std::pair<std::string,std::string>> results;
	const char* sql = "SELECT prompt, response FROM prompts ORDER BY id DESC LIMIT ?";

	sqlite3_stmt* stmt;
	if (sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr) != SQLITE_OK) {
		printf("Prepare failed: %s\n", sqlite3_errmsg(db_));
		return results;
	}

	sqlite3_bind_int(stmt, 1, limit);

	while (sqlite3_step(stmt) == SQLITE_ROW) {
		std::string p = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
		std::string r = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
		results.emplace_back(p, r);
	}

	sqlite3_finalize(stmt);
	return results;
}
