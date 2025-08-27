from fastapi import FastAPI
from fastapi import Request

app = FastAPI()

@app.post("/read")
async def read(req: Request):
    data = await req.json()
    text = data.get("text", "")
    return {"user_sent": text}
