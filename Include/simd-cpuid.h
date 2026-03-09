#pragma once
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

CPUID functions. 
For run-time checking of CPU features.  
This reports "Safe to execute" rather than "CPU supported" (OS needs to be configured also to be safe).

The static variable x86_64_cpu_level will be initialised to hold a best supported microarchitecture level.

Note: Use constants in "environment.h" to check for compiler enabled CPU features.


This is x86_64 only.  
(We don't bother supporting x86_32 for SIMD code, those machines will use the fallback interfaces)

*********************************************************************************************************/


#include "environment.h"

// CPUID paths are x64-specific; other targets use fallback SIMD types.
#if MT_SIMD_ARCH_X64


#include <stdint.h>
#include <intrin.h>
#include <bitset>
#include <string>

class CpuInformation {
private:
	std::bitset<32> ecx1{}; //ecx from function 1
	std::bitset<32> edx1{}; //edx from function 1
	std::bitset<32> ebx7{}; //ebx from function 7
	std::bitset<32> ecx7{}; //ecx from function 7
	std::bitset<32> edx7{}; //edx from function 7
	std::bitset<32> eax7_1{}; //eax from function 7, subleaf 1

public:
	
	//Constructor - Performs CPUIDs and saves results.
	CpuInformation() noexcept {
		int data[4];

		//Get the number of ids
		__cpuid(data,0);
		int max_id = data[0];

		if (max_id >= 1) {
			__cpuid(data, 1);
			ecx1 = data[2];
			edx1 = data[3];
		}
		if (max_id >= 7) {
			__cpuidex(data, 7, 0);
			ebx7 = data[1];
			ecx7 = data[2];
			edx7 = data[3];			
			
			__cpuidex(data, 7, 1);
			eax7_1 = data[0];
		}
	}
	
	bool has_sse() const noexcept { return edx1[25]; }
	bool has_sse2() const noexcept { return edx1[26]; }
	bool has_sse3() const noexcept { return ecx1[0]; }
	bool has_ssse3() const noexcept { return ecx1[9]; }
	bool has_sse41() const noexcept { return ecx1[19]; }
	bool has_sse42() const noexcept { return ecx1[20]; }
	bool has_fma() const noexcept { return ecx1[12] && has_osxsave_state(); }
	bool has_osxsave_state() const noexcept {
		if (!ecx1[26] || !ecx1[27]) return false; //XSAVE + OSXSAVE bits
		const uint64_t xcr0 = get_xcr0();
		//XCR0[1]=XMM state, XCR0[2]=YMM state must both be enabled for AVX.
		return (xcr0 & 0x6) == 0x6;
	}
	bool is_avx512_safe() const noexcept {
		if (!ecx1[26] || !ecx1[27]) return false; //XSAVE + OSXSAVE bits
		const uint64_t xcr0 = get_xcr0();
		//XCR0[5]=opmask, XCR0[6]=ZMM_Hi256, XCR0[7]=Hi16_ZMM are required for AVX-512.
		return (xcr0 & 0xE6) == 0xE6;
	}
	bool has_avx() const noexcept { return ecx1[28] && has_osxsave_state(); }
	bool has_avx_nocheck() const noexcept { return ecx1[28] ; }
	bool has_f16c() const noexcept { return ecx1[29] && has_osxsave_state(); }
	
	bool has_avx2() const noexcept { return ebx7[5] && has_osxsave_state(); }
	bool has_avx512_f() const noexcept { return ebx7[16] && is_avx512_safe(); }
	bool has_avx512_f_nocheck() const noexcept { return ebx7[16]; }
	bool has_avx512_dq() const noexcept { return ebx7[17] && is_avx512_safe(); }
	bool has_avx512_ifma() const noexcept { return ebx7[21] && is_avx512_safe(); }
	bool has_avx512_pf() const noexcept { return ebx7[26] && is_avx512_safe(); }
	bool has_avx512_er() const noexcept { return ebx7[27]&& is_avx512_safe(); }
	bool has_avx512_cd() const noexcept { return ebx7[28]&& is_avx512_safe(); }
	bool has_sha() const noexcept { return ebx7[29]; }
	bool has_avx512_bw() const noexcept { return ebx7[30]&& is_avx512_safe(); }
	bool has_avx512_vl() const noexcept { return ebx7[31]&& is_avx512_safe(); }
	bool has_avx512_vbmi() const noexcept { return ecx7[1]&& is_avx512_safe(); }
	bool has_avx512_vbmi2() const noexcept { return ecx7[6]&& is_avx512_safe(); }
	bool has_avx512_gfni() const noexcept { return ecx7[8]&& is_avx512_safe(); }
	bool has_avx512_vaes() const noexcept { return ecx7[9]&& is_avx512_safe(); }
	
	bool has_avx512_vpclmulqdq() const noexcept { return ecx7[10]&& is_avx512_safe(); }
	bool has_avx512_vnni() const noexcept { return ecx7[11]&& is_avx512_safe(); }
	bool has_avx512_bitalg() const noexcept { return ecx7[12]&& is_avx512_safe(); }
	bool has_avx512_vpopcntdq() const noexcept { return ecx7[14]&& is_avx512_safe(); }
	bool has_avx512_4vnniw() const noexcept { return edx7[2]&& is_avx512_safe(); }
	bool has_avx512_4fmaps() const noexcept { return edx7[3]&& is_avx512_safe(); }
	bool has_avx512_vp2intersect() const noexcept { return edx7[8]&& is_avx512_safe(); }
	bool has_avx512_bf16() const noexcept { return eax7_1[5]&& is_avx512_safe(); }
	bool has_avx512_fp16() const noexcept { return edx7[23]&& is_avx512_safe(); }


	

	/**************************************************************************************************
	* Checks that the CPU implements x86_64 Microarchitecture level 1
	* See: https://en.wikipedia.org/wiki/X86-64#Microarchitecture_levels
	* ************************************************************************************************/
	bool is_level_1() const noexcept {
		return has_sse2() && has_sse();
	}

	/**************************************************************************************************
	* Checks that the CPU implements x86_64 Microarchitecture level 2
	* See: https://en.wikipedia.org/wiki/X86-64#Microarchitecture_levels
	* ************************************************************************************************/
	bool is_level_2() const noexcept  {
		return  has_sse42() && has_sse41() && has_sse3() && has_ssse3() &&  is_level_1();
	}

	/**************************************************************************************************
	* Checks that the CPU implements x86_64 Microarchitecture level 3
	* See: https://en.wikipedia.org/wiki/X86-64#Microarchitecture_levels
	* ************************************************************************************************/
	bool is_level_3() const noexcept {
		return has_osxsave_state() && has_avx2() && has_fma() && has_avx() && has_f16c() && is_level_2();
	}

	/**************************************************************************************************
	* Checks that the CPU implements x86_64 Microarchitecture level 4
	* See: https://en.wikipedia.org/wiki/X86-64#Microarchitecture_levels
	* ************************************************************************************************/
	bool is_level_4() const noexcept {
		return  has_avx512_bw() && has_avx512_cd() && has_avx512_dq() && has_avx512_vl() && has_avx512_f()  && is_level_3();
	}

	/**************************************************************************************************
	* Get x86_64 Microarchitecture level 4
	* See: https://en.wikipedia.org/wiki/X86-64#Microarchitecture_levels
	* ************************************************************************************************/
	int get_level() const noexcept {
		if (is_level_4()) return 4;
		if (is_level_3()) return 3;
		if (is_level_2()) return 2;
		if (is_level_1()) return 1;
		return 0;
	}




	//Returns a multiline string to show user their supported features.
	std::string to_string() const {
		std::string s{};
		s += "Has SSE                 : " + yes_no(has_sse()) + "\n";
		s += "Has SSE2                : " + yes_no(has_sse2()) + "\n";
		s += "Has SSE3                : " + yes_no(has_sse3()) + "\n";
		s += "Has SSE4.1              : " + yes_no(has_sse41()) + "\n";
		s += "Has SSE4.2              : " + yes_no(has_sse42()) + "\n";
		s += "Has FMA                 : " + yes_no(has_fma()) + "\n";
		s += "Has OSXSAVE configured  : " + yes_no(has_osxsave_state()) + "\n";
		if (has_avx_nocheck() && !has_osxsave_state()) s+="The CPU supports AVX, but the operating system does not.  All AVX/FMA/AVX2 checks will report \"No\" for safety.\n";
		s += "Has AVX                 : " + yes_no(has_avx()) + "\n";
		s += "Has AVX2                : " + yes_no(has_avx2()) + "\n";
		if (has_avx512_f_nocheck() && !is_avx512_safe()) s+= "The CPU supports AVX-512F, but the operating system does not.  All AVX-512 checks will report \"No\" for safety.\n";
		s += "Has AVX512 F            : " + yes_no(has_avx512_f()) + "\n";
		s += "Has AVX512 CD           : " + yes_no(has_avx512_cd()) + "\n";
		s += "Has AVX512 ER           : " + yes_no(has_avx512_er()) + "\n";
		s += "Has AVX512 PF           : " + yes_no(has_avx512_pf()) + "\n";
		s += "Has AVX512 4FMAPS       : " + yes_no(has_avx512_4fmaps()) + "\n";
		s += "Has AVX512 4VNNIW       : " + yes_no(has_avx512_4vnniw()) + "\n";
		s += "Has AVX512 VPOPCNTDQ    : " + yes_no(has_avx512_vpopcntdq()) + "\n";
		s += "Has AVX512 VL           : " + yes_no(has_avx512_vl()) + "\n";
		s += "Has AVX512 DQ           : " + yes_no(has_avx512_dq()) + "\n";
		s += "Has AVX512 BW           : " + yes_no(has_avx512_bw()) + "\n";
		s += "Has AVX512 IFMA         : " + yes_no(has_avx512_ifma()) + "\n";
		s += "Has AVX512 VNNI         : " + yes_no(has_avx512_vnni()) + "\n";
		s += "Has AVX512 BF16         : " + yes_no(has_avx512_bf16()) + "\n";
		s += "Has AVX512 VBMI2        : " + yes_no(has_avx512_vbmi2()) + "\n";
		s += "Has AVX512 BITALG       : " + yes_no(has_avx512_bitalg()) + "\n";
		s += "Has AVX512 VPCLMULQDQ   : " + yes_no(has_avx512_vpclmulqdq()) + "\n";
		s += "Has AVX512 GFNI         : " + yes_no(has_avx512_gfni()) + "\n";
		s += "Has AVX512 VAES         : " + yes_no(has_avx512_vaes()) + "\n";
		s += "Has AVX512 VP2INTERSECT : " + yes_no(has_avx512_vp2intersect()) + "\n";
		s += "Has AVX512 FP16         : " + yes_no(has_avx512_fp16()) + "\n";
		return s;
	}

	private:
		inline uint64_t get_xcr0() const noexcept {
		#if defined(_MSC_VER)
				return _xgetbv(0);
		#else
				uint32_t xcr0_eax = 0;
				uint32_t xcr0_edx = 0;
				__asm__ volatile(".byte 0x0f, 0x01, 0xd0" : "=a"(xcr0_eax), "=d"(xcr0_edx) : "c"(0));
				return (static_cast<uint64_t>(xcr0_edx) << 32) | xcr0_eax;
		#endif
		}
		inline std::string yes_no(bool v) const {
			return (v) ? "Yes" : "No";
		}
		
};


/**************************************************************************************************
* Stores the CPU level as a static global variable (detected at runtime).
* ************************************************************************************************/
static const int x86_64_cpu_level = CpuInformation().get_level();


#endif //x86
