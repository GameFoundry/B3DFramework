//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsNullPrerequisites.h"
#include "Image/BsTexture.h"
#include "Managers/BsTextureManager.h"

namespace b3d
{
	/** @addtogroup NullRenderAPI
	 *  @{
	 */

	/**	Handles creation of null textures. */
	class NullTextureManager final : public TextureManager
	{
	public:
		PixelFormat GetNativeFormat(TextureType ttype, PixelFormat format, int usage, bool hwGamma) override;

	protected:
		SPtr<RenderTexture> CreateRenderTextureImpl(const RENDER_TEXTURE_DESC& desc) override;
	};

	namespace render
	{
		/**	Handles creation of null textures. */
		class NullTextureManager : public TextureManager
		{
		protected:
			SPtr<Texture> CreateTextureInternal(const TextureCreateInformation& desc, const SPtr<PixelData>& initialData = nullptr, GpuDeviceFlags deviceMask = GDF_DEFAULT) override;
			SPtr<RenderTexture> CreateRenderTextureInternal(const RENDER_TEXTURE_DESC& desc, u32 deviceIdx = 0) override;
		};

		/**	Null implementation of a texture. */
		class NullTexture final : public Texture
		{
		public:
			NullTexture(const TextureCreateInformation& desc, const SPtr<PixelData>& initialData, GpuDeviceFlags deviceMask);
			~NullTexture();

		protected:
			PixelData LockInternal(GpuLockOptions options, u32 mipLevel = 0, u32 face = 0, u32 deviceIdx = 0, u32 queueIdx = 0) override;
			void UnlockInternal() override;
			void CopyInternal(const SPtr<Texture>& target, const TextureCopyInformation& desc, const SPtr<CommandBuffer>& commandBuffer) override {}
			void ReadDataInternal(PixelData& dest, u32 mipLevel = 0, u32 face = 0, u32 deviceIdx = 0, u32 queueIdx = 0) override {}
			void WriteDataInternal(const PixelData& src, u32 mipLevel = 0, u32 face = 0, bool discardWholeBuffer = false, u32 queueIdx = 0) override {}

		protected:
			PixelData* mMappedBuffer = nullptr;
		};
	} // namespace render

	/** @} */
} // namespace b3d
