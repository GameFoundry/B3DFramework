//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsVulkanUtility.h"
#include "BsVulkanRenderAPI.h"
#include "BsVulkanDevice.h"
#include "Error/BsException.h"
#include "RenderAPI/BsGpuParams.h"

namespace bs { namespace ct
{
	PixelFormat VulkanUtility::getClosestSupportedPixelFormat(const VulkanDevice& device, PixelFormat format,
		TextureType texType, int usage, bool optimalTiling, bool hwGamma)
	{
		// Check for any obvious issues first
		PixelUtil::checkFormat(format, texType, usage);

		// Check actual device for format support
		VkFormatFeatureFlags wantedFeatureFlags = VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT;
		if ((usage & TU_RENDERTARGET) != 0)
			wantedFeatureFlags |= VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT;

		if ((usage & TU_DEPTHSTENCIL) != 0)
			wantedFeatureFlags |= VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT;

		if ((usage & TU_LOADSTORE) != 0)
			wantedFeatureFlags |= VK_FORMAT_FEATURE_STORAGE_IMAGE_BIT;

		VkFormatProperties props;
		auto isSupported = [&](VkFormat vkFmt)
		{
			vkGetPhysicalDeviceFormatProperties(device.getPhysical(), vkFmt, &props);
			VkFormatFeatureFlags featureFlags = optimalTiling ? props.optimalTilingFeatures : props.linearTilingFeatures;

			return (featureFlags & wantedFeatureFlags) != 0;
		};

		VkFormat vkFormat = getPixelFormat(format, hwGamma);
		if(!isSupported(vkFormat))
		{
			if ((usage & TU_DEPTHSTENCIL) != 0)
			{
				bool hasStencil = format == PF_D24S8 || format == PF_D32_S8X24;

				// Spec guarantees at least one depth-only, and one depth-stencil format to be supported
				if(hasStencil)
				{
					if (isSupported(VK_FORMAT_D32_SFLOAT_S8_UINT))
						format = PF_D32_S8X24;
					else
						format = PF_D24S8;

					// We ignore 8-bit stencil-only, and 16/8 depth/stencil combo buffers as engine doesn't expose them,
					// and spec guarantees one of the above must be implemented.
				}
				else
				{
					// The only format that could have failed is 32-bit depth, so we must use the alternative 16-bit.
					// Spec guarantees it is always supported.
					format = PF_D16;
				}
			}
			else
			{
				int bitDepths[4];
				PixelUtil::getBitDepths(format, bitDepths);

				if (bitDepths[0] == 16) // 16-bit format, fall back to 4-channel 16-bit, guaranteed to be supported
					format = PF_RGBA16F;
				else if(format == PF_BC6H) // Fall back to uncompressed alternative
					format = PF_RGBA16F;
				else // Must be 8-bit per channel format, compressed format or some uneven format
					format = PF_RGBA8;
			}
		}

		return format;
	}

