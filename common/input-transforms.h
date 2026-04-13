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

    

Types:




*******************************************************************************************************/
#include "parameters.h"
#include "simd-concepts.h"
#include "linear-algebra.h"

#include <numbers>

/**************************************************************************************************
 *
 * ************************************************************************************************/
inline void build_input_transforms_parameter_list(ParameterList& params) {
	params.add_entry(ParameterEntry::make_group_start(ParameterID::input_transform_group_start, "Input Space Transform"));

	params.add_entry(ParameterEntry::make_number(ParameterID::input_transform_translate_x, "Translate x (%)", -100000.0, 100000.0, 0.0, -100.0, 100.0, 2));
	params.add_entry(ParameterEntry::make_number(ParameterID::input_transform_translate_y, "Translate y(%)", -100000.0, 100000.0, 0.0, -100.0, 100.0, 2));
	params.add_entry(ParameterEntry::make_number(ParameterID::input_transform_scale,"Scale",0.0,10000.0,1.0,0.0,10.0,2));
	

	std::vector<std::string> input_list{};
	input_list.push_back("None");	
	input_list.push_back("Wave");
	input_list.push_back("Sqrt(r)");
	
	input_list.push_back("Abs(x,y)");
	input_list.push_back("Sqrt(Abs(x,y))");

	input_list.push_back("Complex Cosine");
	input_list.push_back("Complex Cosine Sqrt(r)");

	input_list.push_back("Cartesian to Polar");
	params.add_entry(ParameterEntry::make_list(ParameterID::input_transform_type, "Special Transform", std::move(input_list)));

	params.add_entry(ParameterEntry::make_number(ParameterID::input_transform_special1, "Special Parameter 1", -100000.0, 100000.0, 1.0, -50.0, 50.0, 2));
	params.add_entry(ParameterEntry::make_number(ParameterID::input_transform_special2, "Special Parameter 2", -100000.0, 100000.0, 1.0, -50.0, 50.0, 2));
	//params.add_entry(ParameterEntry::make_number(ParameterID::input_transform_special3, "Special Parameter 3", -100000.0, 100000.0, 0.0, -50.0, 100.0, 2));
	//params.add_entry(ParameterEntry::make_number(ParameterID::input_transform_special4, "Special Parameter 4", -100000.0, 100000.0, 0.0, -50.0, 100.0, 2));


	params.add_entry(ParameterEntry::make_group_end(ParameterID::input_transform_group_end));
}



/**************************************************************************************************
 *
 * ************************************************************************************************/
template <mt::SimdFloat S>
vec2<S> perform_input_transform(const std::string& transform_name, vec2<S> p, const ParameterList& params) {
	constexpr typename S::F pi = 3.1415926535897932384626433832795;
	
	using namespace std::numbers;



	//Pre-Transform (don't compute null-ops as they can affect image quality)
	const auto tx = params.get_value(ParameterID::input_transform_translate_x);
	if (tx != 0.0) {
		p.x += static_cast<typename S::F>(tx / 100.0f);
	}
	const auto ty = params.get_value(ParameterID::input_transform_translate_y);
	if (ty != 0.0) {
		p.y += static_cast<typename S::F>(ty / 100.0f);
	}
	const auto ts = params.get_value(ParameterID::input_transform_scale);
	if (ts != 1.0 ) {
		p *= static_cast<typename S::F>(ts);
	}
	

	const auto special1 = static_cast<typename S::F>(params.get_value(ParameterID::input_transform_special1));
	const auto special2 = static_cast<typename S::F>(params.get_value(ParameterID::input_transform_special2));

	
	if (transform_name == "None") return p;

	if (transform_name == "Wave") {
		auto x = p.x;
		auto y = p.y + 0.1f * special2* sin(2.0 * pi * p.x * special1);
		return vec2{x, y};
	}
	
	
	if (transform_name == "Abs(x,y)") {
		return abs(p);
	}

	if (transform_name == "Sqrt(Abs(x,y))") {
		return sqrt(abs(p));
	}

	if (transform_name == "Sqrt(r)") {
		auto r = p.magnitude();
		auto theta = atan2(p.y, p.x);
		r = sqrt(r);
		auto x = r * cos(theta);
		auto y = r * sin(theta);
		
		return vec2{ x, y };
	}


	if (transform_name == "Complex Cosine") {
		p += 0.000001; //Slight offset improves render
		p *= pi;
		if (special1 != 1.0) p.x *= special1;
		if (special2 != 1.0) p.x *= special2;

		auto x = cos(p.x) * cosh(p.y);
		auto y = -sin(p.x) * sinh(p.y);
		return vec2{ x, y };
	}

	if (transform_name == "Complex Cosine Sqrt(r)") {
		p += 0.000001; //Slight offset improves render
		auto r = p.magnitude();
		auto theta = atan2(p.y, p.x);
		r = sqrt(r);
		p.x = r * cos(theta);
		p.y = r * sin(theta);
		p *= pi;
		if (special1 != 1.0) p.x *= special1;
		if (special2 != 1.0) p.x *= special2;
		auto x = cos(p.x) * cosh(p.y);
		auto y = -sin(p.x) * sinh(p.y );
		return vec2{ x, y };
	}


	if (transform_name == "Cartesian to Polar") {
		auto r = p.magnitude();
		auto theta = atan2(p.y, p.x);
		return vec2{r,theta };
	}


	return p;
}


