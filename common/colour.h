/********************************************************************************************************

Authors:		(c) 2023-2026 Maths Town

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

    Colour Types and routines

Types:

   Colour8   = 8 bit per pixel (RGBA colour order).  (sRGB Colour Space)
   ColourRGBA<F> = Floating point colour (F is float or double)  (0.0 .. 1.0 range)
   ColourLinear<F> = Floating point colour (F is float or double)  (0.0 .. 1.0 range)

   TODO: Make compatible with SIMD Types


*******************************************************************************************************/
#pragma once
#include <concepts>
#include <cstdint>
#include <cmath>
#include <string>
#include "simd-concepts.h"
#include "simd-f32.h"
#include "linear-algebra.h"

template <typename T>
concept FloatType = mt::SimdFloat<T> || std::floating_point<T>;

/****Forward Declaration of Types***/
struct Colour8;
template <FloatType F> struct ColourLinear;
template <typename F> requires mt::SimdFloat<F> || std::floating_point<F> struct ColourRGBA;

/****Forward Declaration of Functions***/
template <typename F> requires mt::SimdFloat<F> || std::floating_point<F> static constexpr F srgb_to_linear(F c) noexcept;

static constexpr uint8_t float_to_8bit(std::floating_point auto c) noexcept;
static uint8_t float_to_8bit(mt::SimdFloat auto c) noexcept;

//template <typename F> std::floating_point<F> static constexpr uint32_t float_to_uint(F c) noexcept;
template <typename F> requires mt::SimdFloat<F> || std::floating_point<F> static constexpr ColourRGBA<F> HSLtoRGB(F alpha, F h, F s, F l) noexcept;


/***Constants****/
inline constexpr unsigned char black8 = 0;
inline constexpr unsigned char white8 = 0xff;
inline constexpr float black32 = 0.0;
inline constexpr float white32 = 1.0;




/**************************************************************************************************
Colour8 Type
Type:           Generic 8 bit per pixel 
Gamma:          Generic
Colour Order :  RGBA
*************************************************************************************************/
struct Colour8 {
    public:
        uint8_t red {0};
        uint8_t green {0};
        uint8_t blue {0};
        uint8_t alpha {0xff};

        /**************************************************************************************************
        Constructors
        *************************************************************************************************/
        Colour8() = default;

        //Construct from components
        Colour8(uint8_t r, uint8_t g, uint8_t b, uint8_t a) noexcept {
            alpha = a; red = r; green = g; blue = b;
        }

        /**************************************************************************************************
            Convert to string.
        *************************************************************************************************/
        std::string to_string() const {            
            return "Colour8{r=" + std::to_string(red) + ", g="+ std::to_string(green) + ", b="+ std::to_string(blue) + ", a="+ std::to_string(alpha) + "}";
        }
    

        /*********************************************************************************************************
         Returns an unsigned int.
        RGBA colour order.
        (Mainly useful for string display.  Actual memory layout will be endian dependant)
        *********************************************************************************************************/
        uint32_t to_uint32() const noexcept{        
            return (static_cast<uint32_t>(red) << 24) | (static_cast<uint32_t>(green) << 16) | (static_cast<uint32_t>(blue) << 8 ) | (static_cast<uint32_t>(alpha) );
        }

        /*********************************************************************************************************
        Returns an unsigned 32bit int with the same memory layout as this struct.
        (Useful for packing into image buffers, and should work for either endian)
        *********************************************************************************************************/
        uint32_t to_uint32_keep_memory_layout() const noexcept{        
            static_assert (sizeof(Colour8) == 4, "Colour8 struct should be 4 bytes.");
            return *reinterpret_cast<const uint32_t *>(this);
        }

};




/**************************************************************************************************
ColourRGBA Type
Type:           Floating point (single or double by template)
Gamma:          SRGB
Colour Order :  RGBA

*************************************************************************************************/
template <typename F> requires mt::SimdFloat<F> || std::floating_point<F>
struct ColourRGBA {
    public:
        F red{ 0.0 };
        F green{ 0.0 };
        F blue{ 0.0 };
        F alpha{ 1.0 };

        /**************************************************************************************************
        Constructors
        *************************************************************************************************/

        //Default Contructor
        ColourRGBA() = default;

        ///Construct from components
        ColourRGBA(F r, F g, F b, F a = 1.0) noexcept {
            red = r; green = g; blue = b; alpha = a; 
        }

        ///Construct from 8-bit components
        ColourRGBA(uint8_t r, uint8_t g, uint8_t b, uint8_t a=0xff) noexcept {
            red = static_cast<F>(r) / white8;
            green = static_cast<F>(g) / white8;
            blue = static_cast<F>(b) / white8;
            alpha = static_cast<F>(a) / white8;
        }

