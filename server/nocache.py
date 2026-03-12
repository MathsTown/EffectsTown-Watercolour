"""
Local dev server with caching disabled.
Usage (run from project root):  python server/nocache.py
Then open: http://localhost:8080/effects/watercolour-texture/www/
"""
import http.server
import socketserver

PORT = 8080
DIRECTORY = "public_html"

class NoCacheHandler(http.server.SimpleHTTPRequestHandler):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, directory=DIRECTORY, **kwargs)

    def end_headers(self):
        self.send_header("Cache-Control", "no-store, no-cache, must-revalidate")
        self.send_header("Pragma", "no-cache")
        self.send_header("Expires", "0")
        super().end_headers()

    def log_message(self, format, *args):
        pass  # Suppress per-request noise

with socketserver.TCPServer(("", PORT), NoCacheHandler) as httpd:
    print(f"Serving http://localhost:{PORT}/  (no-cache, from ./{DIRECTORY}/)")
    print("Press Ctrl+C to stop.")
    httpd.serve_forever()
