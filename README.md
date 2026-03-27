<em>This project has been created as part of the 42 curriculum by rzvir, kvalerii</em>

## Description

**Webserv** is a custom-built HTTP server designed to deepen understanding of how the web works at a fundamental level. The project’s primary goal is to recreate the core functionality of a real-world web server - similar to Nginx or Apache HTTP Server, using low-level programming concepts, typically in C or C++.

The server handles HTTP requests from clients (such as web browsers), processes them according to defined configuration rules, and returns appropriate responses, including static files or dynamically generated content. It supports essential features such as request parsing, response generation, error handling, and configuration management.

Through this project, developers gain practical experience with networking, socket programming, concurrency, and protocol implementation.

## Instructions

To compile, install, and run **Webserv**, follow the steps below. These may vary slightly depending on your implementation, but the general workflow remains the same.

### Compilation
The project is typically compiled using a Makefile.

```bash
make
```

This command builds the executable (usually named `webserv`).

Clean compiled files with:

```bash
make clean
```

Remove all generated files:

```bash
make fclean
```

---

### Installation
No formal installation is required. Once compiled, the executable can be run directly from the project directory.

Make sure it has execution permissions:

```bash
chmod +x webserv
```

---

### Execution
Run the server by providing a configuration file:

```bash
./webserv config.conf
```

---

### Configuration
The configuration file defines server behavior, including:
- Port and host settings
- Server names
- Routes and locations
- Error pages
- Index files for both Location and Server block
- Root directories
- Allowed HTTP methods
- CGI configurations

### Configuration Format
The server uses a YAML-like configuration format. Below is a full example:

```yaml
server:
    listen:
        127.0.0.1:8000

    error_pages:
        400: /BadRequestPage.html

    root: data
    index: /another/index.html

    locations:
        - path: /
            allowed_methods: "GET"

        - path: /old
            allowed_methods: "GET"
            redirect: 301 /new

        - path: /new
            allowed_methods: "GET"
            autoindex: on

        - path: /upload_video
            max_body_size: 1000000000
            allowed_methods: "POST"
            index: /another/0-upload.txt

        - path: /delete_video
            allowed_methods: "DELETE"

        - path: /files
            allowed_methods: "GET"
            index: my_index.html
            root: /files_directory
			autoindex: on

    cgi:
        - pass_to: /usr/bin/node
            extensions: ".js"
```

---

### Usage
Once the server is running, access it via a browser:

```
http://localhost:8000
```

Or test using `curl`:

```bash
curl http://localhost:8000
```

---

### Requirements
- Unix-based system (Linux or macOS recommended)
- C/C++ compiler (e.g., `g++`)
- `make` utility

## Resources

### References
The following resources were used throughout the development of **Webserv**:

- RFC 7230–7235: Hypertext Transfer Protocol (HTTP/1.1)
- Nginx documentation: https://nginx.org/en/docs/
- Beej’s Guide to Network Programming: https://beej.us/guide/bgnet/
- MDN Web Docs (HTTP): https://developer.mozilla.org/en-US/docs/Web/HTTP
- C++ reference: https://en.cppreference.com/
- POSIX socket programming documentation

These materials provided essential knowledge about HTTP protocol behavior, server architecture, networking, and low-level system calls.

---

### Use of AI

AI tools (such as ChatGPT) were used as supportive resources during the project in the following ways:

- **Testing**: Generating edge cases and example HTTP requests to validate server behavior.
- **Documentation**: Assisting in writing and structuring project documentation.
- **Utility Functions**: Helping implement small helper functions and improve code readability.
- **Refactoring**: Suggesting cleaner, more maintainable code structures.
- **Learning & Architecture**: Explaining complex concepts related to server design, C++ patterns, and overall architecture.

AI was used strictly as an aid for understanding and productivity, while all core logic, design decisions, and implementations were developed and validated manually.


## Additional Sections

### Features

- HTTP/1.1 request handling
- Support for multiple server blocks
- Configurable routes and locations
- Static file serving
- File uploads handling
- Custom error pages
- CGI execution (e.g. with Node.js)
- Autoindex (directory listing)
- Method restriction (GET, POST, DELETE)
- Redirections

---

### Usage Examples

#### Basic request using a browser


#### Using curl
```bash
curl -X GET http://127.0.0.1:8000/
```

POST request with data

```bash
curl -X POST http://127.0.0.1:8000/post_body -d "hello world"
```

```bash
curl -X DELETE http://127.0.0.1:1111/post_body/file.txt
```

#### Technical Choices

- Language: C++
- Standard: C++17
- I/O Multiplexing: epoll()
- Architecture:
- Event-driven server design
- Non-blocking sockets
- Separation between parsing, routing, and response generation

Configuration Parsing:
- Custom parser for a YAML-like configuration format

Error Handling:
- HTTP-compliant status codes
- Custom error page support

### Project Structure
```text
.
├── data/ # Static files, assets, error pages, CGI scripts
│ ├── cgi-bin/
│ ├── html_errors/
│ └── YoupiBanane/
│
├── includes/ # Header files
│ ├── cgi/
│ ├── execution/
│ └── http/
│
├── sources/ # Source files (.cpp)
│ ├── cgi/
│ ├── execution/
│ └── http/
│
├── tests/ # Test cases (config, HTTP, CGI, etc.)
│ ├── config_tests/
│ ├── get/
│ ├── post/
│ └── malformed/
│
├── scripts/ # Helper scripts (testing, monitoring)
├── docs/ # Documentation (architecture, notes)
├── config/ # Configuration files
│ └── webserv.conf
│
├── Makefile
├── README.md
└── webserv # Compiled binary
```