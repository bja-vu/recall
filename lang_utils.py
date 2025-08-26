# common words
prefix_words = ["on", "in", "with", "using"]

# dictionaries for "safe" and "unsafe" alias of languages
# unsafe ones require a prefix word to assume a coding context.
safe_langs = { 
    "python": ["py", "py3", "python 3", "python3"],
    "c": ["c", "cpp", "c++"],
    "java": ["java"],
    "go": ["go", "golang"],
    "rust": ["rs"]
}

unsafe_langs = {
    "python": ["python"],
    "go": ["go"],
    "ruby": ["ruby"],
    "rust": ["rust"]
}

def which_lang_alias(word):
    for lang, alias in safe_langs.items():
        if word in alias:
            return (True, lang)
    for lang, alias in unsafe_langs.items():
        if word in alias:
            return (False, lang)
    return None

def detect_lang(prompt):
    words = prompt.split()
    first_alias = None

    # prompt unlikely to be 1 word, but just in case "recall python" should be tagged
    if len(words) == 1:
        res = which_lang_alias(words[0])
        if res:
            return res[1]
        return None
    
    for i in range(1, len(words)):
        res = which_lang_alias(words[i])
        if res:
            safe, lang = res
            if safe:
                if first_alias is None:
                    first_alias = lang
            if words[i-1] in prefix_words:
                return lang
    return first_alias if first_alias else None
