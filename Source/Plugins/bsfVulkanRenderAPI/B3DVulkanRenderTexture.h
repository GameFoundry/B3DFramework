//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DVulkanPrerequisites.h"
#include "Image/B3DTexture.h"
#include "RenderAPI/B3DRenderTexture.h"

namespace b3d
{
	/** @addtogroup Vulkan
	 *  @{
	 */

	/**
	 * Vulkan implementation of a render texture.
	 *
	 * @note	Main thread only.
	 */
	class VulkanRenderTexture : public RenderTexture
	{
	public:
		virtual ~VulkanRenderTexture() = default;

	protected:
		friend class VulkanTextureManager;

		VulkanRenderTexture(const RenderTextureCreateInformation& createInformation);
	};

	namespace render
	{
		/**
		 * Vulkan implementation of a render texture.
		 *
		 * @note	Render thread only.
		 */
		class VulkanRenderTexture : public RenderTexture
		{
		public:
			VulkanRenderTexture(VulkanGpuDevice& device, const RenderTextureCreateInformation& createInformation);
			~VulkanRenderTexture() override = default;

			/** Returns the underlying framebuffer object. */
			VulkanFramebuffer* GetFramebuffer() const { return mFramebuffer; }

		protected:
			void Initialize() override;

			VulkanGpuDevice& mGpuDevice;
			VulkanFramebuffer* mFramebuffer;
		};

	} // namespace render

	/** @} */
} // namespace b3d