	VkFormat VulkanUtility::getPixelFormat(PixelFormat format, bool sRGB)
	{
		switch (format)
		{
		case PF_R8:
			if(sRGB)
				return VK_FORMAT_R8_SRGB;

			return VK_FORMAT_R8_UNORM;
		case PF_RG8:
			if (sRGB)
				return VK_FORMAT_R8G8_SRGB;

			return VK_FORMAT_R8G8_UNORM;
		case PF_RGB8:
			if (sRGB)
				return VK_FORMAT_R8G8B8_SRGB;

			return VK_FORMAT_R8G8B8_UNORM;
		case PF_RGBA8:
			if (sRGB)
				return VK_FORMAT_R8G8B8A8_SRGB;

			return VK_FORMAT_R8G8B8A8_UNORM;
		case PF_BGRA8:
			if (sRGB)
				return VK_FORMAT_B8G8R8A8_SRGB;

			return VK_FORMAT_B8G8R8A8_UNORM;
		case PF_R8I:
			return VK_FORMAT_R8_SINT;
		case PF_RG8I:
			return VK_FORMAT_R8G8_SINT;
		case PF_RGBA8I:
			return VK_FORMAT_R8G8B8A8_SINT;
		case PF_R8U:
			return VK_FORMAT_R8_UINT;
		case PF_RG8U:
			return VK_FORMAT_R8G8_UINT;
		case PF_RGBA8U:
			return VK_FORMAT_R8G8B8A8_UINT;
		case PF_R8S:
			return VK_FORMAT_R8_SNORM;
		case PF_RG8S:
			return VK_FORMAT_R8G8_SNORM;
		case PF_RGBA8S:
			return VK_FORMAT_R8G8B8A8_SNORM;
		case PF_R16F:
			return VK_FORMAT_R16_SFLOAT;
		case PF_RG16F:
			return VK_FORMAT_R16G16_SFLOAT;
		case PF_RGBA16F:
			return VK_FORMAT_R16G16B16A16_SFLOAT;
		case PF_R32F:
			return VK_FORMAT_R32_SFLOAT;
		case PF_RG32F:
			return VK_FORMAT_R32G32_SFLOAT;
		case PF_RGB32F:
			return VK_FORMAT_R32G32B32_SFLOAT;
		case PF_RGBA32F:
			return VK_FORMAT_R32G32B32A32_SFLOAT;
		case PF_R16I:
			return VK_FORMAT_R16_SINT;
		case PF_RG16I:
			return VK_FORMAT_R16G16_SINT;
		case PF_RGBA16I:
			return VK_FORMAT_R16G16B16A16_SINT;
		case PF_R16U:
			return VK_FORMAT_R16_UINT;
		case PF_RG16U:
			return VK_FORMAT_R16G16_UINT;
		case PF_RGBA16U:
			return VK_FORMAT_R16G16B16A16_UINT;
		case PF_R32I:
			return VK_FORMAT_R32_SINT;
		case PF_RG32I:
			return VK_FORMAT_R32G32_SINT;
		case PF_RGB32I:
			return VK_FORMAT_R32G32B32_SINT;
		case PF_RGBA32I:
			return VK_FORMAT_R32G32B32A32_SINT;
		case PF_R32U:
			return VK_FORMAT_R32_UINT;
		case PF_RG32U:
			return VK_FORMAT_R32G32_UINT;
		case PF_RGB32U:
			return VK_FORMAT_R32G32B32_UINT;
		case PF_RGBA32U:
			return VK_FORMAT_R32G32B32A32_UINT;
		case PF_R16S:
			return VK_FORMAT_R16_SNORM;
		case PF_RG16S:
			return VK_FORMAT_R16G16_SNORM;
		case PF_RGBA16S:
			return VK_FORMAT_R16G16B16A16_SNORM;
		case PF_R16:
			return VK_FORMAT_R16_UNORM;
		case PF_RG16:
			return VK_FORMAT_R16G16_UNORM;
		case PF_RGBA16:
			return VK_FORMAT_R16G16B16A16_UNORM;
		case PF_BC1:
		case PF_BC1a:
			if (sRGB)
				return VK_FORMAT_BC1_RGB_SRGB_BLOCK;

			return VK_FORMAT_BC1_RGB_UNORM_BLOCK;
		case PF_BC2:
			if (sRGB)
				return VK_FORMAT_BC2_SRGB_BLOCK;

			return VK_FORMAT_BC2_UNORM_BLOCK;
		case PF_BC3:
			if (sRGB)
				return VK_FORMAT_BC3_SRGB_BLOCK;

			return VK_FORMAT_BC3_UNORM_BLOCK;
		case PF_BC4:
			return VK_FORMAT_BC4_SNORM_BLOCK;
		case PF_BC5:
			return VK_FORMAT_BC5_UNORM_BLOCK;
		case PF_BC6H:
			return VK_FORMAT_BC6H_SFLOAT_BLOCK;
		case PF_BC7:
			if (sRGB)
				return VK_FORMAT_BC7_SRGB_BLOCK;

			return VK_FORMAT_BC7_UNORM_BLOCK;
		case PF_D32_S8X24:
			return VK_FORMAT_D32_SFLOAT_S8_UINT;
		case PF_D24S8:
			return VK_FORMAT_D24_UNORM_S8_UINT;
		case PF_D32:
			return VK_FORMAT_D32_SFLOAT;
		case PF_D16:
			return VK_FORMAT_D16_UNORM;
		case PF_RG11B10F:
			return VK_FORMAT_B10G11R11_UFLOAT_PACK32;
		case PF_RGB10A2:
			return VK_FORMAT_A2B10G10R10_UNORM_PACK32;
		case PF_UNKNOWN:
		default:
			return VK_FORMAT_UNDEFINED;
		}
	}

