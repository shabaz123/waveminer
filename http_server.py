#!/usr/bin/python

# dspgen server mode
# rev 1 - April 2022 - shabaz
# Change PROGPATH to suit!
# API format:
# Example, set frequency to 1000 Hz:
#   http://192.168.1.88:8000/dspgen/f1000
# Example, set amplitude to 0.1 (-20 dB)
#   http://192.168.1.88:8000/dspgen/a0.1


import http.server
import socketserver
import os

PORT = 8000
PROGPATH="/home/pi/development/waveminer/"

print(f"Using port {PORT} and path {PROGPATH}")

class myRequestHandlerClass(http.server.SimpleHTTPRequestHandler):
    def do_POST(self):
        p = self.path
        print(f"path is '{p}'")
        torun = PROGPATH + "dspgen -" + p.removeprefix("/dspgen/")
        print(f"string is {torun}")
        if self.path.startswith('/dspgen/f'):
            os.system(torun)
        elif self.path.startswith('/dspgen/a'):
            os.system(torun)
        else:
            print(f"unknown request path!")

        self.send_response(200)
        self.send_header("Content-type", "text/html")
        self.end_headers()
        return

handler_object = myRequestHandlerClass

with socketserver.TCPServer(("", PORT), handler_object) as httpd:
    print("serving at port", PORT)
    httpd.serve_forever()


