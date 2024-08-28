#!/usr/bin/python3
from http import cookies
import cgi

form = cgi.FieldStorage() 

key = form.getvalue('key')
value  = form.getvalue('value')
cookie = cookies.SimpleCookie()
cookie[key] = value
cookie[key]['expires'] = 60;
# path in which the cookie is valid.
# if set to '/' it will valid in the whole domain.
# the default is the script's path.
cookie[key]['path'] = '/'
# the purpose of the cookie to be inspected by the user
cookie[key]['comment'] = 'our custom cookie, lasts for one minute'
# domain in which the cookie is valid. always stars with a dot.
# to make it available in all subdomains
# specify only the domain like .my_site.com
cookie[key]['domain'] = '.localhost'
# discard in x seconds after the cookie is output
# not supported in most browsers
cookie[key]['max-age'] = 60
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