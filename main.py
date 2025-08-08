import sys
from rich import print
from rich.markdown import Markdown
from rich.console import Console
import subprocess
import json
import sqlite3

# globals
model = "capybarahermes-2.5-mistral-7b.Q4_K_M"

prompt_tune = (
       "Always answer in two sentences or under 50 words. No extra explanation. No notes.\n"
    "Assume the user understands the general topic and needs a quick reminder. Freely use slang and jargon where necessary. ALWAYS answer the question.\n"
    "If the question refers to something that does not exist or is incorrect, say so. Do not answer untruthfully.\n"
    "If the question is programming related, be pragmatic with your answers. Opt for code instead of descriptions.\n"
    "Reply using markdown syntax only. Use one asterisk (*text*) for italics, two asterisks (**text**) for bold, and backticks (`text`) for inline code.\n"
    "Infer missing context from previous messages. Never ask for clarification.\n"
)
def init_db():
    # initialises sqlite3 db, returns tuple
    con = sqlite3.connect("data/memory.db")
    cur = con.cursor()

    cur.execute('''
    CREATE TABLE IF NOT EXISTS prompts (
                id INTEGER PRIMARY KEY AUTOINCREMENT,
                prompt TEXT,
                response TEXT,
                type TEXT
                )
    ''')
    return con,cur

def parse_input(args) -> tuple[str, str]:
    if len(args) < 3:
        raise ValueError("No prompt entered.\n")
    if args[1] == 'r':
        prompt_type = "recall"
    elif args[1] == 'c':
        prompt_type = "chat"
    else:
        raise ValueError("Argument must contain either an 'r' or a 'c' as its first entry, signifying recall or chat.")

    prompt_input = args[2:]
    prompt = " ".join(prompt_input)
    if len(prompt_input) < 3:
        print(f"was your prompt correct? - ({prompt})")
        confirm_prompt = input("enter y/n:")
        if confirm_prompt == "n": # doesn't explicitly need a y to continue
            exit()
    return prompt, prompt_type

def chat_context(cur): # NOTE chat history could overload the models token limit
    last_recall = cur.execute(
        "SELECT prompt, response FROM prompts WHERE type='recall' ORDER BY id DESC LIMIT 1"
    ).fetchone()

    history = []
    if last_recall:
        history.append({"role": "assistant", "content": last_recall[1]})

    last_recall_id = 0
    if last_recall:
        last_recall_id = cur.execute(
            "SELECT id FROM prompts WHERE type='recall' ORDER BY id DESC LIMIT 1"
        ).fetchone()[0]

    cur.execute(
        "SELECT prompt, response FROM prompts WHERE id > ? AND type='chat' ORDER BY id",
        (last_recall_id,),
    )
    for prompt, response in cur.fetchall():
        history.append({"role": "user", "content": prompt})
        history.append({"role": "assistant", "content": response})

    return history


def curl_llm(messages):
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
    # process output, return raw text
    out = res.stdout.decode()
    answer = json.loads(out)
    return answer["choices"][0]["message"]["content"].strip()
    
def save_to_db(cur, prompt, raw, prompt_type):
    cur.execute("INSERT INTO prompts (prompt, response, type) VALUES (?, ?, ?)",
                (prompt, raw, prompt_type))

def main():
    con, cur = init_db()
    prompt, prompt_type = parse_input(sys.argv)
    history = chat_context(cur) if prompt_type == "chat" else []

    messages = [{"role": "user", "content": prompt_tune}] + history
    messages.append({"role": "user", "content": prompt})

    raw = curl_llm(messages)
    console = Console()
    console.print(Markdown(raw))
    
    save_to_db(cur, prompt, raw, prompt_type)
    #print(f"added {prompt} - {raw} - {prompt_type}")
    con.commit()
    con.close()

if __name__ == "__main__":
    main()
