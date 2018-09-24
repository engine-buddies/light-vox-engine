#pragma once

#ifdef _DEBUG
#include <iostream>
#define LV_PRINT_DEBUG(x) std::cout << x << std::endl;
#else
#define LV_PRINT_DEBUG(x) 
#endif 

#define SAFE_RELEASE(ptr) if (ptr != nullptr) (ptr)->Release()
