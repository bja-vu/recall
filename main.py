import sys
import requests
import json
from rich import print
from rich.markdown import Markdown
from rich.console import Console

from lang_utils import detect_lang


def parse_input(args):
    if (len(args) < 2):
        raise ValueError("Not enough args.\n")

    mode = args[1]
    if mode == 'h':
        if len(args) > 2:
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


url = "http://127.0.0.1:8000"


def main():
    console = Console()
    prompt, prompt_type = parse_input(sys.argv)
    if prompt_type == "history":
        payload = {"search": prompt}
        req = requests.post(f"{url}/history", json=payload)
        # return last 10 prompt/response pairs and print
        return

    payload = {"prompt": prompt}
    req = requests.post(f"{url}/{prompt_type}", json=payload)

    response = req.json()["text"]
    console.print(Markdown(response))


if __name__ == "__main__":
    main()
