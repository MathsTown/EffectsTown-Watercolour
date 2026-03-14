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
    WWW Host.
    Manages user iterface, and iterfacing with javascript.
    (EMSCRIPTEN only)
    Runs on main GUI thread.
********************************************************************************************************/
#include "jsutil.h"

#include <emscripten.h>
#include <emscripten/bind.h>


using namespace emscripten;

emscripten::val main_worker {};


/*************************************Worker Mangagement*******************************************/

/**************************************************************************************************
 * [Inline Javascript]
 * javascript_run_ui
 *  
 * Javascript for the UI.  
 * A background worker is started and control of rendering is handed over to it.
 * Browser compatability checks should already be done.
 * ************************************************************************************************/
EM_JS (bool, javascript_run_ui, (), {
    let seedInputEl = document.getElementById('seed-input');
    let exportStatusEl = document.getElementById('save-image-status');
    let exportButtons = {
        '4k': document.getElementById('save-image-4k'),
        '8k': document.getElementById('save-image-8k'),
        '16k': document.getElementById('save-image-16k'),
    };
    let exportLongEdges = {
        '4k': 3840,
        '8k': 7680,
        '16k': 15360,
    };
    let seed = (seedInputEl && seedInputEl.value) ? seedInputEl.value : "Effects Town";
    console.log("Seed: " + seed);
    let isShuttingDown = false;
    let backgroundLoaded = false;
    let paramsLoaded = false;
    let exportInProgress = false;
    let flushPending = false;
    let dirtySeed = false;
    let dirtyParams = false;
    let dirtyResize = false;
    let backgroundLoadTimeout = 0;
    let paramsLoadTimeout = 0;
    
    //Start background worker.  We will eventually hand over the canvas to the background worker.
    let worker;
    if (window.Worker){
        worker = new Worker("main-background-cpp.js", {name:'background'});
        worker.onmessage = handleMessage;
        
    }else{
        alert("Web Workers are not supported."); //Should be unreachable, as we checked earlier
    }

    /**************************************************************************************************
    * Javascript function:
    * ************************************************************************************************/
    function showStartupError(message){
        let container = document.getElementById('parameters');
        let node = document.getElementById('startup-status');
        if (!node){
            node = document.createElement('div');
            node.id = 'startup-status';
            node.className = 'menugroup';
            container.appendChild(node);
        }
        node.style.color = '#a00000';
        node.textContent = message;
    }

    /**************************************************************************************************
    * Javascript function:
    * ************************************************************************************************/
    function clearStartupError(){
        let node = document.getElementById('startup-status');
        if (node) node.remove();
    }

    /**************************************************************************************************
    * Javascript function:
    * ************************************************************************************************/
    function setExportStatus(message){
        if (exportStatusEl) exportStatusEl.textContent = message || "";
    }

    /**************************************************************************************************
    * Javascript function:
    * ************************************************************************************************/
    function refreshExportControls(){
        Object.keys(exportButtons).forEach(function(key){
            let button = exportButtons[key];
            if (!button) return;
            button.disabled = (!backgroundLoaded) || exportInProgress;
        });
    }

    /**************************************************************************************************
    * Javascript function:
    * ************************************************************************************************/
    function getCurrentAspectMode(){
        let checked = document.querySelector('input[name="aspect"]:checked');
        return checked ? checked.value : 'auto';
    }

    /**************************************************************************************************
    * Javascript function:
    * ************************************************************************************************/
    function getCurrentAspectRatio(){
        let mode = getCurrentAspectMode();
        if (mode === '1-1') return 1.0;
        if (mode === '16-9') return 16.0 / 9.0;
        if (mode === '9-16') return 9.0 / 16.0;

        let canvas = document.getElementById('canvas');
        if (!canvas) return 1.0;
        let width = Number(canvas.clientWidth);
        let height = Number(canvas.clientHeight);
        if (!Number.isFinite(width) || !Number.isFinite(height) || width <= 0 || height <= 0) return 1.0;
        return width / height;
    }

    /**************************************************************************************************
    * Javascript function:
    * ************************************************************************************************/
    function roundDimension(value){
        let rounded = Math.round(value);
        return rounded > 0 ? rounded : 1;
    }

    /**************************************************************************************************
    * Javascript function:
    * ************************************************************************************************/
    function computeExportSize(label){
        let longEdge = exportLongEdges[label];
        if (!Number.isFinite(longEdge) || longEdge <= 0) return undefined;

        let ratio = getCurrentAspectRatio();
        if (!Number.isFinite(ratio) || ratio <= 0) ratio = 1.0;

        let width = longEdge;
        let height = longEdge;
        if (ratio >= 1.0) {
            height = roundDimension(longEdge / ratio);
        } else {
            width = roundDimension(longEdge * ratio);
        }

        return {
            width: width,
            height: height,
            longEdge: longEdge,
        };
    }

    /**************************************************************************************************
    * Javascript function:
    * ************************************************************************************************/
    function sanitiseFilenamePart(value){
        let text = String(value || "").trim().toLowerCase();
        text = text.replace(/[^a-z0-9]+/g, '-');
        while (text.startsWith('-')) text = text.slice(1);
        while (text.endsWith('-')) text = text.slice(0, -1);
        return text || 'effects-town';
    }

    /**************************************************************************************************
    * Javascript function:
    * ************************************************************************************************/
    function createExportFilename(label, width, height){
        return sanitiseFilenamePart(seed) + '-' + label + '-' + width + 'x' + height + '.png';
    }

    /**************************************************************************************************
    * Javascript function:
    * ************************************************************************************************/
    function triggerDownload(blob, filename){
        let url = URL.createObjectURL(blob);
        let link = document.createElement('a');
        link.href = url;
        link.download = filename;
        document.body.appendChild(link);
        link.click();
        link.remove();
        setTimeout(function(){
            URL.revokeObjectURL(url);
        }, 1000);
    }

    /**************************************************************************************************
    * Javascript function:
    * ************************************************************************************************/
    function requestImageExport(label){
        if (!backgroundLoaded || !worker || exportInProgress) return;

        let canvas = document.getElementById('canvas');
        if (!canvas) return;

        let size = computeExportSize(label);
        if (!size) return;

        exportInProgress = true;
        refreshExportControls();
        setExportStatus('Rendering ' + label + '...');

        worker.postMessage({
            'exportImage': true,
            'label': label,
            'width': size.width,
            'height': size.height,
            'previewWidth': roundDimension(canvas.clientWidth),
            'previewHeight': roundDimension(canvas.clientHeight),
            'filename': createExportFilename(label, size.width, size.height),
            'seed': seed,
            'params': collectParams(),
        });
    }

    backgroundLoadTimeout = setTimeout(function(){
        if (!backgroundLoaded) showStartupError('Startup timeout: background worker did not respond.');
    }, 10000);
    
    //Setup Events
    window.addEventListener('resize',onResize);
    window.addEventListener('beforeunload', onBeforeUnload);

    function queueWorkerFlush(){
        if (isShuttingDown || !worker) return;
        if (flushPending) return;
        flushPending = true;
        requestAnimationFrame(function(){
            flushPending = false;
            if (isShuttingDown || !worker) return;

            let msg = {};
            if (dirtySeed) {
                msg['seed'] = seed;
                msg['isPreview'] = false;
                dirtySeed = false;
            }
            if (dirtyParams) {
                msg['params'] = collectParams();
                dirtyParams = false;
            }
            if (dirtyResize) {
                let canvas = document.getElementById('canvas');
                msg['resize'] = true;
                msg['width'] = canvas.clientWidth;
                msg['height'] = canvas.clientHeight;
                dirtyResize = false;
            }
            if (Object.keys(msg).length > 0) worker.postMessage(msg);
        });
    }

    //Seed input
    function markSeedDirty(){
        if (!backgroundLoaded) return;
        dirtySeed = true;
        queueWorkerFlush();
    }

    if (seedInputEl) {
        seedInputEl.addEventListener('input', function(){
            seed = seedInputEl.value;
            markSeedDirty();
        });
        seedInputEl.addEventListener('change', function(){
            seed = seedInputEl.value;
            markSeedDirty();
        });
    }

    //Aspect ratio buttons
    document.querySelectorAll('input[name="aspect"]').forEach(function(radio){
        radio.addEventListener('change', function(){
            let canvas = document.getElementById('canvas');
            canvas.className = 'aspect-' + this.value;
            markResizeDirty();
        });
    });

    Object.keys(exportButtons).forEach(function(label){
        let button = exportButtons[label];
        if (!button) return;
        button.addEventListener('click', function(){
            requestImageExport(label);
        });
    });


    /**************************************************************************************************
    * Javascript function:
    * ************************************************************************************************/
    function onResize(){
            markResizeDirty();
    }

    /**************************************************************************************************
    * Javascript function:
    * ************************************************************************************************/
    function markResizeDirty(){
        dirtyResize = true;
        queueWorkerFlush();
    }

    /**************************************************************************************************
    * Javascript function:
    * ************************************************************************************************/
    function onBeforeUnload(){
        if (isShuttingDown) return;
        isShuttingDown = true;
        if (backgroundLoadTimeout) clearTimeout(backgroundLoadTimeout);
        if (paramsLoadTimeout) clearTimeout(paramsLoadTimeout);
        window.removeEventListener('resize', onResize);
        window.removeEventListener('beforeunload', onBeforeUnload);
        if (worker){
            worker.terminate();
            worker = undefined;
        }
        exportInProgress = false;
        refreshExportControls();
    }

    /**************************************************************************************************
    * Javascript function:
    * ************************************************************************************************/
    function sendCanvasToWorker(worker){
        let canvas = document.getElementById("canvas");        
        canvas.width = canvas.clientWidth;
        canvas.height = canvas.clientHeight;
        let offscreen = canvas.transferControlToOffscreen();
        worker.postMessage({canvas: offscreen},[offscreen]);
    }
    
    /**************************************************************************************************
    * Javascript function:
    * Syncs the page state to match the current form values.
    * Called on load to reconcile any browser-restored form state with the rest of the page.
    * ************************************************************************************************/
    function syncFormState(){
        let checked = document.querySelector('input[name="aspect"]:checked');
        if (checked) document.getElementById('canvas').className = 'aspect-' + checked.value;
    }

    /**************************************************************************************************
    * Javascript function:
    * ************************************************************************************************/
    function handleMessage(msg){
        if(msg.data.hasOwnProperty('loaded')){  //Initial Worker Loaded Message
            backgroundLoaded = true;
            clearStartupError();
            if (backgroundLoadTimeout) clearTimeout(backgroundLoadTimeout);
            syncFormState();
            worker.postMessage({'seed':seed, 'isPreview':false});
            sendCanvasToWorker(worker);
            refreshExportControls();
            paramsLoadTimeout = setTimeout(function(){
                if (!paramsLoaded) showStartupError('Startup timeout: render workers failed to publish parameters.');
            }, 10000);
            Module["on_worker_load"]();   //callback to c++
            return;
        }
        if(msg.data.hasOwnProperty('paramDefs')){  //Parameter definitions forwarded from first render worker.
            paramsLoaded = true;
            clearStartupError();
            if (paramsLoadTimeout) clearTimeout(paramsLoadTimeout);
            try {
                buildParameterForm(JSON.parse(msg.data['paramDefs']));
            } catch (e) {
                showStartupError('Failed to parse parameters from render worker.');
            }
            return;
        }
        if(msg.data.hasOwnProperty('exportComplete')){
            exportInProgress = false;
            refreshExportControls();
            if (msg.data['blob']) {
                triggerDownload(msg.data['blob'], msg.data['filename'] || 'effects-town.png');
                setExportStatus('Downloaded ' + msg.data['width'] + ' x ' + msg.data['height'] + '.');
            } else {
                setExportStatus('Save failed: missing image data.');
            }
            return;
        }
        if(msg.data.hasOwnProperty('exportFailed')){
            exportInProgress = false;
            refreshExportControls();
            let message = msg.data['message'] || 'Unable to save image.';
            setExportStatus(message);
            return;
        }
        Module["on_worker_message"](msg);
    }

    /**************************************************************************************************
    * Javascript function:
    * Builds the parameter form dynamically from the list supplied by the render worker.
    * Each entry in defs corresponds to one ParameterEntry from the C++ ParameterList.
    * ************************************************************************************************/
    function buildParameterForm(defs){
        let container = document.getElementById('parameters');
        let previous = document.getElementById('dynamic-parameters');
        if (previous) previous.remove();
        let section = document.createElement('div');
        section.id = 'dynamic-parameters';
        section.className = 'menugroup';
        let heading = document.createElement('div');
        heading.className = 'paramheading';
        heading.textContent = 'Parameters:';
        section.appendChild(heading);
        container.appendChild(section);

        let currentGroup = section;

        for (let p of defs) {
            const pType = p['type'];
            const pName = p['name'];
            const pId = p['id'];

            if (pType === 'colour') continue;
            if (pType === 'seed')   continue;

            if (pType === 'group_start') {
                let grp = document.createElement('div');
                grp.className = 'paramgroup';
                let grpName = document.createElement('div');
                grpName.className = 'paramgroupname';
                grpName.textContent = pName;
                grp.appendChild(grpName);
                section.appendChild(grp);
                currentGroup = grp;
                continue;
            }
            if (pType === 'group_end') {
                currentGroup = section;
                continue;
            }

            let row = document.createElement('div');
            row.className = 'paramrow';

            if (pType === 'number' || pType === 'angle' || pType === 'percent') {
                let labelText = pName;
                if (pType === 'angle')   labelText += ' \u00b0';
                if (pType === 'percent') labelText += ' %';
                let precision = Number(p['precision']);
                if (!Number.isFinite(precision)) precision = 0;
                let step = precision > 0 ? 1.0 / Math.pow(10, precision) : 1;
                let sliderMin = Number(p['slider_min']);
                let sliderMax = Number(p['slider_max']);
                let valueMin = Number(p['min']);
                let valueMax = Number(p['max']);
                let initialValue = Number(p['initial_value']);
                if (!Number.isFinite(sliderMin)) sliderMin = 0;
                if (!Number.isFinite(sliderMax)) sliderMax = 1;
                if (!Number.isFinite(valueMin)) valueMin = sliderMin;
                if (!Number.isFinite(valueMax)) valueMax = sliderMax;
                if (!Number.isFinite(initialValue)) initialValue = 0;

                let label = document.createElement('label');
                label.className = 'paramlabel';
                label.textContent = labelText;

                let controls = document.createElement('div');
                controls.className = 'paramcontrols';

                let slider = document.createElement('input');
                slider.type = 'range';
                slider.className = 'paramslider';
                slider.min = String(sliderMin);
                slider.max = String(sliderMax);
                slider.step = step;
                slider.value = String(initialValue);

                let numInput = document.createElement('input');
                numInput.type = 'number';
                numInput.className = 'paramnumber';
                numInput.setAttribute('data-param-id', String(pId));
                numInput.setAttribute('data-param-type', pType);
                numInput.setAttribute('data-param-default', String(initialValue));
                numInput.min = String(valueMin);
                numInput.max = String(valueMax);
                numInput.step = step;
                numInput.value = String(initialValue);

                slider.addEventListener('input', function(){
                    numInput.value = slider.value;
                    onParamsChanged();
                });
                numInput.addEventListener('change', function(){
                    slider.value = numInput.value;
                    onParamsChanged();
                });

                controls.appendChild(slider);
                controls.appendChild(numInput);
                row.appendChild(label);
                row.appendChild(controls);

            } else if (pType === 'list') {
                let label = document.createElement('label');
                label.className = 'paramlabel';
                label.textContent = pName;

                let select = document.createElement('select');
                select.className = 'paramselect';
                select.setAttribute('data-param-id', String(pId));
                select.setAttribute('data-param-type', 'list');
                select.setAttribute('data-param-default', String(Math.floor(Number.isFinite(Number(p['initial_value'])) ? Number(p['initial_value']) : 0)));
                let listItems = p['list'];
                if (!Array.isArray(listItems)) listItems = [];
                listItems.forEach(function(item){
                    let opt = document.createElement('option');
                    opt.value = item;
                    opt.textContent = item;
                    select.appendChild(opt);
                });
                let initialIndex = Number(p['initial_value']);
                if (Number.isFinite(initialIndex)) {
                    initialIndex = Math.floor(initialIndex);
                    if (initialIndex >= 0 && initialIndex < select.options.length) {
                        select.selectedIndex = initialIndex;
                    }
                }
                select.addEventListener('change', onParamsChanged);
                row.appendChild(label);
                row.appendChild(select);

            } else if (pType === 'check') {
                let label = document.createElement('label');
                label.className = 'paramlabel';
                let cb = document.createElement('input');
                cb.type = 'checkbox';
                cb.setAttribute('data-param-id', String(pId));
                cb.setAttribute('data-param-type', 'check');
                let checkedValue = Number(p['initial_value']);
                cb.checked = Number.isFinite(checkedValue) && checkedValue !== 0;
                cb.setAttribute('data-param-default', cb.checked ? '1' : '0');
                cb.addEventListener('change', onParamsChanged);
                label.appendChild(cb);
                label.appendChild(document.createTextNode(' ' + pName));
                row.appendChild(label);
            }

            currentGroup.appendChild(row);
        }

        let resetBtn = document.createElement('button');
        resetBtn.className = 'resetbutton';
        resetBtn.textContent = 'Reset All';
        resetBtn.addEventListener('click', function(){
            document.querySelectorAll('[data-param-id]').forEach(function(el){
                let def = el.getAttribute('data-param-default');
                let type = el.getAttribute('data-param-type');
                if (type === 'check') {
                    el.checked = (def === '1');
                } else if (type === 'list') {
                    let idx = parseInt(def, 10);
                    if (Number.isFinite(idx) && idx >= 0 && idx < el.options.length) el.selectedIndex = idx;
                } else {
                    el.value = def;
                    let slider = el.parentNode ? el.parentNode.querySelector('.paramslider') : null;
                    if (slider) slider.value = def;
                }
            });
            onParamsChanged();
        });
        section.appendChild(resetBtn);

        onParamsChanged();
    }

    /**************************************************************************************************
    * Javascript function:
    * Reads all parameter controls and sends their current values to the background worker.
    * Triggered whenever any parameter control changes.
    * ************************************************************************************************/
    function collectParams(){
        let params = {};
        document.querySelectorAll('[data-param-id]').forEach(function(el){
            let id = parseInt(el.getAttribute('data-param-id'),10);
            if (!Number.isFinite(id)) return;
            let type = el.getAttribute('data-param-type');
            if (type === 'list') {
                params[id] = el.value;
            } else if (type === 'check') {
                params[id] = el.checked ? 1.0 : 0.0;
            } else {
                let v = parseFloat(el.value);
                if (!Number.isFinite(v)) v = parseFloat(el.getAttribute('data-param-default'));
                params[id] = Number.isFinite(v) ? v : 0.0;
            }
        });
        return params;
    }

    /**************************************************************************************************
    * Javascript function:
    * Reads all parameter controls and sends their current values to the background worker.
    * Triggered whenever any parameter control changes.
    * ************************************************************************************************/
    function onParamsChanged(){
        if (!backgroundLoaded) return;
        dirtyParams = true;
        queueWorkerFlush();
    }

    refreshExportControls();
    return true;

})




/**************************************************************************************************
Starts a worker with script file "workerFile"
Returns a val of the started worker object.
*************************************************************************************************/
bool start_ui(){
    return javascript_run_ui();
}

/**************************************************************************************************
[Exported]
The worker thread has completed loading.
*************************************************************************************************/
void on_worker_loaded(){
    //js_console_log("Main Worker Ready");
}

/**************************************************************************************************
[Exported]
The worker has sent a message.
*************************************************************************************************/
void on_worker_message(__attribute__((unused)) val v){
    js_console_log("Worker Message Received");
}

//Bindings to be exported to javascript.
//Note: Closure compile used, so access binding with Module[""];
EMSCRIPTEN_BINDINGS(ui){
    function("on_worker_load", &on_worker_loaded);  
    function("on_worker_message", &on_worker_message);
}

