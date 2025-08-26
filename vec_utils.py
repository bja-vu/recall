from sentence_transformers import SentenceTransformer
import torch
import numpy as np

device = "cuda" if torch.cuda.is_available() else "cpu"
model = SentenceTransformer("sentence-transformers/all-MiniLM-L6-v2", device=device)

def to_vector(model input):
    return model.encode(input, convert_to_tensor=True)

def cosine_similarity(vec1, vec2):
    return np.dot(vec1, vec2) / (np.linalg.norm(vec1) * np.linalg.norm(vec2))


