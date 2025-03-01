#! /usr/bin/python3

import os
from http import cookies

import cgi

uploaded_cookies = cgi.FieldStorage()
key = uploaded_cookies.getvalue("key")
value = uploaded_cookies.getvalue("value")
response_body = ["HTTP1.1 200 OK\n", "Content-type: text/html;charset=utf-8\n"]

cookie = cookies.SimpleCookie()
if "HTTP_COOKIE" in os.environ:
    cookie.load(os.environ["HTTP_COOKIE"])

if key in cookie:
    value = cookie[key].value
else:
    value = "Not defined"

response_body = [
    "HTTP/1.1 200 OK",
    "Content-type: text/html;charset=utf-8\n",
    "<html>",
    "<head>",
    "<title>Value of cookie</title>",
    '<script src="https://cdn.tailwindcss.com"></script>',
    "</head>",
    '<body class="bg-slate-900 h-screen w-screen flex justify-center items-center text-slate-100">',
    '<div class="flex flex-col space-y-7 flex justify-center items-center p-10">',
    '<div class="flex flex-col justify-center items-center space-y-3">',
    f'<h1 class="font-bold text-3xl text-slate-400">The value of cookie "{key}"</span> is:</h1>',
    f'<code class="font-bold text-sm bg-slate-800 text-emerald-400 p-3 rounded-md md:text-xl">{value}</code>',
    "</div>",
    '<a href="../cookies.html"class="py-2 px-7 text-center ring-2 ring-emerald-500 rounded-md transition-all duration-300">Back to cookies</a>',
    "</div>",
    "</body>",
    "</html>",
]
[print(i) for i in response_body]
