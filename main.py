import sys
from rich import print
from rich.markdown import Markdown
from rich.console import Console
import subprocess
import json

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

model = "capybarahermes-2.5-mistral-7b.Q4_K_M"

prompt_tune = (
        "Always answer in two sentences or under 50 words. No extra explanation. No notes.\n"
        "Assume the user understands the general topic and needs a quick reminder. Freely use slang and jargon where necessary. ALWAYS answer the question.\n"
        "If the question refers to something that does not exist or is incorrect, say so. Do not answer untruthfully.\n"
        "If the question is programming related, be pragmatic with your answers. Opt for code instead of descriptions.\n"
        "Reply using markdown syntax only. Use one asterisk (*text*) for italics, two asterisks (**text**) for bold, and backticks (`text`) for inline code.\n"
        )

messages=[
    {"role": "user", "content": prompt_tune},
    {"role": "user", "content": prompt}
]

d_args = {
        "model": model,
        "messages": messages,
        "max_tokens": 300,
        "temperature": 0.2,
        "top_p": 0.9
}

res = subprocess.run(
        [
            "curl",
            "http://127.0.0.1:8080/v1/chat/completions",
            "-H","Content-Type: application/json",
            "-d", json.dumps(d_args)
        ],
        capture_output=True
        )

out = res.stdout.decode()
answer=json.loads(out)
md = Markdown(answer["choices"][0]["message"]["content"].strip())
console = Console()
console.print(md)
#print(answer["choices"][0]["message"]["content"].strip())
