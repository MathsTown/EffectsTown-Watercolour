#********************************************************************************************************
#
#Authors:		(c) 2022 Maths Town
#
#Licence:		The MIT License
#
#*********************************************************************************************************
#Permission is hereby granted, free of charge, to any person obtaining a copy of this software and
#associated documentation files (the "Software"), to deal in the Software without restriction, including
#without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
#copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the
#following conditions:

#The above copyright notice and this permission notice shall be included in all copies or substantial
#portions of the Software.

#THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT
#LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
#IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
#WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
#SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

all: watercolour-texture

builddir := .\build
htmldir := .\public_html


#headers used by renderer
common_depend = common\colour.h common\linear-algebra.h common\noise.h common\parameter-list.h common\input-transforms.h external\simd\include\simd-f32.h external\simd\include\simd-f64.h external\simd\include\simd-concepts.h external\simd\include\simd-uint32.h external\simd\include\simd-uint64.h

#===========================
#Watercolour texture project
#===========================
builddir_fxhash := $(builddir)\watercolour-texture\fxhash
htmldir_fxhash := $(htmldir)\effects\watercolour-texture\fxhash
builddir_www := $(builddir)\watercolour-texture\www
htmldir_www := $(htmldir)\effects\watercolour-texture\www
imagedir := $(htmldir)\images
project_dir := .\watercolour-texture
simd_include := .\external\simd\include

watercolour-texture: watercolour-texture-fxhash watercolour-texture-www

watercolour-texture-fxhash: $(htmldir_fxhash) $(builddir_fxhash) $(htmldir_fxhash)\index.html $(htmldir_fxhash)\main-cpp.js $(htmldir_fxhash)\main-background-cpp.js $(htmldir_fxhash)\main-render-worker-cpp.js $(htmldir_fxhash)\main-render-worker-cpp-simd.js 
#HTML
$(htmldir_fxhash)\index.html: hosts\fxhash\index.html
	copy /y hosts\fxhash\index.html $@

#Main Thread
$(builddir_fxhash)\main.o: hosts\fxhash\main.cpp hosts\fxhash\jsutil.h
	emcc hosts\fxhash\main.cpp -c -std=c++20  -o $@ -Oz -Wall -Wno-unknown-pragmas -Wpedantic -Wextra

$(builddir_fxhash)\ui.o: hosts\fxhash\ui.cpp hosts\fxhash\jsutil.h
	emcc hosts\fxhash\ui.cpp -c -std=c++20  -o $@ -Oz -Wall -Wno-unknown-pragmas -Wpedantic -Wextra

$(htmldir_fxhash)\main-cpp.js : $(builddir_fxhash)\ui.o $(builddir_fxhash)\main.o $(builddir_fxhash)\jsutil.o
	emcc $^ -o  $@ -lembind -O2 -std=c++20  -sENVIRONMENT=web --closure 1 

#Background Thread
$(builddir_fxhash)\main-background.o: hosts\fxhash\main-background.cpp hosts\fxhash\jsutil.h
	emcc hosts\fxhash\main-background.cpp -std=c++20 -c -o $@ -Oz -Wall -Wno-unknown-pragmas -Wpedantic -Wextra

$(htmldir_fxhash)\main-background-cpp.js : $(builddir_fxhash)\main-background.o $(builddir_fxhash)\jsutil.o
	emcc $^ -o  $@ -lembind -O2 -std=c++20  -sENVIRONMENT=web --closure 1 

#Render Worker Thread
$(htmldir_fxhash)\main-render-worker-cpp.js : $(builddir_fxhash)\main-render-worker.o $(builddir_fxhash)\jsutil.o $(builddir_fxhash)\parameters.o
	emcc $^ -o  $@ -lembind -O3 -std=c++20  -sENVIRONMENT=worker --closure 1 

$(builddir_fxhash)\main-render-worker.o: hosts\fxhash\main-render-worker.cpp watercolour-texture\renderer.h watercolour-texture\parameters.h watercolour-texture\parameter-id.h $(common_depend)
	emcc hosts\fxhash\main-render-worker.cpp -I$(project_dir) -I$(simd_include)  -std=c++20 -c -o $@ -O3 -Wall -Wno-unknown-pragmas -Wpedantic -Wextra

$(htmldir_fxhash)\main-render-worker-cpp-simd.js : $(builddir_fxhash)\main-render-worker-simd.o $(builddir_fxhash)\jsutil.o $(builddir_fxhash)\parameters.o
	emcc $^ -o  $@ -lembind -O3 -std=c++20 -msimd128 -sENVIRONMENT=worker --closure 1 

$(builddir_fxhash)\main-render-worker-simd.o: hosts\fxhash\main-render-worker.cpp watercolour-texture\renderer.h watercolour-texture\parameters.h watercolour-texture\parameter-id.h $(common_depend)
	emcc hosts\fxhash\main-render-worker.cpp -I$(project_dir) -I$(simd_include) -std=c++20 -c -o $@ -O3 -msimd128 -Wall -Wno-unknown-pragmas -Wpedantic -Wextra

