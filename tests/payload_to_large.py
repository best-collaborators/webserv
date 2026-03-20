import requests

# Base server URL
BASE_URL = "http://127.0.0.1:3490"

# Test cases: each is a dictionary describing the request and expected behavior
TESTS = [
    {
        "path": "/",
        "method": "POST",
        "body_size": 101,  # scaled down: pretend this exceeds limit
        "expected": 413,
        "description": "Exceeding / max_body_size"
    },
    {
        "path": "/",
        "method": "POST",
        "body_size": 50,
        "expected": 200,
        "description": "Within / max_body_size"
    },
    {
        "path": "/post_body",
        "method": "POST",
        "body_size": 101,
        "expected": 413,
        "description": "Exceeding /post_body max_body_size"
    },
    {
        "path": "/videos",
        "method": "POST",
        "body_size": 10,
        "expected": 413,  # depends on server default max_body_size
        "description": "POST to /videos with no max_body_size defined"
    },
    {
        "path": "/cgi_test",
        "method": "POST",
        "body_size": 150,  # scaled down, simulates 1.5GB > 1GB limit
        "expected": 413,
        "description": "Exceeding CGI max_body_size"
    },
]

def run_test(test):
    url = f"{BASE_URL}{test['path']}"
    body = b"A" * test['body_size']  # generate dummy body
    try:
        if test["method"] == "POST":
            response = requests.post(url, data=body)
        elif test["method"] == "GET":
            response = requests.get(url)
        else:
            print(f"Skipping unsupported method {test['method']}")
            return
        status = response.status_code
        result = "PASS" if status == test["expected"] else "FAIL"
        print(f"[{result}] {test['description']}: got {status}, expected {test['expected']}")
    except Exception as e:
        print(f"[ERROR] {test['description']}: {e}")

if __name__ == "__main__":
    for t in TESTS:
        run_test(t)