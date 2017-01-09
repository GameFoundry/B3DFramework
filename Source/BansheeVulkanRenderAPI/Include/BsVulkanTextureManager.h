//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsVulkanPrerequisites.h"
#include "BsTextureManager.h"

namespace bs { namespace ct
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

	/**	Handles creation of Vulkan textures. */
	class VulkanTextureCoreManager : public TextureCoreManager
	{
	public:
		/** @copydoc TextureCoreManager::onStartUp */
		void onStartUp() override;

		/** Returns a image view that can be used for shader read operations when no other image is bound. */
		VkImageView getDummyReadImageView(UINT32 deviceIdx) const;

		/** Returns a image view that can be used for shader storage operations when no other image is bound. */
		VkImageView getDummyStorageImageView(UINT32 deviceIdx) const;

	protected:
		/** @copydoc TextureCoreManager::createTextureInternal */
		SPtr<TextureCore> createTextureInternal(const TEXTURE_DESC& desc, 
			const SPtr<PixelData>& initialData = nullptr, GpuDeviceFlags deviceMask = GDF_DEFAULT) override;

		/** @copydoc TextureCoreManager::createRenderTextureInternal */
		SPtr<RenderTextureCore> createRenderTextureInternal(const RENDER_TEXTURE_DESC& desc, 
			UINT32 deviceIdx = 0) override;

		SPtr<VulkanTextureCore> mDummyReadTexture;
		SPtr<VulkanTextureCore> mDummyStorageTexture;
	};

	/** @} */
}}