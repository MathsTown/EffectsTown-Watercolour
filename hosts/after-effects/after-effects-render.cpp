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

    Render functions for After Effects host.

*******************************************************************************************************/

//Project Specific includes
#include "config.h"
#include "renderer.h"
#include "parameters.h"

//General Includes
#include "after-effects-render.h"
#include "after-effects-parameter-helper.h"
#include "..\..\common\util.h"

#include "simd-cpuid.h"
#include "simd-f32.h"
#include "simd-uint32.h"

template <mt::SimdFloat S>
struct RenderData {
	int width{};
	int height{};
	Renderer<S> renderer;
	PF_Rect area;
	PF_EffectWorld* inputLayer{};
	PF_EffectWorld* output{};
	uint8_t* input_pixels{};
	A_u_long rowbytes{};
};

constexpr unsigned short adobe_white16 = 0x8000;

/*******************************************************************************************************
Copies a value to the output buffer. (32-bit components)
Note: If we are using SIMD the value may contain multiple pixels.
*******************************************************************************************************/
template <mt::SimdFloat S>
void copy_to_output_32(PF_EffectWorld* output, int x, int y, int max_x, const ColourRGBA<S>& c) {
	//Advance pointer to correct line (y).  (We must multiply by rowbytes in case the lines are padded.)  
	auto ptrY = (uint8_t*)output->data;
	ptrY += y * output->rowbytes;

	for (int i = 0; i < S::number_of_elements(); i++) {
		if (x + i >= max_x) break;
		auto ptrByte = ptrY + (x + i) * 4 * sizeof(float);  //Advance to x location.

		auto ptrFloat = (float*)ptrByte;
		ptrFloat[0] = static_cast<float>(c.alpha.element(i));
		ptrFloat[1] = static_cast<float>(c.red.element(i));
		ptrFloat[2] = static_cast<float>(c.green.element(i));
		ptrFloat[3] = static_cast<float>(c.blue.element(i));
	}
}

/*******************************************************************************************************
Copies a value to the output buffer. (16-bit components)
Note: If we are using SIMD the value may contain multiple pixels.
Note: Adobe uses ARGB colour order, with unmultiplied alpha.
Note: Adobe 16-bit is not full 16-bit.  White is 0x8000
*******************************************************************************************************/
template <mt::SimdFloat S>
void copy_to_output_16(PF_EffectWorld* output, int x, int y, int max_x, ColourRGBA<S> c) {

	
	auto black = S(0.0);
	auto white = S(adobe_white16);

	c.red = clamp(c.red * white, black, white);
	c.green = clamp(c.green * white, black, white);
	c.blue = clamp(c.blue * white, black, white);
	c.alpha = clamp(c.alpha * white, black, white);

	//Advance pointer to correct line (y).  (We must multiply by rowbytes in case the lines are padded.)  
	auto ptrY = (uint8_t*)output->data;
	ptrY += y * output->rowbytes;

	for (int i = 0; i < S::number_of_elements(); i++) {
		if (x + i >= max_x) break;
		auto ptrByte = ptrY + (x + i) * 4 * sizeof(uint16_t);  //Advance to x location.

		auto ptrU16 = (uint16_t*)ptrByte;
		ptrU16[0] = static_cast<uint16_t>(c.alpha.element(i));
		ptrU16[1] = static_cast<uint16_t>(c.red.element(i));
		ptrU16[2] = static_cast<uint16_t>(c.green.element(i));
		ptrU16[3] = static_cast<uint16_t>(c.blue.element(i));
	}
}



