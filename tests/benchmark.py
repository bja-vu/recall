import requests
import json

test_prompts = [
    "how to utilise dicts in python",
    "python list comprehension",
    "cpp pointer syntax",
    "difference between stack and heap",
    "semaphore vs mutex",
    "how to set up basic flask app with routes and templates",
    "explain observer pattern with code example",
    "compare async/await in python vs javascript with examples"
]

short_chat_prompts = [
    "what is recursion",
    "give me a python example",
    "how would i optimise for large inputs"
]
med_chat_prompts = [
    "how do i read a file in cpp",
    "what if its large",
    "how to count each word in each line",
    "how do i write the results to another file",
    "what about non-text files",
]
long_chat_prompts = [
    "whats a variable",
    "whats a function",
    "whats a class",
    "whats a signature",
    "whats defining vs declaring",
    "whats a pointer",
    "how do they work",
    "going back to my first question what are mutables",
    "whats abstraction",
    "what is oop"
]

def main():
    # iterating through test prompts
    # pulling the test results
    # storing them in a file (what kind?) csv
    with open("tests/results/recall_benchmark.csv", "w") as f:
        f.write("prompt,embed,search,gen,total\n")

        for prompt in test_prompts:
            resp = requests.post("http://localhost:8000/recall", json = {"prompt": prompt})
            data = resp.json()
            timing_info = data["timing"]
            embed = timing_info["embed"]
            search = timing_info["search"]
            gen = timing_info["gen"]
            total = timing_info["total"]
            f.write(f"{prompt},{embed},{search},{gen},{total}\n")

    with open("tests/results/chat_benchmark.csv", "w") as f:
        f.write("prompt,history,gen,total\n")

        foo = False
        for prompt in short_chat_prompts:
            if not foo:
                # not keeping the gen timings of this bc its annoying
                resp = requests.post("http://localhost:8000/recall", json = {"prompt": prompt})
                foo = True
            else:
                resp = requests.post("http://localhost:8000/chat", json = {"prompt": prompt})
                data = resp.json()
                timing_info = data["timing"]
                history = timing_info["history"]
                gen = timing_info["gen"]
                total = timing_info["total"]
                f.write(f"{prompt},{history},{gen},{total}\n")

        foo = False
        for prompt in med_chat_prompts:
            if not foo:
                resp = requests.post("http://localhost:8000/recall", json = {"prompt": prompt})
                foo = True
            else:
                resp = requests.post("http://localhost:8000/chat", json = {"prompt": prompt})
                data = resp.json()
                timing_info = data["timing"]
                history = timing_info["history"]
                gen = timing_info["gen"]
                total = timing_info["total"]
                f.write(f"{prompt},{history},{gen},{total}\n")

        foo = False
        for prompt in long_chat_prompts:
            if not foo:
                resp = requests.post("http://localhost:8000/recall", json = {"prompt": prompt})
                foo = True
            else:
                resp = requests.post("http://localhost:8000/chat", json = {"prompt": prompt})
                data = resp.json()
                timing_info = data["timing"]
                history = timing_info["history"]
                gen = timing_info["gen"]
                total = timing_info["total"]
                f.write(f"{prompt},{history},{gen},{total}\n")

if __name__ == "__main__":
    main()
