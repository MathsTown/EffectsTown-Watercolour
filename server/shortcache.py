"""
Local dev server with a short cache time (60 seconds).
Allows 304 Not Modified responses — browser form state IS restored on soft refresh.
Usage (run from project root):  python server/shortcache.py
Then open: http://localhost:8080/effects/watercolour-texture/www/
"""
import http.server
import socketserver

PORT = 8080
DIRECTORY = "public_html"
CACHE_SECONDS = 60

class ShortCacheHandler(http.server.SimpleHTTPRequestHandler):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, directory=DIRECTORY, **kwargs)

    def end_headers(self):
        self.send_header("Cache-Control", f"max-age={CACHE_SECONDS}")
        super().end_headers()

    def log_message(self, format, *args):
        pass  # Suppress per-request noise

with socketserver.TCPServer(("", PORT), ShortCacheHandler) as httpd:
    print(f"Serving http://localhost:{PORT}/  ({CACHE_SECONDS}s cache, from ./{DIRECTORY}/)")
    print("Press Ctrl+C to stop.")
    httpd.serve_forever()
