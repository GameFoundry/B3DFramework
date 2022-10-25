//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsVulkanPrerequisites.h"
#include "Managers/BsTextureManager.h"

namespace bs
{
	/** @addtogroup Vulkan
	 *  @{
	 */

	/**	Handles creation of Vulkan textures. */
	class VulkanTextureManager : public TextureManager
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
		/**	Handles creation of Vulkan textures. */
		class VulkanTextureManager : public TextureManager
		{
		public:
			/** @copydoc TextureManager::onStartUp */
			void OnStartUp() override;

			/** Returns a dummy (empty) texture that can be bound in a shader slot of the requested type. */
			VulkanTexture* GetDummyTexture(GpuParamObjectType type) const;

			/**
			 * Determines Vulkan format required for binding a dummy texture (as returned by getDummyTexture()) to the shader
			 * expecting a format of type @p format.
			 */
			static VkFormat GetDummyViewFormat(GpuBufferFormat format);

		protected:
			/** @copydoc TextureManager::createTextureInternal */
			SPtr<Texture> CreateTextureInternal(const TEXTURE_DESC& desc, const SPtr<PixelData>& initialData = nullptr, GpuDeviceFlags deviceMask = GDF_DEFAULT);

			/** @copydoc TextureManager::createRenderTextureInternal */
			SPtr<RenderTexture> CreateRenderTextureInternal(const RENDER_TEXTURE_DESC& desc, u32 deviceIdx = 0);

			SPtr<VulkanTexture> mDummyReadTextures[7];
			SPtr<VulkanTexture> mDummyStorageTextures[7];
		};
	} // namespace ct

	/** @} */
} // namespace bs
