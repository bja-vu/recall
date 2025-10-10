#include "lang.h"
#include <sstream>
#include <string>

const std::vector<std::string> prefixWords = {"on", "in", "with", "using"};

// I used to have "safe" and unsafe langs to avoid tagging non-coding related prompts with languages
// but I am focusing on coding now, it is safe to assume all prompts can be associated with a programming language if the alias is contained
const std::unordered_map<std::string, std::vector<std::string>> langAliases = {
	{"python", {"py", "py3", "python 3", "python3", "python"}},
	{"c", {"c", "cpp", "c++"}},
	{"java", {"java"}},
	{"go", {"go", "golang"}},
	{"rust", {"rust", "rs"}}
};

std::optional<std::string> whichLangAlias(std::string word) {
	for (char& c : word) c = tolower(c); // convert each word passed to lowercase for the check
	//printf("Checking word: '%s'\n", word.c_str());
	//printf("langAliases size: %zu\n", langAliases.size());
	for (const auto& [k,v] : langAliases) {
		//printf("  Lang '%s' has %zu aliases: ", k.c_str(), v.size());
		//for (const auto& alias : v) {
			//printf("'%s' ", alias.c_str());
		//}
		//printf("\n");
		if (std::find(v.begin(), v.end(), word) != v.end()) {
			return std::make_optional( k);
		}
	}
	return std::nullopt;
}

std::optional<std::string> detectLang(std::string prompt) {
	std::vector<std::string> words;
	std::istringstream iss(prompt);
	std::string word;
	while (iss >> word) {
		words.push_back(word);
	}

	std::string first_alias;

	// check for alias in first word
	const auto& first_check = whichLangAlias(words[0]);
	if (first_check.has_value()) {
		first_alias = first_check.value();
	}

	//printf("DEBUG: prompt='%s', words.size()=%zu\n", prompt.c_str(), words.size());
	//for (size_t i = 0; i < words.size(); i++) {
	    //printf("  words[%zu]='%s'\n", i, words[i].c_str());
	//}

	for (size_t i=1; i < words.size(); i++) {
		const auto& res = whichLangAlias(words[i]);
		if (res.has_value()) {
			if (first_alias.empty()) {
				first_alias = res.value();
			}
			if (std::find(prefixWords.begin(), prefixWords.end(), words[i-1]) != prefixWords.end()) {
				return res.value();
			}
		}
	}
	if (!first_alias.empty()) {return first_alias;}
	return std::nullopt;
}
