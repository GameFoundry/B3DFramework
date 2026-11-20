//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DNullResource.h"
#include "B3DNullResourceManager.h"

namespace b3d::render
{
	NullBufferResource::NullBufferResource(NullResourceManager* owner, const StringView& name, u32 size)
		: IGpuBufferResource(owner, name), mMemory(static_cast<u8*>(B3DAllocate(size)))
	{}

	NullBufferResource::~NullBufferResource()
	{
		B3DFree(mMemory);
	}

	NullImageResource::NullImageResource(NullResourceManager* owner, const StringView& name, u32 faceCount, u32 mipLevelCount, GpuTextureAspectFlags aspectMask)
		: IGpuImageResource(owner, name, faceCount, mipLevelCount, aspectMask)
	{}
} // namespace b3d::render
