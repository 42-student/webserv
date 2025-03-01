#!/usr/bin/python3

import cgi

form = cgi.FieldStorage()

if "varOne" in form:
    var_one = form.getvalue("varOne")
else:
    var_one = " "

if "varTwo" in form:
    var_two = form.getvalue("varTwo")
else:
    var_two = " "

var_one = form.getvalue("varOne")
var_two = form.getvalue("varTwo")

response_body = [
    "HTTP/1.1 200 OK",
    "Content-type: text/html;charset=utf-8\n",
    "<html>",
    "<head>",
    "<title>Welcome</title>",
    '<script src="https://cdn.tailwindcss.com"></script>',
    "</head>",
    '<body class="bg-black h-screen w-screen flex justify-center items-center">',
    '<div class="flex flex-col space-y-7 flex justify-center items-center p-10">',
    '<div class="flex flex-col justify-center items-center space-y-1">',
    f'<h1 class="font-bold text-2xl text-gray-300">First variable: {var_one}</h1>',
    f'<p class="font-bold text-2xl text-gray-300">Second variable: {var_two}</p>',
    "</div>",
    '<div class="flex bg-gradient-to-r from-rose-500 to-emerald-600 p-1 rounded-lg hover:scale-105 transition-all duration-300">',
    '<a href="../get.html" class="px-7 bg-black py-3 font-bold text-center rounded-md text-slate-100">Go Back</a>',
    "</div>",
    "</div>",
    "</body>",
    "</html>",
]
[print(i) for i in response_body]
