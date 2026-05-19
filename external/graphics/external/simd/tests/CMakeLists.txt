cmake_minimum_required(VERSION 3.20)

project(simd_tests LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(SIMD_PUBLIC_HTML_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../build/public_html")

if (NOT CMAKE_CONFIGURATION_TYPES AND NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif ()

if (NOT EMSCRIPTEN)
    set(SIMD_TEST_MODE "AVX2" CACHE STRING "SIMD test mode: SSE2, AVX2 or AVX512")
    set_property(CACHE SIMD_TEST_MODE PROPERTY STRINGS SSE2 AVX2 AVX512)

    if (NOT SIMD_TEST_MODE STREQUAL "SSE2" AND NOT SIMD_TEST_MODE STREQUAL "AVX2" AND NOT SIMD_TEST_MODE STREQUAL "AVX512")
        message(FATAL_ERROR "SIMD_TEST_MODE must be SSE2, AVX2 or AVX512")
    endif ()
endif ()

add_executable(simd_test
    main.cpp
    test_f32.cpp
    test_f64.cpp
    test_i32.cpp
    test_i64.cpp
    test_u32.cpp
    test_u64.cpp
)

target_include_directories(simd_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../include)

if (EMSCRIPTEN)
    file(MAKE_DIRECTORY ${SIMD_PUBLIC_HTML_DIR})

    target_compile_definitions(simd_test PRIVATE SIMD_TEST_MIN_LEVEL=0)
    target_compile_options(simd_test PRIVATE -Wall -Wextra -Werror -msimd128)
    target_link_options(simd_test PRIVATE -msimd128 -sEXIT_RUNTIME=1 -sALLOW_MEMORY_GROWTH=1)

    set_target_properties(simd_test PROPERTIES
        OUTPUT_NAME "simd_test"
        SUFFIX ".html"
        RUNTIME_OUTPUT_DIRECTORY "${SIMD_PUBLIC_HTML_DIR}"
    )
else ()
    if (SIMD_TEST_MODE STREQUAL "SSE2")
        target_compile_definitions(simd_test PRIVATE SIMD_TEST_MIN_LEVEL=1)
    else ()
        target_compile_definitions(simd_test PRIVATE SIMD_TEST_MIN_LEVEL=3)
    endif ()

    if (MSVC OR CMAKE_CXX_SIMULATE_ID STREQUAL "MSVC")
        target_compile_options(simd_test PRIVATE /W4 /WX /EHsc /arch:AVX2)
    elseif (CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
        target_compile_options(simd_test PRIVATE -Wall -Wextra -Werror)
        if (WIN32)
            target_compile_options(simd_test PRIVATE -mstackrealign)
            if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU" AND NOT SIMD_TEST_MODE STREQUAL "SSE2")
                target_compile_options(simd_test PRIVATE -mavx256-split-unaligned-load -mavx256-split-unaligned-store)
            endif ()
        endif ()
        if (SIMD_TEST_MODE STREQUAL "AVX512")
            target_compile_options(simd_test PRIVATE -mavx2 -mfma -mf16c -mavx512f -mavx512dq -mavx512vl -mavx512bw -mavx512cd)
        elseif (SIMD_TEST_MODE STREQUAL "AVX2")
            target_compile_options(simd_test PRIVATE -mavx2 -mfma -mf16c)
        else ()
            target_compile_options(simd_test PRIVATE -msse2 -mno-avx -mno-avx2 -mno-fma)
        endif ()
        if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
            target_compile_options(simd_test PRIVATE -Wno-psabi)
        endif ()
    endif ()
endif ()
