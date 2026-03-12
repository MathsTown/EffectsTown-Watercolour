# Watercolour Texture Plug-In
A watercolour texture plugin for After Effects and OpenFX.

Details: [https://www.effects.town/effects/watercolour-texture/](https://www.effects.town/effects/watercolour-texture/)

This is a simple plug-in I built test intergrating OpenFX and After Effects into the same source tree.  


## License:
My code may be used under the MIT License.

Code in the 3rd-party folder is subject to its own license.



## Compiling Information
Windows:
- Create a project folder (Build resources will end up here.)
- Create a folder "src" and download this repository into it.
- Compile the Windows Versions with Visual Studio 2022.  
- Actual outputs go directly to the relevent host folder.  (After Effects common)
- Use visual studio solution. (The make file is for the webhost)

Web Hosts:
- The Web Host is operational, and can be used to play with the effect in a browser.
- Compiles with the emscripten compiler.
- The makefile is for the www host. 
- For local testing of the `www` host:
  - Run `python server/nocache.py` from the project root.
  - Open `http://localhost:8080/effects/watercolour-texture/www/`
- See [hosts/www/README.md](hosts/www/README.md) and [server/README.md](server/README.md) for details.
