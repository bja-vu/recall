import sys
from rich import print
from rich.markdown import Markdown
from rich.console import Console
import subprocess
import json
import sqlite3

# initialise database
con = sqlite3.connect('data/memory.db')
cur = con.cursor()

cur.execute('''
CREATE TABLE IF NOT EXISTS prompts (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            prompt TEXT,
            response TEXT,
            type TEXT
            )
''')
cur.execute("SELECT name FROM sqlite_master WHERE type='table' AND name='prompts'")
if cur.fetchone() is not None:
    print("DB initialized")
else:
    print("DB not initialized")

# handle user input
if sys.argv[1] == 'r': #TODO: requires proper user input
    prompt_type = "recall"
elif sys.argv[1] == 'c':
    prompt_type = "chat"

prompt_input = sys.argv[2:]
if not prompt_input:
    print("error: no prompt entered.")
    exit()
prompt = " ".join(prompt_input)
if len(prompt_input) < 5:
    print(f"was your prompt correct? - ({prompt})")
    confirm_prompt = input("enter y/n: ")
    if confirm_prompt == "n": # doesn't explicitly need a y to continue
        exit() 

# llm configuration
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

# make curl call to server
res = subprocess.run(
        [
            "curl",
            "http://127.0.0.1:8080/v1/chat/completions",
            "-H","Content-Type: application/json",
            "-d", json.dumps(d_args)
        ],
        capture_output=True
        )

# process output
out = res.stdout.decode()
answer=json.loads(out)
raw = answer["choices"][0]["message"]["content"].strip()
md = Markdown(raw)
console = Console()
console.print(md)
#print(answer["choices"][0]["message"]["content"].strip())

# save to database
cur.execute('''
          INSERT INTO prompts (prompt,response,type) VALUES (?, ?, ?)''', (prompt, raw, prompt_type))
print(f"added {prompt} - {raw} - {prompt_type}")
con.commit()
con.close()
