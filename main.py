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
        limit = 10
        search = "%"

        if (len(args) >= 3):
            if args[2].isdigit():
                limit = int(args[2])
                search = ("%" + (" ".join(args[3:])) + "%") if len(args)  > 3 else "%"
            else:
                search = " ".join(args[2:])
        print(search)
        return (search,limit), "history"


    if len(args) < 3:
        raise ValueError("No prompt was entered.\n")

    prompt_input = args[2:]
    prompt = " ".join(prompt_input)

    if mode == 'r':
        return prompt, "recall"
    if mode == 'c':
        return prompt, "chat"


url = "http://0.0.0.0:8000"


def main():
    console = Console()
    prompt, prompt_type = parse_input(sys.argv)
    if prompt_type == "history":
        payload = {"search": prompt[0], "limit": prompt[1]}
        req = requests.post(f"{url}/history", json=payload)
        resp = req.json()
        for row in resp:
            print("prompt: ", row["prompt"])
            print("response: ", row["response"])
            print("\n-----\n-----\n")
        return

    payload = {"prompt": prompt}
    req = requests.post(f"{url}/{prompt_type}", json=payload)

    response = req.json()["text"]
    console.print(Markdown(response))


if __name__ == "__main__":
    main()
