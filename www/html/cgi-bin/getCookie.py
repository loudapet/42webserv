#!/usr/bin/python3

import os
import sys
import urllib.parse
import http.cookies as Cookie

if os.environ.get('REQUEST_METHOD') == "GET":
	queryStr = os.environ.get('QUERY_STRING')
	if queryStr:
		lst = urllib.parse.parse_qsl(queryStr)
		key = lst[0][1]
		if 'HTTP_COOKIE' in os.environ:
			http_cookie = os.environ['HTTP_COOKIE']

			# Manually split the cookie string by semicolon and space
			cookie = Cookie.SimpleCookie()
			cookies_list = http_cookie.split('; ')
			for cookie_str in cookies_list:
				cookie.load(cookie_str)

			if key in cookie:
				print("Status: 200")
				print("Content-Type: text/html\n")
				print("<p>The value of cookie", key, "is", cookie[key].value, "</p>")
			else:
				print("Status: 200")
				print("Content-Type: text/html\n")
				print("<p>Cookie not found</p>")
		else:
			print("Status: 200")
			print("Content-Type: text/html\n")
			print("<p>No cookies found in the request</p>")
print('<a href="../cookie/cookie.html">Back to cookie page</a>')
