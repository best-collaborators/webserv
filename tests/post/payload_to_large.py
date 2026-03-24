import http.client
import os
import random
import string
import threading

HOST = "127.0.0.1"
PORT = 3490
TEST_DIR = "tests/post/testfiles/"

# ---------- Helpers ----------

def make_file(filename, size):
    with open(path(filename), "wb") as f:
        f.write(os.urandom(size))

def make_text_file(filename, content):
    with open(path(filename), "w") as f:
        f.write(content)

def request(method, path, body=None, headers={}):
    conn = http.client.HTTPConnection(HOST, PORT, timeout=15)
    try:
        conn.request(method, path, body=body, headers=headers)
        res = conn.getresponse()
        data = res.read()
        return res.status, data
    except Exception as e:
        return "ERROR", str(e)
    finally:
        conn.close()

def path(filename):
    return os.path.join(TEST_DIR, filename)

def print_result(name, expected, actual):
    ok = (expected == actual)
    print(f"[{'OK' if ok else 'FAIL'}] {name}: expected={expected}, got={actual}")

# ---------- Tests ----------

def test_small_body():
    make_text_file("small.txt", "hello")
    with open(path("small.txt"), "rb") as f:
        status, _ = request("POST", "/post_body/", f.read())

    print_result("Small body", 201, status)

def test_exact_limit():
    make_text_file("exact.txt", "1234567890")  # 10 bytes
    with open(path("exact.txt"), "rb") as f:
        status, _ = request("POST", "/post_body/", f.read())
    print_result("Exact limit", 201, status)

def test_over_limit():
    make_text_file("too_big.txt", "12345678901")  # 11 bytes
    with open(path("too_big.txt"), "rb") as f:
        status, _ = request("POST", "/post_body/", f.read())
    print_result("Over limit", 413, status)

def test_large_random():
    make_file("big.bin", 2048)
    with open(path("big.bin"), "rb") as f:
        status, _ = request("POST", "/post_body/", f.read())
    print_result("Large random", 413, status)

def test_no_limit_location():
    make_file("big2.bin", 2048)
    with open(path("big2.bin"), "rb") as f:
        status, _ = request("POST", "/videos/", f.read())
    print_result("No limit location", 201, status)

def test_wrong_method():
    status, _ = request("GET", "/post_body/")
    print_result("Wrong method", 405, status)

def test_chunked():
    conn = http.client.HTTPConnection(HOST, PORT)
    try:
        conn.putrequest("POST", "/post_body/")
        conn.putheader("Transfer-Encoding", "chunked")
        conn.endheaders()

        # send chunks
        chunks = [b"a" * 600, b"a" * 600]
        for chunk in chunks:
            conn.send(hex(len(chunk))[2:].encode() + b"\r\n")
            conn.send(chunk + b"\r\n")
        conn.send(b"0\r\n\r\n")

        res = conn.getresponse()
        status = res.status
    except Exception as e:
        status = "ERROR"
    finally:
        conn.close()

    print_result("Chunked transfer", 413, status)

def test_content_length_mismatch():
    body = b"small"
    headers = {"Content-Length": "100000"}
    status, _ = request("POST", "/post_body/", body, headers)
    # Could be 400 or connection error
    ok = status in (400, "ERROR")
    print(f"[{'OK' if ok else 'FAIL'}] Content-Length mismatch: got={status}")

def test_empty_body():
    status, _ = request("POST", "/post_body/", b"")
    print_result("Empty body", 201, status)

def test_stress():
    def worker(results, i):
        body = b"a" * 2010
        status, _ = request("POST", "/post_body", body)
        results[i] = status

    threads = []
    results = {}

    for i in range(10):
        t = threading.Thread(target=worker, args=(results, i))
        threads.append(t)
        t.start()

    for t in threads:
        t.join()

    ok = all(status == 413 for status in results.values())
    print(f"[{'OK' if ok else 'FAIL'}] Stress test")

# ---------- Run All ----------

if __name__ == "__main__":
    print("⚠️ Make sure max_body_size is SMALL (e.g. 10 or 1000)\n")

    test_small_body()
    test_exact_limit()
    test_over_limit()
    test_large_random()
    test_no_limit_location()
    test_wrong_method()
    test_chunked()
    test_content_length_mismatch()
    test_empty_body()
    test_stress()

    print("\nDone.")