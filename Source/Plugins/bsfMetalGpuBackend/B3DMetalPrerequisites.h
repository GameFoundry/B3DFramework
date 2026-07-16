//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"

/**
 * Enables the explicit Metal resource-synchronization path. When disabled, resources must use
 * Metal's tracked hazard mode and the driver provides the primary resource dependency tracking.
 *
 * The explicit path is compile-time only because changing a resource's hazard mode after creation
 * is unsupported. Queue events and timeline fences are valid in both modes.
 */
#ifndef B3D_METAL_USE_EXPLICIT_RESOURCE_SYNCHRONIZATION
#define B3D_METAL_USE_EXPLICIT_RESOURCE_SYNCHRONIZATION 0
#endif

/** @addtogroup Plugins
 *  @{
 */

/** @defgroup MetalGpuBackend MetalGpuBackend
 *	Metal render API implementation for Apple Silicon macOS.
 */

/** @} */

// Metal framework headers are only available to Objective-C++ translation units. Include them here
// guarded so that plain C++ .cpp consumers of the Metal backend headers still compile while .mm
// sources get full access to the Metal types through our backend headers.
#ifdef __OBJC__
#import <Metal/Metal.h>
#import <QuartzCore/CAMetalLayer.h>
#endif

namespace b3d::render
{
	class IMetalRenderWindowSurface;
	class MetalRenderWindowSurface;
	class MetalHeadlessRenderWindowSurface;
	class MetalGpuDevice;
	class MetalGpuQueue;
	class MetalGpuCommandBuffer;

	// Objective-C handle aliases usable from both Objective-C++ (.mm) and plain C++ (.cpp)
	// translation units. Every alias has pointer/integer ABI in both compile modes; ARC controls
	// ownership only within Objective-C++ translation units. This keeps class
	// member layout and vtable layout identical across translation units. Never declare data
	// members or virtual methods behind #ifdef __OBJC__ — use these aliases instead.
#ifdef __OBJC__
	using CAMetalLayerRef = CAMetalLayer*;
	using CAMetalDrawableRef = id<CAMetalDrawable>;
	using MTLTextureRef = id<MTLTexture>;
	using MTLBufferRef = id<MTLBuffer>;
	using MTLPixelFormatValue = MTLPixelFormat;
#else
	using CAMetalLayerRef = void*;
	using CAMetalDrawableRef = void*;
	using MTLTextureRef = void*;
	using MTLBufferRef = void*;
	using MTLPixelFormatValue = unsigned long; // ABI-compatible with MTLPixelFormat (NSUInteger).
#endif
} // namespace b3d::render
