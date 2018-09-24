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

static const UINT kFRAME_COUNT = 2;	//how many back buffer (render targets) this uses
static const UINT kNUM_CONTEXTS = 1;

// Command list submissions from main thread.
static const int kCOMMAND_LIST_COUNT = 3;
static const int kCOMMAND_LIST_PRE = 0;
static const int kCOMMAND_LIST_MID = 1;
static const int kCOMMAND_LIST_POST = 2;