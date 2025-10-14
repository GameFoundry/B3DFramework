//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DNullTexture.h"
#include "B3DNullGpuDevice.h"
#include "Image/B3DPixelData.h"

namespace b3d
{
	namespace render
	{
		NullTexture::NullTexture(NullGpuDevice& gpuDevice, const TextureCreateInformation& createInformation)
			: Texture(createInformation)
		{
			(void)gpuDevice; // Unused parameter
		}

		NullTexture::~NullTexture()
		{
			if (mMappedBuffer)
			{
				B3DDelete(mMappedBuffer);
				mMappedBuffer = nullptr;
			}
		}

		PixelData NullTexture::LockInternal(GpuLockOptions options, u32 mipLevel, u32 face)
		{
			if (mMappedBuffer)
			{
				B3DDelete(mMappedBuffer);
				mMappedBuffer = nullptr;
			}

			mMappedBuffer = B3DNew<PixelData>(mProperties.Width, mProperties.Height, mProperties.Depth, mProperties.Format);
			return *mMappedBuffer;
		}

		TAsyncOp<SPtr<PixelData>> NullTexture::ReadDataAsync(GpuCommandBuffer& commandBuffer, u32 mipLevel, u32 face)
		{
			auto pixelData = B3DMakeShared<PixelData>(mProperties.Width, mProperties.Height, mProperties.Depth, mProperties.Format);

			TAsyncOp<SPtr<PixelData>> asyncOp;
			asyncOp.CompleteOperation(pixelData);

			return asyncOp;
		}
	} // namespace render
} // namespace b3d
