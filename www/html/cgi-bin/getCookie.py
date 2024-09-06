#!/usr/bin/python3

import os
from http import cookies
import cgi 

form = cgi.FieldStorage() 

cookie = cookies.SimpleCookie()
key = form.getvalue('key')
receivedCookieString = os.environ.get('HTTP_COOKIE', '')

cookiesList = receivedCookieString.split(';')
for cookieInstance in cookiesList:
	cookie.load(cookieInstance.strip())

if key in cookie:
	print("Status: 200")
	print("Content-Type: text/html\n")
	print("<p>The value of cookie", key, "is", cookie[key].value, "</p>")
else:
	print("Status: 200")
	print("Content-Type: text/html\n")
	print("<p>Cookie not found</p>")
print('<a href="../cookie/cookie.html">Back to cookie page</a>')
