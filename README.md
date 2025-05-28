[![progress-banner](https://backend.codecrafters.io/progress/http-server/4747f299-a430-45ec-86ca-db1e654e2f94)](https://app.codecrafters.io/users/codecrafters-bot?r=2qF)

# Minimal HTTP Server in C

Welcome to my custom-built HTTP server developed in pure C! This project was created as part of a systems-level networking assignment to explore how HTTP servers function under the hood. It demonstrates low-level socket programming, request parsing, and simple HTTP response handling — all without relying on high-level web frameworks.

# 🚀 Features Implemented

✅ Request Parsing – Efficiently parses raw HTTP requests to extract the method, path, headers, and body.
✅ Endpoint Routing – Supports multiple endpoints with specific behavior:

`/` – Returns a simple success response.

`/user-agent` – Extracts and responds with the User-Agent header from the request.

`/echo/<message>` – Responds with the message provided in the URL.

`/files/<filename>` – Serves files from a user-defined directory (GET) and allows file creation via POST.

✅ Content-Encoding Detection – Detects if the client accepts gzip encoding and adjusts the response headers accordingly.
✅ Custom File Directory Support – Accepts a `--directory` command-line argument to serve and store files in a specified location.
✅ Basic Error Handling – Sends appropriate 404 Not Found or 500 Internal Server Error responses when needed.
✅ HTTP Response Templates – Dynamically builds responses with correct headers and content-lengths.

# 🔧 Tech Stack

Language: C
Networking: POSIX sockets (TCP)
Compilation: gcc
OS: Built and tested on Linux

# 🧪 Usage

```c
gcc -o http_server server.c
./http_server --directory ./files
```

Use curl or any HTTP client to test the endpoints.

# 📂 Example Requests

```bash
curl http://localhost:4221/
curl http://localhost:4221/user-agent
curl http://localhost:4221/echo/hello-world
curl -X POST --data "some content" http://localhost:4221/files/test.txt
curl http://localhost:4221/files/test.txt
```

# 📌 Note

This project is built for educational purposes to understand the internals of an HTTP server. It's minimal, synchronous, and handles one request at a time — perfect for grasping the foundational ideas of networking and HTTP
