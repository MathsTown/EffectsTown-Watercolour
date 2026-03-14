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
    function getLocalWorkerUrl(fileName){
        return new URL(fileName, self.location.href);
    }

    let offscreen;
    let ctx;
    let backBuffer;

    let isPreview=false;
    let seed = "";
    let renderIsPreview = false;
    let renderSeed = "";
    let renderParams = {};
    let previewWidth = 0;
    let previewHeight = 0;
    let activeJobKind = 'preview';
    let activeExportInfo = undefined;

    let workers = [];
    let workerCount = 0;
    let renderWorkerScript = getLocalWorkerUrl("main-render-worker-cpp.js");

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

    function supportsWasmSimd() {
        if (typeof WebAssembly !== "object" || typeof WebAssembly.validate !== "function") return false;
        const simdProbe = new Uint8Array([
            0x00,0x61,0x73,0x6d, 0x01,0x00,0x00,0x00,
            0x01,0x04,0x01,0x60,0x00,0x00,
            0x03,0x02,0x01,0x00,
            0x0a,0x17,0x01,0x15,0x00,
            0xfd,0x0c,
            0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
            0x1a,0x0b
        ]);
        return WebAssembly.validate(simdProbe);
    }

    const simdSupported = supportsWasmSimd();
    renderWorkerScript = simdSupported ? getLocalWorkerUrl("main-render-worker-cpp-simd.js") : getLocalWorkerUrl("main-render-worker-cpp.js");
    console.log("[www background] WebAssembly SIMD support: " + (simdSupported ? "enabled" : "not available") + "; loading " + renderWorkerScript);

    //Setup message handling.
    self.onmessage = handleMessageParent;

    //Start Render Workers
    for (let i=0; i<numberWorkers; i++){
        let worker = new Worker(renderWorkerScript, {name:'render'});
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
            'seed':renderSeed,
            'isPreview':renderIsPreview,
            'params': renderParams,
        };
        w.postMessage(msg);
        
    }

    /**************************************************************************************************
    * Javascript function:  Perform Render
    * ************************************************************************************************/
    function doRender(kind, exportInfo){
        //At least one worker needs to be started.
        if (workerCount == 0) {
           setTimeout(function(){
                doRender(kind, exportInfo);
           }, 20);
           return;
        }
        if (typeof offscreen === 'undefined') return; //Canvas not yet transferred.
        
        renderStartTime = performance.now();
        lastBufferSwap = performance.now();
        jobNumber++;
        lineNumber=0;
        linesRendered=0;
        renderSeed = seed;
        renderIsPreview = isPreview;
        renderParams = currentParams;
        activeJobKind = kind || 'preview';
        activeExportInfo = exportInfo;
        

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
    function resizeCanvas(width,height, kind, exportInfo){
        
        offscreen.width = width;
        offscreen.height = height;
        backBuffer = new ImageData(offscreen.width,offscreen.height);
        doRender(kind, exportInfo);
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
            previewWidth = offscreen.width;
            previewHeight = offscreen.height;
            
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

        if (msg.data.hasOwnProperty('exportImage')){
            if (activeJobKind === 'export') {
                postMessage({'exportFailed':true, 'message':'Save already in progress.'});
                return;
            }

            let exportWidth = Number(msg.data['width']);
            let exportHeight = Number(msg.data['height']);
            if (!Number.isFinite(exportWidth) || !Number.isFinite(exportHeight) || exportWidth <= 0 || exportHeight <= 0) {
                postMessage({'exportFailed':true, 'message':'Save failed: invalid export size.'});
                return;
            }

            if (msg.data.hasOwnProperty('previewWidth')) {
                let width = Number(msg.data['previewWidth']);
                if (Number.isFinite(width) && width > 0) previewWidth = width;
            }
            if (msg.data.hasOwnProperty('previewHeight')) {
                let height = Number(msg.data['previewHeight']);
                if (Number.isFinite(height) && height > 0) previewHeight = height;
            }
            if (msg.data.hasOwnProperty('seed')) seed = msg.data['seed'];
            if (msg.data.hasOwnProperty('params')) currentParams = msg.data['params'];
            isPreview = false;

            resizeCanvas(
                Math.round(exportWidth),
                Math.round(exportHeight),
                'export',
                {
                    filename: msg.data['filename'] || 'effects-town.png',
                    label: msg.data['label'] || 'image',
                }
            );
            return;
        }

        //Canvas needs to be resized to match page.
        if (msg.data.hasOwnProperty('resize')){
            let width = Number(msg.data.width);
            let height = Number(msg.data.height);
            if (Number.isFinite(width) && width > 0) previewWidth = Math.round(width);
            if (Number.isFinite(height) && height > 0) previewHeight = Math.round(height);
            if (activeJobKind === 'export') return;
            resizeCanvas(previewWidth, previewHeight);
            return;
        }

        if (activeJobKind === 'export') return;
        if (needsRender) doRender();

    }

    /**************************************************************************************************
    * Javascript function:
    * ************************************************************************************************/
    function onRenderComplete(){
        const timeTaken = performance.now() - renderStartTime;
        console.log("Render Complete: " + timeTaken.toFixed(1) + " ms (" + offscreen.width + " x " + offscreen.height  +" pixels)");

        if (activeJobKind !== 'export') return;

        let exportInfo = activeExportInfo || {};
        let filename = exportInfo.filename || 'effects-town.png';
        let completedWidth = offscreen.width;
        let completedHeight = offscreen.height;

        offscreen.convertToBlob({type:'image/png'}).then(function(blob){
            postMessage({
                'exportComplete': true,
                'blob': blob,
                'filename': filename,
                'width': completedWidth,
                'height': completedHeight,
            });
        }).catch(function(){
            postMessage({'exportFailed':true, 'message':'Save failed: unable to encode PNG.'});
        }).finally(function(){
            activeJobKind = 'preview';
            activeExportInfo = undefined;
            if (previewWidth > 0 && previewHeight > 0) {
                resizeCanvas(previewWidth, previewHeight);
            } else {
                doRender();
            }
        });
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


