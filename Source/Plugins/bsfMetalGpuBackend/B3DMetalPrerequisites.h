//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"

/** @addtogroup Plugins
 *  @{
 */

/** @defgroup MetalGpuBackend MetalGpuBackend
 *	Metal 4 render API implementation for Apple Silicon macOS 26 or newer.
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
	class MetalResidencySet;

	/** What a GpuMemoryRequirements::MemoryType reported by MetalGpuDevice stands for: the storage mode of the heaps resources are placed in. */
	enum class MetalMemoryType : u32
	{
		Private, /**< MTLStorageModePrivate, for GPU-only resources. */
		Shared, /**< MTLStorageModeShared, for CPU-visible resources. */
		Count /**< Number of memory types. */
	};
} // namespace b3d::render
