# Dev Server

Minimal local HTTP servers for testing the web hosts during development.

## Why a local server is needed

Browsers enforce the Same-Origin Policy for WebAssembly and Web Workers — the web hosts
cannot be opened directly from the filesystem (`file://` URLs) and must be served over HTTP.

## Options

### `nocache.py` — no caching (recommended for development)

```powershell
python server/nocache.py
```

Sends `Cache-Control: no-store` on every response. The browser never caches anything, so
rebuilt `.js`/`.wasm` files are fetched immediately on the next page load.

**Side effect:** because no navigation cache entry is stored, browser form state is NOT
restored on soft refresh — controls always reset to their HTML defaults.

---

### `shortcache.py` — 60-second cache

```powershell
python server/shortcache.py
```

Sends `Cache-Control: max-age=60`, allowing normal 304 Not Modified responses. Useful for
testing behaviour closer to a real deployment (e.g. verifying that form state restoration
is handled correctly by the page's JavaScript).

**Side effect:** rebuilt files will not be picked up until the 60-second TTL expires (or
you do a hard refresh with Ctrl+Shift+R).

---

## URLs

| Host    | URL                                                        |
|---------|------------------------------------------------------------|
| www     | http://localhost:8080/effects/watercolour-texture/www/     |
| fxhash  | http://localhost:8080/effects/watercolour-texture/fxhash/  |

Press `Ctrl+C` to stop either server.