	VkFormat VulkanUtility::getBufferFormat(GpuBufferFormat format)
	{
		static bool lookupInitialized = false;

		static VkFormat lookup[BF_COUNT];
		if (!lookupInitialized)
		{
			lookup[BF_16X1F] = VK_FORMAT_R16_SFLOAT;
			lookup[BF_16X2F] = VK_FORMAT_R16G16_SFLOAT;
			lookup[BF_16X4F] = VK_FORMAT_R16G16B16A16_SFLOAT;
			lookup[BF_32X1F] = VK_FORMAT_R32_SFLOAT;
			lookup[BF_32X2F] = VK_FORMAT_R32G32_SFLOAT;
			lookup[BF_32X3F] = VK_FORMAT_R32G32B32_SFLOAT;
			lookup[BF_32X4F] = VK_FORMAT_R32G32B32A32_SFLOAT;
			lookup[BF_8X1] = VK_FORMAT_R8_UNORM;
			lookup[BF_8X2] = VK_FORMAT_R8G8_UNORM;
			lookup[BF_8X4] = VK_FORMAT_R8G8B8A8_UNORM;
			lookup[BF_16X1] = VK_FORMAT_R16_UNORM;
			lookup[BF_16X2] = VK_FORMAT_R16G16_UNORM;
			lookup[BF_16X4] = VK_FORMAT_R16G16B16A16_UNORM;
			lookup[BF_8X1S] = VK_FORMAT_R8_SINT;
			lookup[BF_8X2S] = VK_FORMAT_R8G8_SINT;
			lookup[BF_8X4S] = VK_FORMAT_R8G8B8A8_SINT;
			lookup[BF_16X1S] = VK_FORMAT_R16_SINT;
			lookup[BF_16X2S] = VK_FORMAT_R16G16_SINT;
			lookup[BF_16X4S] = VK_FORMAT_R16G16B16A16_SINT;
			lookup[BF_32X1S] = VK_FORMAT_R32_SINT;
			lookup[BF_32X2S] = VK_FORMAT_R32G32_SINT;
			lookup[BF_32X3S] = VK_FORMAT_R32G32B32_SINT;
			lookup[BF_32X4S] = VK_FORMAT_R32G32B32A32_SINT;
			lookup[BF_8X1U] = VK_FORMAT_R8_UINT;
			lookup[BF_8X2U] = VK_FORMAT_R8G8_UINT;
			lookup[BF_8X4U] = VK_FORMAT_R8G8B8A8_UINT;
			lookup[BF_16X1U] = VK_FORMAT_R16_UINT;
			lookup[BF_16X2U] = VK_FORMAT_R16G16_UINT;
			lookup[BF_16X4U] = VK_FORMAT_R16G16B16A16_UINT;
			lookup[BF_32X1U] = VK_FORMAT_R32_UINT;
			lookup[BF_32X2U] = VK_FORMAT_R32G32_UINT;
			lookup[BF_32X3U] = VK_FORMAT_R32G32B32_UINT;
			lookup[BF_32X4U] = VK_FORMAT_R32G32B32A32_UINT;

			lookupInitialized = true;
		}

		if (format >= BF_COUNT)
			return VK_FORMAT_UNDEFINED;

		return lookup[(UINT32)format];
	}

