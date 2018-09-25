#pragma once
//for include files that are used often but rarely changed

#if defined(_DEBUG)
#define _CRTDBG_MAP_ALLOC  
#include <stdlib.h>  
#include <crtdbg.h> 
#include <iostream>
#endif

#include <Windows.h>
#include <fstream>
#include <wrl.h>
#include "d3dx12.h"
#include <d3d12.h>
#include <dxgi1_4.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>
#include "DXSampleHelper.h"
#include <pix3.h>

#include "lvUtils.h"


#define LV_FRAME_COUNT 2		//how many frames for swap chain
#define LV_NUM_CONTEXTS 1		//for multi-threading

//indices to the per-frame command list
#define LV_COMMAND_LIST_COUNT 3	 
#define LV_COMMAND_LIST_PRE 0
#define LV_COMMAND_LIST_MID 1
#define LV_COMMAND_LIST_POST 2