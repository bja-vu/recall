import requests

def main():
    url = "http://127.0.0.1:8000/read"
    data = {"text": "hello world"}
    headers = {"Content-Type": "application/json"}

    response = requests.post(url, json=data, headers=headers)
    print(response.text)

if __name__ == "__main__":
    main()
