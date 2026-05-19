/********************************************************************************************************

Authors:        (c) 2026 Maths Town

Licence:        The MIT License

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

Type:           colour

Description:    Umbrella include for the Maths Town colour library. Includes shared colour helpers,
                colour types, conversion member definitions, and gamut member definitions.

Includes:
                Common:               common_colour
                Operations:           colour_conversions gamut
                Types:                GenericRgb Colour8
                                      Srgb SrgbLinear Xyz XyzD50 XyzAces Cielab Cielch Oklab Oklch SrgbHsv SrgbHsl SrgbLinearHsv SrgbLinearHsl
                                      DisplayP3 DisplayP3Linear DisplayP3Hsv DisplayP3Hsl
                                      AdobeRgb AdobeRgbLinear AdobeRgbHsv AdobeRgbHsl
                                      AdobeRgbLinearHsv AdobeRgbLinearHsl
                                      ProPhotoRgb ProPhotoRgbLinear ProPhotoRgbHsv ProPhotoRgbHsl
                                      ProPhotoRgbLinearHsv ProPhotoRgbLinearHsl
                                      Rec709 Rec709Linear Rec709Hsv Rec709Hsl Rec709LinearHsv Rec709LinearHsl
                                      Rec2020 Rec2020Linear Rec2020Hsv Rec2020Hsl Rec2020LinearHsv Rec2020LinearHsl
                                      Aces2065_1 Aces2065_1Hsv Aces2065_1Hsl
                                      Acescg AcescgHsv AcescgHsl

*********************************************************************************************************/
#pragma once

#include "colour/common_colour.h"
#include "colour/generic_rgb.h"
#include "colour/colour8.h"
#include "colour/srgb.h"
#include "colour/srgb_linear.h"
#include "colour/xyz.h"
#include "colour/xyz_d50.h"
#include "colour/xyz_aces.h"
#include "colour/cielab.h"
#include "colour/cielch.h"
#include "colour/oklab.h"
#include "colour/oklch.h"
#include "colour/display_p3.h"
#include "colour/display_p3_linear.h"
#include "colour/display_p3_hsv.h"
#include "colour/display_p3_hsl.h"
#include "colour/adobe_rgb.h"
#include "colour/adobe_rgb_linear.h"
#include "colour/adobe_rgb_hsv.h"
#include "colour/adobe_rgb_hsl.h"
#include "colour/adobe_rgb_linear_hsv.h"
#include "colour/adobe_rgb_linear_hsl.h"
#include "colour/prophoto_rgb.h"
#include "colour/prophoto_rgb_linear.h"
#include "colour/prophoto_rgb_hsv.h"
#include "colour/prophoto_rgb_hsl.h"
#include "colour/prophoto_rgb_linear_hsv.h"
#include "colour/prophoto_rgb_linear_hsl.h"
#include "colour/rec709.h"
#include "colour/rec709_linear.h"
#include "colour/rec709_hsv.h"
#include "colour/rec709_hsl.h"
#include "colour/rec709_linear_hsv.h"
#include "colour/rec709_linear_hsl.h"
#include "colour/rec2020.h"
#include "colour/rec2020_linear.h"
#include "colour/rec2020_hsv.h"
#include "colour/rec2020_hsl.h"
#include "colour/rec2020_linear_hsv.h"
#include "colour/rec2020_linear_hsl.h"
#include "colour/aces2065_1.h"
#include "colour/aces2065_1_hsv.h"
#include "colour/aces2065_1_hsl.h"
#include "colour/acescg.h"
#include "colour/acescg_hsv.h"
#include "colour/acescg_hsl.h"
#include "colour/srgb_hsv.h"
#include "colour/srgb_hsl.h"
#include "colour/srgb_linear_hsv.h"
#include "colour/srgb_linear_hsl.h"
#include "colour/colour_conversions.h"
#include "colour/gamut.h"