	VkFormat VulkanUtility::getVertexType(VertexElementType type)
	{
		static bool lookupInitialized = false;

		static VkFormat lookup[VET_COUNT];
		if (!lookupInitialized)
		{
			lookup[VET_COLOR] = VK_FORMAT_R8G8B8A8_UNORM;
			lookup[VET_COLOR_ABGR] = VK_FORMAT_R8G8B8A8_UNORM;
			lookup[VET_COLOR_ARGB] = VK_FORMAT_R8G8B8A8_UNORM;
			lookup[VET_UBYTE4_NORM] = VK_FORMAT_R8G8B8A8_UNORM;
			lookup[VET_FLOAT1] = VK_FORMAT_R32_SFLOAT;
			lookup[VET_FLOAT2] = VK_FORMAT_R32G32_SFLOAT;
			lookup[VET_FLOAT3] = VK_FORMAT_R32G32B32_SFLOAT;
			lookup[VET_FLOAT4] = VK_FORMAT_R32G32B32A32_SFLOAT;
			lookup[VET_USHORT1] = VK_FORMAT_R16_UINT;
			lookup[VET_USHORT2] = VK_FORMAT_R16G16_UINT;
			lookup[VET_USHORT4] = VK_FORMAT_R16G16B16A16_UINT;
			lookup[VET_SHORT1] = VK_FORMAT_R16_SINT;
			lookup[VET_SHORT2] = VK_FORMAT_R16G16_SINT;
			lookup[VET_SHORT4] = VK_FORMAT_R16G16B16A16_SINT;
			lookup[VET_UINT1] = VK_FORMAT_R32_UINT;
			lookup[VET_UINT2] = VK_FORMAT_R32G32_UINT;
			lookup[VET_UINT3] = VK_FORMAT_R32G32B32_UINT;
			lookup[VET_UINT4] = VK_FORMAT_R32G32B32A32_UINT;
			lookup[VET_INT1] = VK_FORMAT_R32_SINT;
			lookup[VET_INT2] = VK_FORMAT_R32G32_SINT;
			lookup[VET_INT3] = VK_FORMAT_R32G32B32_SINT;
			lookup[VET_INT4] = VK_FORMAT_R32G32B32A32_SINT;
			lookup[VET_UBYTE4] = VK_FORMAT_R8G8B8A8_UINT;

			lookupInitialized = true;
		}

		if (type >= VET_COUNT)
			return VK_FORMAT_UNDEFINED;

		return lookup[(UINT32)type];
	}

	VkSampleCountFlagBits VulkanUtility::getSampleFlags(UINT32 numSamples)
	{
		switch(numSamples)
		{
		case 0:
		case 1:
			return VK_SAMPLE_COUNT_1_BIT;
		case 2:
			return VK_SAMPLE_COUNT_2_BIT;
		case 4:
			return VK_SAMPLE_COUNT_4_BIT;
		case 8:
			return VK_SAMPLE_COUNT_8_BIT;
		case 16:
			return VK_SAMPLE_COUNT_16_BIT;
		case 32:
			return VK_SAMPLE_COUNT_32_BIT;
		case 64:
			return VK_SAMPLE_COUNT_64_BIT;
		}

		return VK_SAMPLE_COUNT_1_BIT;
	}

	VkShaderStageFlagBits VulkanUtility::getShaderStage(GpuProgramType type)
	{
		switch(type)
		{
		case GPT_FRAGMENT_PROGRAM:
			return VK_SHADER_STAGE_FRAGMENT_BIT;
		case GPT_HULL_PROGRAM:
			return VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;
		case GPT_DOMAIN_PROGRAM:
			return VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;
		case GPT_GEOMETRY_PROGRAM:
			return VK_SHADER_STAGE_GEOMETRY_BIT;
		case GPT_VERTEX_PROGRAM:
			return VK_SHADER_STAGE_VERTEX_BIT;
		case GPT_COMPUTE_PROGRAM:
			return VK_SHADER_STAGE_COMPUTE_BIT;
		default:
			break;
		}

		// Unsupported type
		return VK_SHADER_STAGE_VERTEX_BIT;
	}

