### 1. Multiple listen (interface:port) tests
Goal: Verify your server binds correctly and routes requests to the right config block.

#### Tests
Start server → check both:
127.0.0.1:3490
127.0.0.5:8000

Requests:

```
curl http://127.0.0.1:3490/
curl http://127.0.0.5:8000/
```
#### Edge cases
Same server_name, different ports → correct config used?
Invalid port → does server fail gracefully?
Port already in use → proper error?

### 2. Default error pages

Goal: Ensure custom error pages are served.

Tests

Trigger errors:

curl -i http://127.0.0.1:3490/nonexistent   # 404
curl -i -X POST http://127.0.0.1:3490/      # if POST not allowed → 405
Check:
Correct HTTP status code
Correct file served:
/BadRequestPage.html
/html_errors/500ErrorPage.html
Edge cases
Error page file missing → fallback behavior?
Recursive error (error page itself fails)