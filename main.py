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
prompt += "\n<<assistant>>\n"
from llama_cpp import Llama
model = "Phi-3-mini-4k-instruct-q4.gguf"

llm = Llama(model_path=f"models/{model}",verbose=False)

prompt_tune = ( #the formatting is ok, but the tone might need work
        "<<system>>\n"
        "You are a concise assistant. Always answer in two sentences or under 50 words. No extra explanation. "
        "Assume the user understands the general topic and needs a quick reminder.\n"
        "<<user>>\n"
        )
res = llm.create_chat_completion(
        messages=[
            {"role": "user", "content": prompt_tune},
            {"role": "user", "content": prompt}
            ],
        max_tokens=120
        )
print("\nResponse:\n")
print(res["choices"][0]["message"]["content"].strip())