/*******************************************************************************************************
Copies a value to the output buffer. (8-bit components)
Note: If we are using SIMD the value may contain multiple pixels.
Note: Adobe uses ARGB colour order, with unmultiplied alpha.
*******************************************************************************************************/
template <mt::SimdFloat S>
void copy_to_output_8(PF_EffectWorld* output, int x, int y, int max_x, ColourRGBA<S> c) {
	constexpr unsigned short adobe_white8 = 0xff;
	
	auto black = S(0.0);
	auto white = S(adobe_white8);
	c.red = clamp(c.red * white, black, white);
	c.green = clamp(c.green * white, black, white);
	c.blue = clamp(c.blue * white, black, white);
	c.alpha = clamp(c.alpha * white, black, white);


	//Advance pointer to correct line (y).  (We must multiply by rowbytes in case the lines are padded.)
	auto ptrY = (uint8_t*)output->data;
	ptrY += y * output->rowbytes;

	for (int i = 0; i < S::number_of_elements(); i++) {
		if (x + i >= max_x) break;
		auto ptrByte = ptrY + (x + i) * 4 * sizeof(uint8_t);  //Advance to x location.
				
		ptrByte[0] = static_cast<uint8_t>(c.alpha.element(i));
		ptrByte[1] = static_cast<uint8_t>(c.red.element(i));
		ptrByte[2] = static_cast<uint8_t>(c.green.element(i));
		ptrByte[3] = static_cast<uint8_t>(c.blue.element(i));
	}
}

/*******************************************************************************************************
Gathers 32-bit ARGB data into SIMD vectors, returning a ColourRGBA struct.
SSE 128 bit registers
*******************************************************************************************************/
static inline ColourRGBA<mt::Simd128Float32> gather_image_data_sse(float* in) {
	const auto in1 = _mm_loadu_ps(in);
	const auto in2 = _mm_loadu_ps(in + 4);
	const auto in3 = _mm_loadu_ps(in + 8);
	const auto in4 = _mm_loadu_ps(in + 12);
	
	//Shuffle
	const auto r1 = _mm_unpacklo_ps(in1, in2);
	const auto r3 = _mm_unpacklo_ps(in3, in4);
	auto alpha = _mm_movelh_ps(r1, r3);
	auto red = _mm_movehl_ps(r3, r1);
	const auto r2 = _mm_unpackhi_ps(in1, in2);
	const auto r4 = _mm_unpackhi_ps(in3, in4);
	auto green = _mm_movelh_ps(r2, r4);
	auto blue =  _mm_movehl_ps(r4, r2);

	return ColourRGBA<mt::Simd128Float32>(red, green, blue, alpha);
}

/*******************************************************************************************************
Gathers 32-bit ARGB data into SIMD vectors, returning a ColourRGBA struct.
AVX 256 bit registers
AVX2 Required.
*******************************************************************************************************/
static inline ColourRGBA<mt::Simd256Float32> gather_image_data_avx(float* in) {	
	const auto alpha = _mm256_i32gather_ps(in, _mm256_set_epi32(28, 24, 20, 16, 12, 8, 4, 0), 4);
	const auto red =   _mm256_i32gather_ps(in, _mm256_set_epi32(29, 25, 21, 17, 13, 9, 5, 1), 4);
	const auto green = _mm256_i32gather_ps(in, _mm256_set_epi32(30, 26, 22, 18, 14, 10, 6, 2), 4);
	const auto blue =  _mm256_i32gather_ps(in, _mm256_set_epi32(31, 27, 23, 19, 15, 11, 7, 3), 4);	 
	return ColourRGBA<mt::Simd256Float32>(red, green, blue, alpha);
}

/*******************************************************************************************************
Gathers 32-bit ARGB data into SIMD vectors, returning a ColourRGBA struct.
AVX 512 bit registers
AVX-512F Required.
*******************************************************************************************************/
static inline ColourRGBA<mt::Simd512Float32> gather_image_data_avx512(float* in) {
	const auto alpha = _mm512_i32gather_ps(_mm512_set_epi32(60, 56, 52, 48, 44, 40, 36, 32, 28, 24, 20, 16, 12, 8, 4, 0), in, 4);
	const auto red =   _mm512_i32gather_ps(_mm512_set_epi32(61, 57, 53, 49, 45, 41, 37, 33, 29, 25, 21, 17, 13, 9, 5, 1), in, 4);
	const auto green = _mm512_i32gather_ps(_mm512_set_epi32(62, 58, 54, 50, 46, 42, 38, 34, 30, 26, 22, 18, 14, 10, 6, 2), in, 4);
	const auto blue =  _mm512_i32gather_ps(_mm512_set_epi32(63, 59, 55, 51, 47, 43, 39, 35, 31, 27, 23, 19, 15, 11, 7, 3), in, 4);
	return ColourRGBA<mt::Simd512Float32>(red, green, blue, alpha);
}





