#!/usr/bin/env python

import hashlib
import time
import http.cookies as Cookie
import os
import shelve

cookie = Cookie.SimpleCookie()
string_cookie = os.environ.get('HTTP_COOKIE')

# If new session
if not string_cookie:
    # The sid will be a hash of the server time
    sid = hashlib.sha1(repr(time.time()).encode()).hexdigest()
    # Set the sid in the cookie
    cookie['sid'] = sid
    # Will expire in a year
    cookie['sid']['expires'] = 30
    # Set message to print out
    message = 'New session'
# If already existent session
else:
    cookie.load(string_cookie)
    sid = cookie['sid'].value

#print (cookie)
#print ('Content-Type: text/html\n')
#print ('<html><body>')
#
#if string_cookie:
#    print ('<p>Already existent session</p>')
#else:
#    print ('<p>New session</p>')
#
#print ('<p>SID =', sid, '</p>')
#print ('</body></html>')

# The shelve module will persist the session data
# and expose it as a dictionary
session_dir = 'www/html/cgi-bin/.sessions'
session = shelve.open('%s/sess_%s' % (session_dir, sid), writeback=True)

# Retrieve last visit time from the session
lastvisit = session.get('lastvisit')
if lastvisit:
	# Set message to print out
    message = 'Welcome back. Your last visit was at ' + \
      time.asctime(time.gmtime(float(lastvisit)))
# Save the current time in the session
session['lastvisit'] = repr(time.time())

print ("""\
%s
Content-Type: text/html\n
<html><body>
<p>%s</p>
<p>SID = %s</p>
</body></html>
""" % (cookie, message, sid))
