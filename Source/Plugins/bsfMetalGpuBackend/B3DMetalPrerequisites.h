//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"

/**
 * Enables the explicit Metal resource-synchronization path. When disabled, resources must use
 * Metal's tracked hazard mode and the driver provides the primary resource dependency tracking.
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

// Metal framework headers are only available to Objective-C++ translation units
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

	// TODO - The backend always builds with ARC: remove the dead `#if !__has_feature(objc_arc)` blocks and MRC comments.
	// Also convert the remaining .cpp files to .mm so the whole plugin is Objective-C++, then remove the `__OBJC__` guards
	// and the void* handle aliases below, and fold Pimpls that only exist to hide Objective-C handles back into their classes

	// Objective-C handle aliases usable from both Objective-C++ (.mm) and plain C++ (.cpp) translation units
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
