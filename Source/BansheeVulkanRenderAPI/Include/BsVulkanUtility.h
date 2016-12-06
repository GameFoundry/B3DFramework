//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsVulkanPrerequisites.h"
#include "BsPixelUtil.h"
#include "BsVertexDeclaration.h"

namespace bs
{
	/** @addtogroup Vulkan
	 *  @{
	 */

	/** Contains various helper methods for dealing with Vulkan. */
	class VulkanUtility
	{
	public:
		/** Converts between Banshee and Vulkan pixel format. */
		static VkFormat getPixelFormat(PixelFormat format, bool sRGB = false);

		/** Converts between Banshee and Vulkan buffer element format. */
		static VkFormat getBufferFormat(GpuBufferFormat format);

		/** Converts between Banshee and Vulkan vertex element types. */
		static VkFormat getVertexType(VertexElementType type);

		/**	Converts between Banshee and Vulkan texture addressing mode. */
		static VkSamplerAddressMode getAddressingMode(TextureAddressingMode mode);

		/**	Converts between Banshee and Vulkan blend factor. */
		static VkBlendFactor getBlendFactor(BlendFactor factor);

		/**	Converts between Banshee and Vulkan blend operation. */
		static VkBlendOp getBlendOp(BlendOperation op);

		/**	Converts between Banshee and Vulkan comparison operation. */
		static VkCompareOp getCompareOp(CompareFunction op);

		/**	Converts between Banshee and Vulkan cull mode. */
		static VkCullModeFlagBits getCullMode(CullingMode mode);

		/**	Converts between Banshee and Vulkan polygon mode. */
		static VkPolygonMode getPolygonMode(PolygonMode mode);

		/**	Converts between Banshee and Vulkan stencil op. */
		static VkStencilOp getStencilOp(StencilOperation op);

		/**	Converts between Banshee and Vulkan index type. */
		static VkIndexType getIndexType(IndexType op);

		/**	Converts between Banshee and Vulkan draw operation (i.e. primitive topology). */
		static VkPrimitiveTopology getDrawOp(DrawOperationType op);

		/**	Converts between Banshee and Vulkan texture filtering modes. */
		static VkFilter getFilter(FilterOptions filter);

		/**	Converts between Banshee and Vulkan texture filtering modes. */
		static VkSamplerMipmapMode getMipFilter(FilterOptions filter);

		/** Gets Vulkan flags representing the number of samples in an image. Sample count must be a power of 2. */
		static VkSampleCountFlagBits getSampleFlags(UINT32 numSamples);

		/** Gets Vulkan flags representing a certain shader stage. */
		static VkShaderStageFlagBits getShaderStage(GpuProgramType type);

		/** 
		 * Populates the provided array with Vulkan devices that correspond to provided flags. Sets null in unused slots. 
		 * Each device is placed at its own index in the output array.
		 */
		static void getDevices(const VulkanRenderAPI& rapi, GpuDeviceFlags flags, VulkanDevice* (&devices)[BS_MAX_DEVICES]);

		/** Checks is a flag for a particular device enabled. */
		static bool isDeviceIdxSet(const VulkanRenderAPI& rapi, UINT32 idx, GpuDeviceFlags flags);
	};

	/** @} */
}