/*******************************************************************************************************
8-bit
*******************************************************************************************************/
template <mt::SimdFloat S>
static inline ColourRGBA<S> read_input_pixel8(const RenderData<S>* rd, int x, int y) {	
	const int sourceOffset = ((rd->inputLayer->rowbytes * y) + (x * 4 * sizeof(uint8_t)));
	uint8_t* ptr = reinterpret_cast<uint8_t*>(sourceOffset + reinterpret_cast<uint8_t*>(rd->inputLayer->data));

	//Convert to float data  (probably better to use simd)
	alignas(sizeof(S)) std::array<float, S::number_of_elements() * 4> float_data;
	for (int i = 0; i < S::number_of_elements() * 4; i++) {
		float_data[i] = static_cast<float>(*ptr++) / static_cast<float>(white8);
	}

	//Gather colour data into SIMD vectors
	if constexpr (sizeof(S) == 16) 	return gather_image_data_sse(&float_data[0]);
	if constexpr (sizeof(S) == 32) 	return gather_image_data_avx(&float_data[0]);
	if constexpr (sizeof(S) == 64) 	return gather_image_data_avx512(&float_data[0]);
	

	return ColourRGBA<S>{};
	
}

/*******************************************************************************************************
16-bit
*******************************************************************************************************/
template <mt::SimdFloat S>
static inline ColourRGBA<S> read_input_pixel16(const RenderData<S>* rd, int x, int y) {
	const int sourceOffset = ((rd->inputLayer->rowbytes * y) + (x * 4 * sizeof(uint16_t)));
	uint16_t* ptr = reinterpret_cast<uint16_t*>(sourceOffset + reinterpret_cast<uint8_t*>(rd->inputLayer->data));
	
	//Convert to float data (probably better to use simd)
	alignas(sizeof(S)) std::array<float, S::number_of_elements() * 4> float_data;
	for (int i = 0; i < S::number_of_elements() * 4; i++) {
		float_data[i] = static_cast<float>(*ptr++)/ static_cast<float>(adobe_white16);
	}	

	//Gather colour data into SIMD vectors
	if constexpr (sizeof(S) == 16) return gather_image_data_sse(&float_data[0]);
	if constexpr (sizeof(S) == 32) return gather_image_data_avx(&float_data[0]);
	if constexpr (sizeof(S) == 64) return gather_image_data_avx512(&float_data[0]);
	

	return ColourRGBA<S>{};
}

/*******************************************************************************************************
32-bit
*******************************************************************************************************/
template <mt::SimdFloat S>
static inline ColourRGBA<S> read_input_pixel32(const RenderData<S>* rd, int x, int y) {
	const int sourceOffset = ((rd->inputLayer->rowbytes * y) + (x * 4 * sizeof(float))) ;
	float* ptr = reinterpret_cast<float*>(sourceOffset + reinterpret_cast<uint8_t*>(rd->inputLayer->data));
	
	//Check size of SIMD lane
	if constexpr (sizeof(typename S::F) == 4) {
		//Gather colour data into SIMD vectors
		if constexpr (sizeof(S) == 16) return gather_image_data_sse(ptr);
		if constexpr (sizeof(S) == 32) return gather_image_data_avx(ptr);
		if constexpr (sizeof(S) == 64) return gather_image_data_avx512(ptr);
	}
	else {
		//TODO
	}
	
	return ColourRGBA<S>{};
}



/*******************************************************************************************************
8-bit
Renders a pixel (or a simd vector's worth of pixels)
Passes of to actual project renderer
*******************************************************************************************************/
template <mt::SimdFloat S>
static inline void render_pixel8(const RenderData<S> * rd, int x, int y) {
	if constexpr (project_uses_input) {
		ColourRGBA<S> input_colour = read_input_pixel8(rd,x,y);
		auto c =  rd->renderer.render_pixel_with_input(S::make_sequential(static_cast<S::F>(x)), S(static_cast<S::F>(y)), input_colour);
		copy_to_output_8(rd->output, x, y, rd->area.right, c);
	}
	else {
		auto c = rd->renderer.render_pixel(S::make_sequential(static_cast<S::F>(x)), S(static_cast<S::F>(y)));
		copy_to_output_8(rd->output, x, y, rd->area.right, c);
	}
}