	VkSamplerAddressMode VulkanUtility::getAddressingMode(TextureAddressingMode mode)
	{
		switch (mode)
		{
		case TAM_WRAP:
			return VK_SAMPLER_ADDRESS_MODE_REPEAT;
		case TAM_MIRROR:
			return VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
		case TAM_CLAMP:
			return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		case TAM_BORDER:
			return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
		}

		return VK_SAMPLER_ADDRESS_MODE_REPEAT;
	}

	VkBorderColor VulkanUtility::getBorderColor(const Color& color)
	{
		if(color.r > 0.0f || color.g > 0.0f || color.b > 0.0f)
			return VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;

		if(color.a > 0.0f)
			return VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK;

		return VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK;
	}

	VkBlendFactor VulkanUtility::getBlendFactor(BlendFactor factor)
	{
		switch (factor)
		{
		case BF_ONE:
			return VK_BLEND_FACTOR_ONE;
		case BF_ZERO:
			return VK_BLEND_FACTOR_ZERO;
		case BF_DEST_COLOR:
			return VK_BLEND_FACTOR_DST_COLOR;
		case BF_SOURCE_COLOR:
			return VK_BLEND_FACTOR_SRC_COLOR;
		case BF_INV_DEST_COLOR:
			return VK_BLEND_FACTOR_ONE_MINUS_DST_COLOR;
		case BF_INV_SOURCE_COLOR:
			return VK_BLEND_FACTOR_ONE_MINUS_SRC_COLOR;
		case BF_DEST_ALPHA:
			return VK_BLEND_FACTOR_DST_ALPHA;
		case BF_SOURCE_ALPHA:
			return VK_BLEND_FACTOR_SRC_ALPHA;
		case BF_INV_DEST_ALPHA:
			return VK_BLEND_FACTOR_ONE_MINUS_DST_ALPHA;
		case BF_INV_SOURCE_ALPHA:
			return VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
		}

		// Unsupported type
		return VK_BLEND_FACTOR_ZERO;
	}

	VkBlendOp VulkanUtility::getBlendOp(BlendOperation op)
	{
		switch (op)
		{
		case BO_ADD:
			return VK_BLEND_OP_ADD;
		case BO_SUBTRACT:
			return VK_BLEND_OP_SUBTRACT;
		case BO_REVERSE_SUBTRACT:
			return VK_BLEND_OP_REVERSE_SUBTRACT;
		case BO_MIN:
			return VK_BLEND_OP_MIN;
		case BO_MAX:
			return VK_BLEND_OP_MAX;
		}

		// Unsupported type
		return VK_BLEND_OP_ADD;
	}

	VkCompareOp VulkanUtility::getCompareOp(CompareFunction op)
	{
		switch (op)
		{
		case CMPF_ALWAYS_FAIL:
			return VK_COMPARE_OP_NEVER;
		case CMPF_ALWAYS_PASS:
			return VK_COMPARE_OP_ALWAYS;
		case CMPF_LESS:
			return VK_COMPARE_OP_LESS;
		case CMPF_LESS_EQUAL:
			return VK_COMPARE_OP_LESS_OR_EQUAL;
		case CMPF_EQUAL:
			return VK_COMPARE_OP_EQUAL;
		case CMPF_NOT_EQUAL:
			return VK_COMPARE_OP_NOT_EQUAL;
		case CMPF_GREATER_EQUAL:
			return VK_COMPARE_OP_GREATER_OR_EQUAL;
		case CMPF_GREATER:
			return VK_COMPARE_OP_GREATER;
		};

		// Unsupported type
		return VK_COMPARE_OP_ALWAYS;
	}

	VkCullModeFlagBits VulkanUtility::getCullMode(CullingMode mode)
	{
		switch (mode)
		{
		case CULL_NONE:
			return VK_CULL_MODE_NONE;
		case CULL_CLOCKWISE:
			return VK_CULL_MODE_FRONT_BIT;
		case CULL_COUNTERCLOCKWISE:
			return VK_CULL_MODE_BACK_BIT;
		}

		// Unsupported type
		return VK_CULL_MODE_NONE;
	}

