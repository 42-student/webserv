#!/usr/bin/python3

import datetime

dt = datetime.datetime
time: str = dt.strftime(dt.now(), "%H:%M:%S")
response_body = [
    "HTTP/1.1 200 OK",
    "Content-type: text/html;charset=utf-8\n",
    "<html>",
    "<head>",
    "<title>Current Time</title>",
    "</head>",
    "<body>",
    f"<h1>{time}</h1>",
    "</body>",
    "</html>",
]
[print(i) for i in response_body]