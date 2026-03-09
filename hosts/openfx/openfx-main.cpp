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

	The main entry for point for OpenFX Plug-ins
	This is a .dll (renamed .ofx)
********************************************************************************************************/


#include "config.h"
#include "parameters.h"
#include "parameter-id.h"
#include "../../common/util.h"

#include "openfx-render.h"
#include "openfx-helper.h"
#include "openfx-parameter-helper.h"
#include "openfx-instance-data.h"

#include "simd-cpuid.h"

#include <exception>
#include <string.h>


/*******************************************************************************************************
Globals
*******************************************************************************************************/
OfxHost* global_OFXHost{ nullptr };
HostData global_hostData{};
const OfxImageEffectSuiteV1* global_EffectSuite{ nullptr };
const OfxPropertySuiteV1* global_PropertySuite{ nullptr };
const OfxParameterSuiteV1* global_ParameterSuite{ nullptr };
const OfxMultiThreadSuiteV1* global_MultiThreadSuite{ nullptr };

//TODO:  To be safe we should copy this for each instance.  It should be done on the create instance action.
ParameterHelper master_parameter_helper{};

/*******************************************************************************************************
Forward Declarations
*******************************************************************************************************/
static OfxStatus openfx_plugin_main(const char* action, const void* handle, OfxPropertySetHandle inArgs, OfxPropertySetHandle outArgs) noexcept;
static void openfx_set_host_function(OfxHost* hostStruct) noexcept;
static OfxStatus openfx_describe_action(const OfxImageEffectHandle effect);
static OfxStatus openfx_on_load_action(void);
static OfxStatus openfx_describe_in_context_action(const OfxImageEffectHandle effect, OfxPropertySetHandle inArgs);
static OfxStatus openfx_image_effect_action_get_clip_preferences(const OfxImageEffectHandle effect, OfxPropertySetHandle out_args);
static OfxStatus openfx_create_instance_action(OfxImageEffectHandle instance);
static OfxStatus openfx_destroy_instance_action([[maybe_unused]] OfxImageEffectHandle effect);


/*******************************************************************************************************
A struct that provides the host with information about this OFX plugin.
*******************************************************************************************************/
static OfxPlugin pluginStruct =
{
      kOfxImageEffectPluginApi,     //API
      1,                            //API version
      PluginIdentifier,             //Plugin Unique Identifier (see Config.h)
      PluginMajorVersion,           //Plugin Major Version (see Config.h)
      PluginMinorVersion,           //Plugin Minor Version (see Config.h)
      openfx_set_host_function,     //Set callback function.
      openfx_plugin_main            //Set callback function.
};

/*******************************************************************************************************
[DLL Export]
(Mandated OFX function)
Returns the number of plug-ins in this file.
The first call from the OpenFX host. It will ask for the number of plugins in this file.
*******************************************************************************************************/
OfxExport int OfxGetNumberOfPlugins(void){

    //Check if the CPU is supported by this build
    if constexpr (mt::environment::compiler_has_avx512dq) {
        int level = CpuInformation().get_level();
        if (level < 4) {
             return 0;
        }
    }
    if constexpr (mt::environment::compiler_has_avx2) {
        int level = CpuInformation().get_level();
        if (level < 3) {
            return 0;
        }
    }
    if constexpr (mt::environment::compiler_has_sse4_2) {
        int level = CpuInformation().get_level();
        if (level < 2) {
            return 0;
        }
    }
    
    return 1;
}

/*******************************************************************************************************
[DLL Export]
(Mandated OFX function)
Returns the plugin struct, that gives the host information about this plug-in.
The host will call this once for each plug-in.  
The plug-in struct contains function pointers for those plug-ins.
*******************************************************************************************************/
OfxExport OfxPlugin* OfxGetPlugin(int nth){   
    if (nth == 0) return &pluginStruct;
    return 0;
}


/*******************************************************************************************************
(OFX Callback)
Callback called by the host to provide information about the host.
*******************************************************************************************************/
static void openfx_set_host_function(OfxHost* hostStruct) noexcept {
    global_OFXHost = hostStruct;
}