        ColourRGBA(const vec3<F>& v) noexcept {
            red = v.x; 
            green = v.y; 
            blue = v.z; 
            alpha = 1.0;
        }

        
        template <typename F2>
        ColourRGBA(const ColourRGBA<F2> & c) noexcept {		
            red = static_cast<F>(c.red);
            green = static_cast<F>(c.green);
            blue = static_cast<F>(c.blue);
            alpha = static_cast<F>(c.alpha);
        }

        //Construct from HSL values.  All inputs 0..1
        static ColourRGBA<F> from_hsl(F alpha, F hue, F saturation, F lightness){
            return HSLtoRGB<F>(alpha, hue, saturation, lightness);
        }

        /**************************************************************************************************
        Convert to string.
        *************************************************************************************************/
        std::string to_string() const {
            return "ColourRGBA{r=" + std::to_string(red) + ", g="+ std::to_string(green) + ", b="+ std::to_string(blue) + ", a="+ std::to_string(alpha) + "}";
        }

        

        /**************************************************************************************************
        Convert to a linear colour.
        Assumes:  1. Not premultiplied alpha.
                  2. that no gamma is applied to the alpha channel.
        *************************************************************************************************/
        ColourLinear<F> to_linear() const noexcept {
            return ColourLinear<F>(srgb_to_linear(red), srgb_to_linear(green), srgb_to_linear(blue), alpha);
        }    

        /**************************************************************************************************
        Convert to simple using a simple 2.2 gamma adjustment (may be faster than piecewise method).
        Assumes:  1. Not premultiplied alpha.
                  2. that no gamma is applied to the alpha channel.
        *************************************************************************************************/
        ColourLinear<F> to_linear_simple() const noexcept {
            using std::pow;
            return ColourLinear<F>(pow(red, F{ 2.2 }), pow(green, static_cast<F>(2.2)), pow(blue, F{ 2.2 }), alpha);
        } 

        /**************************************************************************************************
        Convert to Colour8
        *************************************************************************************************/
        Colour8 to_colour8() const noexcept {
            return Colour8(float_to_8bit(red), float_to_8bit(green), float_to_8bit(blue), float_to_8bit(alpha) );
        }

        /**************************************************************************************************
        Returns colour as an unsigned int.        
        RGBA colour order.        
        *************************************************************************************************/
        uint32_t to_uint32() const noexcept {          
            return  (float_to_uint(red) << 24) | (float_to_uint(green) << 16) | (float_to_uint(blue) << 8 ) | (float_to_uint(alpha) );            
        }



        /**************************************************************************************************
        Multiplies the alpha channel through the colour. (For pre-mulitplied alpha buffers)
        Note: Routines in this library assume alpha is not pre-multiplied.
        *************************************************************************************************/
        ColourRGBA<F> premultiply_alpha() const noexcept {
            auto c = *this;
            c.red *= c.alpha;
            c.green *= c.alpha;
            c.blue *= c.alpha;
            return c;
        }

        /**************************************************************************************************
        Un-multiplies the alpha channel through the colour. (For pre-mulitplied alpha buffers)
        *************************************************************************************************/
        ColourRGBA<F> un_premultiply_alpha() const noexcept requires std::floating_point<F>  {
            auto c = *this;
            if (c.alpha == 0.0f) return c;
            c.red /= c.alpha;
            c.green /= c.alpha;
            c.blue /= c.alpha;
            return c;
        }
        ColourRGBA<F> un_premultiply_alpha() const noexcept requires mt::SimdFloat<F> {
            auto c = *this;
            
            // Replace zero alpha lanes with 1.0 if alpha is zero so division is always safe.
            using mt::if_equal;
            const auto safe_alpha = if_equal(c.alpha, F(0.0f), F(1.0f), c.alpha);            
            c.red = c.red / safe_alpha;
            c.green = c.green / safe_alpha;
            c.blue = c.blue / safe_alpha;
            return c;
        }

        /**************************************************************************************************
        Clamps all colour components to 0.0 ... 1.0
        *************************************************************************************************/
        ColourRGBA<F> clamp() const noexcept {
            const auto r = clamp_01(red);
            const auto g = clamp_01(green);
            const auto b = clamp_01(blue);
            const auto a = clamp_01(alpha);
            return ColourRGBA<F>(r,g,b,a);
        } 

        ColourRGBA<F> no_negatives() const noexcept {
            using std::max;
            const auto r = max(0.0f,red);
            const auto g = max(0.0f, green);
            const auto b = max(0.0f, blue);
            const auto a = max(0.0f, alpha);
            return ColourRGBA<F>(r, g, b, a);
        }

