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

def parse_input(args) -> tuple[str | None, str]:
    if len(args) < 2:
        raise ValueError("Not enough args.\n")
    
    mode = args[1] # recall, chat, history
    if mode == 'h': # history, optional search term
        if len(args) >  2:
            search = " ".join(args[2:])
        else:
            search = None
        return search, "history"

    if len(args) < 3:
        raise ValueError("No prompt was entered.\n")
    
    prompt_input = args[2:]
    prompt = " ".join(prompt_input)
    if len(prompt_input) < 3:
        print("was your prompt input correct?")
        confirm = input("enter y/n:")
        while True:
            confirm = input("enter y/n:")
            if confirm.lower().strip() == 'y':
                break
            if confirm.lower().strip() == 'n':
                exit()

    if mode == 'r':
        return prompt, "recall"
    elif mode == 'c':
        return prompt, "chat"

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

def chat_history(cur, term=None):
    if term: # with term searches, you want many more
        cur.execute('SELECT prompt, response FROM prompts WHERE prompt LIKE ? OR response LIKE ? ORDER BY id',(f"%{term}%", f"%{term}%"))
    else: # with no search, just display last 10
        cur.execute('SELECT prompt, response FROM prompts ORDER BY id DESC LIMIT 10' )
    return cur.fetchall()

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
    if not prompt or not raw:
        raise ValueError("Prompt or response is empty, exiting...")
    cur.execute("INSERT INTO prompts (prompt, response, type) VALUES (?, ?, ?)",
                (prompt, raw, prompt_type))

def main():
    console = Console()
    con, cur = init_db()

    prompt, prompt_type = parse_input(sys.argv)
    if prompt_type == "history":
        rows =chat_history(cur,prompt)
        for p, r in rows:
            console.print(f"Q: ", Markdown(p), "\nA: ", Markdown(r))
            console.rule()
        return
    history = chat_context(cur) if prompt_type == "chat" else []

    messages = [{"role": "user", "content": prompt_tune}] + history
    messages.append({"role": "user", "content": prompt})

    raw = curl_llm(messages)
    console.print(Markdown(raw))
    
    save_to_db(cur, prompt, raw, prompt_type)
    con.commit()
    con.close()

if __name__ == "__main__":
    main()