/*******************************************************************************************************
(OFX Callback)
Main entry point for calls to the plug-in.
Dispatches to another function based on the requested action.
*******************************************************************************************************/
static OfxStatus openfx_plugin_main(const char* action, const void* handle,  OfxPropertySetHandle inArgs, OfxPropertySetHandle out_args) noexcept {

    try {
        //dev_log(std::string("Action : ") + action);
        #pragma warning(suppress:26462 26493) 
        const OfxImageEffectHandle effect = (const OfxImageEffectHandle)handle;  //Effect Handle (A blind struct*)

        if (strcmp(action, kOfxImageEffectActionRender) == 0) return openfx_render(effect, inArgs);
        if (strcmp(action, kOfxActionCreateInstance) == 0) return openfx_create_instance_action(effect);
        if (strcmp(action, kOfxActionDestroyInstance) == 0) return openfx_destroy_instance_action(effect);;
        if (strcmp(action, kOfxActionLoad) == 0) return openfx_on_load_action();
        if (strcmp(action, kOfxActionDescribe) == 0) return openfx_describe_action(effect);
        if (strcmp(action, kOfxImageEffectActionDescribeInContext) == 0) return openfx_describe_in_context_action(effect, inArgs);
        if (strcmp(action, kOfxImageEffectActionGetClipPreferences) == 0) return openfx_image_effect_action_get_clip_preferences(effect, out_args);


        return kOfxStatReplyDefault;
    }
    catch (const OfxStatus) {
        dev_log("Exception (OFX Code) ");
        return kOfxStatFailed;
    }
    catch (const std::exception e) {
        dev_log(std::string("Uncaught Exception: ") + e.what());
        return kOfxStatFailed;
    }
    catch (...) {
        return kOfxStatFailed;
    }

}


