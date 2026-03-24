#!/bin/bash
curl -X POST http://127.0.0.1:3490/post_body \
-H "Content-Type: application/json"  \
-H "Transfer-Encoding: chunked"   \
--data-binary @file_500mb.bin
