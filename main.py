import sys
prompt_input = sys.argv[1:]
if not prompt_input:
    print("error: no prompt entered.")
    exit()
prompt = " ".join(prompt_input)
if len(prompt_input) < 5:
    print(f"was your prompt correct? - ({prompt})")
    confirm_prompt = input("enter y/n: ")
    if confirm_prompt == "n": # doesn't explicitly need a y to continue
        exit() 
from llama_cpp import Llama
model = "capybarahermes-2.5-mistral-7b.Q4_K_M.gguf"

llm = Llama(model_path=f"models/{model}",verbose=False)

prompt_tune = (
        "Answer ONLY with the direct response. Do NOT add any notes, or explanations. "
        "Always answer in two sentences or under 50 words. No extra explanation. No notes. "
        "Assume the user understands the general topic and needs a quick reminder. Freely use slang and jargon where necessary.\n"
        )
res = llm.create_chat_completion(
        messages=[
            {"role": "user", "content": prompt_tune},
            {"role": "user", "content": prompt}
            ],
        max_tokens=120,
        temperature=0.2,
        top_p=0.9
        )
print(res["choices"][0]["message"]["content"].strip())
