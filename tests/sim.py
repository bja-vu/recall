from sentence_transformers import SentenceTransformer
import torch

device = "cuda" if torch.cuda.is_available() else "cpu"
model = SentenceTransformer("sentence-transformers/all-MiniLM-L6-v2", device=device)

examples = [
        "how to iterate through a list in py",
        "how to loop through a py list",
        "removing dupes from a py list",
        "convert a py list to a set",
        "how to remove duplicates from a list in c" 
        ]

embeddings = model.encode(examples, convert_to_tensor=True)

new_prompt = "how to remove duplicates from a list in python"
new_embedding = model.encode([new_prompt], convert_to_tensor=True)

sims = torch.nn.functional.cosine_similarity(new_embedding, embeddings)
most_sim = torch.argmax(sims).item()

print("most similar = ", examples[most_sim])
print("num: ", sims[most_sim].item())
