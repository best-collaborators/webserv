siege -c 50 -t 1M https://example.com
-c 50 = 50 concurrent users
-t 1M = run for 1 minute

siege -c 20 -r 10 https://example.com
-r 10 = each user repeats 10 times


siege -c 50 -t 1M \
'http://localhost:8080/upload POST file=@test.bin'