/*******************************************************************************************************
16-bit
Renders a pixel (or a simd vector's worth of pixels)
Passes of to actual project renderer
*******************************************************************************************************/
template <mt::SimdFloat S>
static inline void render_pixel16(const RenderData<S>* rd, int x, int y) {
	if constexpr (project_uses_input) {
		ColourRGBA<S> input_colour = read_input_pixel16(rd,x,y);
		auto c = rd->renderer.render_pixel_with_input(S::make_sequential(static_cast<S::F>(x)), S(static_cast<S::F>(y)), input_colour);
		copy_to_output_16(rd->output, x, y, rd->area.right, c);
	}
	else {
		auto c = rd->renderer.render_pixel(S::make_sequential(static_cast<S::F>(x)), S(static_cast<S::F>(y)));
		copy_to_output_16(rd->output, x, y, rd->area.right, c);
	}
}

/*******************************************************************************************************
32-bit
Renders a pixel (or a simd vector's worth of pixels)
Passes of to actual project renderer
*******************************************************************************************************/
template <mt::SimdFloat S>
static inline void render_pixel32(const RenderData<S>* rd, int x, int y) {
	if constexpr (project_uses_input) {
		ColourRGBA<S> input_colour = read_input_pixel32(rd,x,y);
		auto c = rd->renderer.render_pixel_with_input(S::make_sequential(static_cast<S::F>(x)), S(static_cast<S::F>(y)), input_colour);
		copy_to_output_32(rd->output, x, y, rd->area.right, c);
	}
	else {
		auto c = rd->renderer.render_pixel(S::make_sequential(static_cast<S::F>(x)), S(static_cast<S::F>(y)));
		copy_to_output_32(rd->output, x, y, rd->area.right, c);
	}
}



/*******************************************************************************************************
Callback for After Effects Iteration Suite.  Renders an 32-bit line of pixels.
This thread callback will give us a line to render.
Note: Adobe uses ARGB colour order, with unmultiplied alpha.
*******************************************************************************************************/
template <mt::SimdFloat S>
static PF_Err render_8bit_pixel_callback(void* refcon, [[maybe_unused]]  A_long thread_idxL, A_long  i, [[maybe_unused]] A_long itrtL) noexcept {
	const auto rd = static_cast<RenderData<S> *>(refcon);
	const auto y = i;
	if (y < rd->area.top || y >= rd->area.bottom) [[unlikely]] return PF_Err_NONE;  //Check vertical bounds

	int x = rd->area.left;
	for (; x < rd->area.right - S::number_of_elements() + 1; x += S::number_of_elements()) {		
		render_pixel8<S>(rd, x, y);		
	}
	
	//Handle the case where the width is not a multiple of S::number_of_elements
	if (x < rd->area.right && rd->area.right>S::number_of_elements()) [[unlikely]] {		
		x -= S::number_of_elements() - (rd->area.right - x);		
		render_pixel8<S>(rd, x, y);		
	}

	return PF_Err_NONE;

}

/*******************************************************************************************************
Callback for After Effects Iteration Suite.  Renders an 16-bit pixel.
Note: Adobe 16 bit is not full 16-bit.  White is 0x8000
Note: Adobe uses ARGB colour order, with unmultiplied alpha.
*******************************************************************************************************/
template <mt::SimdFloat S>
static PF_Err render_16bit_pixel_callback(void* refcon, A_long , A_long  i, [[maybe_unused]]  A_long itrtL) noexcept {
	const auto rd = static_cast<RenderData<S> *>(refcon);
	const auto y = i;
	if (y < rd->area.top || y >= rd->area.bottom) [[unlikely]] return PF_Err_NONE;  //Check vertical bounds

	int x = rd->area.left;
	for (; x < rd->area.right - S::number_of_elements() + 1; x += S::number_of_elements()) {
		render_pixel16<S>(rd, x, y);		
	}

	//Handle the case where the width is not a multiple of S::number_of_elements
	if (x < rd->area.right && rd->area.right>S::number_of_elements()) [[unlikely]] {
		x -= S::number_of_elements() - (rd->area.right - x);
		render_pixel16<S>(rd, x, y);		
	}
	return PF_Err_NONE;	
}



