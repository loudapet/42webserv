#!/usr/bin/python3

import cgi
import shelve
import uuid
import os
import time
from http.cookies import SimpleCookie

# Session timeout in seconds
SESSION_TIMEOUT = 20

class UserDatabase:
	def __init__(self, filename='sessions/user_database'):
		# Initialize the user database with the given filename
		self.filename = filename

	def add_user(self, username, password):
		# Add a new user to the database
		with shelve.open(self.filename, writeback=True) as db:
			db[username] = {'password': password}

	def get_user(self, username):
		# Retrieve user data from the database
		with shelve.open(self.filename) as db:
			return db.get(username)
	
	def get_password(self, username):
		# Retrieve the password for a given username
		with shelve.open(self.filename) as db:
			user = db.get(username)
			if user:
				return user['password']
			return None
	
	def user_exists(self, username):
		# Check if a user exists in the database
		with shelve.open(self.filename) as db:
			return username in db

class Session:
	def __init__(self, session_dir='sessions/'):
		# Initialize the session manager with the given session directory
		self.session_dir = session_dir
		if not os.path.exists(session_dir):
			# Create the session directory if it does not exist
			os.makedirs(session_dir)

	def create_session(self, username):
		# Generate a new session ID using UUID
		session_id = str(uuid.uuid4())
		# Define the session file path
		session_file = os.path.join(self.session_dir, "SID_" + session_id)
		# Write the username and current timestamp to the session file
		with open(session_file, 'w') as f:
			f.write(f"{username}\n{int(time.time())}")
		# Return the generated session ID
		return session_id
	
	def get_username(self, session_id):
		# Define the session file path
		session_file = os.path.join(self.session_dir, "SID_" + session_id)
		if os.path.exists(session_file):
			# If the session file exists, read the username from the first line
			with open(session_file, 'r') as f:
				return f.readline().strip()  # Read only the first line and strip any extra whitespace
		# Return None if the session file does not exist
		return None

	def delete_session(self, session_id):
		# Define the session file path
		session_file = os.path.join(self.session_dir, "SID_" + session_id)
		if os.path.exists(session_file):
			# If the session file exists, delete it
			os.remove(session_file)

	def is_session_expired(self, session_id, timeout=SESSION_TIMEOUT):
		# Define the session file path
		session_file = os.path.join(self.session_dir, "SID_" + session_id)
		if os.path.exists(session_file):
			# If the session file exists, check if it has expired
			with open(session_file, 'r') as f:
				f.readline()  # Skip the username line
				timestamp = int(f.readline().strip())  # Read the timestamp
				# Check if the current time exceeds the timestamp by the timeout duration
				if time.time() - timestamp > timeout:
					return True
		# Return False if the session is not expired
		return False

	def clean_expired_sessions(self, timeout=SESSION_TIMEOUT):
		# Iterate over all files in the session directory
		for session_file in os.listdir(self.session_dir):
			if session_file.startswith("SID_"):
				# Extract the session ID from the file name
				session_id = session_file[4:]
				# Delete the session file if it has expired
				if self.is_session_expired(session_id, timeout):
					self.delete_session(session_id)

def printHeaders():
	# Print HTTP headers
	print("Status: 200")
	print("Content-Type: text/html\n")

def printLoginPage():
	# Print the login page
	printHeaders()
	print("""
	<!DOCTYPE html>
	<html lang="en">
	<head>
		<meta charset="UTF-8">
		<meta name="viewport" content="width=device-width, initial-scale=1.0">
		<title>Login</title>
		<script>
			window.onload = function() {
				var xhr = new XMLHttpRequest();
				xhr.open("POST", "../cgi/manageSession.py", true);
				xhr.onreadystatechange = function() {
					if (xhr.readyState == 4 && xhr.status == 200) {
						console.log(xhr.responseText);
					}
				};
				xhr.send();
			};
		</script>
	</head>
	<body>
		<h2>Login</h2>
		<form id="loginForm" action="../cgi/manageSession.py" method="post" enctype="multipart/form-data" accept-charset="UTF-8">
			<div class="container"> 
				<input type="hidden" name="action" value="login">
				<label for="username">Username:</label><br>
				<input type="text" id="username" name="username" required><br><br>
				<label for="password">Password:</label><br>
				<input type="password" id="password" name="password" required><br><br>
				<button type="submit">Login</button>
			</div> 
		</form>
		<p>Don't have an account? <a href="../account/registration.html">Register here</a></p>
	</body>
	</html>
	""")

