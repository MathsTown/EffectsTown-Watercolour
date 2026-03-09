/********************************************************************************************************

Authors:		(c) 2023 Maths Town

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

	Render functions for the openFX host.

TODO: 8-BIT Support
    : Premultiplied alpha Support

********************************************************************************************************/
#pragma once
#include "openfx-render.h"
#include "openfx-parameter-helper.h"
#include "openfx-instance-data.h"

//Project Specific Includes
#include "renderer.h"
#include "parameters.h"
#include "config.h"


#include "..\..\common\linear-algebra.h"
#include "simd-cpuid.h"
#include "simd-f32.h"
#include "simd-uint32.h"


#include <bit>
#include <memory>


//Contains data that will be sent to different threads.
template <SimdFloat S>
struct RenderThreadData {
    Renderer<S>* renderer {};
    ClipHolder* output{};
    std::unique_ptr<ClipHolder> input{};
    OfxRectI* render_window{};
};


/***Forward Declarations***/
static void ReplaceTransparentWithSource(OfxRectI renderWindow, ClipHolder& source, ClipHolder& output) noexcept;
static ParameterList read_parameters(ParameterHelper& parameter_helper, OfxTime time);
template <SimdFloat S> void thread_entry_pixel_render(unsigned int threadIndex, [[maybe_unused]] unsigned int threadMax, void* customArg);
template <SimdFloat S> static void render_line(RenderThreadData<S>* rd, int y);
template <SimdFloat S> static void do_render(OfxImageEffectHandle instance, OfxRectI& render_window, Renderer<S>& renderer, [[maybe_unused]] int width, [[maybe_unused]] int height, ClipHolder& output, const OfxTime& time);
template <SimdFloat S> static void setup_render(Renderer<S>& renderer, int width, int height, ParameterHelper& parameter_helper, OfxTime time);
template <SimdFloat S> static inline void render_pixel32(RenderThreadData<S>* rd, int x, int y);
template <SimdFloat S> static void render_line32(RenderThreadData<S>* rd, int y);



/*******************************************************************************************************
Render
*******************************************************************************************************/
OfxStatus openfx_render(const OfxImageEffectHandle instance, OfxPropertySetHandle in_args) {
    //dev_log(std::string("Render Action"));
    
    //Get the instance data
    InstanceData* instance_data{ nullptr };
    OfxPropertySetHandle effectProps;
    global_EffectSuite->getPropertySet(instance, &effectProps);
    global_PropertySuite->propGetPointer(effectProps, kOfxPropInstanceData, 0, (void**)&instance_data);

    
    //Get the context;
    char* cstr;
    OfxPropertySetHandle instanceProperties;
    check_openfx(global_EffectSuite->getPropertySet(instance, &instanceProperties));
    global_PropertySuite->propGetString(instanceProperties, kOfxImageEffectPropContext, 0, &cstr);
    //dev_log(std::string("Render Action - Context is: ") + cstr);
    [[maybe_unused]] auto context = GetContextFromString(cstr);

    //Get the current project time (we need it to checkout clips)
    OfxTime time{};
    check_openfx(global_PropertySuite->propGetDouble(in_args, kOfxPropTime, 0, &time));

    //Get the render window rectangle.
    OfxRectI renderWindow;
    check_openfx(global_PropertySuite->propGetIntN(in_args, kOfxImageEffectPropRenderWindow, 4, &renderWindow.x1));

    //Get the output clip handle 
    ClipHolder output_clip(instance, "Output", time);



    //Get Dimensions
    const int width = output_clip.bounds.x2 - output_clip.bounds.x1;
    const int height = output_clip.bounds.y2 - output_clip.bounds.y1;
    //dev_log(std::string("Size: " + std::to_string(width) + " x " + std::to_string(height)));




    //CPU Dispatch (assuming x86_64 for now)
    static_assert(mt::environment::is_x64, "Only x86_64 implemented");
    if constexpr (mt::environment::compiler_has_avx512dq && mt::environment::compiler_has_avx512f) {
        //AVX-512 & AVX-512DQ supported by compiler.
        Renderer<Simd512Float32> renderer{};
        setup_render(renderer, width, height, instance_data->parameter_helper, time);
        do_render(instance, renderWindow, renderer, width, height, output_clip, time);
    }
    else if constexpr (mt::environment::compiler_has_avx2 && mt::environment::compiler_has_avx && mt::environment::compiler_has_fma) {
        Renderer<Simd256Float32> renderer{};
        setup_render(renderer, width, height, instance_data->parameter_helper, time);
        do_render(instance, renderWindow, renderer, width, height, output_clip, time);
    }
    else {
        //Compiler mode just supports basic x86_64 (SSE2), so we will perform runtime dispatch to AVX2 code if supported.
        CpuInformation cpu_info{};
        if (Simd256UInt64::cpu_supported(cpu_info) && Simd256Float32::cpu_supported(cpu_info) && Simd256UInt32::cpu_supported(cpu_info)) {
            //AVX & AVX2
            Renderer<Simd256Float32> renderer{};
            setup_render(renderer, width, height, instance_data->parameter_helper, time);
            do_render(instance, renderWindow, renderer, width, height, output_clip, time);
        }
        else {
            //SSE2 (Generic x86_64)
            Renderer<Simd128Float32> renderer{};
            setup_render(renderer, width, height, instance_data->parameter_helper, time);
            do_render(instance, renderWindow, renderer, width, height, output_clip, time);
        }
    }


    //Get & Mix Souce image.
    if constexpr (project_uses_input && project_overlay_on_input) {
        if (context == OFXContext::general || context == OFXContext::filter) {
            //There should be an input image. 
            ClipHolder inputClip(instance, "Source", time);
            ReplaceTransparentWithSource(renderWindow, inputClip, output_clip);
        }
    }

    if constexpr (!project_is_solid_render) {
        if (output_clip.preMultiplied && output_clip.componentsPerPixel == 4) {
            //TODO
            dev_log("***TODO: Multiply Alpha");
        }
    }

    return kOfxStatOK;
}


