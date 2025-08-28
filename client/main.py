import requests

def main():
    url = "http://127.0.0.1:8000/read"
    data = {"text": "What is the capital of Portugal?"}
    headers = {"Content-Type": "application/json"}

    response = requests.post(url, json=data, headers=headers)
    print(response.json()["response"])

if __name__ == "__main__":
    main()

