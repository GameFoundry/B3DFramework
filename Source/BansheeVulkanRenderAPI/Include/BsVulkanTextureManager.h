//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsVulkanPrerequisites.h"
#include "BsTextureManager.h"

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
		PixelFormat getNativeFormat(TextureType ttype, PixelFormat format, int usage, bool hwGamma) override;

	protected:		
		/** @copydoc TextureManager::createRenderTextureImpl */
		SPtr<RenderTexture> createRenderTextureImpl(const RENDER_TEXTURE_DESC& desc) override;
	};

	namespace ct
	{
	/**	Handles creation of Vulkan textures. */
	class VulkanTextureManager : public TextureManager
	{
	public:
		/** @copydoc TextureManager::onStartUp */
		void onStartUp() override;

		/** Returns a image view that can be used for shader operations when no other image is bound. */
		VkImageView getDummyImageView(GpuParamObjectType type, UINT32 deviceIdx) const;

	protected:
		/** @copydoc TextureManager::createTextureInternal */
		SPtr<Texture> createTextureInternal(const TEXTURE_DESC& desc, 
			const SPtr<PixelData>& initialData = nullptr, GpuDeviceFlags deviceMask = GDF_DEFAULT) override;

		/** @copydoc TextureManager::createRenderTextureInternal */
		SPtr<RenderTexture> createRenderTextureInternal(const RENDER_TEXTURE_DESC& desc, 
			UINT32 deviceIdx = 0) override;

		SPtr<VulkanTexture> mDummyReadTextures[7];
		SPtr<VulkanTexture> mDummyStorageTextures[7];
	};
	}
	/** @} */
}