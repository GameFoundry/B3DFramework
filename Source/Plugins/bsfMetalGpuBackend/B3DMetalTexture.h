//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DMetalPrerequisites.h"
#include "B3DMetalGpuDevice.h"
#include "Image/B3DTexture.h"

namespace b3d
{
	namespace render
	{

		/** @addtogroup MetalGpuBackend
		 *  @{
		 */

		/**
		 * Metal implementation of a texture.
		 *
		 * Owns a single private-storage @c MTLTexture. Phase-2 Metal textures are not directly
		 * mappable, so @c Map returns an invalid @c GpuTextureMappedScope and callers are expected
		 * to route CPU traffic through @c TextureUtility::Write / @c TextureUtility::Read, which
		 * drives @c CopyBufferToTexture / @c CopyTextureToBuffer on the command buffer.
		 */
		class MetalTexture : public Texture
		{
		public:
			MetalTexture(MetalGpuDevice& gpuDevice, const TextureCreateInformation& createInformation);
			~MetalTexture();

			void SetName(const StringView& name) override;
			GpuDevice& GetDevice() const override { return mGpuDevice; }
			GpuQueueMask GetUseMask(u32 mipLevel, u32 arrayLayer, GpuAccessFlags accessFlags = GpuAccessFlag::Read | GpuAccessFlag::Write) const override { return GpuQueueMask(); }
			u32 GetBoundCount(u32 subresourceIdx = 0) const override { (void)subresourceIdx; return 0; }
			u32 GetUseCount(u32 subresourceIdx = 0) const override { (void)subresourceIdx; return 0; }
			void Flush(u32 mipLevel, u32 arrayLayer) override;

#ifdef __OBJC__
			/** Returns the underlying MTLTexture. May be nil if Initialize() failed or has not been called yet. */
			id<MTLTexture> GetMetalTexture() const;

			/**
			 * Returns a lazily-created @c MTLTexture view that reinterprets the storage with a different
			 * pixel format. Typical use is sampling the depth plane of a combined depth-stencil texture
			 * from a shader. Views are cached for the lifetime of the texture.
			 */
			id<MTLTexture> GetShaderReadView(MTLPixelFormat viewFormat);
#endif

		protected:
			friend class MetalGpuDevice;

			void Initialize() override;
			void RecreateInternalTexture() override;
			GpuTextureMappedScope Map(u32 mipLevel, u32 arrayLayer, GpuMapOptions options) override;

		private:
			/** Creates/recreates the underlying MTLTexture based on the current properties. */
			void CreateInternalTexture();

			/** Releases the underlying MTLTexture, if any. */
			void ReleaseInternalTexture();

			struct Impl;

			MetalGpuDevice& mGpuDevice;
			UPtr<Impl> mImpl;
			// Note: no local @c mName — we inherit @c Texture::mName and its @c GetName accessor,
			// delegating the store through @c Texture::SetName in our override.
		};

		/** @} */
	} // namespace render
} // namespace b3d
