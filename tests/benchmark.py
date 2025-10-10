import requests
import json

test_prompts = [
    "how to utilise dicts in python",
    "python list comprehension",
    "cpp pointer syntax",
    "semaphore vs mutex",
    "difference between stack and heap",
    "how to set up basic flask app with routes and templates",
    "explain observer pattern with code example",
    "compare async/await in python vs javascript with examples",
    "how to utilise llama.cpp as a low level api in a crow server utilising vector embedding via a python fastapi service with sbert"
]

chat_prompts = [
    "how do i read a file in python",
    "what if the file is very large",
    "how do i process it line by line",
    "can i keep a count of all words while reading",
    "how do i handle encoding errors",
    "what about csv files specifically",
    "how do i write the results of counting to a new file",
    "what if i need to do this for all files in a directory",
    "how do i check file permissions",
    "how do i handle if a file cannot be read",
    "can i read several files at once",
    "could i use multithreading for this",
    "can i combine the results from multiple files",
    "can i group the results of files based on file extension",
    "what if i need to handle nested directories",
    "can i track progress while processing",
    "what if the process is interrupted",
    "can i keep the progress"
]

def main():
    # iterating through test prompts
    # pulling the test results
    # storing them in a file (what kind?) csv
    with open("tests/results/recall_benchmark.csv", "w") as f:
        f.write("prompt,embed,search,gen,total\n")

        for prompt in test_prompts:
            print(f"prompt: {prompt}\n-----\n")
            resp = requests.post("http://localhost:8000/recall", json = {"prompt": prompt})
            data = resp.json()
            timing_info = data["timing"]
            embed = timing_info["embed"]
            search = timing_info["search"]
            gen = timing_info["gen"]
            total = timing_info["total"]
            f.write(f"{prompt},{embed},{search},{gen},{total}\n")
            print(f"response: {data["text"]}\n\n-----\n\n")

    with open("tests/results/chat_benchmark.csv", "w") as f:
        f.write("prompt,history,gen,total\n")

        foo = False
        for prompt in chat_prompts:
            print(f"prompt: {prompt}\n-----\n")
            if not foo:
                # not keeping the gen timings of this bc its annoying
                resp = requests.post("http://localhost:8000/recall", json = {"prompt": prompt})
                data = resp.json()
                foo = True
            else:
                resp = requests.post("http://localhost:8000/chat", json = {"prompt": prompt})
                data = resp.json()
                timing_info = data["timing"]
                history = timing_info["history"]
                gen = timing_info["gen"]
                total = timing_info["total"]
                f.write(f"{prompt},{history},{gen},{total}\n")
            print(f"response: {data["text"]}\n\n-----\n\n")

if __name__ == "__main__":
    main()