/*******************************************************************************************************
Callback for After Effects Iteration Suite.  Renders an 32-bit line of pixels.
This thread callback will give us a line to render.
Note: Adobe uses ARGB colour order, with unmultiplied alpha.
*******************************************************************************************************/
template <mt::SimdFloat S>
static PF_Err render_32bit_pixel_callback(void* refcon, A_long , A_long  i, [[maybe_unused]]  A_long itrtL) noexcept {
	const auto rd = static_cast<RenderData<S> *>(refcon);
	const auto y = i;
	
	
	if (y < rd->area.top || y >= rd->area.bottom) [[unlikely]] return PF_Err_NONE;  //Check vertical bounds

	int x = rd->area.left;
	for (; x < rd->area.right - S::number_of_elements() + 1; x += S::number_of_elements()) {
		render_pixel32<S>(rd, x, y);
	}

	//Handle the case where the width is not a multiple of S::number_of_elements
	if (x < rd->area.right && rd->area.right>S::number_of_elements()) [[unlikely]] {
		x -= S::number_of_elements() - (rd->area.right - x);
		render_pixel32<S>(rd, x, y);
	}
	
	return PF_Err_NONE;

}


/*******************************************************************************************************
Calculates the width and height of the image we are working with.  (From layer size)
Size may vary in low resolution renders.
*******************************************************************************************************/
static std::tuple<int, int> calculate_size(const PF_InData* in_data) {
	//We may be redering a low resolution version, get the scaleFactor
	const float scaleFactorX = static_cast<float>(in_data->downsample_x.den) / static_cast<float>(in_data->downsample_x.num);
	const float scaleFactorY = static_cast<float>(in_data->downsample_y.den) / static_cast<float>(in_data->downsample_y.num);

	//Calculate the size of our output using the full layer size.
	int width = static_cast<int>(static_cast<float>(in_data->width) / scaleFactorX);
	int height = static_cast<int>(static_cast<float>(in_data->height) / scaleFactorY);

	return std::tuple<int, int>(width, height);
}

/*******************************************************************************************************
Read Parameters from After Effects and put into data into our parameter list.
*******************************************************************************************************/
ParameterList read_parameters() {
	auto params = build_project_parameters();
	for (auto & p : params.entries) {
		switch (p.type) {
		case ParameterType::seed:
			p.value = ParameterHelper::ReadSlider(p.id);
			p.value_integer = static_cast<int>(round(p.value));
			break;
		case ParameterType::number:
			p.value = ParameterHelper::ReadSlider(p.id);		
			p.value_integer = static_cast<int>(round(p.value));
			break;
		case ParameterType::list:			
			p.value_string = ParameterHelper::ReadListAsString(p.id, p.list);
			break;
		case ParameterType::colour:
			auto pixel = ParameterHelper::ReadColour(p.id);
			p.red = pixel.red / 255.0f;
			p.green = pixel.green / 255.0f;
			p.blue = pixel.blue / 255.0f;
			break;

		default:
			break;
		}
	}

	return params;
}


/*******************************************************************************************************
Setup Host Independant Renderer
*******************************************************************************************************/
template <typename S>
void setup_render(Renderer<S>& renderer, const PF_InData* in_data, int width, int height) {
	check_null(in_data);
	auto params = read_parameters();
	
	renderer.set_size(width, height);
	renderer.set_seed("After Effects");
	if (params.contains(ParameterID::seed)) {
		renderer.set_seed_int(static_cast<uint32_t>(params.get_value(ParameterID::seed)));
	}
	
	renderer.set_parameters(std::move(params));
	
}

/*******************************************************************************************************
After Effects SmartPreRender Command
Called by After Effects to get information about the render.  
We need to set the output rectangles and checkout any layers that we need.
*******************************************************************************************************/
void after_effects_smart_pre_render(const PF_InData* in_data, PF_PreRenderExtra* preRender) {
	check_null(in_data);
	check_null(preRender);
	check_null(in_data->effect_ref);
	check_null(preRender->output);
	check_null(preRender->input);

	const auto [width, height] = calculate_size(in_data);

	//Checkout the input layer
	PF_CheckoutResult inputLayer{};
	if constexpr (project_uses_input) {
		check_after_effects(preRender->cb->checkout_layer(in_data->effect_ref, 0, 0, &preRender->input->output_request, in_data->current_time, in_data->time_step, in_data->time_scale, &inputLayer));
	}
	const auto r = preRender->input->output_request.rect;
	//const auto in = inputLayer.result_rect;
	


	//Max output rectangle.
	preRender->output->max_result_rect.top = 0;
	preRender->output->max_result_rect.bottom = height;
	preRender->output->max_result_rect.left = 0;
	preRender->output->max_result_rect.right = width;
	
	//Output rectangle
	preRender->output->result_rect.top = std::max(0, r.top);
	preRender->output->result_rect.bottom = std::min(height, r.bottom);
	preRender->output->result_rect.left = std::max(0, r.left);
	preRender->output->result_rect.right = std::min(width, r.right);

	//Is output solid?
	preRender->output->solid = project_is_solid_render; //Project setting from config.h
}


