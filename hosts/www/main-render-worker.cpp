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
#include <algorithm>
#include <sstream>
#include <vector>

//We use the native type, so it can switch between wasm-simd and scalar opertions depending on compiler support.
thread_local Renderer<SimdNativeFloat32> renderer {};
thread_local ParameterList current_params {};
thread_local bool params_initialized = false;
thread_local std::vector<uint32_t> line_buffer {};

template <SimdFloat S>
static uint32_t pack_colour_lane(const ColourRGBA<S>& c, int lane) noexcept {
    return Colour8(
        float_to_8bit(c.red.element(lane)),
        float_to_8bit(c.green.element(lane)),
        float_to_8bit(c.blue.element(lane)),
        float_to_8bit(c.alpha.element(lane))
    ).to_uint32_keep_memory_layout();
}

/**************************************************************************************************
 * [Inline Javascript]
 * 
 * ************************************************************************************************/
EM_JS (bool, run_javascript, (), {
    //console.log("Render Worker Loaded");

    //***Javascript function to render a line.  Called by handleMessage***
    function render_line_job(data){
        //Apply parameters sent from the background worker before rendering.
        if (data.hasOwnProperty('params')) {
            let params = data['params'];
            for (let id in params) {
                let val = params[id];
                if (typeof val === 'string') {
                    Module["set_parameter_string"](parseInt(id), val);
                } else {
                    Module["set_parameter_value"](parseInt(id), val);
                }
            }
        }

        Module["setup_renderer"](data['width'], data['height']);
        Module["set_seed"](data['seed']);
        Module["render_line"](data['line']);

        const pixelCount = Module["get_line_buffer_length"]();
        const ptr = Module["get_line_buffer_ptr"]() >>> 2;
        const src = HEAPU32.subarray(ptr, ptr + pixelCount);
        var buf = new ArrayBuffer(pixelCount * 4);
        new Uint32Array(buf).set(src);

        postMessage({'result':true, 'buffer':buf, 'jobNumber': data['jobNumber'], 'line':data['line']},[buf]);
    }

    //*** Handel incoming messages ***
    function handleMessage(msg){
        if(msg.data.hasOwnProperty('render')){
            render_line_job(msg.data);
            return;
        }
    }

    self.onmessage = handleMessage;

    //Send loaded message to parent thread, including serialised parameter definitions for the UI.
    postMessage({'loaded':true, 'paramDefs': Module["get_parameters_json"]()}); //Note: Using string indexes to avoid minification by closure compiler.
    return true;
})

/**************************************************************************************************
* 
* Exported to JS
*************************************************************************************************/
void setup_renderer(uint32_t width, uint32_t height){
    if (!params_initialized) {
        current_params = build_project_parameters();
        params_initialized = true;
    }
    renderer.set_size(width,height);
    renderer.set_parameters(current_params);
}

/**************************************************************************************************
* Serialise the parameter list to a JSON string.
* Exported to JS — called once by the first render worker on startup to build the UI form.
*************************************************************************************************/
std::string get_parameters_json(){
    auto escape_json_string = [](const std::string& s) {
        std::ostringstream out;
        for (unsigned char ch : s) {
            switch (ch) {
                case '\"': out << "\\\""; break;
                case '\\': out << "\\\\"; break;
                case '\b': out << "\\b";  break;
                case '\f': out << "\\f";  break;
                case '\n': out << "\\n";  break;
                case '\r': out << "\\r";  break;
                case '\t': out << "\\t";  break;
                default:
                    if (ch < 0x20) {
                        const char* hex = "0123456789abcdef";
                        out << "\\u00";
                        out << hex[(ch >> 4) & 0x0f];
                        out << hex[ch & 0x0f];
                    } else {
                        out << static_cast<char>(ch);
                    }
                    break;
            }
        }
        return out.str();
    };

    auto params = build_project_parameters();
    std::string json = "[";
    bool first = true;
    for (const auto& p : params.entries) {
        if (!first) json += ",";
        first = false;
        std::string type_str;
        switch (p.type) {
            case ParameterType::seed:        type_str = "seed";        break;
            case ParameterType::number:      type_str = "number";      break;
            case ParameterType::percent:     type_str = "percent";     break;
            case ParameterType::angle:       type_str = "angle";       break;
            case ParameterType::list:        type_str = "list";        break;
            case ParameterType::check:       type_str = "check";       break;
            case ParameterType::group_start: type_str = "group_start"; break;
            case ParameterType::group_end:   type_str = "group_end";   break;
            case ParameterType::colour:      type_str = "colour";      break;
        }
        json += "{";
        json += "\"id\":"           + std::to_string(static_cast<int>(p.id));
        json += ",\"name\":\""      + escape_json_string(p.name) + "\"";
        json += ",\"type\":\""      + type_str + "\"";
        json += ",\"initial_value\":"  + std::to_string(p.initial_value);
        json += ",\"min\":"            + std::to_string(p.min);
        json += ",\"max\":"            + std::to_string(p.max);
        json += ",\"slider_min\":"     + std::to_string(p.slider_min);
        json += ",\"slider_max\":"     + std::to_string(p.slider_max);
        json += ",\"precision\":"      + std::to_string(static_cast<int>(p.precision));
        if (!p.list.empty()) {
            json += ",\"list\":[";
            bool first_item = true;
            for (const auto& item : p.list) {
                if (!first_item) json += ",";
                first_item = false;
                json += "\"" + escape_json_string(item) + "\"";
            }
            json += "]";
        }
        json += "}";
    }
    json += "]";
    return json;
}

