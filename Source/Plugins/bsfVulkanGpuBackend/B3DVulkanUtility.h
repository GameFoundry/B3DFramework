//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DVulkanPrerequisites.h"
#include "Image/B3DPixelUtility.h"
#include "Managers/B3DVulkanTextureManager.h"
#include "Math/B3DArea2.h"
#include "GpuBackend/B3DGpuCommandBuffer.h"
#include "GpuBackend/B3DGpuQueries.h"
#include "GpuBackend/B3DVertexDescription.h"
#include "B3DVulkanResource.h"

namespace b3d
{
	namespace render
	{
		/** @addtogroup Vulkan
		 *  @{
		 */

		/** Contains various helper methods for dealing with Vulkan. */
		class VulkanUtility
		{
		public:
			/**	Finds the closest pixel format that a specific Vulkan device supports. */
			static PixelFormat GetClosestSupportedPixelFormat(const VulkanGpuDevice& device, PixelFormat format, TextureType texType, TextureUsageFlags usage, bool optimalTiling, bool hwGamma);

			/** Converts between framework's and Vulkan pixel format. */
			static VkFormat GetPixelFormat(PixelFormat format, bool sRGB = false);

			/** Converts between framework's and Vulkan buffer element format. */
			static VkFormat GetBufferFormat(GpuBufferFormat format);

			/** Converts between framework's and Vulkan vertex element types. */
			static VkFormat GetVertexType(VertexElementType type);

			/**	Converts between framework's and Vulkan texture addressing mode. */
			static VkSamplerAddressMode GetAddressingMode(TextureAddressingMode mode);

			/**
			 * Attempts to map the provided color to one of the built-in border colors. Maps to black if no better match
			 * is found.
			 */
			static VkBorderColor GetBorderColor(const Color& color);

			/**	Converts between framework's and Vulkan blend factor. */
			static VkBlendFactor GetBlendFactor(BlendFactor factor);

			/**	Converts between framework's and Vulkan blend operation. */
			static VkBlendOp GetBlendOp(BlendOperation op);

			/**	Converts between framework's and Vulkan comparison operation. */
			static VkCompareOp GetCompareOp(CompareFunction op);

			/**	Converts between framework's and Vulkan cull mode. */
			static VkCullModeFlagBits GetCullMode(CullingMode mode);

			/**	Converts between framework's and Vulkan polygon mode. */
			static VkPolygonMode GetPolygonMode(PolygonMode mode);

			/**	Converts between framework's and Vulkan stencil op. */
			static VkStencilOp GetStencilOp(StencilOperation op);

			/**	Converts between framework's and Vulkan index type. */
			static VkIndexType GetIndexType(IndexType op);

			/**	Converts between framework's and Vulkan draw operation (i.e. primitive topology). */
			static VkPrimitiveTopology GetDrawOp(DrawOperationType op);

			/**	Converts between framework's and Vulkan texture filtering modes. */
			static VkFilter GetFilter(FilterOptions filter);

			/**	Converts between framework's and Vulkan texture filtering modes. */
			static VkSamplerMipmapMode GetMipFilter(FilterOptions filter);

			/** Gets Vulkan flags representing the number of samples in an image. Sample count must be a power of 2. */
			static VkSampleCountFlagBits GetSampleFlags(u32 numSamples);

			/** Gets Vulkan flags representing a certain shader stage. */
			static VkShaderStageFlagBits GetShaderStage(GpuProgramType type);

			/** Maps a framework ImageLayout enum to Vulkan VkImageLayout. */
			static VkImageLayout GetImageLayout(ImageLayout layout);

			/** Converts a set of shader stage flags into a pipeline stage flags set containing the relevant shader stages. */
			static VkPipelineStageFlags ShaderToPipelineStage(VkShaderStageFlags shaderStageFlags);

			/** Converts a set of shader stage flags into a resource use flags containing the relevant shader stages. */
			static GpuResourceUseFlags ShaderToResourceUseFlags(VkShaderStageFlags shaderStageFlags);

			/** Converts engine flags representing GPU query type into Vulkan enum. */
			static VkQueryType GetQueryType(GpuQueryType queryType);

			/** Converts engine flags representing multiple GPU pipeline statistic query bits into a Vulkan bitmask. */
			static VkQueryPipelineStatisticFlags GetPipelineStatisticQueryBits(GpuPipelineStatisticsQueryBits bits);

			/** Converts engine texture aspect mask into VkImageAspectFlags. */
			static VkImageAspectFlags GetAspectMask(GpuTextureAspectFlags aspectMask);

			/** Converts the engine rectangle into a VkRect2D. */
			static VkRect2D ToVulkanRect(const Area2I& input);

			/** Converts the engine rectangle into a VkViewport. */
			static VkViewport ToVulkanViewport(const Area2I& input, float minDepth, float maxDepth);

			/** Converts engine texture subresource range into VkImageSubresourceRange. */
			static VkImageSubresourceRange ToVulkanImageSubresourceRange(const GpuTextureSubresourceRange& subresourceRange);

			/**
			 * Subdivides an image subresource range by cutting it with another range. If the ranges don't overlap, or the
			 * @p cutWith range completely overs the @p toCut range, the original @p toCut range is output.
			 *
			 * @param[in]	toCut		Range to cut.
			 * @param[in]	cutWith		Range to cut with.
			 * @param[out]	output		Pieces of the range that was cut.
			 * @param[out]	numAreas	Number of pieces in the @p output array.
			 */
			static void CutRange(const VkImageSubresourceRange& toCut, const VkImageSubresourceRange& cutWith, std::array<VkImageSubresourceRange, 5>& output, u32& numAreas);

			/** Checks if the two image subresource ranges have any overlapping subresources. */
			static bool RangeOverlaps(const VkImageSubresourceRange& a, const VkImageSubresourceRange& b);

			/** Checks if the two image subresource ranges are identical. */
			static bool RangeEquals(const VkImageSubresourceRange& a, const VkImageSubresourceRange& b);

			/** Converts a VkImageLayout enum value to a readable string representing the layout name. */
			static const char* GetImageLayoutName(VkImageLayout layout);

			/**
			 * Returns pipeline stages based on the resource usage flags. This allows for fine-grained control
			 * of which shader stages are involved when individual shader stage flags are used (VertexShader,
			 * FragmentShader, ComputeShader). When using the combined Shader flag or other non-shader usage
			 * flags, appropriate pipeline stages are determined automatically.
			 */
			static VkPipelineStageFlags GetPipelineStageFlags(GpuResourceUseFlags usage, VkAccessFlags accessFlags);

			/** Returns a set of pipeline stages that can are allowed to be used for the specified set of access flags. */
			static VkPipelineStageFlags GetPipelineStageFlags(VkAccessFlags accessFlags);

			/**
			 * Converts VulkanResourceAccessTypeFlags and access flags into VkPipelineStageFlags and VkAccessFlags.
			 *
			 * @param accessStage	Vulkan resource access stage flags specifying where is the resource accessed and how.
			 * @param access		Type of access (read/write) for the resource.
			 * @param outStages		Output parameter that receives the corresponding Vulkan pipeline stage flags.
			 * @param outAccessMask Output parameter that receives the corresponding Vulkan access flags.
			 */
			static void GetPipelineStageAndAccessMask(GpuStageFlags accessStage, GpuAccessFlags access, VkPipelineStageFlags& outStages, VkAccessFlags& outAccessMask);

		};

		/** @} */
	} // namespace render
} // namespace b3d
