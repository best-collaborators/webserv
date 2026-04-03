#!/usr/bin/env python3

# ============================
# 0. PREVENT CGI BROKEN PIPE CRASH
# ============================

import signal
import sys
import os
import json
import cgi
import html
import tempfile
import io

from urllib.parse import parse_qs

def _silent_exit(signum, frame):
    os._exit(0)

signal.signal(signal.SIGPIPE, _silent_exit)

# Write raw bytes directly — avoids TextIOWrapper closed-file bugs
def out(text):
    try:
        sys.stdout.buffer.write(text.encode('utf-8'))
        sys.stdout.buffer.flush()
    except BrokenPipeError:
        sys.exit(0)

# ============================
# 1. ENVIRONMENT VARIABLES
# ============================

METHOD         = os.environ.get('REQUEST_METHOD', 'GET')
QUERY_STRING   = os.environ.get('QUERY_STRING', '')
CONTENT_TYPE   = os.environ.get('CONTENT_TYPE', '')
CONTENT_LENGTH = int(os.environ.get('CONTENT_LENGTH', '0') or '0')
SCRIPT_NAME    = os.environ.get('SCRIPT_NAME', '')
SERVER_NAME    = os.environ.get('SERVER_NAME', '')
SERVER_PORT    = os.environ.get('SERVER_PORT', '')

# print(f"CGI started: {METHOD} {SCRIPT_NAME}", file=sys.stderr)

# ============================
# 2. HELPERS
# ============================

def send_headers(content_type="text/html"):
    out(f"Content-Type: {content_type}\r\n\r\n")

def escape_html(value):
    return html.escape(str(value), quote=True)

# ============================
# 3. PARSE GET
# ============================

def parse_query_string(qs):
    """Flat dict, last value wins — matches Node querystring.parse behaviour."""
    parsed = parse_qs(qs, keep_blank_values=True)
    return {k: v[-1] for k, v in parsed.items()}

GET = parse_query_string(QUERY_STRING)

# ============================
# 4. READ STDIN
# ============================

def read_stdin():
    if METHOD == 'POST' and CONTENT_LENGTH > 0:
        raw = sys.stdin.buffer.read(CONTENT_LENGTH)
        if len(raw) > CONTENT_LENGTH:
            print("Payload exceeded declared size", file=sys.stderr)
            sys.exit(1)
        return raw
    return b''

# ============================
# 5 & 6. PARSE POST
# ============================

def parse_post(raw_body):
    if METHOD != 'POST':
        return {}

    if CONTENT_TYPE.startswith('application/x-www-form-urlencoded'):
        return parse_query_string(raw_body.decode('utf-8', errors='replace'))

    if CONTENT_TYPE.startswith('multipart/form-data'):
        return parse_multipart(raw_body)

    if CONTENT_TYPE.startswith('application/json'):
        try:
            return json.loads(raw_body.decode('utf-8', errors='replace'))
        except json.JSONDecodeError:
            return {}

    return {}

def parse_multipart(raw_body):
    """Parse multipart/form-data from already-read bytes."""
    result = {}

    environ = os.environ.copy()
    environ['CONTENT_LENGTH'] = str(len(raw_body))

    form = cgi.FieldStorage(
        fp=io.BytesIO(raw_body),
        environ=environ,
        keep_blank_values=True
    )

    for key in form.keys():
        field = form[key]
        if field.filename:
            safe_filename = os.path.basename(field.filename)
            upload_path = os.path.join(tempfile.gettempdir(), safe_filename)
            with open(upload_path, 'wb') as f:
                f.write(field.file.read())
            result[key] = {"filename": safe_filename, "savedTo": upload_path}
        else:
            result[key] = field.value

    return result

# ============================
# 7. MAIN EXECUTION
# ============================

def main():
    raw_body = read_stdin()
    body_str = raw_body.decode('utf-8', errors='replace')
    POST = parse_post(raw_body)

    if GET.get('format') == 'json':
        send_headers("application/json")
        out(json.dumps({
            "env": {
                "METHOD": METHOD,
                "SERVER_NAME": SERVER_NAME,
                "SERVER_PORT": SERVER_PORT,
            },
            "GET": GET,
            "POST": POST,
        }, indent=2))
        return

    send_headers("text/html")

    out("<html><body>")
    out("<h1>Stateless Python CGI</h1>")

    out("<h2>Environment</h2><pre>")
    out(escape_html(json.dumps({
        "METHOD": METHOD,
        "SCRIPT_NAME": SCRIPT_NAME,
        "SERVER_NAME": SERVER_NAME,
        "SERVER_PORT": SERVER_PORT,
    }, indent=2)))
    out("</pre>")

    out("<h2>GET</h2><pre>")
    out(escape_html(json.dumps(GET, indent=2)))
    out("</pre>")

    out("<h2>POST</h2><pre>")
    out(escape_html(json.dumps(POST, indent=2)))
    out("</pre>")

    out("<h2>BODY</h2><pre>")
    out(escape_html(body_str))
    out("</pre>")

    out("</body></html>")

if __name__ == '__main__':
    main()