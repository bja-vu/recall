import sys
from rich import print
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
    "Q: What is a pointer?\n"
    "A: [bold]Pointer[/bold]: [italic]A variable that holds a memory address[/italic].\n\n"
    "Q: How to print in Python?\n"
    "A: [red]print(\"Hello\")[/red] – [italic]prints text to the console[/italic].\n\n"
    "Q: What is a list?\n"
    "A: [bold]List[/bold]: [green]An ordered, mutable collection[/green].\n\n"
        "Format your response using markup: use [bold]text[/bold] for bold, [italic]text[/italic] for italics, [colorname]text[/colorname] (e.g., [red]text[/red]) for coloured text. Mimic the markup above. Ensure all markup is correct.\n"
        "Answer ONLY with the direct response. Do NOT add any notes, or explanations."
        "Always answer in two sentences or under 50 words. No extra explanation. No notes.\n"
        "Assume the user understands the general topic and needs a quick reminder. Freely use slang and jargon where necessary. ALWAYS answer the question.\n"
        "If the question refers to something that does not exist or is incorrect, say so. Do not answer untruthfully.\n"
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
print(answer["choices"][0]["message"]["content"].strip())
