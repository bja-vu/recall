from sentence_transformers import SentenceTransformer
import torch

def which_lang(prompt): # TODO a better solution could require word like "in", "with" before lang
    words = prompt.split()
    for word in reversed(words):
        for lang, alias in langs.items():
            if word in alias:
                return lang
    return None

device = "cuda" if torch.cuda.is_available() else "cpu"
model = SentenceTransformer("sentence-transformers/all-MiniLM-L6-v2", device=device)

langs = { # add more as required
        "python": ["py", "py3", "python", "python 3", "python3"],
        "c": ["c", "cpp", "c++"]
        }

emb_1 = model.encode(["how to iterate like c through lists in py"], convert_to_tensor=True)
emb1_lang = which_lang("how to iterate like c through lists in py")

emb_2 = model.encode(["how to iterate over a list in c"], convert_to_tensor=True)
emb2_lang = which_lang("how to iterate over a list in c")

emb_3 = model.encode(["how to python loop over a list in c"], convert_to_tensor=True)
emb3_lang = which_lang("how to python loop over a list in c")

emb_4 = model.encode(["iterate over py list"], convert_to_tensor=True)
emb4_lang = which_lang("iterate over py list")

embeddings = {
    "py_iterate": (emb_1, emb1_lang),
    "c_iterate_1": (emb_2, emb2_lang),
    "c_iterate_2": (emb_3, emb3_lang),
    "py_loop": (emb_4, emb4_lang)
}

names = list(embeddings.keys())
for i in range(len(names)):
    for j in range(i + 1, len(names)):
        emb_i, lang_i = embeddings[names[i]]
        emb_j, lang_j = embeddings[names[j]]
        # Only compare if same language
        if lang_i == lang_j:
            sim = torch.nn.functional.cosine_similarity(emb_i, emb_j)
            print(f"Cosine similarity between '{names[i]}' and '{names[j]}': {sim.item():.4f}")

