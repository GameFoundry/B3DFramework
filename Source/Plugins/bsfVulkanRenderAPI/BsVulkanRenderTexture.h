//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsVulkanPrerequisites.h"
#include "Image/BsTexture.h"
#include "RenderAPI/BsRenderTexture.h"

namespace bs
{
	/** @addtogroup Vulkan
	 *  @{
	 */

	/**
	 * Vulkan implementation of a render texture.
	 *
	 * @note	Sim thread only.
	 */
	class VulkanRenderTexture : public RenderTexture
	{
	public:
		virtual ~VulkanRenderTexture() = default;

	protected:
		friend class VulkanTextureManager;

		VulkanRenderTexture(const RENDER_TEXTURE_DESC& desc);

		const RenderTargetProperties& GetPropertiesInternal() const override { return mProperties; }

		RenderTextureProperties mProperties;
	};

	namespace ct
	{
		/**
		 * Vulkan implementation of a render texture.
		 *
		 * @note	Core thread only.
		 */
		class VulkanRenderTexture : public RenderTexture
		{
		public:
			VulkanRenderTexture(const RENDER_TEXTURE_DESC& desc, u32 deviceIdx);
			~VulkanRenderTexture() override;

			void GetCustomAttribute(const String& name, void* data) const override;

		protected:
			void Initialize() override;
			const RenderTargetProperties& GetPropertiesInternal() const override { return mProperties; }

			RenderTextureProperties mProperties;
			u32 mDeviceIdx;
			VulkanFramebuffer* mFramebuffer;
		};

	} // namespace ct

	/** @} */
} // namespace bs
