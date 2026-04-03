import socket
import time

HOST = "127.0.0.1"
PORT = 3490

test_cases = [
    # --- Line ending issues ---
    ("Only LF", "GET / HTTP/1.1\nHost: example.com\n\n"),
    ("Only CR", "GET / HTTP/1.1\rHost: example.com\r\r"),
    ("Mixed endings", "GET / HTTP/1.1\r\nHost: example.com\n\n"),

    # --- Structural issues ---
    ("Missing final CRLF", "GET / HTTP/1.1\r\nHost: example.com\r\n"),
    ("Missing colon", "GET / HTTP/1.1\r\nHost example.com\r\n\r\n"),
    ("Space before colon", "GET / HTTP/1.1\r\nHost : example.com\r\n\r\n"),

    # --- Request line issues ---
    ("Missing HTTP version", "GET /\r\nHost: example.com\r\n\r\n"),
    ("Invalid method", "G@T / HTTP/1.1\r\nHost: example.com\r\n\r\n"),
    ("Absolute URI", "GET http://evil.com/ HTTP/1.1\r\nHost: example.com\r\n\r\n"),

    # --- Header edge cases ---
    ("Duplicate Content-Length",
     "POST / HTTP/1.1\r\nHost: example.com\r\nContent-Length: 5\r\nContent-Length: 10\r\n\r\n12345"),

    ("Conflicting TE + CL",
     "POST / HTTP/1.1\r\nHost: example.com\r\nTransfer-Encoding: chunked\r\nContent-Length: 4\r\n\r\n4\r\nWiki\r\n0\r\n\r\n"),

    ("Header folding (obsolete)",
     "GET / HTTP/1.1\r\nHost: example.com\r\nX-Test: value\r\n more\r\n\r\n"),

    ("Very long header",
     "GET / HTTP/1.1\r\nHost: example.com\r\nX-Long: " + "A"*10000 + "\r\n\r\n"),

    ("Null byte in header",
     "GET / HTTP/1.1\r\nHost: example.com\x00\r\n\r\n"),

    # --- Chunked encoding issues ---
    ("Invalid chunk size",
     "POST / HTTP/1.1\r\nHost: example.com\r\nTransfer-Encoding: chunked\r\n\r\nZ\r\nHello\r\n0\r\n\r\n"),

    ("Missing final chunk",
     "POST / HTTP/1.1\r\nHost: example.com\r\nTransfer-Encoding: chunked\r\n\r\n4\r\nWiki\r\n"),

    ("Chunk size mismatch",
     "POST / HTTP/1.1\r\nHost: example.com\r\nTransfer-Encoding: chunked\r\n\r\n5\r\nWiki\r\n0\r\n\r\n"),

    # --- Smuggling attempts ---
    ("CL.TE smuggling",
     "POST / HTTP/1.1\r\nHost: example.com\r\nContent-Length: 13\r\nTransfer-Encoding: chunked\r\n\r\n0\r\n\r\nSMUGGLED"),

    ("TE.CL smuggling",
     "POST / HTTP/1.1\r\nHost: example.com\r\nTransfer-Encoding: chunked\r\nContent-Length: 13\r\n\r\n0\r\n\r\nSMUGGLED"),

    # --- Weird spacing ---
    ("Extra spaces",
     "GET    /    HTTP/1.1\r\nHost:    example.com\r\n\r\n"),

    ("Tab in header",
     "GET / HTTP/1.1\r\nHo\tst:\texample.com\r\n\r\n"),
     
    ("Two request",
     "GET / HTTP/1.1\r\nHost:\texample.com\r\n\r\nPOST /videos HTTP/1.1\r\nHost; ffff\r\n\r\n")
]


def classify_response(response_bytes):
    if not response_bytes:
        return "NO RESPONSE"

    try:
        text = response_bytes.decode(errors="ignore")
    except:
        return "INVALID RESPONSE"

    if "400" in text:
        return "REJECTED (400)"
    elif "200" in text:
        return "ACCEPTED (200)"
    elif "HTTP" in text:
        return "OTHER HTTP RESPONSE"
    else:
        return "UNKNOWN"


def run_test(name, payload):
    print(f"\n=== {name} ===")
    try:
        with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
            s.settimeout(100000)
            s.connect((HOST, PORT))

            s.sendall(payload.encode("utf-8"))

            try:
                response = s.recv(4096)
                result = classify_response(response)

                print(f"Result: {result}")
                print("Raw response:")
                print(response.decode(errors="replace"))

            except socket.timeout:
                print("Result: TIMEOUT")

    except Exception as e:
        print(f"Result: ERROR ({e})")


if __name__ == "__main__":
    summary = {}

    for name, payload in test_cases:
        run_test(name, payload)
        time.sleep(0.5)

    print("\n=== Done ===")