//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DMetalPrerequisites.h"
#include "GpuBackend/Allocators/B3DGpuResource.h"

namespace b3d::render
{
	class MetalResourceManager;

	/** @addtogroup MetalGpuBackend
	 *  @{
	 */

	/**
	 * Base class for all Metal GPU resources that need lifetime tracking. Inherits the cross-backend
	 * lifetime state machine (Notify*/Destroy/deferred-destroy) from IGpuResource. Subclasses release
	 * their id<MTL...> handle inside OnWillDestroy.
	 */
	class MetalResource : public IGpuResource
	{
	public:
		MetalResource(MetalResourceManager* owner, const StringView& name = "");
	};

	/** @} */
} // namespace b3d::render
