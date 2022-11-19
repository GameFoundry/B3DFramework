//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsNullPrerequisites.h"
#include "Image/BsTexture.h"
#include "Managers/BsTextureManager.h"

namespace bs
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

	namespace ct
	{
		/**	Handles creation of null textures. */
		class NullTextureManager : public TextureManager
		{
		protected:
			SPtr<Texture> CreateTextureInternal(const TEXTURE_DESC& desc, const SPtr<PixelData>& initialData = nullptr, GpuDeviceFlags deviceMask = GDF_DEFAULT) override;
			SPtr<RenderTexture> CreateRenderTextureInternal(const RENDER_TEXTURE_DESC& desc, u32 deviceIdx = 0) override;
		};

		/**	Null implementation of a texture. */
		class NullTexture final : public Texture
		{
		public:
			NullTexture(const TEXTURE_DESC& desc, const SPtr<PixelData>& initialData, GpuDeviceFlags deviceMask);
			~NullTexture();

		protected:
			PixelData LockImpl(GpuLockOptions options, u32 mipLevel = 0, u32 face = 0, u32 deviceIdx = 0, u32 queueIdx = 0) override;
			void UnlockImpl() override;
			void CopyImpl(const SPtr<Texture>& target, const TextureCopyInformation& desc, const SPtr<CommandBuffer>& commandBuffer) override {}
			void ReadDataImpl(PixelData& dest, u32 mipLevel = 0, u32 face = 0, u32 deviceIdx = 0, u32 queueIdx = 0) override {}
			void WriteDataImpl(const PixelData& src, u32 mipLevel = 0, u32 face = 0, bool discardWholeBuffer = false, u32 queueIdx = 0) override {}

		protected:
			PixelData* mMappedBuffer = nullptr;
		};
	} // namespace ct

	/** @} */
} // namespace bs