/*******************************************************************************************************
Template function built based on SIMD type (which is CPU dependant)
*******************************************************************************************************/
template <mt::SimdFloat S>
void after_effect_cpu_dispatch(int width, int height, PF_InData* in_data, [[maybe_unused]]  const PF_Rect& area, int bit_depth, [[maybe_unused]]  PF_EffectWorld* inputLayer, [[maybe_unused]] PF_EffectWorld* output, RenderData<S>& rd) {
	//Setup parameters
	setup_render(rd.renderer, in_data, width, height);
	
	//Perform the render.
	AEGP_SuiteHandler suites(in_data->pica_basicP);
	switch (bit_depth) {
	case 8:
	{
		check_after_effects(suites.Iterate8Suite1()->iterate_generic(rd.height, &rd , render_8bit_pixel_callback<S>));
		break;
	}
	case 16: {
		check_after_effects(suites.Iterate8Suite1()->iterate_generic(rd.height, &rd, render_16bit_pixel_callback<S>));
		break;
	}
	case 32: {
		check_after_effects(suites.Iterate8Suite1()->iterate_generic(rd.height, &rd, render_32bit_pixel_callback<S>));
		break;
	}
	default: break;
	}
}


/*******************************************************************************************************
Common Render Function to Smart and Non-Smart rendering.
Sets up the renderer and dispatches based on CPU
*******************************************************************************************************/
void after_effects_common_render(int width, int height, PF_InData* in_data, const PF_Rect& area, int bit_depth, PF_EffectWorld* inputLayer, PF_EffectWorld* output) {
	AEGP_SuiteHandler suites(in_data->pica_basicP);	
	int precision = 0;

	//Get Render precision.
	if constexpr (project_is_multiprecision) {
		precision = ParameterHelper::ReadList(ParameterID::render_precision) - 1;
	}

	static_assert(mt::environment::is_x86_64, "Only x86_64 implemented");
	if constexpr (mt::environment::compiler_has_avx512dq && mt::environment::compiler_has_avx512f) {
		//Compiler mode supports micro architecture level 4 (AVX-512).  
		if (precision == 0) {
			RenderData<mt::Simd512Float32> rd{};
			rd.width = width;
			rd.height = height;
			rd.area = area;
			rd.output = output;
			rd.inputLayer = inputLayer;
			after_effect_cpu_dispatch(width, height, in_data, area, bit_depth, inputLayer, output, rd);
		}
		else {
			RenderData<mt::Simd512Float64> rd{};
			rd.width = width;
			rd.height = height;
			rd.area = area;
			rd.output = output;
			rd.inputLayer = inputLayer;
			after_effect_cpu_dispatch(width, height, in_data, area, bit_depth, inputLayer, output, rd);
		}
	}
	else if constexpr (mt::environment::compiler_has_avx2 && mt::environment::compiler_has_avx && mt::environment::compiler_has_fma) {
		if (precision == 0) {
			//Compiler mode supports micro architecture level 3 (AVX2).  
			RenderData<mt::Simd256Float32> rd{};
			rd.width = width;
			rd.height = height;
			rd.area = area;
			rd.output = output;
			rd.inputLayer = inputLayer;
			after_effect_cpu_dispatch(width, height, in_data, area, bit_depth, inputLayer, output, rd);
		}
		else if (precision == 1) {
			//Compiler mode supports micro architecture level 3 (AVX2).  
			RenderData<mt::Simd256Float64> rd{};
			rd.width = width;
			rd.height = height;
			rd.area = area;
			rd.output = output;
			rd.inputLayer = inputLayer;
			after_effect_cpu_dispatch(width, height, in_data, area, bit_depth, inputLayer, output, rd);
		}
	}
	else {
		//Compiler mode just supports basic x86_64 (SSE2), so we will perform runtime dispatch to AVX2 code if supported..
		mt::CpuInformation cpu_info{};
		if (mt::Simd256UInt32::cpu_supported(cpu_info) && mt::Simd256Float32::cpu_supported(cpu_info)) {
			if (precision == 0) {
				//AVX & AVX2
				RenderData<mt::Simd256Float32> rd{};
				rd.width = width;
				rd.height = height;
				rd.area = area;
				rd.output = output;
				rd.inputLayer = inputLayer;
				after_effect_cpu_dispatch(width, height, in_data, area, bit_depth, inputLayer, output, rd);
			}
			else if (precision == 1) {
				//AVX & AVX2
				RenderData<mt::Simd256Float64> rd{};
				rd.width = width;
				rd.height = height;
				rd.area = area;
				rd.output = output;
				rd.inputLayer = inputLayer;
				after_effect_cpu_dispatch(width, height, in_data, area, bit_depth, inputLayer, output, rd);
			}
		}
		else {
			if (precision == 0) {
				//SSE2 (Generic x86_64)
				RenderData<mt::Simd128Float32> rd{};
				rd.width = width;
				rd.height = height;
				rd.area = area;
				rd.output = output;
				rd.inputLayer = inputLayer;
				after_effect_cpu_dispatch(width, height, in_data, area, bit_depth, inputLayer, output, rd);
			}
			else {
				//SSE2 (Generic x86_64)
				RenderData<mt::Simd128Float64> rd{};
				rd.width = width;
				rd.height = height;
				rd.area = area;
				rd.output = output;
				rd.inputLayer = inputLayer;
				after_effect_cpu_dispatch(width, height, in_data, area, bit_depth, inputLayer, output, rd);
			}
		}
	}
}


