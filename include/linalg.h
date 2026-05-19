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

Type:           linalg

Description:    Umbrella include for the Maths Town linear algebra library. Includes shared helpers,
                vector types, and square matrix types.

Includes:
                Vectors:              Vec2 Vec3 Vec4 (aliased to vec2 vec3 vec4, for easy glsl porting)
                Matrices:             Mat2 Mat3 Mat4 (aliased to mat2 mat3 mat4, for easy glsl porting)

*********************************************************************************************************/
#pragma once

#include "linalg/common.h"
#include "linalg/vec2.h"
#include "linalg/vec3.h"
#include "linalg/vec4.h"
#include "linalg/mat2.h"
#include "linalg/mat3.h"
#include "linalg/mat4.h"
