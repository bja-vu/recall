from llama_cpp import Llama
model = "Phi-3-mini-4k-instruct-q4.gguf"

llm = Llama(model_path=f"models/{model}",verbose=False)

prompt_tune = ( #the formatting is ok, but the tone might need work
        "You are a helpful, concise assistant. Always answer in two sentences, or under 50 words. Never exceed this limit. "
        "No extra explanation, no fluff. "
        "If the question is programming related, opt for inline code examples instead of code blocks. "
        "You can assume the user has a general grasp on the concepts, and just needs help recalling information. "
        "You are adept at programming, and can use jargon where necessary. "
        )
prompt = "how to use dicts in python" #hardcoded, will take an input eventually
res = llm.create_chat_completion(
        messages=[
            {"role": "user", "content": prompt_tune},
            {"role": "user", "content": prompt}
            ],
        max_tokens=120
        )
print("\nResponse:\n")
print(res["choices"][0]["message"]["content"])