	VkPolygonMode VulkanUtility::getPolygonMode(PolygonMode mode)
	{
		switch (mode)
		{
		case PM_WIREFRAME:
			return VK_POLYGON_MODE_LINE;
		case PM_SOLID:
			return VK_POLYGON_MODE_FILL;
		}

		return VK_POLYGON_MODE_FILL;
	}

	VkStencilOp VulkanUtility::getStencilOp(StencilOperation op)
	{
		switch (op)
		{
		case SOP_KEEP:
			return VK_STENCIL_OP_KEEP;
		case SOP_ZERO:
			return VK_STENCIL_OP_ZERO;
		case SOP_REPLACE:
			return VK_STENCIL_OP_REPLACE;
		case SOP_INCREMENT:
			return VK_STENCIL_OP_INCREMENT_AND_CLAMP;
		case SOP_DECREMENT:
			return VK_STENCIL_OP_DECREMENT_AND_CLAMP;
		case SOP_INCREMENT_WRAP:
			return VK_STENCIL_OP_INCREMENT_AND_WRAP;
		case SOP_DECREMENT_WRAP:
			return VK_STENCIL_OP_DECREMENT_AND_WRAP;
		case SOP_INVERT:
			return VK_STENCIL_OP_INVERT;
		}

		// Unsupported type
		return VK_STENCIL_OP_KEEP;
	}

	VkIndexType VulkanUtility::getIndexType(IndexType op)
	{
		switch(op)
		{
		case IT_16BIT:
			return VK_INDEX_TYPE_UINT16;
		case IT_32BIT:
			return VK_INDEX_TYPE_UINT32;
		}

		// Unsupported type
		return VK_INDEX_TYPE_UINT32;
	}

	VkPrimitiveTopology VulkanUtility::getDrawOp(DrawOperationType op)
	{
		switch (op)
		{
		case DOT_POINT_LIST:
			return VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
		case DOT_LINE_LIST:
			return VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
		case DOT_LINE_STRIP:
			return VK_PRIMITIVE_TOPOLOGY_LINE_STRIP;
		case DOT_TRIANGLE_LIST:
			return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		case DOT_TRIANGLE_STRIP:
			return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
		case DOT_TRIANGLE_FAN:
			return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN;
		}

		// Unsupported type
		return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	}

	VkFilter VulkanUtility::getFilter(FilterOptions filter)
	{
		switch(filter)
		{
		case FO_LINEAR:
		case FO_ANISOTROPIC:
			return VK_FILTER_LINEAR;
		case FO_POINT:
		case FO_NONE:
			return VK_FILTER_NEAREST;
		}

		// Unsupported type
		return VK_FILTER_LINEAR;
	}

	VkSamplerMipmapMode VulkanUtility::getMipFilter(FilterOptions filter)
	{
		switch (filter)
		{
		case FO_LINEAR:
		case FO_ANISOTROPIC:
			return VK_SAMPLER_MIPMAP_MODE_LINEAR;
		case FO_POINT:
		case FO_NONE:
			return VK_SAMPLER_MIPMAP_MODE_NEAREST;
		}

		// Unsupported type
		return VK_SAMPLER_MIPMAP_MODE_LINEAR;
	}

	void VulkanUtility::getDevices(const VulkanRenderAPI& rapi, GpuDeviceFlags flags, VulkanDevice*(&devices)[BS_MAX_DEVICES])
	{
		UINT32 numDevices = rapi._getNumDevices();

		for (UINT32 i = 0; i < BS_MAX_DEVICES; i++)
		{
			if(i >= numDevices)
			{
				devices[i] = nullptr;
				continue;
			}

			VulkanDevice* device = rapi._getDevice(i).get();

			if (isDeviceIdxSet(rapi, i, flags))
				devices[i] = device;
			else
				devices[i] = nullptr;
		}
	}

