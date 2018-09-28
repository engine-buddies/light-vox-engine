#pragma once
//for include files that are used often but rarely changed

#if defined(_DEBUG)
#define _CRTDBG_MAP_ALLOC  
#include <stdlib.h>  
#include <crtdbg.h> 
#include <iostream>
#endif

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
