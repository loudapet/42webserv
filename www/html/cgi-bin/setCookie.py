#!/usr/bin/python3
from http import cookies as Cookie
import os
import urllib.parse

if os.environ.get('REQUEST_METHOD') == "GET":
	cookieStr = os.environ.get('QUERY_STRING')
	if cookieStr:
		lst = urllib.parse.parse_qsl(cookieStr)
		key = lst[0][1]
		value = lst[1][1]
		cookie = Cookie.SimpleCookie()
		cookie[key] = value
		# print("HTTP/1.1 204 OK")
		# print(cookie.output())
		# print("\r\n")
		# # Instantiate a SimpleCookie object
		# cookie = cookies.SimpleCookie()

		# name/value pair
		# cookie['key'] = str(time.time())

		# expires in x seconds after the cookie is output.
		# the default is to expire when the browser is closed
		cookie[key]['expires'] = 60;

		# path in which the cookie is valid.
		# if set to '/' it will valid in the whole domain.
		# the default is the script's path.
		cookie[key]['path'] = '/'

		# the purpose of the cookie to be inspected by the user
		cookie[key]['comment'] = 'holds the last user\'s visit date'

		# domain in which the cookie is valid. always stars with a dot.
		# to make it available in all subdomains
		# specify only the domain like .my_site.com
		cookie[key]['domain'] = '.localhost'

		# discard in x seconds after the cookie is output
		# not supported in most browsers
		cookie[key]['max-age'] = 60;

		# secure has no value. If set directs the user agent to use
		# only (unspecified) secure means to contact the origin
		# server whenever it sends back this cookie
		cookie[key]['secure'] = ''

		# a decimal integer, identifies to which version of
		# the state management specification the cookie conforms.
		# cookie['lastvisit']['version'] = 1

		print("Status: 204")
		print(cookie.output())
		print("")
