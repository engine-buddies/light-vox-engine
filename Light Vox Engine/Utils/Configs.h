#pragma once
/*Our 'fake' config file*/

#define LV_FRAME_COUNT 2		//how many frames for swap chain
#define LV_NUM_CONTEXTS 1		//for multi-threading

#define LV_NUM_GBUFFER_RTV 3
#define LV_NUM_NULL_SRV LV_NUM_GBUFFER_RTV
#define LV_NUM_RTV_PER_FRAME (LV_NUM_GBUFFER_RTV + 1)
#define LV_NUM_CBVSRV_PER_FRAME (LV_NUM_GBUFFER_RTV + 1)

//indices to the per-frame command list
#define LV_COMMAND_LIST_COUNT 2
#define LV_COMMAND_LIST_INIT 0
#define LV_COMMAND_LIST_LIGHTING_PASS 1

#define LV_RTV_CLEAR_COLOR { 0.0f, 0.0f, 0.0f, 1.0f }
#define LV_RTV_CLEAR_BG_COLOR { 0.392f, 0.584f, 0.929f, 1.0f }

#define LV_MAX_INSTANCE_COUNT 100

//Root signature
#define LV_ROOT_SIGNATURE_INSTANCED_DATA 0
#define LV_ROOT_SIGNATURE_GBUFFER_SRV 1
#define LV_ROOT_SIGNATURE_CBV 2
#define LV_ROOT_SIGNATURE_SAMPLER 3

#define LV_ROOT_SIGNATURE_COUNT 4