/*******************************************************************************************************
Sets up the host-independant renderer object. 
Templated on the datatype
*******************************************************************************************************/
template <SimdFloat S>
static void setup_render(Renderer<S>& renderer, int width, int height, ParameterHelper& parameter_helper, OfxTime time) {
    auto params = read_parameters(parameter_helper, time);

    renderer.set_size(width, height);
    renderer.set_seed("OpenFX");
    if (params.contains(ParameterID::seed)) {
        renderer.set_seed_int(static_cast<uint64_t>(std::bit_cast<uint32_t>(params.get_value_integer(ParameterID::seed))));
    }

    renderer.set_parameters(std::move(params));
}


/*******************************************************************************************************
Read the parameters values from the host and store in a host-independant parameter list.
*******************************************************************************************************/
static ParameterList read_parameters(ParameterHelper& parameter_helper, OfxTime time) {
    auto params = build_project_parameters();
    
    for (auto& p : params.entries) {
        switch (p.type) {
        case ParameterType::seed:
            p.value_integer = parameter_helper.read_integer(p.id, time);
            break;
        case ParameterType::number:
            p.value = parameter_helper.read_slider(p.id, time);
            break;
        case ParameterType::list:
        {
            int i = parameter_helper.read_list(p.id, time);
            p.value_string = p.list[i];
            break;
        }

        default:
            break;
        }
    }
    return params;
}

/*******************************************************************************************************

*******************************************************************************************************/
template <SimdFloat S>
inline static void copy_pixel_to_output_buffer(ClipHolder& output, int x, int y, int max_x, ColourRGBA<S> c) {
    const bool hasAlpha = output.componentsPerPixel == 4;

    if constexpr (!project_is_solid_render) {
        if (output.preMultiplied) c = c.premultiply_alpha();
    }

   
    switch (output.bitDepth) {
    case 8:
         {
            //TODO: Code not tested.
            constexpr auto w8 = static_cast<Precision>(white8);
            
            for (int i = 0; i < S::number_of_elements(); i++) {
                if (x + i >= max_x) break;
                const auto ptrDest = output.pixelAddress8(x+i, y);
                ptrDest[0] = static_cast<uint8_t>(clamp(c.red.element(i) * w8,0.0f,w8));
                ptrDest[1] = static_cast<uint8_t>(clamp(c.green.element(i) * w8, 0.0f, w8));
                ptrDest[2] = static_cast<uint8_t>(clamp(c.blue.element(i) * w8, 0.0f, w8));
                if (hasAlpha) ptrDest[3] = static_cast<uint8_t>(clamp(c.alpha.element(i) * w8, 0.0f, w8));
            }
            break;
         }
    case 32:
        {
            //TODO: Use SIMD       
            
            for (int i = 0; i < S::number_of_elements(); i++) {
                if (x + i >= max_x) break;
                const auto ptrDest = output.pixelAddressFloat(x+i, y);
                ptrDest[0] = static_cast<float>(c.red.element(i));
                ptrDest[1] = static_cast<float>(c.green.element(i)); 
                ptrDest[2] = static_cast<float>(c.blue.element(i)); 
                if (hasAlpha) ptrDest[3] = static_cast<float>(c.alpha.element(i)); 

            }
            break;
        }
    default:
        dev_log("Unexpected Pixel Format");
    }
    
}


