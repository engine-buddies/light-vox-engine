#pragma once
//for include files that are used often but rarely changed

//For windows and graphics
#if defined(_WIN32) || defined(_WIN64)
#include <Windows.h>
#include <wrl.h>
#include "Utils/d3dx12.h"
#include <dxgi1_4.h>
#include <d3dcompiler.h>
#include <stdlib.h>  

//events
#include <pix3.h>
#endif

#include "Utils/LVutils.h"
#include "Utils/DXSampleHelper.h"

//Math 
#define GLM_FORCE_SSE2 // or GLM_FORCE_SSE42 if your processor supports it
#define GLM_FORCE_ALIGNED
//#define GLM_FORCE_PURE
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <glm/gtx/norm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>

namespace glm
{
    typedef mat<4, 4, float, packed_highp>		mat4x4_packed;
    typedef vec<3, float, packed_highp>		vec3_packed;
    typedef vec<2, float, packed_highp>		vec2_packed;
}

//perlin noise
#include <FastNoiseSIMD.h>

//Debug
#if defined(_WIN32) || defined(_WIN64)
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
#endif

//utils
#include "Utils/Configs.h"
#include <cstdint>
