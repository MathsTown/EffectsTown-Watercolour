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

A 'meta header' to include all required After Effects SDK headers.


*******************************************************************************************************/
#pragma once


/***********************************************************************************************************
Project Configuration Notes :

A Visual Studio project should have the following set to complile After Effects Effects SDK.

	"Additional Include Directories": ../../external/AfterEffectsSDK/Examples/Headers/;../../external/AfterEffectsSDK/Examples/Headers/SP;../../external/AfterEffectsSDK/Examples/Util;
	"Preprocessor Definitions": MSWindows;WIN32;_WINDOWS;

	"Struct Member Alignment": default
	(Not sure why examples default to something else, but it caused failures for me if I follow the examples)

	"Target File Extension": .aex

Setup a custom build tool for PIPL resource file "ae.r"  (put in project directory):

	"Command": 
	cl /I "$(ProjectDir)..\..\external\AfterEffectsSDK\Examples\Headers" /EP "%(Filename).r" > "$(IntDir)"\\"%(Filename).rr"
	"$(ProjectDir)..\..\external\AfterEffectsSDK\Examples\Resources\PiPLTool" "$(IntDir)%(Filename).rr" "$(IntDir)%(Filename).rrc"
	cl /D "MSWindows" /EP $(IntDir)%(Filename).rrc >               "$(ProjectDir)"\\"%(Filename)".rc

	"Outputs":
	$(ProjectDir)%(Filename).rc;

Include the output ae.rc in project

**********************************************************************************************************/

typedef unsigned char		u_char;
typedef unsigned short		u_short;
typedef unsigned short		u_int16;
typedef unsigned long		u_long;
typedef short int			int16;

#define PF_TABLE_BITS	12
#define PF_TABLE_SZ_16	4096
#define PF_DEEP_COLOR_AWARE 1	// make sure we get 16bpc pixels; 

//Supress code warning in included headers
#include <codeanalysis/warnings.h>
#pragma warning(push)
#pragma warning(disable : ALL_CODE_ANALYSIS_WARNINGS)

//These includes are from the After Effects SDK
#include "AEConfig.h"
#include "entry.h"
#include "AE_Effect.h"
#include "AE_EffectCB.h"
#include "AE_Macros.h"
#include "Param_Utils.h"
#include "AE_EffectCBSuites.h"
#include "String_Utils.h"
#include "AE_GeneralPlug.h"
#include "AEFX_ChannelDepthTpl.h"
#include "AEGP_SuiteHandler.h"

#pragma warning(pop)

//Our globally accesible variables.
extern thread_local PF_InData* globalTL_in_data;

//A helper function that checks for an error and throws if there is one.
//Note: It is very important to ensure that all AE errors get back to After Effects, and they occur often, because it includes UI stopping render.
inline void check_after_effects(PF_Err err) {
	if (err != PF_Err_NONE) throw (err);
}

//Main Entry Point
extern "C" {
	DllExport PF_Err EffectMain(PF_Cmd cmd, PF_InData* in_data, PF_OutData* out_data, PF_ParamDef* params[], PF_LayerDef* output, void* extra);
}
