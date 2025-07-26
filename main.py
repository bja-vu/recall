import sys
from llama_cpp import Llama
from rich import print

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

model = "capybarahermes-2.5-mistral-7b.Q4_K_M.gguf"

llm = Llama(model_path=f"models/{model}",verbose=False)

prompt_tune = (
        "Format your response using markup: use [bold]text[/bold], [italic]text[/italic], [colorname]text[/colorname] (e.g., [red]text[/red]), and other rich styles as appropriate to emphasize key points. Do not let formatting affect the correctness or meaning of your answer."
        "Answer ONLY with the direct response. Do NOT add any notes, or explanations."
        "Always answer in two sentences or under 50 words. No extra explanation. No notes.\n"
        "Assume the user understands the general topic and needs a quick reminder. Freely use slang and jargon where necessary. ALWAYS answer the question.\n"
        "If the question refers to something that does not exist or is incorrect, say so. Do not answer untruthfully.\n"
        )
res = llm.create_chat_completion(
        messages=[
            {"role": "user", "content": prompt_tune},
            {"role": "user", "content": prompt}
            ],
        max_tokens=300,
        temperature=0.2,
        top_p=0.9
        )
print(res["choices"][0]["message"]["content"].strip())
