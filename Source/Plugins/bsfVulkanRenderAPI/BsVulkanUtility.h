//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsVulkanPrerequisites.h"
#include "Image/BsPixelUtil.h"
#include "RenderAPI/BsVertexDeclaration.h"

namespace bs { namespace ct
{
	/** @addtogroup Vulkan
	 *  @{
	 */

	/** Contains various helper methods for dealing with Vulkan. */
	class VulkanUtility
	{
	public:
		/**	Finds the closest pixel format that a specific Vulkan device supports. */
		static PixelFormat getClosestSupportedPixelFormat(const VulkanDevice& device, PixelFormat format,
			TextureType texType, int usage, bool optimalTiling, bool hwGamma);

		/** Converts between framework's and Vulkan pixel format. */
		static VkFormat getPixelFormat(PixelFormat format, bool sRGB = false);

		/** Converts between framework's and Vulkan buffer element format. */
		static VkFormat getBufferFormat(GpuBufferFormat format);

		/** Converts between framework's and Vulkan vertex element types. */
		static VkFormat getVertexType(VertexElementType type);

		/**	Converts between framework's and Vulkan texture addressing mode. */
		static VkSamplerAddressMode getAddressingMode(TextureAddressingMode mode);

		/** 
		 * Attempts to map the provided color to one of the built-in border colors. Maps to black if no better match
		 * is found.
		 */
		static VkBorderColor getBorderColor(const Color& color);

		/**	Converts between framework's and Vulkan blend factor. */
		static VkBlendFactor getBlendFactor(BlendFactor factor);

		/**	Converts between framework's and Vulkan blend operation. */
		static VkBlendOp getBlendOp(BlendOperation op);

		/**	Converts between framework's and Vulkan comparison operation. */
		static VkCompareOp getCompareOp(CompareFunction op);

		/**	Converts between framework's and Vulkan cull mode. */
		static VkCullModeFlagBits getCullMode(CullingMode mode);

		/**	Converts between framework's and Vulkan polygon mode. */
		static VkPolygonMode getPolygonMode(PolygonMode mode);

		/**	Converts between framework's and Vulkan stencil op. */
		static VkStencilOp getStencilOp(StencilOperation op);

		/**	Converts between framework's and Vulkan index type. */
		static VkIndexType getIndexType(IndexType op);

		/**	Converts between framework's and Vulkan draw operation (i.e. primitive topology). */
		static VkPrimitiveTopology getDrawOp(DrawOperationType op);

		/**	Converts between framework's and Vulkan texture filtering modes. */
		static VkFilter getFilter(FilterOptions filter);

		/**	Converts between framework's and Vulkan texture filtering modes. */
		static VkSamplerMipmapMode getMipFilter(FilterOptions filter);

		/** Gets Vulkan flags representing the number of samples in an image. Sample count must be a power of 2. */
		static VkSampleCountFlagBits getSampleFlags(UINT32 numSamples);

		/** Gets Vulkan flags representing a certain shader stage. */
		static VkShaderStageFlagBits getShaderStage(GpuProgramType type);

		/** Converts a set of shader stage flags into a pipeline stage flags set containing the relevant shader stages. */
		static VkPipelineStageFlags shaderToPipelineStage(VkShaderStageFlags shaderStageFlags);

		/**
		 * Populates the provided array with Vulkan devices that correspond to provided flags. Sets null in unused slots. 
		 * Each device is placed at its own index in the output array.
		 */
		static void getDevices(const VulkanRenderAPI& rapi, GpuDeviceFlags flags, VulkanDevice* (&devices)[BS_MAX_DEVICES]);

		/** Checks is a flag for a particular device enabled. */
		static bool isDeviceIdxSet(const VulkanRenderAPI& rapi, UINT32 idx, GpuDeviceFlags flags);

		/** 
		 * Subdivides an image subresource range by cutting it with another range. If the ranges don't overlap, or the
		 * @p cutWith range completely overs the @p toCut range, the original @p toCut range is output. 
		 * 
		 * @param[in]	toCut		Range to cut.
		 * @param[in]	cutWith		Range to cut with.
		 * @param[out]	output		Pieces of the range that was cut.
		 * @param[out]	numAreas	Number of pieces in the @p output array.
		 */
		static void cutRange(const VkImageSubresourceRange& toCut, const VkImageSubresourceRange& cutWith,
							 std::array<VkImageSubresourceRange, 5>& output, UINT32& numAreas);

		/** Checks if the two image subresource ranges have any overlapping subresources. */
		static bool rangeOverlaps(const VkImageSubresourceRange& a, const VkImageSubresourceRange& b);

		/**
		 * Calculates the size and alignment of a single element within a shader interface block using the std140 layout.
		 *
		 * @param[in]		type		Type of the element. Structs are not supported.
		 * @param[in]		arraySize	Number of array elements of the element (1 if it's not an array).
		 * @param[in, out]	offset		Current location in some parent buffer at which the element should be placed at. If the
		 *								location doesn't match the element's alignment, the value will be modified to a valid
		 *								alignment. In multiples of 4 bytes.
		 * @return						Size of the element, in multiples of 4 bytes.
		 */
		static UINT32 calcInterfaceBlockElementSizeAndOffset(GpuParamDataType type, UINT32 arraySize, UINT32& offset);
	};

	/** @} */
}}