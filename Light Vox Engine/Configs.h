#pragma once
/*Our 'fake' config file*/

#define LV_FRAME_COUNT 2		//how many frames for swap chain
#define LV_NUM_CONTEXTS 1		//for multi-threading

//indices to the per-frame command list
#define LV_COMMAND_LIST_COUNT 3	 
#define LV_COMMAND_LIST_PRE 0
#define LV_COMMAND_LIST_MID 1
#define LV_COMMAND_LIST_POST 2

#define LV_MAX_INSTANCE_COUNT 10000