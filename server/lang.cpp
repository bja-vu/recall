#include "lang.h"
#include <sstream>

const std::vector<std::string> prefixWords = {"on", "in", "with", "using"};

// maps for safe and unsafe alias' of languages
// unsafe ones require a prefix word to assume a coding context

const std::unordered_map<std::string, std::vector<std::string>> safeLangs = {
	{"python", {"py", "py3", "python 3", "python3"}},
	{"c", {"c", "cpp", "c++"}},
	{"java", {"java"}},
	{"go", {"go", "golang"}},
	{"rust", {"rs"}}
};

extern const std::unordered_map<std::string, std::vector<std::string>> unsafeLangs = {
	{"python", {"python"}},
	{"go", {"go"}},
	{"ruby", {"ruby"}},
	{"rust", {"rust"}}
};

std::optional<std::pair<bool, std::string>> whichLangAlias(std::string word) {
    printf("Checking word: '%s'\n", word.c_str());
    printf("safeLangs size: %zu\n", safeLangs.size());
    for (const auto& [k,v] : safeLangs) {
        printf("  Lang '%s' has %zu aliases: ", k.c_str(), v.size());
        for (const auto& alias : v) {
            printf("'%s' ", alias.c_str());
        }
        printf("\n");
        if (std::find(v.begin(), v.end(), word) != v.end()) {
            return std::make_optional(std::make_pair(true, k));
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

	// check for safelang in first word
	const auto& first_check = whichLangAlias(words[0]);
	if (first_check.has_value() && first_check.value().first) {
		first_alias = first_check.value().second;
	}

	if (words.size() == 1) {
		const auto& res = whichLangAlias(words[0]);
		if (res.has_value()) {
			return res.value().second;
		}
		return std::nullopt;
	}
	printf("DEBUG: prompt='%s', words.size()=%zu\n", prompt.c_str(), words.size());
	for (size_t i = 0; i < words.size(); i++) {
	    printf("  words[%zu]='%s'\n", i, words[i].c_str());
	}

	for (size_t i=1; i < words.size(); i++) {
		const auto& res = whichLangAlias(words[i]);
		if (res.has_value()) {
			if (res.value().first) {
				if (first_alias.empty()) {
					first_alias = res.value().second;
				}
				if (std::find(prefixWords.begin(), prefixWords.end(), words[i-1]) != prefixWords.end()) {
					return res.value().second;
				}
			}
		}
	}
	if (!first_alias.empty()) {return first_alias;}
	return std::nullopt;
}
