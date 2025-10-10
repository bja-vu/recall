import requests

test_queries = [
    ("python lists", True),
    ("python lists", False),
    ("python list", False),
    ("lists in c", True),
    ("cpp lists", False),
    ("python dicts", True)
]

for prompt, expected_bool in test_queries:
    resp = requests.post("http://localhost:8000/recall", json={"prompt": prompt})
    gen_time = resp.json()['timing']['gen']
    expected = "Generate" if expected_bool else "Memoise"
    actual = "Memoise" if gen_time == 0 else "Generate"
    test_pass = "✓" if expected == actual else "✗"
    print(f"{prompt:20} | {gen_time:4}ms | {expected:8} | {actual:8} {test_pass}")
