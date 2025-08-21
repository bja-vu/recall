import unittest

lang_prefix = ["on", "in", "with", "using"]
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

def which_lang(prompt):
    words = prompt.split()
    if len(words) == 1:
        res = which_lang_alias(words[0])
        if res:
            return res[1]
        return None
    for i in range(len(words)-1, -1, -1):
        res = which_lang_alias(words[i])
        if res:
            safe, lang = res
            if safe:
                return lang
            elif i > 0 and words[i-1] in lang_prefix:
                return lang
    return None

class TestLangInference(unittest.TestCase):

    def test_lang_inference_no_prefix_unsafe(self):
        lang = which_lang("what is the largest python")
        self.assertIsNone(lang, f"no prefix word meaning non-programming context, actual: {lang}")

    def test_lang_inference_no_prefix_safe(self):
        lang = which_lang("how to use dicts py")
        self.assertEqual(lang, "python", f"no prefix word but safe alias means programming context, actual: {lang}")

    def test_lang_inference_prefix_unsafe(self):
        lang = which_lang("array syntax in python")
        self.assertEqual(lang, "python", f"prefix word with unsafe alias means programming context, actual: {lang}")

if __name__ == '__main__':
    unittest.main()

