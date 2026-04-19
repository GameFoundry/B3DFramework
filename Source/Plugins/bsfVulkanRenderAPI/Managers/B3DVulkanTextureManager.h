//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DVulkanPrerequisites.h"
#include "Managers/B3DTextureManager.h"

namespace b3d
{
	/** @addtogroup Vulkan
	 *  @{
	 */

	/**	Handles creation of Vulkan textures. */
	class VulkanTextureManager : public TextureManager
	{
	public:
		PixelFormat GetNativeFormat(TextureType ttype, PixelFormat format, TextureUsageFlags usage, bool hwGamma) override;

	protected:
		SPtr<RenderTexture> CreateRenderTextureImpl(const RenderTextureCreateInformation& desc) override;
	};

	namespace render
	{
		/**	Handles creation of Vulkan textures. */
		class VulkanTextureManager : public TextureManager
		{
		public:
			VulkanTextureManager(GpuDevice& gpuDevice)
				:TextureManager(gpuDevice)
			{ }

			void OnStartUp() override;

			/** Returns a dummy (empty) texture that can be bound in a shader slot of the requested type. */
			VulkanTexture* GetDummyTexture(GpuParameterObjectType type) const;

			/**
			 * Determines Vulkan format required for binding a dummy texture (as returned by getDummyTexture()) to the shader
			 * expecting a format of type @p format.
			 */
			static VkFormat GetDummyViewFormat(GpuBufferFormat format);

		protected:
			SPtr<RenderTexture> CreateRenderTextureInternal(const RenderTextureCreateInformation& desc) override;

			SPtr<VulkanTexture> mDummyReadTextures[7];
			SPtr<VulkanTexture> mDummyStorageTextures[7];
		};
	} // namespace render

	/** @} */
} // namespace b3d
