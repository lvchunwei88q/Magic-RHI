#pragma once

/*
* This is the base RHI configuration.
*/

// Swap chain close full screen
#ifndef RHI_SWAP_CHAIN_CLOSE_FULL_SCREEN
#define RHI_SWAP_CHAIN_CLOSE_FULL_SCREEN true
#endif

// Descriptor heap size
#ifndef RHI_DESCRIPTOR_HEAP_SIZE_STANDARD
#define RHI_DESCRIPTOR_HEAP_SIZE_STANDARD 1024
#endif

// Sampler heap size
#ifndef RHI_DESCRIPTOR_HEAP_SIZE_SAMPLER
#define RHI_DESCRIPTOR_HEAP_SIZE_SAMPLER 256
#endif

// Render target heap size
#ifndef RHI_DESCRIPTOR_HEAP_SIZE_RENDER_TARGET
#define RHI_DESCRIPTOR_HEAP_SIZE_RENDER_TARGET 32
#endif

// Depth stencil heap size
#ifndef RHI_DESCRIPTOR_HEAP_SIZE_DEPTH_STENCIL
#define RHI_DESCRIPTOR_HEAP_SIZE_DEPTH_STENCIL 32
#endif

/*
* For modern APIs, we need to manually configure multiple buffers, so you can change the value here as needed.
* Be aware that this will also create the corresponding amount of necessary resources, so bigger isn't always better.
*/
#ifndef RHI_MULTI_BUFFERING
#define RHI_MULTI_BUFFERING 2
#endif

#ifndef DRAW_MAX_RENDER_TARGETS
#define DRAW_MAX_RENDER_TARGETS 8 // Maximum number of render targets
#endif

// Enable resource debug info
#ifndef RHI_ENABLE_DEBUG_INFO 
    #ifdef _DEBUG
        #define RHI_ENABLE_DEBUG_INFO 1   // Debug: Run
    #else
        #define RHI_ENABLE_DEBUG_INFO 0   // Release: Close
    #endif
#endif