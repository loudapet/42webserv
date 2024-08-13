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
				print("HTTP/1.1 200 OK")
				print("Content-Type: text/plain\n")
				print("The value of Cookie", key, "is", cookie[key].value)
				print()
			else:
				print("HTTP/1.1 200 OK")
				print("Content-Type: text/plain\n")
				print("Cookie not found")
		else:
			print("HTTP/1.1 200 OK")
			print("Content-Type: text/plain\n")
			print("No cookies found in the request")
