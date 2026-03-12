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
    Main Background Worker Thread Entry Point.  This thread manages rendering.
    (EMSCRIPTEN only)
    Called as a WebWorker    
    Owns the OffscreenCanvas.
    This threads starts a number of child "Render Workers" for the actual calculations.
*************************************************************************************************/

#include "jsutil.h"

#include <emscripten.h>
#include <emscripten/val.h>


/**************************************************************************************************
 * 
 * [Inline Javascript]
 * 
 * ************************************************************************************************/
EM_JS (emscripten::EM_VAL, setup_workers, (), {
    let offscreen;
    let ctx;
    let backBuffer;

    let isPreview=false;
    let seed = "";

    let workers = [];
    let workerCount = 0;

    let jobNumber = 0;        /// Current Job (update on resize etc).  Zero indicates not ready.
    let lineNumber = 0;       /// Next line to send to worker for rendering
    let linesRendered=0;      /// Number of lines completed by workers
    let renderStartTime = 0;  /// Timing variable
    let lastBufferSwap=0;     /// Timer variable used to rate limit updating of canvas.
    let currentParams = {};   /// Latest parameter values received from the GUI thread.

    let numberWorkers = Number(navigator.hardwareConcurrency) || 4;
    if (numberWorkers<1) numberWorkers=1;
    if (numberWorkers<4) numberWorkers=4;
    if (numberWorkers>64) numberWorkers=64;  //We start getting memory allocation issues above 80 or so.

    //Setup message handling.
    self.onmessage = handleMessageParent;

    //Start Render Workers
    for (let i=0; i<numberWorkers; i++){
        let worker = new Worker("main-render-worker-cpp.js", {name:'render'});
        worker.onmessage = handleMessageRender;       
    }

    //Send loaded message to UI Thread.
    postMessage({'loaded':true});  //Note: Using string indexes to avoid minification by closure compiler.

    /**************************************************************************************************
    * Javascript function:  
    * ************************************************************************************************/
    function startWorkerRender(w){
        
        if (lineNumber >= offscreen.height) return;
        
        //Note: Using string indexes to avoid minification by closure compiler.
        let msg = {
            'render':true,
            'jobNumber': jobNumber,
            'width': offscreen.width,
            'height': offscreen.height,
            'line':lineNumber++,
            'seed':seed,
            'isPreview':isPreview,
            'params': currentParams,
        };
        w.postMessage(msg);
        
    }

    /**************************************************************************************************
    * Javascript function:  Perform Render
    * ************************************************************************************************/
    function doRender(){
        //At least one worker needs to be started.
        if (workerCount == 0) {
           setTimeout(doRender, 20);
           return;
        }
        if (typeof offscreen === 'undefined') return; //Canvas not yet transferred.
        
        renderStartTime = performance.now();
        lastBufferSwap = performance.now();
        jobNumber++;
        lineNumber=0;
        linesRendered=0;
        

        for (let i=0; i< workerCount; i++){
            startWorkerRender(workers[i]);
        }

        //Clear backbuffer
        backBuffer.data.fill(0);
    }

    /**************************************************************************************************
    * Javascript function:  Resize event.
    * A resize event is sent via message from the GUI thread, but we need to resize canvas here.
    * ************************************************************************************************/
    function resizeCanvas(width,height){
        
        offscreen.width = width;
        offscreen.height = height;
        backBuffer = new ImageData(offscreen.width,offscreen.height);
        doRender();
    }

    /**************************************************************************************************
    * Javascript function: Handles messages from GUI thread (Parent thread)
    * ************************************************************************************************/
    function handleMessageParent(msg){                
        // We will be sent an offscreen canvas
        if(msg.data.hasOwnProperty('canvas')){  

            offscreen = msg.data.canvas;
            ctx = offscreen.getContext('2d');
            backBuffer = new ImageData(offscreen.width,offscreen.height);
            
            doRender();
            
            return;
        }

        if (msg.data.hasOwnProperty('shutdown')) {
            for (let i = 0; i < workers.length; i++) {
                workers[i].terminate();
            }
            workers = [];
            workerCount = 0;
            close();
            return;
        }

        let needsRender = false;

        if(msg.data.hasOwnProperty('seed')){
            seed = msg.data['seed'];
            isPreview = msg.data['isPreview'];
            needsRender = true;
        }

        if(msg.data.hasOwnProperty('params')){
            currentParams = msg.data['params'];
            needsRender = true;
        }

        // Don't process resize or render work if we don't have the canvas yet.
        if (typeof(offscreen) == "undefined" || typeof(ctx) == "undefined") return;

        //Canvas needs to be resized to match page.
        if (msg.data.hasOwnProperty('resize')){
            resizeCanvas(msg.data.width, msg.data.height);
            return;
        }

        if (needsRender) doRender();

    }

    /**************************************************************************************************
    * Javascript function:
    * ************************************************************************************************/
    function onRenderComplete(){
        const timeTaken = performance.now() - renderStartTime;
        console.log("Render Complete: " + timeTaken.toFixed(1) + " ms (" + offscreen.width + " x " + offscreen.height  +" pixels)");
    }

    /**************************************************************************************************
    * Javascript function:
    * ************************************************************************************************/
    function processRenderedLine(buf, line){
        linesRendered++;
        const u8view = new Uint8Array(buf);        
        const offset = backBuffer.width*4*line; 
        
        backBuffer.data.set(u8view,offset);

        //update offscreen canvas if a small amount of time has passed since last update or last couple of lines
        let now = performance.now();
        if ((linesRendered == offscreen.height) || ( now - lastBufferSwap > 200)){
            ctx.putImageData(backBuffer,0,0);
            lastBufferSwap = performance.now();   
        }

        if(linesRendered == offscreen.height) onRenderComplete();
    }

    /**************************************************************************************************
    * Javascript function: Handles messages from render workers (children of this thread)
    * ************************************************************************************************/
    function handleMessageRender(msg){   
        let w = msg.target;
        if (msg.data.hasOwnProperty('result')){            
            startWorkerRender(w);
            if (msg.data['jobNumber'] == jobNumber) processRenderedLine(msg.data['buffer'], msg.data['line']);
            return;
        }

        if(msg.data.hasOwnProperty('loaded')){
            if (workerCount == 0) postMessage({'paramDefs': msg.data['paramDefs']}); //Forward param defs from first worker to GUI.
            if (jobNumber>0) startWorkerRender(w); //If we have already started rendering we should initiate this thread.
            workers.push(w);
            workerCount++;
            if (workerCount == numberWorkers) console.log(workerCount + " workers started.");
            return;
        }
        
    }
   
});




/**************************************************************************************************
 * Main Entry Point for WebWorker
 * ************************************************************************************************/
int main(){     
    //js_console_log("Background Thread Started");
    setup_workers();

    return 0;
}