        /**************************************************************************************************
        Blend Operators.
        Alpha is ignored, and LHS alpha is kept.
        *************************************************************************************************/
        ColourRGBA<F>& operator+=(const F rhs) noexcept { red += rhs; green += rhs; blue += rhs; return *this; }
        ColourRGBA<F>& operator+=(const ColourRGBA<F>& rhs) noexcept { red += rhs.red; green += rhs.green; blue += rhs.blue; return *this; }
        ColourRGBA<F>& operator*=(const F rhs) noexcept { red *= rhs; green *= rhs; blue *= rhs; return *this; }
        ColourRGBA<F>& operator*=(const ColourRGBA<F>& rhs) noexcept { red *= rhs.red; green *= rhs.green; blue *= rhs.blue; return *this; }
};


template <typename F> inline ColourRGBA<F> operator+(ColourRGBA<F> lhs, const ColourRGBA<F>& rhs) noexcept { lhs += rhs;	return lhs; }
template <typename F, typename F2> inline ColourRGBA<F> operator+(ColourRGBA<F> lhs, F2 rhs) noexcept { lhs += static_cast<F>(rhs);	return lhs; }
template <typename F> inline ColourRGBA<F> operator*(ColourRGBA<F> lhs, const ColourRGBA<F>& rhs) noexcept { lhs *= rhs;	return lhs; }
template <typename F, typename F2> inline ColourRGBA<F> operator*(ColourRGBA<F> lhs, F2 rhs) noexcept { lhs *= static_cast<F>(rhs);	return lhs; }
template <typename F, typename F2> inline ColourRGBA<F> operator*(F2 lhs, ColourRGBA<F> rhs) noexcept { rhs *= static_cast<F>(lhs);	return rhs; }

template <typename F>
inline ColourRGBA<F> mix_colours(const ColourRGBA<F> & c1, const ColourRGBA<F> & c2, F weight) noexcept {
	return ColourRGBA<F>(
		c1.red * (1.0 - weight) + c2.red * weight,
		c1.green * (1.0 - weight) + c2.green * weight,
		c1.blue * (1.0 - weight) + c2.blue * weight,
        c1.alpha * (1.0-weight) + c2.alpha * weight
		);
}


/**************************************************************************************************
Converts a single component of sRGB colour to linear.
Uses the peicewise function as per the specification.  However, uses ^2.2 if outide 0.0 .. 1.0
*************************************************************************************************/
template <typename F> requires std::floating_point<F>
static constexpr F srgb_to_linear(F c) noexcept{
    if (c > 0.04045 && c < 1.0) [[likely]] return std::pow((c+0.055)/1.055, static_cast<F>(2.4));      
    if (c <= 0.04045 && c > 0.0 ) return c/12.92;
    return std::pow(c, static_cast<F>(2.2));  //If outside range use 2.2
} 

/**************************************************************************************************
Converts a single colour component from float to 8-bit (returns as a 8 bit unsigned char)
*************************************************************************************************/

inline static constexpr uint8_t float_to_8bit(std::floating_point auto c) noexcept {
    const decltype(c) a = c * white8;
    if (a <= 0.0) return 0;
    if (a >= white8) return white8;
    return static_cast<uint8_t>(a);
}

inline static uint8_t float_to_8bit(mt::SimdFloat auto c) noexcept {
    const decltype(c) a = clamp(c * white8, 0.0, white8);
    return static_cast<uint8_t>(a.v);
}




/**************************************************************************************************
Converts a single colour component from float to 8-bit (returns as a 32 bit unsigned int)
*************************************************************************************************/
/*template <typename F> 
static constexpr uint32_t float_to_uint(F c) noexcept {
    const F a = c * white8;
    if (a <= 0.0) return 0;
    if (a >= white8) return white8;
    return static_cast<uint32_t>(a);
}*/




//Just a helper for HSL to RGB
template <typename F>
static constexpr  F _HuetoRGB(F v1, F v2, F h) noexcept {
	if (h < 0) h += 1;
	if (h > 1) h -= 1;
	if (h * 6 < 1) return v1 + (v2 - v1) * 6 * h;
	if (h * 2 < 1) return v2;
	if (h * 3 < 2) return v1 + (v2 - v1) * ((2.0 / 3.0) - h) * 6;
	return v1;
}

/**************************************************************************************************
Conver HSL to RGB.  HSL values are all 0..1
***************************************************************************************************/
template <typename F>
static constexpr ColourRGBA<F> HSLtoRGB(F alpha, F h, F s, F l) noexcept{
	if (s == 0) return Colour(alpha, l, l, l);
	const F v2 = (l< 0.5) ? l * (1 + s) : (l + s) - (l*s);
	const F v1 = 2 * l - v2;
	return Colour(alpha,_HuetoRGB<F>(v1,v2,h+(1.0/3.0)), _HuetoRGB<F>(v1, v2, h), _HuetoRGB<F>(v1, v2, h - (1.0 / 3.0)));
}