/*******************************************************************************************************
Assume the output already contains the top image.  Any transparent parts are filled with source.
*******************************************************************************************************/
[[maybe_unused]]
static void ReplaceTransparentWithSource(OfxRectI renderWindow, ClipHolder& source, ClipHolder& output) noexcept {
    if (output.componentsPerPixel != 4) return;

    if (output.bitDepth == 32) {
        for (int y = renderWindow.y1; y < renderWindow.y2; y++) {
            for (int x = renderWindow.x1; x < renderWindow.x2; x++) {
                const auto ptrTop = output.pixelAddressFloat(x, y);
                if (!ptrTop) continue;
                if (ptrTop[3] < 1.0f) {
                    const auto ptrBot = source.pixelAddressFloat(x, y);
                    if (!ptrBot) continue;
                    ptrTop[0] = ptrTop[0] + ptrBot[0] * (1 - ptrTop[3]); //red
                    ptrTop[1] = ptrTop[1] + ptrBot[1] * (1 - ptrTop[3]); //green
                    ptrTop[2] = ptrTop[2] + ptrBot[2] * (1 - ptrTop[3]); //blue
                    ptrTop[3] = ptrTop[3] + ptrBot[3] * (1 - ptrTop[3]); //alpha
                }

            }
        }
    }
    if (output.bitDepth == 8) {
        //TODO
        dev_log("***TODO: 8 but mixing");
    }
}



/*******************************************************************************************************
Thread Entry Point for rendering.
Used as a callback by OpenFX host.

Currently just balances worklaod using % operator.
*******************************************************************************************************/
template <SimdFloat S>
void thread_entry_pixel_render(unsigned int threadIndex, [[maybe_unused]] unsigned int threadMax, void* customArg) {
    RenderThreadData<S>* rd = static_cast<RenderThreadData<S>*>(customArg);
    for (int y = rd->render_window->y1; y < rd->render_window->y2; y++) {
        if (y % threadMax == threadIndex) {
            render_line(rd, y);
        }
    }
}

/*******************************************************************************************************
Do a full render.
Dispatches lines to worker threads.
(Called on a worker thread)
*******************************************************************************************************/
template <SimdFloat S>
static void do_render(OfxImageEffectHandle instance, OfxRectI& render_window, Renderer<S>& renderer, [[maybe_unused]] int width, [[maybe_unused]] int height, ClipHolder& output, const OfxTime& time) {

    RenderThreadData<S> rd{};
    rd.renderer = &renderer;
    rd.output = &output;
    rd.render_window = &render_window;
    rd.input = nullptr;

    //Get input clup handle (if input will be used at rendering phase)
    if constexpr (project_uses_input && !project_overlay_on_input) {
        rd.input = std::make_unique<ClipHolder>(instance, "Source", time);
    }

    unsigned int num_threads;
    global_MultiThreadSuite->multiThreadNumCPUs(&num_threads);
    //dev_log(std::string("Number of threads : ") + std::to_string(num_threads));

    if (num_threads > 1) [[likely]] {
        global_MultiThreadSuite->multiThread(thread_entry_pixel_render<S>, num_threads, &rd);
    }
    else {
        for (int y = render_window.y1; y < render_window.y2; y++) {
            if (global_EffectSuite->abort(instance)) return;
            render_line(&rd, y);
        }
    }
}


/*******************************************************************************************************
Render a line.
(Called on a worker thread)
*******************************************************************************************************/
template <SimdFloat S>
static void render_line(RenderThreadData<S>* rd, int y) {
    if (rd->output->bitDepth == 32 && rd->output->componentsPerPixel == 4) {
        render_line32(rd, y);
    }
    //TODO.  Other Bit Depths
}

/*******************************************************************************************************
Render a line.  
(Called on a worker thread)
*******************************************************************************************************/
template <SimdFloat S>
static void render_line32(RenderThreadData<S>* rd, int y) {
    //dev_log("Render Line " + std::to_string(y));

    int x = rd->render_window->x1;
    for (; x < rd->render_window->x2 - S::number_of_elements() + 1; x += S::number_of_elements()) {
        render_pixel32(rd, x, y);
    }
    //Handle the case where the width is not a multiple of S::number_of_elements
    if (x < rd->render_window->x2 && rd->render_window->x2 > S::number_of_elements()) [[unlikely]] {
        x -= S::number_of_elements() - (rd->render_window->x2 - x);
        render_pixel32(rd, x, y);

    }
}



/*******************************************************************************************************
32-bit
Renders a pixel (or a simd vector's worth of pixels)
Passes of to actual project renderer
*******************************************************************************************************/
template <SimdFloat S>
static inline void render_pixel32(RenderThreadData<S>* rd, int x, int y) {
    ColourRGBA<S> c;
    if constexpr (project_uses_input) {
        
        //Loads pixels from input buffer. 
        auto ptr = rd->input->pixelAddressFloat(x, y);
        ColourRGBA<S> input_colour;
        if (ptr) {
            for (int i = 0; i < S::number_of_elements(); i++) {
                input_colour.red.set_element(i, *(ptr++));
                input_colour.green.set_element(i, *(ptr++));
                input_colour.blue.set_element(i, *(ptr++));
                input_colour.alpha.set_element(i, *(ptr++));
            }
        }
        c = rd->renderer->render_pixel_with_input(S::make_sequential(static_cast<S::F>(x)), S(static_cast<S::F>(y)), input_colour);        
    }
    else {
        c = rd->renderer->render_pixel(S::make_sequential(static_cast<S::F>(x)), S(static_cast<S::F>(y)));        
    }
    copy_pixel_to_output_buffer(*rd->output, x, y, rd->render_window->x2, c);
}