	VkPipelineStageFlags VulkanUtility::shaderToPipelineStage(VkShaderStageFlags shaderStageFlags)
	{
		VkPipelineStageFlags output = 0;

		if((shaderStageFlags & VK_SHADER_STAGE_VERTEX_BIT) != 0)
			output |= VK_PIPELINE_STAGE_VERTEX_SHADER_BIT;

		if((shaderStageFlags & VK_SHADER_STAGE_FRAGMENT_BIT) != 0)
			output |= VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;

		if((shaderStageFlags & VK_SHADER_STAGE_GEOMETRY_BIT) != 0)
			output |= VK_PIPELINE_STAGE_GEOMETRY_SHADER_BIT;

		if((shaderStageFlags & VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT) != 0)
			output |= VK_PIPELINE_STAGE_TESSELLATION_CONTROL_SHADER_BIT;

		if((shaderStageFlags & VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT) != 0)
			output |= VK_PIPELINE_STAGE_TESSELLATION_EVALUATION_SHADER_BIT;

		if((shaderStageFlags & VK_SHADER_STAGE_COMPUTE_BIT) != 0)
			output |= VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;

		return output;
	}

	bool VulkanUtility::isDeviceIdxSet(const VulkanRenderAPI& rapi, UINT32 idx, GpuDeviceFlags flags)
	{
		VulkanDevice* device = rapi._getDevice(idx).get();

		return ((flags & (1 << idx)) != 0 || (flags == GDF_DEFAULT && device->isPrimary()));
	}

	void cutHorizontal(const VkImageSubresourceRange& toCut, const VkImageSubresourceRange& cutWith,
					   VkImageSubresourceRange* output, UINT32& numAreas)
	{
		numAreas = 0;

		INT32 leftCut = Math::clamp((INT32)cutWith.baseArrayLayer - (INT32)toCut.baseArrayLayer, 0, (INT32)toCut.layerCount);
		INT32 rightCut = Math::clamp((INT32)(cutWith.baseArrayLayer + cutWith.layerCount) - (INT32)toCut.baseArrayLayer, 0, (INT32)toCut.layerCount);

		if (leftCut > 0 && leftCut < (INT32)toCut.layerCount)
		{
			output[numAreas] = toCut;
			VkImageSubresourceRange& range = output[numAreas];

			range.baseArrayLayer = toCut.baseArrayLayer;
			range.layerCount = leftCut;

			numAreas++;
		}

		if (rightCut > 0 && rightCut < (INT32)toCut.layerCount)
		{
			output[numAreas] = toCut;
			VkImageSubresourceRange& range = output[numAreas];

			range.baseArrayLayer = toCut.baseArrayLayer + rightCut;
			range.layerCount = toCut.layerCount - rightCut;

			numAreas++;
		}

		// Middle cut
		if (rightCut > leftCut)
		{
			output[numAreas] = toCut;
			VkImageSubresourceRange& range = output[numAreas];

			range.baseArrayLayer = toCut.baseArrayLayer + leftCut;
			range.layerCount = toCut.layerCount - (toCut.layerCount - rightCut) - leftCut;

			numAreas++;
		}

		// Nothing to cut
		if (numAreas == 0)
		{
			output[numAreas] = toCut;
			numAreas++;
		}
	}

