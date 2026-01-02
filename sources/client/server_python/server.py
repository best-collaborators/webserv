from http.server import HTTPServer, BaseHTTPRequestHandler
import cgi
import io
import datetime

taskList = ['Task 1', 'Task 2', 'Task 3']

def logger(ip, method, request_target, request_version, status_code, body, headers):
	logs = ''
	logs += (
		f"======= {ip}\t"
		f"[{datetime.datetime.now().strftime('%H:%M:%S %x')}]\t"
		f"\"{method} {request_target} {request_version}\"\t"
		f"{status_code} =======\n\n"
	)

	for head in headers:
		logs += (
			f'{head}: {headers.get(head)}\n'
		)
	
	if body != "":
		logs += (
			''
			'\nRaw body: \n'
			f'{body}\n'
			f'\nDecoded body:\n'
			f'{body.decode("utf-8", errors="replace")}\n'
		)

	with open("server.log.txt", 'a') as f:
		f.write(logs)


class helloHandler(BaseHTTPRequestHandler):
	def do_GET(self):
		if self.path.endswith("/list"):
			logger(self.client_address[0], self.command, self.path, self.request_version, 200, "", self.headers)
			
			self.send_response(200)
			self.send_header('content-type', 'text/html')
			self.end_headers()

			output = ''
			output += '<html><body>'
			output += '<h1>List</h1>'
			output += '<h3><a href="/list/new"> Add New </a></h3>'
			for task in taskList:
				output += task
				output += '</br>'
			output += '</body></html>'
			self.wfile.write(output.encode())

		if self.path.endswith("/new"):
			logger(self.client_address[0], self.command, self.path, self.request_version, 200, "", self.headers)

			self.send_response(200)
			self.send_header('content-type', 'text/html')
			self.end_headers()

			output = ''
			output += '<html><body>'
			output += '<h1>Add new</h1>'

			output += '<form method="POST" enctype="multipart/form-data" action="/tasklist/new">'
			output += '<input name="task" type="text" placeholder="Add new task">'
			output += '<input type="submit" value="Add">'
			output += '</form>'
			output += '</body></html>'

			self.wfile.write(output.encode())

	def do_POST(self):
		if self.path.endswith("/new"):
			ctype, pdict = cgi.parse_header(self.headers.get('content-type'))


			if ctype == 'multipart/form-data':

				pdict['boundary'] =  bytes(pdict['boundary'], "utf-8")
				content_len = int(self.headers.get('Content-Length'))
				pdict['CONTENT-LENGTH'] = content_len
	
				body = self.rfile.read(content_len)
				logger(self.client_address[0], self.command, self.path, self.request_version, 301, body, self.headers)

				fields = cgi.parse_multipart(io.BytesIO(body), pdict)
				new_task = fields.get('task')
				taskList.append(new_task[0])

			self.send_response(301)
			self.send_header('Content-type', 'text/html')
			self.send_header('Location', '/list')
			self.end_headers()


def main():
	PORT = 8000
	server = HTTPServer(('', PORT), helloHandler)
	print("Server is running on port %s" % PORT)
	server.serve_forever()

if __name__ == '__main__':
	main()