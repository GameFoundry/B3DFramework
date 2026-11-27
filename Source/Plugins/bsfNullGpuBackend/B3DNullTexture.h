//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DNullPrerequisites.h"
#include "B3DNullGpuDevice.h"
#include "Image/B3DTexture.h"
#include "B3DNullResource.h"

namespace b3d
{
	namespace render
	{

		/** @addtogroup NullGpuBackend 
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
			NullTexture(NullGpuDevice& gpuDevice, const TextureCreateInformation& createInformation, const GpuAllocation& allocation, GpuObjectCreateFlags flags);
			~NullTexture();

			void SetName(const StringView& name) override { mName = name; }
			GpuDevice& GetDevice() const override { return mGpuDevice; }
			GpuQueueMask GetUseMask(u32 mipLevel, u32 arrayLayer, GpuAccessFlags accessFlags = GpuAccessFlag::Read | GpuAccessFlag::Write) const override { return GpuQueueMask(); }
			u32 GetBoundCount(u32 subresourceIdx = 0) const override { (void)subresourceIdx; return 0; }
			u32 GetUseCount(u32 subresourceIdx = 0) const override { (void)subresourceIdx; return 0; }
			IGpuResource* GetGpuResource() const override { return mImage; }

		protected:
			friend class NullGpuDevice;

			void RecreateInternalTexture() override;
			void SwapInternalTexture(Texture& other) override;
			GpuTextureMappedScope Map(u32 mipLevel, u32 arrayLayer, GpuMapOptions options) override;

		private:
			NullGpuDevice& mGpuDevice;
			PixelData* mMappedBuffer = nullptr;
			String mName;
			/** Creates a native resource for the texture. */
			NullImageResource* CreateImageResource();

			GpuAllocation mAllocation; /**< Memory the texture is placed at. Freed on destruction if owned. */
			NullImageResource* mImage = nullptr; /**< Native resource of the texture. */
		};

		/** @} */
	} // namespace render
} // namespace b3d