/**************************************************************************************************
* Set a numerical parameter value.
* Exported to JS — called per-line with values from the UI form.
*************************************************************************************************/
void set_parameter_value(int id, double value){
    try {
        if (!params_initialized) {
            current_params = build_project_parameters();
            params_initialized = true;
        }
        current_params.set_value(static_cast<ParameterID>(id), value);
    } catch (...) {}
}

/**************************************************************************************************
* Set a string parameter value (used for list/dropdown selections).
* Exported to JS — called per-line with values from the UI form.
*************************************************************************************************/
void set_parameter_string(int id, const std::string& value){
    try {
        if (!params_initialized) {
            current_params = build_project_parameters();
            params_initialized = true;
        }
        current_params.set_value_string(static_cast<ParameterID>(id), value);
    } catch (...) {}
}

/**************************************************************************************************
* 
* Exported to JS
*************************************************************************************************/
void set_seed(const std::string str){
    renderer.set_seed(str);
}

/**************************************************************************************************
 * Render an entire line into a wasm-side buffer.
 * Exported to JS
 * ************************************************************************************************/
void render_line(uint32_t y){
    if (renderer.get_width() <= 0 || renderer.get_height() <= 0) {
        line_buffer.assign(1, 0xff0000ff);
        return;
    }

    using S = SimdNativeFloat32;
    constexpr int lane_width = S::number_of_elements();
    const uint32_t width = static_cast<uint32_t>(renderer.get_width());
    line_buffer.resize(width);

    const S y_vec(static_cast<S::F>(y));
    for (uint32_t x = 0; x < width; x += lane_width) {
        auto c = renderer.render_pixel(S::make_sequential(static_cast<S::F>(x)), y_vec);
        const uint32_t valid_lanes = std::min<uint32_t>(lane_width, width - x);
        for (uint32_t lane = 0; lane < valid_lanes; ++lane) {
            line_buffer[x + lane] = pack_colour_lane(c, static_cast<int>(lane));
        }
    }
}

/**************************************************************************************************
 * Return the current line buffer pointer.
 * Exported to JS
 * ************************************************************************************************/
uint32_t get_line_buffer_ptr(){
    return static_cast<uint32_t>(reinterpret_cast<uintptr_t>(line_buffer.data()));
}

/**************************************************************************************************
 * Return the current line buffer length in pixels.
 * Exported to JS
 * ************************************************************************************************/
uint32_t get_line_buffer_length(){
    return static_cast<uint32_t>(line_buffer.size());
}


/**************************************************************************************************
 * Main Entry Point for WebWorker
 * ************************************************************************************************/
int main(){     
    
    
    run_javascript();

   

    return 0;
}
using namespace emscripten;
EMSCRIPTEN_BINDINGS(main_render_worker){
    function("render_line", &render_line);
    function("get_line_buffer_ptr", &get_line_buffer_ptr);
    function("get_line_buffer_length", &get_line_buffer_length);
    function("setup_renderer", &setup_renderer);
    function("set_seed", &set_seed);
    function("get_parameters_json", &get_parameters_json);
    function("set_parameter_value", &set_parameter_value);
    function("set_parameter_string", &set_parameter_string);
}
