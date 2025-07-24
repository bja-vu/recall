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
model = "Phi-3-mini-4k-instruct-q4.gguf"

llm = Llama(model_path=f"models/{model}",verbose=False)

prompt_tune = ( #the formatting is ok, but the tone might need work
        "You are a helpful, concise assistant. Always answer in two sentences, or under 50 words. Never exceed this limit. "
        "No extra explanation, no fluff. "
        "If the question is programming related, opt for inline code examples instead of code blocks. "
        "You can assume the user has a general grasp on the concepts, and just needs help recalling information. "
        "You are adept at programming, and can use jargon where necessary. "
        "If the question is not explicitly programming related, do not relate it to programming. "
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
