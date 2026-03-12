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

    The host independant renderer for the project.

*******************************************************************************************************/
#pragma once

#include <concepts>
#include <string>
#include <vector>
#include <numbers>
#include <typeinfo>

#include "../common/colour.h"
#include "../common/linear-algebra.h"
#include "../common/noise.h"
#include "../common/parameter-list.h"
#include "..\common\input-transforms.h"

#include "simd-cpuid.h"
#include "simd-f32.h"
#include "simd-uint32.h"
#include "simd-concepts.h"



/**************************************************************************************************
 * The renderer class.
 * Implements a host independent pixel renderer.
 * Use type parameter to select floating point precision.
 * ************************************************************************************************/
template <SimdFloat S>
class Renderer{
    

    private:
        int width {};
        int height {};
        typename S::F width_f {};
        typename S::F height_f {};
        typename S::F aspect {};
        std::string seed_string{};
        uint32_t seed{};
        ParameterList params{};

    public:
        //Constructor
        Renderer() noexcept {}

        //Size
        void set_size(int width, int height) noexcept;
        int get_width() const  { return width;}
        int get_height() const { return height;}

        //Set the seed as a string (an integer seed will be calculated)
        void set_seed(const std::string & s){
            this->seed=string_to_seed(s);             
            this->seed_string = s; 
        }
        //Set an integer seed. (string will be ignored)
        void set_seed_int(uint32_t s){
            this->seed = s;
        }
        std::string get_seed() const { return seed_string;}
        uint32_t get_seed_int() const { return seed;}
        
        //Parameters
        void set_parameters(ParameterList plist){
            params = plist;
        }

        //Render
        ColourRGBA<S> render_pixel(S x, S y) const;
        ColourRGBA<S> render_pixel_with_input(S x, S y, ColourRGBA<S>) const;

    private:


};


/**************************************************************************************************
 * Set the size of the image to render in pixels.
 * ************************************************************************************************/
template <SimdFloat S>
void Renderer<S>::set_size(int w, int h) noexcept {
    this->width = w;
    this->height = h;
    this->width_f = static_cast<typename S::F>(w);
    this->height_f = static_cast<typename S::F>(h);
    if (height==0) return;
    this->aspect = width_f/height_f;
}





/**************************************************************************************************
 * Render a pixel (or batch of pixels if using SIMD)
 * 
 * ************************************************************************************************/
template <SimdFloat S>
ColourRGBA<S> Renderer<S>::render_pixel(S x, S y) const {
    if (width <=0 || height <=0) return ColourRGBA<S>{};
    next_random<typename S::F>(seed); //Reset random seed so it is the same for each pixel
    
    auto parameter_scale = static_cast<typename S::F>(params.get_value(ParameterID::scale));
    const auto parameter_directional_bias = static_cast<typename S::F>(params.get_value(ParameterID::directional_bias));
    const auto parameter_evolve1 = 0.1f * static_cast<typename S::F>(params.get_value(ParameterID::evolve1));
    const auto parameter_evolve2 = static_cast<typename S::F>(2.0* std::numbers::pi) * static_cast<typename S::F>(params.get_value(ParameterID::evolve2));
    
    if (parameter_scale <= 0.0f) parameter_scale = 0.000001f;

    S xf = x ;
    S yf = y ;


    //Normalise to range: Hight = -1..1  Width = proportional zero centered.
    vec2<S> p(aspect * (static_cast<typename S::F>(2.0) * xf / width_f - static_cast<typename S::F>(1.0)), static_cast<typename S::F>(2.0) * yf / height_f - static_cast<typename S::F>(1.0));
 
    //Apply Input Transforms
    p = perform_input_transform(params.get_string(ParameterID::input_transform_type),p, params);

    
    //Apply Directional Bias
    vec2<S> d{1.0,1.0};
    if (signbit(parameter_directional_bias)) d.x -= parameter_directional_bias; else d.y += parameter_directional_bias;
    p = p * normalize(d) * static_cast<typename S::F>(sqrt(2)) * parameter_scale;
    
    auto evolve_x = S(parameter_evolve1 * cos(parameter_evolve2));
    auto evolve_y = S(parameter_evolve1 * sin(parameter_evolve2));
    
    
    auto p3 = vec4(p, evolve_x, evolve_y);
    


    auto nVec2 = p + (vec2(fbm(p3*0.05, 8, seed), fbm(p3*0.05 + 10.0f, 8, seed)) - 0.5f)*5.0f;
    



    p3 = vec4(nVec2, evolve_x + 99.2 , evolve_y-99.2);    
    auto nVec3 = nVec2 + vec2(fbm(p3 + 55.0f, 4, seed), fbm(p3 + 79.0f, 4, seed)) - 0.5f;

    p3 = vec4(nVec3, nVec3.x+ evolve_x - 44.2, nVec3.y+evolve_y + 44.2);
    auto nVec4 = nVec3 + vec2(fbm(p3 + 25.0f, 4, seed), fbm(p3 + 19.0f, 4, seed)) - 0.5f;


    auto nVec5 = nVec4 + vec2(fbm(nVec4 - 12.0f, 4, seed), fbm(nVec4 - 19.0f, 4, seed)) - 0.5f;
    auto nVec6 = nVec5 + vec2(fbm(nVec5 - 35.0f, 4, seed), fbm(nVec5 + 99.0f, 4, seed)) - 0.5f;
    auto nVec7 = nVec6 + vec2(fbm(nVec6 - 88.0f, 4, seed), fbm(nVec6 - 1.0f, 4, seed)) - 0.5f;
    
    auto r = fbm(vec4(nVec5, evolve_x*0.3f, evolve_y * 0.3f), 8,seed) * 0.65f;
    auto g = fbm(vec4(nVec6, evolve_x*0.25f, evolve_y * 0.3f), 8, seed) * 0.65f;
    auto b = fbm(vec4(nVec7, evolve_x*0.19f, evolve_y * 0.3f), 8, seed) * 0.65f;
    

    
    return ColourRGBA{r,g,b}; 





  

}    

/**************************************************************************************************
 * Render a pixel (or batch of pixels if using SIMD)
 * an input pixel is given
 * ************************************************************************************************/
template <SimdFloat S>
ColourRGBA<S> Renderer<S>::render_pixel_with_input(S x, S y, ColourRGBA<S>) const {
    render_pixel(x, y);
}

















