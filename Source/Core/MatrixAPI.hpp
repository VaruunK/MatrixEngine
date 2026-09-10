#pragma once

#if defined(_WIN32)
    #if defined(MATRIXENGINE_EXPORTS)
        #define MATRIX_API __declspec(dllexport)
    #elif defined(MATRIXENGINE_IMPORTS)
        #define MATRIX_API __declspec(dllimport)
    #else
        #define MATRIX_API
    #endif
    
#elif defined(__GNUC__) || defined(__clang__)
    #if defined(MATRIXENGINE_EXPORTS)
        #define MATRIX_API __attribute__((visibility("default")))
    #else
        #define MATRIX_API
    #endif
#else
    #define MATRIX_API
#endif
