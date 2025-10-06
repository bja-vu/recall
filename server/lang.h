#include <string>
#include <vector>
#include <unordered_map>
#include <optional>
#include <utility>
#include <algorithm>
#include <sstream>

extern const std::vector<std::string> prefixWords;

extern const std::unordered_map<std::string, std::vector<std::string>> safeLangs;

std::optional<std::string> whichLangAlias(std::string word);

std::optional<std::string> detectLang(std::string prompt);
