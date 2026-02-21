#!/usr/bin/env python3

import os
import sys
import json
import html
import tempfile
from urllib.parse import parse_qs

# ============================
# 1. ENVIRONMENT VARIABLES
# ============================

METHOD         = os.environ.get("REQUEST_METHOD", "GET")
QUERY_STRING   = os.environ.get("QUERY_STRING", "")
CONTENT_TYPE   = os.environ.get("CONTENT_TYPE", "")
CONTENT_LENGTH = int(os.environ.get("CONTENT_LENGTH", "0") or 0)

SCRIPT_NAME    = os.environ.get("SCRIPT_NAME", "")
SERVER_NAME    = os.environ.get("SERVER_NAME", "")
SERVER_PORT    = os.environ.get("SERVER_PORT", "")

print(f"CGI started: {METHOD} {SCRIPT_NAME}", file=sys.stderr)

# ============================
# 2. HELPERS
# ============================

def send_headers(content_type="text/html"):
    sys.stdout.write(f"Content-Type: {content_type}\r\n\r\n")

def escape_html(s):
    return html.escape(str(s))

def flatten(d):
    return {k: v[0] if len(v) == 1 else v for k, v in d.items()}

# ============================
# 3. PARSE GET
# ============================

GET = flatten(parse_qs(QUERY_STRING))

# ============================
# 4. READ RAW BODY (SAFE)
# ============================

raw_body = b""

if METHOD == "POST" and CONTENT_LENGTH > 0:
    raw_body = sys.stdin.buffer.read(CONTENT_LENGTH)

    if len(raw_body) > CONTENT_LENGTH:
        print("Payload exceeded declared size", file=sys.stderr)
        sys.exit(1)

# ============================
# 5. MULTIPART PARSER (MANUAL)
# ============================

def parse_multipart(body, content_type):
    result = {}

    # extract boundary
    parts = content_type.split("boundary=")
    if len(parts) != 2:
        return result

    boundary = parts[1]
    boundary_bytes = ("--" + boundary).encode()

    sections = body.split(boundary_bytes)

    for section in sections:
        if b"Content-Disposition" not in section:
            continue

        header_end = section.find(b"\r\n\r\n")
        if header_end == -1:
            continue

        headers = section[:header_end].decode(errors="ignore")
        value = section[header_end + 4:]

        value = value.rstrip(b"\r\n--")

        # extract name
        name = None
        filename = None

        for line in headers.split("\r\n"):
            if "Content-Disposition" in line:
                parts = line.split(";")
                for part in parts:
                    part = part.strip()
                    if part.startswith("name="):
                        name = part.split("=", 1)[1].strip('"')
                    if part.startswith("filename="):
                        filename = part.split("=", 1)[1].strip('"')

        if not name:
            continue

        if filename:
            safe_name = os.path.basename(filename)
            upload_path = os.path.join(tempfile.gettempdir(), safe_name)

            with open(upload_path, "wb") as f:
                f.write(value)

            result[name] = {
                "filename": safe_name,
                "savedTo": upload_path
            }
        else:
            result[name] = value.decode(errors="ignore")

    return result

# ============================
# 6. PARSE POST
# ============================

POST = {}

if METHOD == "POST":

    if CONTENT_TYPE.startswith("application/json"):
        try:
            POST = json.loads(raw_body.decode())
        except Exception:
            POST = {}

    elif CONTENT_TYPE.startswith("application/x-www-form-urlencoded"):
        POST = flatten(parse_qs(raw_body.decode()))

    elif CONTENT_TYPE.startswith("multipart/form-data"):
        POST = parse_multipart(raw_body, CONTENT_TYPE)

# ============================
# 7. JSON MODE
# ============================

if GET.get("format") == "json":
    send_headers("application/json")
    sys.stdout.write(json.dumps({
        "env": {
            "METHOD": METHOD,
            "SERVER_NAME": SERVER_NAME,
            "SERVER_PORT": SERVER_PORT
        },
        "GET": GET,
        "POST": POST
    }, indent=2))
    sys.exit(0)

# ============================
# 8. HTML OUTPUT
# ============================

send_headers("text/html")

sys.stdout.write("<html><body>")
sys.stdout.write("<h1>Stateless Python CGI (no cgi module)</h1>")

sys.stdout.write("<h2>Environment</h2><pre>")
sys.stdout.write(escape_html(json.dumps({
    "METHOD": METHOD,
    "SCRIPT_NAME": SCRIPT_NAME,
    "SERVER_NAME": SERVER_NAME,
    "SERVER_PORT": SERVER_PORT
}, indent=2)))
sys.stdout.write("</pre>")

sys.stdout.write("<h2>GET</h2><pre>")
sys.stdout.write(escape_html(json.dumps(GET, indent=2)))
sys.stdout.write("</pre>")

sys.stdout.write("<h2>POST</h2><pre>")
sys.stdout.write(escape_html(json.dumps(POST, indent=2)))
sys.stdout.write("</pre>")

sys.stdout.write("</body></html>")