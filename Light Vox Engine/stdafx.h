#pragma once
//for include files that are used often but rarely changed

//For windows and graphics
#include <Windows.h>
#include <wrl.h>
#include "Utils/d3dx12.h"
#include <dxgi1_4.h>
#include <d3dcompiler.h>
#include <stdlib.h>  
//events
#include <pix3.h>

//Math 
#define GLM_FORCE_SSE2 // or GLM_FORCE_SSE42 if your processor supports it
#define GLM_FORCE_ALIGNED
//#define GLM_FORCE_PURE
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/euler_angles.hpp>

//typedef glm::m
namespace glm
{
    typedef mat<4, 4, float, highp>		mat4x4_packed;
    typedef vec<3, float, highp>		vec3_packed;
    typedef vec<2, float, highp>		vec2_packed;
}

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

//utils
#include "Utils/DXSampleHelper.h"
#include "Utils/Configs.h"
#include <cstdint>
#include "Utils/LVutils.h"
