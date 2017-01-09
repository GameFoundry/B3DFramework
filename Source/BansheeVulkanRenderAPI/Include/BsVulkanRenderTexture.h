//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsVulkanPrerequisites.h"
#include "BsTexture.h"
#include "BsRenderTexture.h"

namespace bs { namespace ct
{
	/** @addtogroup Vulkan
	 *  @{
	 */

	class VulkanRenderTexture;

	/**
	 * Vulkan implementation of a render texture.
	 *
	 * @note	Core thread only.
	 */
	class VulkanRenderTextureCore : public RenderTextureCore
	{
	public:
		VulkanRenderTextureCore(const RENDER_TEXTURE_DESC& desc, UINT32 deviceIdx);
		virtual ~VulkanRenderTextureCore();

		/** @copydoc RenderTextureCore::getCustomAttribute */
		void getCustomAttribute(const String& name, void* data) const override;

	protected:
		/** @copydoc CoreObjectCore::initialize() */
		void initialize() override;

		/** @copydoc RenderTextureCore::getProperties */
		const RenderTargetProperties& getPropertiesInternal() const override { return mProperties; }

		RenderTextureProperties mProperties;
		UINT32 mDeviceIdx;
		VulkanFramebuffer* mFramebuffer;
	};

	/**
	 * Vulkan implementation of a render texture.
	 *
	 * @note	Sim thread only.
	 */
	class VulkanRenderTexture : public RenderTexture
	{
	public:
		virtual ~VulkanRenderTexture() { }

	protected:
		friend class VulkanTextureManager;

		VulkanRenderTexture(const RENDER_TEXTURE_DESC& desc);

		/** @copydoc RenderTexture::getProperties */
		const RenderTargetProperties& getPropertiesInternal() const override { return mProperties; }

		RenderTextureProperties mProperties;
	};

	/** @} */
}}