$(builddir_fxhash)\parameters.o: watercolour-texture\parameters.h watercolour-texture\parameters.cpp watercolour-texture\parameter-id.h
	emcc watercolour-texture\parameters.cpp  -I$(project_dir) -I$(simd_include)  -std=c++20 -c -o $@ -O2 -Wall -Wno-unknown-pragmas -Wpedantic -Wextra

#Common
$(builddir_fxhash)\jsutil.o: hosts\fxhash\jsutil.cpp hosts\fxhash\jsutil.h 
	emcc hosts\fxhash\jsutil.cpp -c -std=c++20 -o $@  -Oz -Wall -Wno-unknown-pragmas  -Wextra

#Directories
$(htmldir_fxhash):
	mkdir $@
$(builddir_fxhash):
	mkdir $@


#WWW Host
watercolour-texture-www: $(htmldir_www) $(builddir_www) $(imagedir) $(htmldir_www)\index.html $(imagedir)\effectstowntitle.jpg $(htmldir_www)\main-cpp.js $(htmldir_www)\main-background-cpp.js $(htmldir_www)\main-render-worker-cpp.js $(htmldir_www)\main-render-worker-cpp-simd.js 
#HTML
$(htmldir_www)\index.html: hosts\www\index.html
	copy /y hosts\www\index.html $@

$(imagedir)\effectstowntitle.jpg: hosts\www\effectstowntitle.jpg
	copy /y hosts\www\effectstowntitle.jpg $@

#Main Thread
$(builddir_www)\main.o: hosts\www\main.cpp hosts\www\jsutil.h
	emcc hosts\www\main.cpp -c -std=c++20  -o $@ -Oz -Wall -Wno-unknown-pragmas -Wpedantic -Wextra

$(builddir_www)\ui.o: hosts\www\ui.cpp hosts\www\jsutil.h
	emcc hosts\www\ui.cpp -c -std=c++20  -o $@ -Oz -Wall -Wno-unknown-pragmas -Wpedantic -Wextra

$(htmldir_www)\main-cpp.js : $(builddir_www)\ui.o $(builddir_www)\main.o $(builddir_www)\jsutil.o
	emcc $^ -o  $@ -lembind -O2 -std=c++20  -sENVIRONMENT=web --closure 1 

#Background Thread
$(builddir_www)\main-background.o: hosts\www\main-background.cpp hosts\www\jsutil.h
	emcc hosts\www\main-background.cpp -std=c++20 -c -o $@ -Oz -Wall -Wno-unknown-pragmas -Wpedantic -Wextra

$(htmldir_www)\main-background-cpp.js : $(builddir_www)\main-background.o $(builddir_www)\jsutil.o
	emcc $^ -o  $@ -lembind -O2 -std=c++20  -sENVIRONMENT=web --closure 1 

#Render Worker Thread
$(htmldir_www)\main-render-worker-cpp.js : $(builddir_www)\main-render-worker.o $(builddir_www)\jsutil.o $(builddir_www)\parameters.o
	emcc $^ -o  $@ -lembind -O3 -std=c++20  -sENVIRONMENT=worker --closure 1 

$(builddir_www)\main-render-worker.o: hosts\www\main-render-worker.cpp watercolour-texture\renderer.h watercolour-texture\parameters.h watercolour-texture\parameter-id.h $(common_depend)
	emcc hosts\www\main-render-worker.cpp -I$(project_dir) -I$(simd_include)  -std=c++20 -c -o $@ -O3 -Wall -Wno-unknown-pragmas -Wpedantic -Wextra

$(htmldir_www)\main-render-worker-cpp-simd.js : $(builddir_www)\main-render-worker-simd.o $(builddir_www)\jsutil.o $(builddir_www)\parameters.o
	emcc $^ -o  $@ -lembind -O3 -std=c++20 -msimd128 -sENVIRONMENT=worker --closure 1 

$(builddir_www)\main-render-worker-simd.o: hosts\www\main-render-worker.cpp watercolour-texture\renderer.h watercolour-texture\parameters.h watercolour-texture\parameter-id.h $(common_depend)
	emcc hosts\www\main-render-worker.cpp -I$(project_dir) -I$(simd_include) -std=c++20 -c -o $@ -O3 -msimd128 -Wall -Wno-unknown-pragmas -Wpedantic -Wextra

$(builddir_www)\parameters.o: watercolour-texture\parameters.h watercolour-texture\parameters.cpp watercolour-texture\parameter-id.h
	emcc watercolour-texture\parameters.cpp  -I$(project_dir) -I$(simd_include)  -std=c++20 -c -o $@ -O2 -Wall -Wno-unknown-pragmas -Wpedantic -Wextra

#Common
$(builddir_www)\jsutil.o: hosts\www\jsutil.cpp hosts\www\jsutil.h 
	emcc hosts\www\jsutil.cpp -c -std=c++20 -o $@  -Oz -Wall -Wno-unknown-pragmas  -Wextra

#Directories
$(htmldir_www):
	mkdir $@
$(builddir_www):
	mkdir $@
$(imagedir):
	mkdir $@







clean:
	rmdir /s /q $(builddir)