/*******************************************************************************************************
"onLoad" Action.

Global startup.

We find out what features the host can support here.  We stash this info in "global_hostData".

From: https://openfx.readthedocs.io/en/master/Reference/ofxImageEffectActions.html
"This action is the first action passed to a plug-in after the binary containing the plug-in has been loaded.
It is there to allow a plug-in to create any global data structures it may need and is also when the plug-in should fetch suites from the host.
The handle, inArgs and outArgs arguments to the mainEntry are redundant and should be set to NULL."
*******************************************************************************************************/
static OfxStatus openfx_on_load_action(void) {
    dev_log("===================================================\nOnLoad Action");
    if (!global_OFXHost) return kOfxStatErrMissingHostFeature;

    //Get Suites
    global_EffectSuite = static_cast<const OfxImageEffectSuiteV1*>(global_OFXHost->fetchSuite(global_OFXHost->host, kOfxImageEffectSuite, 1));
    if (!global_EffectSuite) return kOfxStatErrMissingHostFeature;

    global_PropertySuite = static_cast<const OfxPropertySuiteV1*>(global_OFXHost->fetchSuite(global_OFXHost->host, kOfxPropertySuite, 1));
    if (!global_PropertySuite) return kOfxStatErrMissingHostFeature;

    global_ParameterSuite = static_cast<const OfxParameterSuiteV1*>(global_OFXHost->fetchSuite(global_OFXHost->host, kOfxParameterSuite, 1));
    if (!global_ParameterSuite)return kOfxStatErrMissingHostFeature;

    global_MultiThreadSuite = static_cast<const OfxMultiThreadSuiteV1*>(global_OFXHost->fetchSuite(global_OFXHost->host, kOfxMultiThreadSuite, 1));
    if (!global_ParameterSuite) return kOfxStatErrMissingHostFeature;

    char* cstr;
    int count;
    int v;

    //Collect Information about the host.
    global_PropertySuite->propGetString(global_OFXHost->host, kOfxPropLabel, 0, &global_hostData.hostLabel);
    dev_log(std::string("Host Application : ") + global_hostData.hostLabel);

    global_PropertySuite->propGetString(global_OFXHost->host, kOfxPropVersionLabel, 0, &global_hostData.hostVersion);
    dev_log(std::string("Host Version : ") + global_hostData.hostVersion);

    global_PropertySuite->propGetInt(global_OFXHost->host, kOfxImageEffectPropSupportsMultiResolution, 0, &v);
    global_hostData.supportsMultiReolution = static_cast<bool>(v);
    dev_log(std::string("Supports Multi Resolution : ") + truefalse(v));

    global_PropertySuite->propGetInt(global_OFXHost->host, kOfxImageEffectPropSupportsTiles, 0, &v);
    global_hostData.supportsTiles = static_cast<bool>(v);
    dev_log(std::string("Supports Tiles : ") + truefalse(v));

    global_PropertySuite->propGetInt(global_OFXHost->host, kOfxImageEffectPropTemporalClipAccess, 0, &v);
    global_hostData.supportsTemporalClipAccess = static_cast<bool>(v);
    dev_log(std::string("Supports Temporal Clip Access : ") + truefalse(v));

    global_PropertySuite->propGetInt(global_OFXHost->host, kOfxImageEffectPropSupportsOverlays, 0, &v);
    global_hostData.supportsOverlays = static_cast<bool>(v);
    dev_log(std::string("Supports Overlays : ") + truefalse(v));

    global_PropertySuite->propGetInt(global_OFXHost->host, kOfxImageEffectPropSupportsMultipleClipPARs, 0, &v);
    global_hostData.supportsMultipleClipAspectRatios = static_cast<bool>(v);
    dev_log(std::string("Supports Multiple Aspect Ratios : ") + truefalse(v));




    //Check Context Support
    global_PropertySuite->propGetDimension(global_OFXHost->host, kOfxImageEffectPropSupportedContexts, &count);
    for (int i = 0; i < count; i++) {
        global_PropertySuite->propGetString(global_OFXHost->host, kOfxImageEffectPropSupportedContexts, i, &cstr);
        dev_log(std::string("Context Supported : ") + cstr);
        if (strcmp(cstr, kOfxImageEffectContextGenerator) == 0) global_hostData.supportsContextGenerator = true;
        if (strcmp(cstr, kOfxImageEffectContextFilter) == 0)  global_hostData.supportsContextFilter = true;
        if (strcmp(cstr, kOfxImageEffectContextTransition) == 0) global_hostData.supportsContextTransition = true;
        if (strcmp(cstr, kOfxImageEffectContextPaint) == 0) global_hostData.supportsContextPaint = true;
        if (strcmp(cstr, kOfxImageEffectContextGeneral) == 0) global_hostData.supportsContextGeneral = true;
        if (strcmp(cstr, kOfxImageEffectContextRetimer) == 0) global_hostData.supportsContextRetimer = true;
    }
    if (!(global_hostData.supportsContextGenerator || global_hostData.supportsContextFilter || global_hostData.supportsContextGeneral)) return kOfxStatErrMissingHostFeature;


    //Check Pixel Mode Support
    global_PropertySuite->propGetDimension(global_OFXHost->host, kOfxImageEffectPropSupportedComponents, &count);
    for (int i = 0; i < count; i++) {
        global_PropertySuite->propGetString(global_OFXHost->host, kOfxImageEffectPropSupportedComponents, i, &cstr);
        dev_log(std::string("Pixel Mode Supported : ") + cstr);
        if (strcmp(cstr, kOfxImageComponentRGBA) == 0) global_hostData.supportsComponentRGBA = true;
        if (strcmp(cstr, kOfxImageComponentRGB) == 0)  global_hostData.supportsComponentRGB = true;
        if (strcmp(cstr, kOfxImageComponentAlpha) == 0) global_hostData.supportsComponentA = true;
    }
    if (!(global_hostData.supportsComponentRGBA || global_hostData.supportsComponentRGB)) return kOfxStatErrMissingHostFeature;

    global_PropertySuite->propGetInt(global_OFXHost->host, kOfxImageEffectPropSupportsMultipleClipDepths, 0, &v);
    global_hostData.supportsMultipleClipDepths = static_cast<bool>(v);
    dev_log(std::string("Supports Multiple Clip Depths : ") + truefalse(v));


    //Check Bit Depth.
    //Not technically a supported call here according to https://openfx.readthedocs.io/en/master/Reference/ofxPropertiesByObject.html#properties-on-the-image-effect-host
    //However, it seems to be supported by hosts, so we will read it if there is data.
    global_PropertySuite->propGetDimension(global_OFXHost->host, kOfxImageEffectPropSupportedPixelDepths, &count);
    if (count > 0) {
        for (int i = 0; i < count; i++) {
            global_PropertySuite->propGetString(global_OFXHost->host, kOfxImageEffectPropSupportedPixelDepths, i, &cstr);
            dev_log(std::string("Pixel Depth Supported : ") + cstr);
            if (strcmp(cstr, kOfxBitDepthByte) == 0) global_hostData.supportsBitDepthByte = true;
            if (strcmp(cstr, kOfxBitDepthShort) == 0)  global_hostData.supportsBitDepthShort = true;
            if (strcmp(cstr, kOfxBitDepthHalf) == 0) global_hostData.supportsBitDepthHalf = true;
            if (strcmp(cstr, kOfxBitDepthFloat) == 0) global_hostData.supportsBitDepthFloat = true;
        }
        if (!(global_hostData.supportsBitDepthByte || global_hostData.supportsBitDepthFloat)) return kOfxStatErrMissingHostFeature;
    }

    return kOfxStatOK;
}




