from fastapi import FastAPI, Request
from llama_cpp import Llama

llm = Llama(model_path="models/capybarahermes-2.5-mistral-7b.Q4_K_M.gguf", gpu_layers=32)

app = FastAPI()

@app.post("/read")
async def read(req: Request):
    data = await req.json()
    prompt = data.get("text", "")
    if not prompt:
        return {"error":"no prompt entered"}

    output = llm(prompt, max_tokens=200)
    return {"response": output["choices"][0]["text"]}