/*******************************************************************************************************
After Effects Smart Render Command
This render likely comes from After Effects.
*******************************************************************************************************/
void after_effects_smart_render(PF_InData* in_data, PF_OutData* out_data, PF_SmartRenderExtra* smartRender) {
	check_null(in_data);
	check_null(out_data);
	check_null(smartRender);

	const auto [width, height] = calculate_size(in_data);
	uint8_t* pixel_ptrP{};

	//Checkout the input buffer
	PF_EffectWorld* inputLayer{ nullptr };
	if constexpr (project_uses_input) {
		check_after_effects(smartRender->cb->checkout_layer_pixels(in_data->effect_ref, 0, &inputLayer));
		if (!inputLayer) throw (std::exception("Unable to checkout input layer."));
		pixel_ptrP = reinterpret_cast<uint8_t*>(inputLayer->data);
	}

	//Checkout Output buffer
	PF_EffectWorld* output{ nullptr };
	check_after_effects(smartRender->cb->checkout_output(in_data->effect_ref, &output));
	if (!output) throw (std::exception("Unable to checkout input layer."));

	//Area of interest
	PF_Rect area;
	area.left = 0;
	area.right = output->width;
	area.top = 0;
	area.bottom = output->height;
	
	

	after_effects_common_render(width, height, in_data, area, smartRender->input->bitdepth, inputLayer, output);	
}

/*******************************************************************************************************
After Effects Render Command (Old fashioned Non-Smart Mode)
This render likely comes from Premiere Pro.
*******************************************************************************************************/
void after_effects_non_smart_render(PF_InData* in_data, PF_OutData* out_data, PF_ParamDef* params[], PF_LayerDef* output) {
	check_null(in_data);
	check_null(out_data);
	check_null(output);
	check_null(params);

	const auto [width, height] = calculate_size(in_data);
	
	//Area of interest
	PF_Rect area;
	area.left = 0;
	area.right = width;
	area.top = 0;
	area.bottom = height;

	auto inputLayer = &params[0]->u.ld;
	
	int bit_depth = (output->world_flags & PF_WorldFlag_DEEP) ? 16 : 8;

	after_effects_common_render(width, height, in_data, area, bit_depth, inputLayer, output);
}