/*******************************************************************************************************
"describe" Action.

It is our turn to tell the host what we can support.

From: https://openfx.readthedocs.io/en/master/Reference/ofxImageEffectActions.html
"The kOfxActionDescribe is the second action passed to a plug-in. It is where a plugin defines how it behaves and the resources it needs to function.
Note that the handle passed in acts as a descriptor for, rather than an instance of the plugin. The handle is global and unique. 
The plug-in is at liberty to cache the handle away for future reference until the plug-in is unloaded.
Most importantly, the effect must set what image effect contexts it is capable of working in.
This action must be trapped, it is not optional."
*******************************************************************************************************/
static OfxStatus openfx_describe_action(const OfxImageEffectHandle effect) {
    dev_log("Describe Action");
    //Get the effect's properties structure. (This is a blind structure manipulated with the PropertySuite).
    OfxPropertySetHandle effectProperties;
    check_openfx(global_EffectSuite->getPropertySet(effect, &effectProperties));



    //Set the effects properties.
    check_openfx(global_PropertySuite->propSetString(effectProperties, kOfxPropLabel, 0, PluginName));                                        //Plug-in name
    check_openfx(global_PropertySuite->propSetString(effectProperties, kOfxImageEffectPluginPropGrouping, 0, PluginMenu));                    //Plug-in menu location
    
    check_openfx(global_PropertySuite->propSetString(effectProperties, kOfxImageEffectPluginRenderThreadSafety, 0, kOfxImageEffectRenderFullySafe)); //We are going fully thread-safe
    
    check_openfx(global_PropertySuite->propSetInt(effectProperties, kOfxImageEffectPropSupportsTiles, 0, true));                             
    check_openfx(global_PropertySuite->propSetInt(effectProperties, kOfxImageEffectPluginPropHostFrameThreading, 0, true));
    check_openfx(global_PropertySuite->propSetInt(effectProperties, kOfxImageEffectPluginPropFieldRenderTwiceAlways, 0, false));
    check_openfx(global_PropertySuite->propSetInt(effectProperties, kOfxImageEffectPropSupportsMultiResolution, 0, false));


    //Indicate which bit depths we can support.
    check_openfx(global_PropertySuite->propSetInt(effectProperties, kOfxImageEffectPropSupportsMultipleClipDepths, 0, false));                //Multiple Bit Depths
    //check_openfx(global_PropertySuite->propSetString(effectProperties, kOfxImageEffectPropSupportedPixelDepths, 0, kOfxBitDepthByte));        //8 Bit Colour
    check_openfx(global_PropertySuite->propSetString(effectProperties, kOfxImageEffectPropSupportedPixelDepths, 2, kOfxBitDepthFloat));       //32 Bit Float Colour

    // define the contexts we can be used in
    if constexpr (project_is_generator) check_openfx(global_PropertySuite->propSetString(effectProperties, kOfxImageEffectPropSupportedContexts, 0, kOfxImageEffectContextGenerator));  //Support Generator context
    if constexpr (project_uses_input) check_openfx(global_PropertySuite->propSetString(effectProperties, kOfxImageEffectPropSupportedContexts, 1, kOfxImageEffectContextFilter));     //Support Effect Context
    check_openfx(global_PropertySuite->propSetString(effectProperties, kOfxImageEffectPropSupportedContexts, 2, kOfxImageEffectContextGeneral));    //General Effect Context
    return kOfxStatOK;
}


