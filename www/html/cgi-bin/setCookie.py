#!/usr/bin/python3

from http import cookies
import time

# # Instantiate a SimpleCookie object
cookie = cookies.SimpleCookie()

# # The SimpleCookie instance is a mapping
cookie['lastvisit'] = str(time.time())

# # Output the HTTP message containing the cookie
print (cookie)
print ('Content-Type: text/html\r\n')
print ('<html><body>')
print ('Server time is', time.asctime(time.localtime()))
print ('</body></html>')

# import os
# from http import cookies
# # Import modules for CGI handling 
# import cgi, cgitb 

# # Create instance of FieldStorage 
# form = cgi.FieldStorage() 

# # Get data from fields
# key = form.getvalue('key')
# value  = form.getvalue('value')
# cookie = cookies.SimpleCookie()
# cookie[key] = value
# print("HTTP/1.1 204 OK")
# print(cookie.output())
# print("\r\n")