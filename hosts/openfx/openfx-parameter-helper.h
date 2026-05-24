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

	A helper class for managing OpenFX Parameters.

*******************************************************************************************************/
#pragma once

#include "openfx-helper.h"
#include "parameter-id.h"

#include <colour.h>

#include <array>
#include <climits>
#include <optional>
#include <string>
#include <vector>

class ParameterHelper {
private:
	int paramsAdded{};  ///Counter used to track parameters as they are added .
	std::array<bool, static_cast<int>(ParameterID::__last)> param_is_added; ///To translate param ID to handle
	std::array<OfxParamHandle, static_cast<int>(ParameterID::__last)> param_handle; ///To translate param ID to handle
	std::array<std::string, static_cast<int>(ParameterID::__last)> param_name; ///Name (used by OFX as the index)


	OfxParamSetHandle paramset{ nullptr };

	static std::string make_param_name(ParameterID id);
	static std::string make_group_name(ParameterID id);
	static void set_display_label(OfxPropertySetHandle param_properties, const std::string& label);
	static void set_parent_group(OfxPropertySetHandle param_properties, std::optional<ParameterID> parent_group);
	void add_to_lookup(ParameterID id, const std::string& ofx_name);

public:
	ParameterHelper();
	void set_paramset(OfxParamSetHandle p) { paramset = p; }  //Must be called at the start of each action that uses this class
	void load_handles();									  //Must be called in create instance action.

	void add_group(ParameterID id, const std::string& label, std::optional<ParameterID> parent_group, bool open);
	void add_slider(ParameterID id, const std::string& label, double min, double max, double sliderMin, double sliderMax, double value, short precision, std::optional<ParameterID> parent_group = std::nullopt);
	double read_slider(ParameterID id, OfxTime time);
	void add_integer(ParameterID id, const std::string& label, int min = INT_MIN, int max = INT_MAX, int slider_min = INT_MIN, int slider_max = INT_MAX, int value = 0, std::optional<ParameterID> parent_group = std::nullopt);

	int read_integer(ParameterID id, OfxTime time);

	void add_list(ParameterID id, const std::string& label, const std::vector<std::string>& list, std::optional<ParameterID> parent_group = std::nullopt);

	int read_list(ParameterID id, OfxTime time);

	void add_colour(ParameterID id, const std::string& label, const mt::GenericRgb<double>& colour, std::optional<ParameterID> parent_group = std::nullopt);
	mt::GenericRgb<double> read_colour(ParameterID id, OfxTime time);

	

	

};