/*******************************************************************************************************
Add the parameters to the project
*******************************************************************************************************/
void add_parameters(const OfxImageEffectHandle effect) {
    //Get the effect's parameter set and store it in our helper class.
    OfxParamSetHandle paramset;
    global_EffectSuite->getParamSet(effect, &paramset);
    master_parameter_helper.set_paramset(paramset);

    //Get parameters from project
    auto params = build_project_parameters();

    //Build After Effect Parameters
    for (auto p : params.entries) {
        switch (p.type) {
        case ParameterType::seed:
            //master_parameter_helper.add_slider(p.id, p.name, static_cast<float>(p.min), static_cast<float>(p.max), static_cast<float>(p.slider_min), static_cast<float>(p.slider_max), static_cast<float>(p.initial_value), 0);
            master_parameter_helper.add_integer(p.id, p.name);
            break;
        case ParameterType::number:
            master_parameter_helper.add_slider(p.id, p.name, static_cast<float>(p.min), static_cast<float>(p.max), static_cast<float>(p.slider_min), static_cast<float>(p.slider_max), static_cast<float>(p.initial_value), p.precision);
            break;

        case ParameterType::list:
            master_parameter_helper.add_list(p.id,p.name,p.list);
            break;

        default:
            break;
        }
    }
}

/*******************************************************************************************************
"describeInContext" Action.

Describe what each specific context can support. (Input & Output format specifications.)

Specify parameters in this action.

Usually called when effect is actually applied.

From: https://openfx.readthedocs.io/en/master/Reference/ofxImageEffectActions.html
"This action is unique to OFX Image Effect plug-ins. Because a plugin is able to exhibit different behaviour depending on the context of use,
each separate context will need to be described individually. It is within this action that image effects describe which parameters and input clips it requires.
This action will be called multiple times, one for each of the contexts the plugin says it is capable of implementing. If a host does not support a certain context,
then it need not call kOfxImageEffectActionDescribeInContext for that context."
*******************************************************************************************************/
static OfxStatus openfx_describe_in_context_action(const OfxImageEffectHandle effect, OfxPropertySetHandle inArgs) {
    dev_log("DescribeInContext Action");

    //Find out which context we are setting up.
    char* contextString;
    OFXContext context;
    check_openfx(global_PropertySuite->propGetString(inArgs, kOfxImageEffectPropContext, 0, &contextString));
    context = GetContextFromString(contextString);
    if (context == OFXContext::invalid) return kOfxStatFailed;
    dev_log(std::string("Context is ") + contextString);


    //A properties structure (blind struct, manipulated with PropertySuite)
    OfxPropertySetHandle properties;

    //Define the mandated output clip for all contexts
    dev_log("Adding Output Clip");
    check_openfx(global_EffectSuite->clipDefine(effect, "Output", &properties));
    if (global_hostData.supportsComponentRGBA) check_openfx(global_PropertySuite->propSetString(properties, kOfxImageEffectPropSupportedComponents, 0, kOfxImageComponentRGBA));  //RGBA format 
    //if (global_hostData.supportsComponentRGB) check_openfx(global_PropertySuite->propSetString(properties, kOfxImageEffectPropSupportedComponents, 1, kOfxImageComponentRGB));   //RGB format 


    
    //We only have an input clip for general and filter contexts.
    if constexpr (project_uses_input) {
        if (context == OFXContext::filter || context == OFXContext::general) {
            dev_log("Adding Input Clip");
            check_openfx(global_EffectSuite->clipDefine(effect, "Source", &properties));
            if (global_hostData.supportsComponentRGBA) check_openfx(global_PropertySuite->propSetString(properties, kOfxImageEffectPropSupportedComponents, 0, kOfxImageComponentRGBA)); //RGBA format
            //if (global_hostData.supportsComponentRGB) check_openfx(global_PropertySuite->propSetString(properties, kOfxImageEffectPropSupportedComponents, 1, kOfxImageComponentRGB)); //RGB format
        }
    }
    

    add_parameters(effect);
    


    

    //dev_log("DescribeInContext Action Complete");
    return kOfxStatOK;
}


