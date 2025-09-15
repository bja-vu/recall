#include <sqlite3.h>
#include <string>
#include <vector>
#include <utility>
#include <climits>

class Database {
public:
	Database(const std::string& path);
	~Database();

	void savePrompt(const std::string& prompt,
		 	const std::string& response,
		 	const std::string& type,
		 	const std::vector<float>& vec,
		 	const std::string& lang);
	std::vector<std::pair<std::string,std::string>> chatHistory(int limit=10);
	std::vector<std::pair<std::string,std::string>> historySearch(const std::string& search, int limit);
	void close();
private:
	sqlite3* db_;
};