def printDashboardPage():
	# Print the dashboard page
	printHeaders()
	print("<h2>Dashboard</h2>")
	print(f"<p>Hi, {user_sessions.get_username(session_id)}! This is your dashboard.</p>")
	print('<p><a href="../">Return to homepage</a></p>')
	print('<p><a href="manageSession.py?action=logout">Logout here</a></p>')

def printWhereNextLogOrReg():
	# Print options for login or registration
	print('<p><a href="../account/login.html">Try to login again</a></p>')
	print('<p><a href="../account/registration.html">Go to registration</a></p>')

def printLogoutPage():
	# Print the logout page
	printHeaders()
	print("""
	<!DOCTYPE html>
	<html lang="en">
	<head>
		<meta charset="UTF-8">
		<meta name="viewport" content="width=device-width, initial-scale=1.0">
		<title>Logout</title>
	</head>
	<body>
		<h2>Logout</h2>
		<p>You have been logged out.</p>
		<p><a href="../account/login.html">Login again</a></p>
		<p><a href="../">Return to homepage</a></p>
	</body>
	</html>
	""")

# Initialize user database and session manager
user_db = UserDatabase()
user_sessions = Session()

# Clean up expired sessions - runs the checker on each execution of the script
user_sessions.clean_expired_sessions()

# Retrieve cookies and form data
cookie = SimpleCookie(os.environ.get("HTTP_COOKIE"))
session_id = cookie.get('SID')
form = cgi.FieldStorage()
username = form.getvalue("username")
password = form.getvalue("password")
action = form.getvalue("action")

if session_id:
	# If session ID exists, check if the session is expired
	session_id = session_id.value
	if user_sessions.is_session_expired(session_id):
		# If session is expired, delete it and show logout page
		user_sessions.delete_session(session_id)
		printLogoutPage()
	elif action == "logout":
		# If action is logout, delete session and show logout page
		user_sessions.delete_session(session_id)
		printLogoutPage()
	elif action == "account":
		# If user is already logged in and clicks on the Account dropdown, show a message indicating that and direct to Dashboard
		printHeaders()
		print("<h2>You're already logged in</h2>")
		print('<p><a href="manageSession.py">Find your Dashboard here</a></p>')
	else:
		# Otherwise, show the Dashboard page
		printDashboardPage()
else:
	if action == "dashboard":
		# If trying to access Dashboard without login, show unauthorized access message
		printHeaders()
		print("<h2>Unauthorized access</h2>")
		print('<p><a href="../account/login.html">Login here</a></p>')
	elif action == "register":
		# Handle user registration
		if user_db.user_exists(username):
			existing_password = user_db.get_password(username)
			printHeaders()
			print("<h2>User already exists</h2>")
			print(f"<p>User {username} already exists in the database</p>")
			print('<p><a href="../account/login.html">Login here</a></p>')
		else:
			user_db.add_user(username, password)
			printHeaders()
			print("<h2>Registration successful</h2>")
			print(f"<p>Username {username} has been created in the database.</p>")
			print('<p><a href="../account/login.html">Login here</a></p>')
	elif action == "login":
		# Handle user login
		if username is None and password is None:
			print("<h2>Error: Username and password are required!</h2>")
		else:
			if user_db.user_exists(username):
				existing_password = user_db.get_password(username)
				if existing_password == password:
					session_id = user_sessions.create_session(username)
					print(f"Set-Cookie: SID={session_id}; Path=/; Max-Age={SESSION_TIMEOUT}")
					printDashboardPage()
				else:
					printHeaders()
					print("<h2>Login Failed</h2>")
					print("<p>Incorrect password.</p>")
					printWhereNextLogOrReg()
			else:
				printHeaders()
				print("<h2>Login Failed</h2>")
				print("<p>User does not exist.</p>")
				printWhereNextLogOrReg()
	else:
		printLoginPage()
