//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DNullTexture.h"
#include "B3DNullGpuDevice.h"
#include "Image/B3DPixelData.h"

namespace b3d
{
	namespace render
	{
		NullTexture::NullTexture(NullGpuDevice& gpuDevice, const TextureCreateInformation& createInformation, const GpuResourceLocation& location)
			: Texture(createInformation, location), mGpuDevice(gpuDevice)
		{
			if(location.IsPending())
			{
				const GpuMemoryRequirements requirements = gpuDevice.GetMemoryRequirements(createInformation);

				const bool ok = location.Allocator->TryAllocate(requirements.Size, (u32)requirements.Alignment, requirements.Kind, nullptr, mAllocation);
				B3D_ASSERT(ok && "Allocator failed to satisfy the allocation request.");
				(void)ok;
			}
			else
				mAllocation = location;
		}

		NullTexture::~NullTexture()
		{
			if(mAllocation.IsOwned())
				mAllocation.Allocator->Free(mAllocation);

			if (mMappedBuffer)
			{
				B3DDelete(mMappedBuffer);
				mMappedBuffer = nullptr;
			}
		}

		GpuTextureMappedScope NullTexture::Map(u32 mipLevel, u32 arrayLayer, GpuMapOptions options)
		{
			if (mMappedBuffer)
			{
				B3DDelete(mMappedBuffer);
				mMappedBuffer = nullptr;
			}

			const u32 mipWidth = std::max(1u, mProperties.Width >> mipLevel);
			const u32 mipHeight = std::max(1u, mProperties.Height >> mipLevel);
			const u32 mipDepth = std::max(1u, mProperties.Depth >> mipLevel);

			mMappedBuffer = B3DNew<PixelData>(mipWidth, mipHeight, mipDepth, mProperties.Format);
			mMappedBuffer->AllocateInternalBuffer();

			return GpuTextureMappedScope(
				*mMappedBuffer,
				std::static_pointer_cast<Texture>(GetShared()),
				GpuTextureSubresource(mipLevel, arrayLayer),
				options
			);
		}

	} // namespace render
} // namespace b3d
