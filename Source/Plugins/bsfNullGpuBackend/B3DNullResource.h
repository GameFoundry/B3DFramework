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
	 * Native resource of a NullGpuBuffer. It gives the buffer an identity that use tracking and the transfer queue key on, like the native
	 * resources of other backends, and owns the CPU memory of the buffer, which copies between native resources read and write.
	 */
	class NullBufferResource : public IGpuBufferResource
	{
	public:
		/** Allocates @p size bytes of CPU memory for the buffer, none if @p size is zero. */
		NullBufferResource(NullResourceManager* owner, const StringView& name, u32 size);
		~NullBufferResource() override;

		/** Returns the CPU memory of the buffer, or null if it has none, as virtual buffers do. */
		u8* GetMemory() const { return mMemory; }

	private:
		u8* mMemory = nullptr;
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
