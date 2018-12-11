#pragma once
/*Our 'fake' config file*/
//TODO - make this more like a config reader rather than a bunch of #defines

#define LV_FRAME_COUNT 2		//how many frames for swap chain
#define LV_NUM_CONTEXTS 1		//for multi-threading (temp)

//color to clear with
#define LV_RTV_CLEAR_COLOR { 0.0f, 0.0f, 0.0f, 1.0f }
#define LV_RTV_CLEAR_BG_COLOR { 0.392f, 0.584f, 0.929f, 1.0f }

//maximum instance count

//recommended count & size
// count: 256    size: 8
// count: 2048   size: 16
// count: 16384  size: 32
// count: 131072 size: 64

#define LV_MAX_INSTANCE_COUNT 256
#define LV_MAX_WORLD_SIZE 8
