#pragma once
//for include files that are used often but rarely changed

//For windows and graphics
#include <Windows.h>
#include <wrl.h>
#include "Utils/d3dx12.h"
#include <dxgi1_4.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>
#include <stdlib.h>  
//events
#include <pix3.h>

//utils
#include "Utils/lvUtils.h"
#include "Utils/DXSampleHelper.h"
#include "Utils/Configs.h"
#include <cstdint>

//Math 
#define GLM_FORCE_SSE2 // or GLM_FORCE_SSE42 if your processor supports it
#define GLM_FORCE_ALIGNED
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtx/euler_angles.hpp>

//Debug
#if defined(_DEBUG)
#define _CRTDBG_MAP_ALLOC  

#include <crtdbg.h> 

#define DEBUG_NEW new(_NORMAL_BLOCK, __FILE__, __LINE__)
#define new DEBUG_NEW
#include <iostream>
#else
#include <stdexcpt.h>
#include <stdexcept>
#endif

// Debug Macros

#ifdef _DEBUG

#define DEBUG_PRINT(a, ...) printf("%s: %d(): " a "\n", __FILE__, __LINE__, __VA_ARGS__);

#else

#define DEBUG_PRINT(a, ...)

#endif