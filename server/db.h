#include <sqlite3.h>
#include <string>
#include <vector>
#include <utility>
#include <climits>
#include <optional>

class Database {
public:
	Database(const std::string& path);
	~Database();

	void savePrompt(const std::string& prompt,
		 	const std::string& response,
		 	const std::string& type,
		 	const std::vector<float>& vec,
		 	const std::string& lang);
	std::vector<std::pair<std::string,std::string>> chatHistory(int limit);
	std::vector<std::pair<std::string,std::string>> historySearch(std::string search, int limit);
	std::string chatHistoryStr(int limit);
	std::vector<std::pair<int, std::vector<float>>> get_embeddings() const; // const at end makes function safe for const Database
	std::pair<std::string,std::string> get_entry(int id);
	std::optional<std::string> get_lang_from_id(int id) const; // ^^
	void close();
private:
	sqlite3* db_;
};
