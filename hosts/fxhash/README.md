# fxhash Host

Emscripten-based web host targeting the [fxhash](https://www.fxhash.xyz/) generative art platform.

## Testing locally

Run the dev server from the project root (required — browsers block WASM under `file://`):

```powershell
python server/nocache.py
```

Then open: `http://localhost:8080/effects/watercolour-texture/fxhash/`

See [server/README.md](../../server/README.md) for full server options.
