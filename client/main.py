import subprocess

def main():
    response = subprocess.run([
        "curl",
        "-s",
        "-X", "POST",
        "http://127.0.0.1:8000/read",
        "-H", "Content-Type: application/json",
        "-d", '{"text": "hello world"}'
        ], capture_output=True, text=True)

    print(response.stdout)

if __name__ == "__main__":
    main()
