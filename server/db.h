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
	std::vector<std::pair<std::string,std::string>> chatHistory();
	std::vector<std::pair<std::string,std::string>> historySearch(std::optional<std::string> search, std::optional<int> limit);
	std::string chatHistoryStr();
	void close();
private:
	sqlite3* db_;
};
