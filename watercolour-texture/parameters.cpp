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

	This is the list of parameters that will actually be displayed to the user.  
	This function and list is host-independant.

	
	Each entry must have a unique parameter-id

	After Effects requires that ID remain the same accross different versions.
	So, do not remove ununsed parameter-ids, just add new ones.

	Add ParameterIF::seed if you'd like to expose the random seed selection to user.

*******************************************************************************************************/

#include "parameters.h"
#include "parameter-id.h" 
#include "..\common\input-transforms.h"

#include <vector>
#include <string>

ParameterList build_project_parameters() {
	
	//Build working colourspace list
	/*std::vector<std::string> working_colourspace_list{};
	working_colourspace_list.push_back("Rec.709 (gamma 2.4)");
	working_colourspace_list.push_back("Rec.2020 (gamma 2.4)");
	working_colourspace_list.push_back("sRGB");*/

	
	ParameterList params;
	//params.add_entry(ParameterEntry::make_list(ParameterID::working_colour_space,"Working Colour Space", working_colourspace_list));
	

	params.add_entry(ParameterEntry::make_seed(ParameterID::seed, "Random Seed"));
	params.add_entry(ParameterEntry::make_number(ParameterID::scale, "Scale Noise",0.0000001,10000.0,1.0,0.000001,100.0,2));
	params.add_entry(ParameterEntry::make_number(ParameterID::directional_bias, "Directional Bias", -10000, 10000.0, 0.0, -100.0, 100.0, 2));

	params.add_entry(ParameterEntry::make_number(ParameterID::evolve1, "Evolve (Linear/Speed)", -10000.0, 10000.0, 1.0, 0, 100.0, 2));
	params.add_entry(ParameterEntry::make_number(ParameterID::evolve2, "Evolve (Loop)", -10000.0, 10000.0, 0.0, 0, 1, 4));

	//Input Transforms (builds from common set used in multiple projects)
	build_input_transforms_parameter_list(params);

	//Mixing space
	std::vector<std::string> mixing_colourspace_list{};
	mixing_colourspace_list.push_back("sRGB");
	mixing_colourspace_list.push_back("Rec.709 (gamma 2.4)");
	mixing_colourspace_list.push_back("Rec.2020 (gamma 2.4)");
	mixing_colourspace_list.push_back("Oklab");
	mixing_colourspace_list.push_back("Oklch");
	params.add_entry(ParameterEntry::make_list(ParameterID::mixing_colour_space, "Mixing Space (Artistic)", mixing_colourspace_list));
	

	return params;
}