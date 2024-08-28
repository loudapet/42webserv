#!/usr/bin/python3
import sys
#sys.stderr.write('Python running...\n')
import cgi
import cgitb
import os
import datetime
#sys.stderr.write('Python running...1\n')
cgitb.enable()
#sys.stderr.write('Python running...2\n')
form = cgi.FieldStorage()
#sys.stderr.write('Python running...3\n')
print("Content-type: text/html\r\n\r\n")
print("<html>")
print("<head>")
print("<title>File Upload Result</title>")
print('<style>')
print('    body { font-family: Arial, sans-serif; background-color: #f2f2f2; }')
print('    .result { width: 500px; margin: 0 auto; padding: 20px; \
                background-color: #fff; border: 1px solid #ccc; border-radius: 5px; \
                box-shadow: 0 0 10px rgba(0, 0, 0, 0.2); }')
print('    .error { color: red; }')
print('    h1 { color: navy; }')
print('    h3 { color: #333; }')
print('    .file-info { font-size: 16px; margin-top: 10px; }')
print('</style>')
print("</head>")
print("<body>")
#sys.stderr.write('Python running...4\n')
if "file" in form:
    file_item = form["file"]
    if file_item.filename:
        file_name = file_item.filename
        file_path = 'www/html/uploads/' + file_name
        #print(file_path, file=sys.stderr)
        with open(file_path, 'wb') as file:
            file.write(file_item.file.read())
        print(f"<h1>42webserv</h1>")
        print("<h3>CGI Script to Receive File from Form</h3>")
        print(f"<h2>File '{file_name}' uploaded successfully.</h2>")
        upload_datetime = datetime.datetime.now().strftime('%Y-%m-%d %H:%M:%S')
        print(f"<div class='file-info'>Uploaded on: {upload_datetime}</div>")
        local_path = os.path.abspath(file_path)
        print("<div class='file-info'>")
        print("Instructions to view the uploaded file locally on your drive:")
        print("<ul>")
        print("<li>Locate the uploaded file in the <a href=\"/uploads/\">/uploads/</a> folder.</li>")
        print("<li>Open the file using your preferred local application \
        (e.g., a text editor, image viewer, etc.).</li>")
        print("</ul>")
        print("</div>")
    else:
        print("<h1 class='error'>Error: No file was uploaded.</h1>")
else:
    print("<h1 class='error'>Error: No file field found in the form.</h1>")
#sys.stderr.write('Python running...5\n')
print("</body>")
print("</html>")
quit()