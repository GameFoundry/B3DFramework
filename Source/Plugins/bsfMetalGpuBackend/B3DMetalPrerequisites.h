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

#import <Metal/Metal.h>
#import <QuartzCore/CAMetalLayer.h>

namespace b3d::render
{
	class IMetalRenderWindowSurface;
	class MetalRenderWindowSurface;
	class MetalHeadlessRenderWindowSurface;
	class MetalGpuDevice;
	class MetalGpuQueue;
	class MetalGpuCommandBuffer;
} // namespace b3d::render
