#pragma once
/*Our 'fake' config file*/
//TODO - make this more like a config reader rather than a bunch of #defines

#define LV_FRAME_COUNT 2		//how many frames for swap chain
#define LV_NUM_CONTEXTS 1		//for multi-threading (temp)

//color to clear with
#define LV_RTV_CLEAR_COLOR { 0.0f, 0.0f, 0.0f, 1.0f }
#define LV_RTV_CLEAR_BG_COLOR { 0.392f, 0.584f, 0.929f, 1.0f }

//maximum instance count
#define LV_MAX_INSTANCE_COUNT 150000
