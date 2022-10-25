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
		/** @copydoc TextureManager::getNativeFormat */
		PixelFormat GetNativeFormat(TextureType ttype, PixelFormat format, int usage, bool hwGamma);

	protected:
		/** @copydoc TextureManager::createRenderTextureImpl */
		SPtr<RenderTexture> CreateRenderTextureImpl(const RENDER_TEXTURE_DESC& desc);
	};

	namespace ct
	{
		/**	Handles creation of null textures. */
		class NullTextureManager : public TextureManager
		{
		protected:
			/** @copydoc TextureManager::createTextureInternal */
			SPtr<Texture> CreateTextureInternal(const TEXTURE_DESC& desc, const SPtr<PixelData>& initialData = nullptr, GpuDeviceFlags deviceMask = GDF_DEFAULT);

			/** @copydoc TextureManager::createRenderTextureInternal */
			SPtr<RenderTexture> CreateRenderTextureInternal(const RENDER_TEXTURE_DESC& desc, u32 deviceIdx = 0);
		};

		/**	Null implementation of a texture. */
		class NullTexture final : public Texture
		{
		public:
			NullTexture(const TEXTURE_DESC& desc, const SPtr<PixelData>& initialData, GpuDeviceFlags deviceMask);
			~NullTexture();

		protected:
			/** @copydoc Texture::lockImpl */
			PixelData LockImpl(GpuLockOptions options, u32 mipLevel = 0, u32 face = 0, u32 deviceIdx = 0, u32 queueIdx = 0);

			/** @copydoc Texture::unlockImpl */
			void UnlockImpl();

			/** @copydoc Texture::copyImpl */
			void CopyImpl(const SPtr<Texture>& target, const TEXTURE_COPY_DESC& desc, const SPtr<CommandBuffer>& commandBuffer) {}

			/** @copydoc Texture::readData */
			void ReadDataImpl(PixelData& dest, u32 mipLevel = 0, u32 face = 0, u32 deviceIdx = 0, u32 queueIdx = 0) {}

			/** @copydoc Texture::writeData */
			void WriteDataImpl(const PixelData& src, u32 mipLevel = 0, u32 face = 0, bool discardWholeBuffer = false, u32 queueIdx = 0) {}

		protected:
			PixelData* mMappedBuffer = nullptr;
		};
	} // namespace ct

	/** @} */
} // namespace bs
