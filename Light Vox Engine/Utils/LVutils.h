#pragma once

#ifdef _DEBUG

#define DEBUG_PRINT(a, ...) printf("%s: %d(): " a "\n", __FILE__, __LINE__, __VA_ARGS__);

#else

#define DEBUG_PRINT(a, ...)

#endif

#if defined(_WIN32) || defined(_WIN64)
typedef HRESULT LV_RESULT;
#else
typedef long LV_RESULT;
#endif


#if defined(_WIN32) || defined(_WIN64)
typedef HWND LV_HANDLE;
#else
typedef void* LV_HANDLE;
#endif