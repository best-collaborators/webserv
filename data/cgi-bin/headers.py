#!/usr/bin/env python3
# cgi-bin/show_headers.py
# chmod +x show_headers.py

import os
import html  # for escaping HTML special characters

# 1️⃣ Print the HTTP header with proper CRLF terminator
print("Content-Type: text/html\r\n\r\n")  # ⚠️ Must be first

# 2️⃣ Start HTML document
print("<!DOCTYPE html>")
print("<html lang='en'>")
print("<head><meta charset='UTF-8'><title>CGI Headers</title></head>")
print("<body>")
print("<h1>CGI Environment Variables</h1>")
print("<table border='1' cellpadding='4' cellspacing='0'>")
print("<tr><th>Variable</th><th>Value</th></tr>")

# 3️⃣ Print all CGI environment variables
for key in sorted(os.environ.keys()):
    value = html.escape(os.environ[key])
    print(f"<tr><td>{key}</td><td>{value}</td></tr>")

print("</table>")

# 4️⃣ Print HTTP headers separately
print("<h1>HTTP Headers</h1>")
print("<table border='1' cellpadding='4' cellspacing='0'>")
print("<tr><th>Header</th><th>Value</th></tr>")

for key, value in sorted(os.environ.items()):
    if key.startswith("HTTP_"):
        header_name = key[5:].replace("_", "-").title()
        header_value = html.escape(value)
        print(f"<tr><td>{header_name}</td><td>{header_value}</td></tr>")

# 5️⃣ End HTML document
print("</table>")
print("</body></html>")