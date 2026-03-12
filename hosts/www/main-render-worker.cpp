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
    Main entry poiint for render workers threads.
    (EMSCRIPTEN only)
    Called as a WebWorker
******************************************************************************************************/


#include <emscripten.h>
#include <emscripten/val.h>
#include <emscripten/bind.h>

#include "../../watercolour-texture/parameters.h"
#include "simd-f32.h"

#include "../../watercolour-texture/renderer.h"
#include "../../common/colour.h"
#include "jsutil.h"

//We force the renderer to use the fallback type because we are in WASM.
thread_local Renderer<FallbackFloat32> renderer {};

/**************************************************************************************************
 * [Inline Javascript]
 * 
 * ************************************************************************************************/
EM_JS (bool, run_javascipt, (), {
    //console.log("Render Worker Loaded");

    //***Javascript function to render a line.  Called by handelMessage***
    function render_line(data){
        var buf = new ArrayBuffer(data['width']*4);
        var u32 = new Uint32Array(buf);        
        
        Module["setup_renderer"](data['width'], data['height']);
        Module["set_seed"](data['seed']);
        //console.log(data['seed']);
        for (let x=0; x< data['width']; x++){
            u32[x] = Module["render_pixel"](x, data['line']);
        }        
    
        postMessage({'result':true, 'buffer':buf, 'jobNumber': data['jobNumber'], 'line':data['line']},[buf]);
    }
    
    //*** Handel incoming messages ***
    function handelMessage(msg){ 
        if(msg.data.hasOwnProperty('render')){  
            //console.log("Render Request Line : ", msg.data.line);
            render_line(msg.data);
            return;
        }        
    }

    self.onmessage = handelMessage;

    //Send loaded message to parent thread.
    postMessage({'loaded':true}); //Note: Using string indexes to avoid minification by closure compiler.
    return true;
})

/**************************************************************************************************
* 
* Exported to JS
*************************************************************************************************/
void setup_renderer(uint32_t width, uint32_t height){
    auto params = build_project_parameters();

    renderer.set_size(width,height);    
    renderer.set_parameters(params);
}

/**************************************************************************************************
* 
* Exported to JS
*************************************************************************************************/
void set_seed(const std::string str){
    renderer.set_seed(str);
}

/**************************************************************************************************
 * Render Pixel
 * Exported to JS
 * ************************************************************************************************/
uint32_t render_pixel(uint32_t x, uint32_t y){
    if (renderer.get_width() <= 0 || renderer.get_height() <=0) return 0xff0000ff; //Return red if caller has not set size.
    

    auto c = renderer.render_pixel( x,y);
    //if (y==0) js_console_log(std::to_string(x) +  " " + std::to_string(c.red.v) + " " + std::to_string(c.green.v) + " " +  std::to_string(c.blue.v) );

    return c.to_colour8().to_uint32_keep_memory_layout();

}


/**************************************************************************************************
 * Main Entry Point for WebWorker
 * ************************************************************************************************/
int main(){     
    
    
    run_javascipt();

   

    return 0;
}
using namespace emscripten;
EMSCRIPTEN_BINDINGS(main_render_worker){
    function("render_pixel", &render_pixel);
    function("setup_renderer", &setup_renderer); 
    function("set_seed", &set_seed); 
}