/*******************************************************************************************************
"ImageEffectActionGetClipPreferences" Action.

Tell the host our preferred formats.

From: https://openfx.readthedocs.io/en/master/Reference/ofxImageEffectActions.html#group__ImageEffectActions_1gae66560a2d269b17e160613adf22fd7d0
"This action allows a plugin to dynamically specify its preferences for input and output clips. 
Please see Image Effect Clip Preferences for more details on the behaviour. Clip preferences are 
constant for the duration of an effect, so this action need only be called once per clip, not once per frame.
This should be called once after creation of an instance, each time an input clip is changed, and 
whenever a parameter named in the kOfxImageEffectPropClipPreferencesSlaveParam has its value changed."

*******************************************************************************************************/
static OfxStatus openfx_image_effect_action_get_clip_preferences([[maybe_unused]] const OfxImageEffectHandle effect, OfxPropertySetHandle out_args) {
    
    //Set preferred pre-multiplication state
    if constexpr (project_is_solid_render) {
        check_openfx(global_PropertySuite->propSetString(out_args, kOfxImageEffectPropPreMultiplication, 0, kOfxImageOpaque));
    }
    else {
        check_openfx(global_PropertySuite->propSetString(out_args, kOfxImageEffectPropPreMultiplication, 0, kOfxImageUnPreMultiplied));
    }

    //Continuous sampling (can generate frames between frames).
    check_openfx(global_PropertySuite->propSetInt(out_args, kOfxImageClipPropContinuousSamples, 0, 1 /*true*/));

    //Has time varying effect even in parameters constant?
    check_openfx(global_PropertySuite->propSetInt(out_args, kOfxImageEffectFrameVarying, 0, 0 /*false*/));   


    return kOfxStatOK;
}

/*******************************************************************************************************
"CreateInstance" Action.
*******************************************************************************************************/
static OfxStatus openfx_create_instance_action(OfxImageEffectHandle instance) {
    
    //Create new instance data object on heap
    InstanceData* instance_data = new InstanceData();
    
    //Copy parameter information into instance. 
    //Each instance will have its own handels and data, so we need a separate copy for each instance.
    instance_data->parameter_helper = master_parameter_helper;

    //Load all the handels for the parameters
    OfxParamSetHandle paramset;
    global_EffectSuite->getParamSet(instance, &paramset);
    instance_data->parameter_helper.set_paramset(paramset);
    instance_data->parameter_helper.load_handles();

    //Send a point to the host.
    OfxPropertySetHandle effectProps;
    global_EffectSuite->getPropertySet(instance, &effectProps);
    global_PropertySuite->propSetPointer(effectProps, kOfxPropInstanceData, 0, (void*)instance_data);

    return kOfxStatOK;
}

/*******************************************************************************************************
"DestroyInstance" Action.
*******************************************************************************************************/
static OfxStatus openfx_destroy_instance_action( OfxImageEffectHandle instance) {
    InstanceData* instance_data {nullptr};

    //Get the pointer to the instance data from the host.
    OfxPropertySetHandle effectProps;
    global_EffectSuite->getPropertySet(instance, &effectProps);
    global_PropertySuite->propGetPointer(effectProps, kOfxPropInstanceData, 0, (void**) &instance_data);

    //Release the instance data
    delete instance_data;

    return kOfxStatOK;
}