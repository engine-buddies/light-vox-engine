#pragma once
//for include files that are used often but rarely changed

//For windows and graphics
#include <Windows.h>
#include <wrl.h>
#include "d3dx12.h"
#include <dxgi1_4.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>

//events
#include <pix3.h>

//utils
#include "lvUtils.h"
#include "DXSampleHelper.h"
#include "Configs.h"
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
#include <stdlib.h>  
#include <crtdbg.h> 

#define DEBUG_NEW new(_NORMAL_BLOCK, __FILE__, __LINE__)
#define new DEBUG_NEW
#include <iostream>
#else
#include <stdexcpt.h>
#include <stdexcept>
#endif