	void cutVertical(const VkImageSubresourceRange& toCut, const VkImageSubresourceRange& cutWith,
					 VkImageSubresourceRange* output, UINT32& numAreas)
	{
		numAreas = 0;

		INT32 topCut = Math::clamp((INT32)cutWith.baseMipLevel - (INT32)toCut.baseMipLevel, 0, (INT32)toCut.levelCount);
		INT32 bottomCut = Math::clamp((INT32)(cutWith.baseMipLevel + cutWith.levelCount) - (INT32)toCut.baseMipLevel, 0, (INT32)toCut.levelCount);

		if (topCut > 0 && topCut < (INT32)toCut.levelCount)
		{
			output[numAreas] = toCut;
			VkImageSubresourceRange& range = output[numAreas];

			range.baseMipLevel = toCut.baseMipLevel;
			range.levelCount = topCut;

			numAreas++;
		}

		if (bottomCut > 0 && bottomCut < (INT32)toCut.levelCount)
		{
			output[numAreas] = toCut;
			VkImageSubresourceRange& range = output[numAreas];

			range.baseMipLevel = toCut.baseMipLevel + bottomCut;
			range.levelCount = toCut.levelCount - bottomCut;

			numAreas++;
		}

		// Middle cut
		if (bottomCut > topCut)
		{
			output[numAreas] = toCut;
			VkImageSubresourceRange& range = output[numAreas];

			range.baseMipLevel = toCut.baseMipLevel + topCut;
			range.levelCount = toCut.levelCount - (toCut.levelCount - bottomCut) - topCut;

			numAreas++;
		}

		// Nothing to cut
		if (numAreas == 0)
		{
			output[numAreas] = toCut;
			numAreas++;
		}
	}

	void VulkanUtility::cutRange(const VkImageSubresourceRange& toCut, const VkImageSubresourceRange& cutWith,
				  std::array<VkImageSubresourceRange, 5>& output, UINT32& numAreas)
	{
		numAreas = 0;

		// Cut horizontally
		UINT32 numHorzCuts = 0;
		std::array<VkImageSubresourceRange, 3> horzCuts;
		cutHorizontal(toCut, cutWith, horzCuts.data(), numHorzCuts);

		// Cut vertically
		for (UINT32 i = 0; i < numHorzCuts; i++)
		{
			VkImageSubresourceRange& range = horzCuts[i];

			if (range.baseArrayLayer >= cutWith.baseArrayLayer &&
				(range.baseArrayLayer + range.layerCount) <= (cutWith.baseArrayLayer + cutWith.layerCount))
			{
				UINT32 numVertCuts = 0;
				cutVertical(range, cutWith, output.data() + numAreas, numVertCuts);

				numAreas += numVertCuts;
			}
			else
			{
				output[numAreas] = range;
				numAreas++;
			}
		}

		assert(numAreas <= 5);
	}

	bool VulkanUtility::rangeOverlaps(const VkImageSubresourceRange& a, const VkImageSubresourceRange& b)
	{
		INT32 aRight = a.baseArrayLayer + (INT32)a.layerCount;
		INT32 bRight = b.baseArrayLayer + (INT32)b.layerCount;

		INT32 aBottom = a.baseMipLevel + (INT32)a.levelCount;
		INT32 bBottom = b.baseMipLevel + (INT32)b.levelCount;

		if ((INT32)a.baseArrayLayer < bRight && aRight >(INT32)b.baseArrayLayer &&
			(INT32)a.baseMipLevel < bBottom && aBottom >(INT32)b.baseMipLevel)
			return true;

		return false;
	}

	UINT32 VulkanUtility::calcInterfaceBlockElementSizeAndOffset(GpuParamDataType type, UINT32 arraySize, UINT32& offset)
	{
		const GpuParamDataTypeInfo& typeInfo = bs::GpuParams::PARAM_SIZES.lookup[type];
		UINT32 size = (typeInfo.baseTypeSize * typeInfo.numColumns * typeInfo.numRows) / 4;
		UINT32 alignment = typeInfo.alignment / 4;

		// Fix alignment if needed
		UINT32 alignOffset = offset % alignment;
		if (alignOffset != 0)
		{
			UINT32 padding = (alignment - alignOffset);
			offset += padding;
		}

		if (arraySize > 1)
		{
			// Array elements are always padded and aligned to vec4
			alignOffset = size % 4;
			if (alignOffset != 0)
			{
				UINT32 padding = (4 - alignOffset);
				size += padding;
			}

			alignOffset = offset % 4;
			if (alignOffset != 0)
			{
				UINT32 padding = (4 - alignOffset);
				offset += padding;
			}

			return size;
		}
		else
			return size;
	}
}}
