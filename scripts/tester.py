#!/usr/bin/env python3
import requests

# Colors
GREEN = "\033[32m"
RED = "\033[31m"
YELLOW = "\033[33m"
RESET = "\033[0m"

BASE_URL = "http://localhost:3490"

# Define tests as objects (Python dicts)
tests = [
    {
        "name": "Root index",
        "path": "/",
        "expected": 200,
        "contains": "It's root index"
    },
    {
        "name": "Existing file",
        "path": "/index.html",
        "expected": 200,
        "contains": "<!DOCTYPE html>"
    },
    {
        "name": "Non existing file",
        "path": "/no_file.html",
        "expected": 404
    },
    {
        "name": "Forbidden directory",
        "path": "/private/",
        "expected": 403
    },
    {
        "name": "CGI success",
        "path": "/cgi-bin/cgi.py",
        "expected": 200,
        "contains": "Stateless Python CGI"
    }
]

passed = 0
total = len(tests)

for t in tests:
    url = BASE_URL + t["path"] if t["path"].startswith("/") else BASE_URL + "/" + t["path"]
    try:
        r = requests.get(url)
        status_ok = r.status_code == t["expected"]
        content_ok = True
        if "contains" in t:
            content_ok = t["contains"] in r.text
    except requests.RequestException as e:
        status_ok = False
        content_ok = False
        r = None
        print(f"{RED}[ERROR]{RESET} {t['name']} - request failed: {e}")

    if status_ok and content_ok:
        print(f"{GREEN}[PASS]{RESET} {t['name']} (Status: {r.status_code if r else 'N/A'})")
        passed += 1
    else:
        print(f"{RED}[FAIL]{RESET} {t['name']}")
        print(f"  URL: {url}")
        print(f"  Expected Status: {t['expected']}, Got: {r.status_code if r else 'N/A'}")
        if "contains" in t:
            print(f"  Expected content: '{t['contains']}'")

print()
print(f"{YELLOW}Summary:{RESET} Passed {passed}/{total} tests")