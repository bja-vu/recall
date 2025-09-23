from sentence_transformers import SentenceTransformer
from fastapi import FastAPI, Request
import torch

device = "cuda" if torch.cuda.is_available() else "cpu"
model = SentenceTransformer("sentence-transformers/all-MiniLM-L6-v2", device=device)

app = FastAPI()

@app.post("/embed")
async def embed(req: Request):
    print("embed request received")
    data = await req.json()
    text = data.get("prompt", "")
    print("prompt: " + text)
    vec = model.encode(text, convert_to_numpy=True).tolist()
    return {"embedding": vec}
