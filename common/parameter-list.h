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

	Tools to build a host independant parameter list.

*********************************************************************************************************/
#pragma once

//Project Specific Includes
#include "parameter-id.h"
#include "../common/colour.h"

#include <string>
#include <vector>
#include <stdexcept>
	

/*********************************************************************************************************
Type of Parameters
*********************************************************************************************************/
enum class ParameterType {
	seed,
	number,
	percent,
	angle,
	list,
	check,
	group_start,
	group_end,
	colour,
	
};

/*********************************************************************************************************
A Parameter Entry
*********************************************************************************************************/
struct ParameterEntry {
	ParameterID id{};
	std::string name{ };
	ParameterType type {};
	double initial_value{};
	std::string contents{};
	double min{};
	double max{};
	double slider_min{};
	double slider_max{};
	short precision{};
	double value{};
	int value_integer{};
	std::string value_string {};
	std::vector<std::string> list {};
	float red{};
	float green{};
	float blue{};

	static ParameterEntry make_seed(ParameterID parameter_id, std::string parameter_name) {
		ParameterEntry p{};
		p.id = parameter_id;
		p.name = parameter_name;
		p.type = ParameterType::seed;
		p.initial_value = 0.0;
		p.min = 0.0;
		p.max = static_cast<double>(std::numeric_limits<uint32_t>::max());
		p.slider_min = p.min;
		p.slider_max = p.max;
		p.value = p.initial_value;
		return p;
	}

	static ParameterEntry make_number(ParameterID parameter_id, std::string parameter_name, double minimum = 0.0, double maximum = 1000.0, double default_value = 0.0, double slider_minimum = 0.0, double slider_maximum = 1000.00, short decimals=2) {
		ParameterEntry p{};
		p.id = parameter_id;
		p.name = parameter_name;
		p.type = ParameterType::number;
		p.initial_value = default_value;
		p.min = minimum;
		p.max = maximum;
		p.slider_min = slider_minimum;
		p.slider_max = slider_maximum;
		p.precision = decimals;
		p.value = p.initial_value;
		return p;
	}	

	static ParameterEntry make_group_start(ParameterID parameter_id, std::string parameter_name) {
		ParameterEntry p{};
		p.id = parameter_id;
		p.name = parameter_name;
		p.type = ParameterType::group_start;
		return p;
	}
	static ParameterEntry make_group_end(ParameterID parameter_id) {
		ParameterEntry p{};
		p.id = parameter_id;
		p.type = ParameterType::group_end;
		return p;
	}

	static ParameterEntry make_list(ParameterID parameter_id, std::string parameter_name, std::vector<std::string> list) {
		ParameterEntry p{};
		p.id = parameter_id;
		p.name = parameter_name;
		p.type = ParameterType::list;
		p.list = list;
		return p;
	}

	static ParameterEntry make_colour(ParameterID parameter_id, std::string parameter_name, float r, float g, float b) {
		ParameterEntry p{};
		p.id = parameter_id;
		p.name = parameter_name;
		p.type = ParameterType::colour;
		p.red = r;
		p.blue = b;
		p.green = g;
		return p;
	}

};


/*********************************************************************************************************
A List of parameters.
Simply a std::vector wrapped in a struct with helper functions.
*********************************************************************************************************/
struct ParameterList {
	std::vector<ParameterEntry> entries{};

	//Add a new entry
	void add_entry(ParameterEntry entry) {
		entries.push_back(entry);
	}

	//Get a value (as a double float)
	bool contains(ParameterID id) const {
		for (const auto& e : entries) {
			if (id == e.id) return true;
		}
		return false;
	}

	//Get a value (as a double float)
	double get_value(ParameterID id) const  {
		for (const auto & e : entries) {
			if (id == e.id) return e.value;
		}
		return 0.0;
	}
	//Get a value (as a single float)
	float get_valuef(ParameterID id) const {
		for (const auto& e : entries) {
			if (id == e.id) return static_cast<float>(e.value);
		}
		return 0.0f;
	}

	//Get a value (as an integer)
	int get_value_integer(ParameterID id) const {
		for (const auto& e : entries) {
			if (id == e.id) return e.value_integer;
		}
		return 0;
	}

	//Get a value (as a string)
	std::string get_string(ParameterID id) const {
		for (const auto & e : entries) {
			if (id == e.id) return e.value_string;
		}
		return std::string();
	}

	//Set a numerical value
	void set_value(ParameterID id, double v) {
		for (auto & e : entries) {
			if (id == e.id){
			 	e.value = v;
				return;
			}
		}
		throw(std::runtime_error("Parameter not found."));
	}

	//Set a string value
	void set_value_string(ParameterID id, std::string v) {
		for (auto & e : entries) {
			if (id == e.id){
				e.value_string = v;
				return;
			}
		}
		throw(std::runtime_error("Parameter not found."));
	}

	ColourRGBA<float> get_colour(ParameterID id) const {
		for (auto& e : entries) {
			if (id == e.id) return ColourRGBA<float>(e.red,e.green,e.blue);
		}
		throw(std::runtime_error("Parameter not found."));
	}

};
