/********************************************************************************************************

Authors:		(c) 2022 Maths Town

Licence:		The MIT License

*********************************************************************************************************
Permission is hereby granted, free of charge, to any person obtaining a copy of this software and
associated documentation files (the "Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the
following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial
portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT
LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
********************************************************************************************************

Description:
    Main Entry point. Runs on GUI thread.
    (EMSCRIPTEN only)     
    Host actually consists of 3 seperate wasm programs which run on different threads. (GUI, Background, Workers)
    No shared memory objects are used on the WWW host. Everything is managed by messages. 
*******************************************************************************************************/
#include "jsutil.h"
#include "ui.h"

#include <emscripten.h>

/**************************************************************************************************
 * [Inline Javascript]
 * Checks if the browser supports the required function.  Displays an error message if not.
 * Returns true/false to indicate support
 * ************************************************************************************************/
EM_JS (bool, check_browser_support, (), {
    if (window.Worker && typeof OffscreenCanvas !== 'undefined') return true;
    document.body.innerHTML = "<div style='margin-top:50px;width:100%;text-align:center;'><h3>Unsupported Browser</h3><br><br>The features required by this application are not all supported by your browser.  (WebWorkers, OffscreenCanvas).<br> The minimum supported browser versions are listed below<br><br>Chrome 69<br>Firefox 105<br>Edge 79<br>Opera 56<br><br></div>";
    return false;
});

/**************************************************************************************************
 * [Inline Javascript]
 * ************************************************************************************************/
EM_JS (bool, setLoaded, (), {
    globalThis['cppLoaded'] = true;
    return true;
});


/**************************************************************************************************
 * Main Entry Point
 * ************************************************************************************************/
int main(){    
    setLoaded();
    if (!check_browser_support()) return 0;
    start_ui();
    return 0;
}















