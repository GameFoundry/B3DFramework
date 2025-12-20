//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DNullPrerequisites.h"
#include "Image/B3DTexture.h"

namespace b3d
{
	namespace render
	{
		class NullGpuDevice;

		/** @addtogroup Null
		 *  @{
		 */

		/**
		 * Null implementation of a texture.
		 *
		 * Stores texture properties but does not allocate GPU memory. Lock operations return fake
		 * pixel buffers that can be written to but have no effect on actual rendering.
		 */
		class NullTexture : public Texture
		{
		public:
			NullTexture(NullGpuDevice& gpuDevice, const TextureCreateInformation& createInformation);
			~NullTexture();

			void SetName(const StringView& name) override { mName = name; }
			GpuDevice& GetGpuDevice() const override { return mGpuDevice; }
			GpuQueueMask GetUseMask(u32 mipLevel, u32 arrayLayer, GpuAccessFlags accessFlags) const override { return GpuQueueMask(); }
			u32 GetBoundCount(u32 mipLevel, u32 arrayLayer) const override { return 0; }
			u32 GetUseCount(u32 mipLevel, u32 arrayLayer) const override { return 0; }

		protected:
			friend class NullGpuDevice;

			void Initialize() override {}
			GpuTextureMappedScope Map(u32 mipLevel, u32 arrayLayer, GpuMapOptions options) override;
			void CopyInternal(GpuCommandBuffer& commandBuffer, const SPtr<Texture>& target, const TextureCopyInformation& copyInformation) override {}
			void BlitInternal(GpuCommandBuffer& commandBuffer, const SPtr<Texture>& target, const TextureBlitInformation& blitInformation) override {}
			TAsyncOp<SPtr<PixelData>> ReadDataAsync(GpuCommandBuffer& commandBuffer, u32 mipLevel = 0, u32 face = 0) override;
			void ReadDataInternal(PixelData& destination, u32 mipLevel = 0, u32 face = 0, const SPtr<GpuQueue>& gpuQueue = nullptr) override {}

		private:
			NullGpuDevice& mGpuDevice;
			PixelData* mMappedBuffer = nullptr;
			String mName;
		};

		/** @} */
	} // namespace render
} // namespace b3d
