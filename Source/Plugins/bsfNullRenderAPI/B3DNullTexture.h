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

		protected:
			friend class NullGpuDevice;

			void Initialize() override {}
			PixelData LockInternal(GpuLockOptions options, u32 mipLevel = 0, u32 face = 0) override;
			void UnlockInternal() override {}
			void CopyInternal(GpuCommandBuffer& commandBuffer, const SPtr<Texture>& target, const TextureCopyInformation& copyInformation) override {}
			void BlitInternal(GpuCommandBuffer& commandBuffer, const SPtr<Texture>& target, const TextureBlitInformation& blitInformation) override {}
			TAsyncOp<SPtr<PixelData>> ReadDataAsync(GpuCommandBuffer& commandBuffer, u32 mipLevel = 0, u32 face = 0) override;
			void ReadDataInternal(PixelData& destination, u32 mipLevel = 0, u32 face = 0, const SPtr<GpuQueue>& gpuQueue = nullptr) override {}
			void WriteDataInternal(const PixelData& source, u32 mipLevel = 0, u32 face = 0, bool discardWholeBuffer = false, const SPtr<GpuCommandBuffer>& commandBuffer = nullptr) override {}

		private:
			PixelData* mMappedBuffer = nullptr;
			String mName;
		};

		/** @} */
	} // namespace render
} // namespace b3d
