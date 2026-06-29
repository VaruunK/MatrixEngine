#pragma once

#ifdef MATRIXENGINE_EXPORTS
#define MATRIX_API __declspec(dllexport)
#else
#define MATRIX_API __declspec(dllimport)
#endif