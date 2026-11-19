//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DNullPrerequisites.h"
#include "GpuBackend/Allocators/B3DGpuResource.h"

namespace b3d::render
{
	class NullResourceManager;

	/** @addtogroup NullGpuBackend
	 *  @{
	 */

	/**
	 * Native resource of a NullGpuBuffer. It has no memory of its own. It gives the buffer an identity that use tracking and the transfer
	 * queue key on, like the native resources of other backends.
	 */
	class NullBufferResource : public IGpuBufferResource
	{
	public:
		NullBufferResource(NullResourceManager* owner, const StringView& name);
	};

	/**
	 * Native resource of a NullTexture. It has no memory of its own. It gives the texture an identity that use tracking and the transfer
	 * queue key on, like the native resources of other backends.
	 */
	class NullImageResource : public IGpuImageResource
	{
	public:
		NullImageResource(NullResourceManager* owner, const StringView& name, u32 faceCount, u32 mipLevelCount, GpuTextureAspectFlags aspectMask);
	};

	/** @} */
} // namespace b3d::render
