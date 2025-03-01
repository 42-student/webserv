#! /usr/bin/python3

from http import cookies
import cgi, cgitb 

uploaded_cookies = cgi.FieldStorage()

key = uploaded_cookies.getvalue('key')
value  = uploaded_cookies.getvalue('value')
cookie = cookies.SimpleCookie()
cookie[key] = value
print("HTTP/1.1 204 OK")
print(cookie.output())
