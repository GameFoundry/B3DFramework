//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DVulkanUtility.h"
#include "B3DVulkanGpuDevice.h"
#include "Image/B3DTexture.h"
#include "Math/B3DArea2.h"
#include "RenderAPI/B3DGpuParameterSet.h"

using namespace b3d;
using namespace b3d::render;

PixelFormat VulkanUtility::GetClosestSupportedPixelFormat(const VulkanGpuDevice& device, PixelFormat format, TextureType texType, TextureUsageFlags usage, bool optimalTiling, bool hwGamma)
{
	// Check for any obvious issues first
	PixelUtility::CheckFormat(format, texType, usage);

	// Check actual device for format support
	VkFormatFeatureFlags wantedFeatureFlags = VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT;
	if(usage.IsSet(TextureUsageFlag::RenderTarget))
		wantedFeatureFlags |= VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT;

	if(usage.IsSet(TextureUsageFlag::DepthStencil))
		wantedFeatureFlags |= VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT;

	if(usage.IsSet(TextureUsageFlag::AllowUnorderedAccessOnTheGPU))
		wantedFeatureFlags |= VK_FORMAT_FEATURE_STORAGE_IMAGE_BIT;

	VkFormatProperties props;
	auto fnIsFormatSupported = [&](VkFormat vkFmt)
	{
		vkGetPhysicalDeviceFormatProperties(device.GetPhysical(), vkFmt, &props);
		VkFormatFeatureFlags featureFlags = optimalTiling ? props.optimalTilingFeatures : props.linearTilingFeatures;

		return (featureFlags & wantedFeatureFlags) != 0;
	};

	VkFormat vkFormat = GetPixelFormat(format, hwGamma);
	if(!fnIsFormatSupported(vkFormat))
	{
		const PixelFormat originalFormat = format;
		bool safeFallback = false; // Set to true to avoid any log messages about fallback format

		if(usage.IsSet(TextureUsageFlag::DepthStencil))
		{
			bool hasStencil = format == PF_D24S8 || format == PF_D32_S8X24;

			// Spec guarantees at least one depth-only, and one depth-stencil format to be supported
			if(hasStencil)
			{
				if(fnIsFormatSupported(VK_FORMAT_D32_SFLOAT_S8_UINT))
				{
					if(format == PF_D24S8)
						safeFallback = true;

					format = PF_D32_S8X24;
				}
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
			const PixelComponentType& formatComponentType = PixelUtility::GetElementType(format);

			if(formatComponentType == PCT_FLOAT16) // 16-bit format, fall back to 4-channel 16-bit, guaranteed to be supported
				format = PF_RGBA16F;
			else if(formatComponentType == PCT_FLOAT32) // 32-bit float format, fall back to 4-channel 32-bit, guaranteed to be supported
				format = PF_RGBA32F;
			else if(formatComponentType == PCT_INT) // 32-bit integer format, fall back to 4-channel 32-bit, guaranteed to be supported
				format = PF_RGBA32I;
			else if(formatComponentType == PCT_BYTE) // 8-bit integer Type, fall back to 4-channel 8-bit, guaranteed to be supported
				format = PF_RGBA8;
			else // Must be 8-bit per channel format, compressed format or some uneven format
				format = PF_RGBA8;

			// Final check in case the format is still not supported
			const VkFormat vkFormat = GetPixelFormat(format, hwGamma);
			if(format != originalFormat && format != PF_RGBA8 && !fnIsFormatSupported(vkFormat))
				format = PF_RGBA8;
		}

		if(!safeFallback)
			B3D_LOG(Error, LogGeneric, "Provided an unsupported Vulkan image format with ID={0}. Falling back to format with ID={1}", originalFormat, format);
	}

	return format;
}

VkFormat VulkanUtility::GetPixelFormat(PixelFormat format, bool sRGB)
{
	switch(format)
	{
	case PF_R8:
		if(sRGB)
			return VK_FORMAT_R8_SRGB;

		return VK_FORMAT_R8_UNORM;
	case PF_RG8:
		if(sRGB)
			return VK_FORMAT_R8G8_SRGB;

		return VK_FORMAT_R8G8_UNORM;
	case PF_RGB8:
		if(sRGB)
			return VK_FORMAT_R8G8B8_SRGB;

		return VK_FORMAT_R8G8B8_UNORM;
	case PF_RGBA8:
		if(sRGB)
			return VK_FORMAT_R8G8B8A8_SRGB;

		return VK_FORMAT_R8G8B8A8_UNORM;
	case PF_BGRA8:
		if(sRGB)
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
		if(sRGB)
			return VK_FORMAT_BC1_RGB_SRGB_BLOCK;

		return VK_FORMAT_BC1_RGB_UNORM_BLOCK;
	case PF_BC2:
		if(sRGB)
			return VK_FORMAT_BC2_SRGB_BLOCK;

		return VK_FORMAT_BC2_UNORM_BLOCK;
	case PF_BC3:
		if(sRGB)
			return VK_FORMAT_BC3_SRGB_BLOCK;

		return VK_FORMAT_BC3_UNORM_BLOCK;
	case PF_BC4:
		return VK_FORMAT_BC4_SNORM_BLOCK;
	case PF_BC5:
		return VK_FORMAT_BC5_UNORM_BLOCK;
	case PF_BC6H:
		return VK_FORMAT_BC6H_SFLOAT_BLOCK;
	case PF_BC7:
		if(sRGB)
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

VkFormat VulkanUtility::GetBufferFormat(GpuBufferFormat format)
{
	static bool lookupInitialized = false;

	static VkFormat lookup[BF_COUNT];
	if(!lookupInitialized)
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
		lookup[BF_64X1F] = VK_FORMAT_R64_SFLOAT;
		lookup[BF_64X2F] = VK_FORMAT_R64G64_SFLOAT;
		lookup[BF_64X3F] = VK_FORMAT_R64G64B64_SFLOAT;
		lookup[BF_64X4F] = VK_FORMAT_R64G64B64A64_SFLOAT;
		lookup[BF_64X1S] = VK_FORMAT_R64_SINT;
		lookup[BF_64X2S] = VK_FORMAT_R64G64_SINT;
		lookup[BF_64X3S] = VK_FORMAT_R64G64B64_SINT;
		lookup[BF_64X4S] = VK_FORMAT_R64G64B64A64_SINT;
		lookup[BF_64X1U] = VK_FORMAT_R64_UINT;
		lookup[BF_64X2U] = VK_FORMAT_R64G64_UINT;
		lookup[BF_64X3U] = VK_FORMAT_R64G64B64_UINT;
		lookup[BF_64X4U] = VK_FORMAT_R64G64B64A64_UINT;

		lookupInitialized = true;
	}

	if(format >= BF_COUNT)
		return VK_FORMAT_UNDEFINED;

	return lookup[(u32)format];
}

VkFormat VulkanUtility::GetVertexType(VertexElementType type)
{
	static bool lookupInitialized = false;

	static VkFormat lookup[VET_COUNT];
	if(!lookupInitialized)
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
		lookup[VET_HALF1] = VK_FORMAT_R16_SFLOAT;
		lookup[VET_HALF2] = VK_FORMAT_R16G16_SFLOAT;
		lookup[VET_HALF3] = VK_FORMAT_R16G16B16_SFLOAT;
		lookup[VET_HALF4] = VK_FORMAT_R16G16B16A16_SFLOAT;

		lookupInitialized = true;
	}

	if(type >= VET_COUNT)
		return VK_FORMAT_UNDEFINED;

	return lookup[(u32)type];
}

VkSampleCountFlagBits VulkanUtility::GetSampleFlags(u32 numSamples)
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

VkShaderStageFlagBits VulkanUtility::GetShaderStage(GpuProgramType type)
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

VkSamplerAddressMode VulkanUtility::GetAddressingMode(TextureAddressingMode mode)
{
	switch(mode)
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

VkBorderColor VulkanUtility::GetBorderColor(const Color& color)
{
	if(color.R > 0.0f || color.G > 0.0f || color.B > 0.0f)
		return VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;

	if(color.A > 0.0f)
		return VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK;

	return VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK;
}

VkBlendFactor VulkanUtility::GetBlendFactor(BlendFactor factor)
{
	switch(factor)
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

VkBlendOp VulkanUtility::GetBlendOp(BlendOperation op)
{
	switch(op)
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

VkCompareOp VulkanUtility::GetCompareOp(CompareFunction op)
{
	switch(op)
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

VkCullModeFlagBits VulkanUtility::GetCullMode(CullingMode mode)
{
	switch(mode)
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

VkPolygonMode VulkanUtility::GetPolygonMode(PolygonMode mode)
{
	switch(mode)
	{
	case PM_WIREFRAME:
		return VK_POLYGON_MODE_LINE;
	case PM_SOLID:
		return VK_POLYGON_MODE_FILL;
	}

	return VK_POLYGON_MODE_FILL;
}

VkStencilOp VulkanUtility::GetStencilOp(StencilOperation op)
{
	switch(op)
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

VkIndexType VulkanUtility::GetIndexType(IndexType op)
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

VkPrimitiveTopology VulkanUtility::GetDrawOp(DrawOperationType op)
{
	switch(op)
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

VkFilter VulkanUtility::GetFilter(FilterOptions filter)
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

VkSamplerMipmapMode VulkanUtility::GetMipFilter(FilterOptions filter)
{
	switch(filter)
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

VkPipelineStageFlags VulkanUtility::ShaderToPipelineStage(VkShaderStageFlags shaderStageFlags)
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

GpuResourceUseFlags VulkanUtility::ShaderToResourceUseFlags(VkShaderStageFlags shaderStageFlags)
{
	GpuResourceUseFlags output = GpuResourceUseFlag::Undefined;

	if((shaderStageFlags & VK_SHADER_STAGE_VERTEX_BIT) != 0)
		output |= GpuResourceUseFlag::StageVertexShader;

	if((shaderStageFlags & VK_SHADER_STAGE_FRAGMENT_BIT) != 0)
		output |= GpuResourceUseFlag::StageFragmentShader;

	if((shaderStageFlags & VK_SHADER_STAGE_COMPUTE_BIT) != 0)
		output |= GpuResourceUseFlag::StageComputeShader;

	return output;
}

VkQueryType VulkanUtility::GetQueryType(GpuQueryType queryType)
{
	switch(queryType)
	{
	default:
	case GpuQueryType::Timestamp:
		return VK_QUERY_TYPE_TIMESTAMP;
	case GpuQueryType::Occlusion: 
		return VK_QUERY_TYPE_OCCLUSION;
	case GpuQueryType::PipelineStatistics:
		return VK_QUERY_TYPE_PIPELINE_STATISTICS;
	}
}

VkQueryPipelineStatisticFlags VulkanUtility::GetPipelineStatisticQueryBits(GpuPipelineStatisticsQueryBits bits)
{
	VkQueryPipelineStatisticFlags flags = 0;

	if(bits.IsSet(GpuPipelineStatisticsQueryBit::VertexCount))
		flags |= VK_QUERY_PIPELINE_STATISTIC_INPUT_ASSEMBLY_VERTICES_BIT;

	if(bits.IsSet(GpuPipelineStatisticsQueryBit::PrimitiveCount))
		flags |= VK_QUERY_PIPELINE_STATISTIC_INPUT_ASSEMBLY_PRIMITIVES_BIT;

	if(bits.IsSet(GpuPipelineStatisticsQueryBit::VertexShaderInvocationCount))
		flags |= VK_QUERY_PIPELINE_STATISTIC_VERTEX_SHADER_INVOCATIONS_BIT;

	if(bits.IsSet(GpuPipelineStatisticsQueryBit::FragmentShaderInvocationCount))
		flags |= VK_QUERY_PIPELINE_STATISTIC_FRAGMENT_SHADER_INVOCATIONS_BIT;

	if(bits.IsSet(GpuPipelineStatisticsQueryBit::ComputeShaderInvocationCount))
		flags |= VK_QUERY_PIPELINE_STATISTIC_COMPUTE_SHADER_INVOCATIONS_BIT;

	if(bits.IsSet(GpuPipelineStatisticsQueryBit::ClippingInvocationCount))
		flags |= VK_QUERY_PIPELINE_STATISTIC_CLIPPING_INVOCATIONS_BIT;

	if(bits.IsSet(GpuPipelineStatisticsQueryBit::ClippingGeneratedPrimitiveCount))
		flags |= VK_QUERY_PIPELINE_STATISTIC_CLIPPING_PRIMITIVES_BIT;

	return flags;
}

VkImageAspectFlags VulkanUtility::GetAspectMask(GpuTextureAspectFlags aspectMask)
{
	VkImageAspectFlags flags = 0;

	if(aspectMask.IsSet(GpuTextureAspectFlag::Color)) flags |= VK_IMAGE_ASPECT_COLOR_BIT;
	if(aspectMask.IsSet(GpuTextureAspectFlag::Depth)) flags |= VK_IMAGE_ASPECT_DEPTH_BIT;
	if(aspectMask.IsSet(GpuTextureAspectFlag::Stencil)) flags |= VK_IMAGE_ASPECT_STENCIL_BIT;

	return flags;
}

VkRect2D VulkanUtility::ToVulkanRect(const Area2I& input)
{
	VkRect2D output;
	output.offset.x = input.X;
	output.offset.y = input.Y;
	output.extent.width = input.Width;
	output.extent.height = input.Height;

	return output;
}

VkViewport VulkanUtility::ToVulkanViewport(const Area2I& input, float minDepth, float maxDepth)
{
	VkViewport output;
	output.x = (float)input.X;
	output.y = (float)input.Y;
	output.width = (float)input.Width;
	output.height = (float)input.Height;
	output.minDepth = minDepth;
	output.maxDepth = maxDepth;
	
	return output;
}

VkImageSubresourceRange VulkanUtility::ToVulkanImageSubresourceRange(const GpuTextureSubresourceRange& subresourceRange)
{
	VkImageSubresourceRange output;
	output.aspectMask = GetAspectMask(subresourceRange.AspectMask);
	output.baseMipLevel = subresourceRange.BaseMipLevel;
	output.levelCount = subresourceRange.MipLevelCount == ~0u ? VK_REMAINING_MIP_LEVELS : subresourceRange.MipLevelCount;
	output.baseArrayLayer = subresourceRange.BaseArrayLayer;
	output.layerCount = subresourceRange.ArrayLayerCount == ~0u ? VK_REMAINING_ARRAY_LAYERS : subresourceRange.ArrayLayerCount;

	return output;
}

void cutHorizontal(const VkImageSubresourceRange& toCut, const VkImageSubresourceRange& cutWith, VkImageSubresourceRange* output, u32& numAreas)
{
	numAreas = 0;

	i32 leftCut = Math::Clamp((i32)cutWith.baseArrayLayer - (i32)toCut.baseArrayLayer, 0, (i32)toCut.layerCount);
	i32 rightCut = Math::Clamp((i32)(cutWith.baseArrayLayer + cutWith.layerCount) - (i32)toCut.baseArrayLayer, 0, (i32)toCut.layerCount);

	if(leftCut > 0 && leftCut < (i32)toCut.layerCount)
	{
		output[numAreas] = toCut;
		VkImageSubresourceRange& range = output[numAreas];

		range.baseArrayLayer = toCut.baseArrayLayer;
		range.layerCount = leftCut;

		numAreas++;
	}

	if(rightCut > 0 && rightCut < (i32)toCut.layerCount)
	{
		output[numAreas] = toCut;
		VkImageSubresourceRange& range = output[numAreas];

		range.baseArrayLayer = toCut.baseArrayLayer + rightCut;
		range.layerCount = toCut.layerCount - rightCut;

		numAreas++;
	}

	// Middle cut
	if(rightCut > leftCut)
	{
		output[numAreas] = toCut;
		VkImageSubresourceRange& range = output[numAreas];

		range.baseArrayLayer = toCut.baseArrayLayer + leftCut;
		range.layerCount = toCut.layerCount - (toCut.layerCount - rightCut) - leftCut;

		numAreas++;
	}

	// Nothing to cut
	if(numAreas == 0)
	{
		output[numAreas] = toCut;
		numAreas++;
	}
}

void cutVertical(const VkImageSubresourceRange& toCut, const VkImageSubresourceRange& cutWith, VkImageSubresourceRange* output, u32& numAreas)
{
	numAreas = 0;

	i32 topCut = Math::Clamp((i32)cutWith.baseMipLevel - (i32)toCut.baseMipLevel, 0, (i32)toCut.levelCount);
	i32 bottomCut = Math::Clamp((i32)(cutWith.baseMipLevel + cutWith.levelCount) - (i32)toCut.baseMipLevel, 0, (i32)toCut.levelCount);

	if(topCut > 0 && topCut < (i32)toCut.levelCount)
	{
		output[numAreas] = toCut;
		VkImageSubresourceRange& range = output[numAreas];

		range.baseMipLevel = toCut.baseMipLevel;
		range.levelCount = topCut;

		numAreas++;
	}

	if(bottomCut > 0 && bottomCut < (i32)toCut.levelCount)
	{
		output[numAreas] = toCut;
		VkImageSubresourceRange& range = output[numAreas];

		range.baseMipLevel = toCut.baseMipLevel + bottomCut;
		range.levelCount = toCut.levelCount - bottomCut;

		numAreas++;
	}

	// Middle cut
	if(bottomCut > topCut)
	{
		output[numAreas] = toCut;
		VkImageSubresourceRange& range = output[numAreas];

		range.baseMipLevel = toCut.baseMipLevel + topCut;
		range.levelCount = toCut.levelCount - (toCut.levelCount - bottomCut) - topCut;

		numAreas++;
	}

	// Nothing to cut
	if(numAreas == 0)
	{
		output[numAreas] = toCut;
		numAreas++;
	}
}

void VulkanUtility::CutRange(const VkImageSubresourceRange& toCut, const VkImageSubresourceRange& cutWith, std::array<VkImageSubresourceRange, 5>& output, u32& numAreas)
{
	numAreas = 0;

	// Cut horizontally
	u32 numHorzCuts = 0;
	std::array<VkImageSubresourceRange, 3> horzCuts;
	cutHorizontal(toCut, cutWith, horzCuts.data(), numHorzCuts);

	// Cut vertically
	for(u32 i = 0; i < numHorzCuts; i++)
	{
		VkImageSubresourceRange& range = horzCuts[i];

		if(range.baseArrayLayer >= cutWith.baseArrayLayer &&
		   (range.baseArrayLayer + range.layerCount) <= (cutWith.baseArrayLayer + cutWith.layerCount))
		{
			u32 numVertCuts = 0;
			cutVertical(range, cutWith, output.data() + numAreas, numVertCuts);

			numAreas += numVertCuts;
		}
		else
		{
			output[numAreas] = range;
			numAreas++;
		}
	}

	B3D_ASSERT(numAreas <= 5);
}

bool VulkanUtility::RangeOverlaps(const VkImageSubresourceRange& a, const VkImageSubresourceRange& b)
{
	i32 aRight = a.baseArrayLayer + (i32)a.layerCount;
	i32 bRight = b.baseArrayLayer + (i32)b.layerCount;

	i32 aBottom = a.baseMipLevel + (i32)a.levelCount;
	i32 bBottom = b.baseMipLevel + (i32)b.levelCount;

	if((i32)a.baseArrayLayer < bRight && aRight > (i32)b.baseArrayLayer &&
	   (i32)a.baseMipLevel < bBottom && aBottom > (i32)b.baseMipLevel)
		return true;

	return false;
}

bool VulkanUtility::RangeEquals(const VkImageSubresourceRange& a, const VkImageSubresourceRange& b)
{
	return a.baseArrayLayer == b.baseArrayLayer && a.baseMipLevel == b.baseMipLevel && a.layerCount == b.layerCount && a.levelCount == b.levelCount && a.aspectMask == b.aspectMask;
}

u32 VulkanUtility::CalcInterfaceBlockElementSizeAndOffset(GpuDataParameterType type, u32 arraySize, u32& offset)
{
	const GpuDataParameterTypeInformation& typeInfo = b3d::GpuParameterSet::kParamSizes.Lookup[type];
	u32 size = (typeInfo.BaseTypeSize * typeInfo.NumColumns * typeInfo.NumRows) / 4;
	u32 alignment = typeInfo.Alignment / 4;

	// Fix alignment if needed
	u32 alignOffset = offset % alignment;
	if(alignOffset != 0)
	{
		u32 padding = (alignment - alignOffset);
		offset += padding;
	}

	if(arraySize > 1)
	{
		// Array elements are always padded and aligned to vec4
		alignOffset = size % 4;
		if(alignOffset != 0)
		{
			u32 padding = (4 - alignOffset);
			size += padding;
		}

		alignOffset = offset % 4;
		if(alignOffset != 0)
		{
			u32 padding = (4 - alignOffset);
			offset += padding;
		}

		return size;
	}
	else
		return size;
}

const char* VulkanUtility::GetAccessStageName(VulkanAccessStageFlag flag)
{
	switch(flag)
	{
	case VulkanAccessStageFlag::None: return "None";
	case VulkanAccessStageFlag::DrawIndirect: return "DrawIndirect";
	case VulkanAccessStageFlag::VertexInputAttributes: return "VertexInputAttributes";
	case VulkanAccessStageFlag::VertexInputIndices: return "VertexInputIndices";
	case VulkanAccessStageFlag::VertexShaderNonUniform: return "VertexShaderNonUniform";
	case VulkanAccessStageFlag::FragmentShaderNonUniform: return "FragmentShaderNonUniform";
	case VulkanAccessStageFlag::ComputeShaderNonUniform: return "ComputeShaderNonUniform";
	case VulkanAccessStageFlag::VertexShaderUniform: return "VertexShaderUniform";
	case VulkanAccessStageFlag::FragmentShaderUniform: return "FragmentShaderUniform";
	case VulkanAccessStageFlag::ComputeShaderUniform: return "ComputeShaderUniform";
	case VulkanAccessStageFlag::EarlyFragmentTests: return "EarlyFragmentTests";
	case VulkanAccessStageFlag::LateFragmentTests: return "LateFragmentTests";
	case VulkanAccessStageFlag::ColorAttachment: return "ColorAttachment";
	case VulkanAccessStageFlag::Transfer: return "Transfer";
	case VulkanAccessStageFlag::Host: return "Host";
	case VulkanAccessStageFlag::AllShader: return "AllShader";
	case VulkanAccessStageFlag::All: return "All";
	default: return "Unknown";
	}
}

void VulkanUtility::GetAccessStageNames(VulkanAccessStageFlags flags, StringStream& output)
{
	bool isFirst = true;
	u32 flagsAsInteger = (u32)flags;
	while(flagsAsInteger != 0)
	{
		const u32 flagIndex = Bitwise::LeastSignificantBit(flagsAsInteger);
		const VulkanAccessStageFlag flag = (VulkanAccessStageFlag)(1 << flagIndex);

		if(!isFirst)
			output << " | ";

		output << GetAccessStageName(flag);
	
		flagsAsInteger &= ~(1 << flagIndex);
		isFirst = false;
	}
}

const char* VulkanUtility::GetImageLayoutName(VkImageLayout layout)
{
	switch(layout)
	{
	case VK_IMAGE_LAYOUT_UNDEFINED: return "UNDEFINED";
	case VK_IMAGE_LAYOUT_GENERAL: return "GENERAL";
	case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL: return "COLOR_ATTACHMENT_OPTIMAL";
	case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL: return "DEPTH_STENCIL_ATTACHMENT_OPTIMAL";
	case VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL: return "DEPTH_STENCIL_READ_ONLY_OPTIMAL";
	case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL: return "SHADER_READ_ONLY_OPTIMAL";
	case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL: return "TRANSFER_SRC_OPTIMAL";
	case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL: return "TRANSFER_DST_OPTIMAL";
	case VK_IMAGE_LAYOUT_PREINITIALIZED: return "PREINITIALIZED";
	case VK_IMAGE_LAYOUT_PRESENT_SRC_KHR: return "PRESENT_SRC_KHR";
	case VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL_KHR: return "DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL";
	case VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL_KHR: return "DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL";
	default: return "UNKNOWN";
	}
}

VkPipelineStageFlags VulkanUtility::GetPipelineStageFlags(GpuResourceUseFlags usage, VkAccessFlags accessFlags)
{
	VkPipelineStageFlags flags = 0;

	// Handle non-shader usage flags using standard access flag conversion
	if((accessFlags & VK_ACCESS_INDIRECT_COMMAND_READ_BIT) != 0)
		flags |= VK_PIPELINE_STAGE_DRAW_INDIRECT_BIT;

	if((accessFlags & (VK_ACCESS_INDEX_READ_BIT | VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT)) != 0)
		flags |= VK_PIPELINE_STAGE_VERTEX_INPUT_BIT;

	// Handle shader stages - use individual flags if specified, otherwise use all shader stages
	if((accessFlags & (VK_ACCESS_UNIFORM_READ_BIT | VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT)) != 0)
	{
		// Check for individual shader stage flags
		if(usage.IsSet(GpuResourceUseFlag::StageVertexShader))
			flags |= VK_PIPELINE_STAGE_VERTEX_SHADER_BIT;

		if(usage.IsSet(GpuResourceUseFlag::StageFragmentShader))
			flags |= VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;

		if(usage.IsSet(GpuResourceUseFlag::StageComputeShader))
			flags |= VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;

		// If no individual shader stages are specified, default to all shader stages
		// (This handles the case where the old Shader flag is used or when combined with other access types)
		if(flags == 0)
		{
			flags |= VK_PIPELINE_STAGE_VERTEX_SHADER_BIT;
			flags |= VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
			flags |= VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;

			// MoltenVK doesn't support geometry and tessellation shaders
			// Note: Once we upgrade to a newer version they should be supported and we can remove this
#if !B3D_PLATFORM_MACOS
			flags |= VK_PIPELINE_STAGE_GEOMETRY_SHADER_BIT;
			flags |= VK_PIPELINE_STAGE_TESSELLATION_CONTROL_SHADER_BIT;
			flags |= VK_PIPELINE_STAGE_TESSELLATION_EVALUATION_SHADER_BIT;
#endif
		}
	}

	if((accessFlags & VK_ACCESS_INPUT_ATTACHMENT_READ_BIT) != 0)
		flags |= VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;

	if((accessFlags & (VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT)) != 0)
		flags |= VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

	if((accessFlags & (VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT)) != 0)
		flags |= VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;

	if((accessFlags & (VK_ACCESS_TRANSFER_READ_BIT | VK_ACCESS_TRANSFER_WRITE_BIT)) != 0)
		flags |= VK_PIPELINE_STAGE_TRANSFER_BIT;

	if((accessFlags & (VK_ACCESS_HOST_READ_BIT | VK_ACCESS_HOST_WRITE_BIT)) != 0)
		flags |= VK_PIPELINE_STAGE_HOST_BIT;

	if(flags == 0)
		flags = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;

	return flags;
}

VkPipelineStageFlags VulkanUtility::GetPipelineStageFlags(VkAccessFlags accessFlags)
{
	return GetPipelineStageFlags(GpuResourceUseFlag::Undefined, accessFlags);
}

void VulkanUtility::GetPipelineStageAndAccessMask(VulkanAccessStageFlags accessStage, GpuAccessFlags access, VkPipelineStageFlags& outStages, VkAccessFlags& outAccessMask)
{
	outStages = 0;
	outAccessMask = 0;

	const bool isRead = access.IsSet(GpuAccessFlag::Read);
	const bool isWrite = access.IsSet(GpuAccessFlag::Write);

	if(accessStage.IsSet(VulkanAccessStageFlag::DrawIndirect))
	{
		outStages |= VK_PIPELINE_STAGE_DRAW_INDIRECT_BIT;
		if(isRead) outAccessMask |= VK_ACCESS_INDIRECT_COMMAND_READ_BIT;
	}

	if(accessStage.IsSet(VulkanAccessStageFlag::VertexInputAttributes))
	{
		outStages |= VK_PIPELINE_STAGE_VERTEX_INPUT_BIT;
		if(isRead) outAccessMask |= VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT;
	}

	if(accessStage.IsSet(VulkanAccessStageFlag::VertexInputIndices))
	{
		outStages |= VK_PIPELINE_STAGE_VERTEX_INPUT_BIT;
		if(isRead) outAccessMask |= VK_ACCESS_INDEX_READ_BIT;
	}

	if(accessStage.IsSet(VulkanAccessStageFlag::VertexShaderNonUniform))
	{
		outStages |= VK_PIPELINE_STAGE_VERTEX_SHADER_BIT;
		if(isRead) outAccessMask |= VK_ACCESS_SHADER_READ_BIT;
		if(isWrite) outAccessMask |= VK_ACCESS_SHADER_WRITE_BIT;
	}

	if(accessStage.IsSet(VulkanAccessStageFlag::FragmentShaderNonUniform))
	{
		outStages |= VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		if(isRead) outAccessMask |= VK_ACCESS_SHADER_READ_BIT;
		if(isWrite) outAccessMask |= VK_ACCESS_SHADER_WRITE_BIT;
	}

	if(accessStage.IsSet(VulkanAccessStageFlag::ComputeShaderNonUniform))
	{
		outStages |= VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
		if(isRead) outAccessMask |= VK_ACCESS_SHADER_READ_BIT;
		if(isWrite) outAccessMask |= VK_ACCESS_SHADER_WRITE_BIT;
	}

	if(accessStage.IsSet(VulkanAccessStageFlag::VertexShaderUniform))
	{
		outStages |= VK_PIPELINE_STAGE_VERTEX_SHADER_BIT;
		if(isRead) outAccessMask |= VK_ACCESS_UNIFORM_READ_BIT;
	}

	if(accessStage.IsSet(VulkanAccessStageFlag::FragmentShaderUniform))
	{
		outStages |= VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		if(isRead) outAccessMask |= VK_ACCESS_UNIFORM_READ_BIT;
	}

	if(accessStage.IsSet(VulkanAccessStageFlag::ComputeShaderUniform))
	{
		outStages |= VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
		if(isRead) outAccessMask |= VK_ACCESS_UNIFORM_READ_BIT;
	}

	if(accessStage.IsSet(VulkanAccessStageFlag::EarlyFragmentTests))
	{
		outStages |= VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
		if(isRead) outAccessMask |= VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT;
		if(isWrite) outAccessMask |= VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
	}

	if(accessStage.IsSet(VulkanAccessStageFlag::LateFragmentTests))
	{
		outStages |= VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
		if(isRead) outAccessMask |= VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT;
		if(isWrite) outAccessMask |= VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
	}

	if(accessStage.IsSet(VulkanAccessStageFlag::ColorAttachment))
	{
		outStages |= VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		if(isRead) outAccessMask |= VK_ACCESS_COLOR_ATTACHMENT_READ_BIT;
		if(isWrite) outAccessMask |= VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	}

	if(accessStage.IsSet(VulkanAccessStageFlag::Transfer))
	{
		outStages |= VK_PIPELINE_STAGE_TRANSFER_BIT;
		if(isRead) outAccessMask |= VK_ACCESS_TRANSFER_READ_BIT;
		if(isWrite) outAccessMask |= VK_ACCESS_TRANSFER_WRITE_BIT;
	}

	if(accessStage.IsSet(VulkanAccessStageFlag::Host))
	{
		outStages |= VK_PIPELINE_STAGE_HOST_BIT;
		if(isRead) outAccessMask |= VK_ACCESS_HOST_READ_BIT;
		if(isWrite) outAccessMask |= VK_ACCESS_HOST_WRITE_BIT;
	}
}


VulkanAccessStageFlags VulkanUtility::GetVulkanAccessStageFlags(GpuResourceUseFlags usage)
{
	VulkanAccessStageFlags accessStageFlags;

	if(usage.IsSet(GpuResourceUseFlag::ShaderAccess))
	{
		if(usage.IsSet(GpuResourceUseFlag::StageVertexShader))
			accessStageFlags |= VulkanAccessStageFlag::VertexShaderNonUniform;

		if(usage.IsSet(GpuResourceUseFlag::StageFragmentShader))
			accessStageFlags |= VulkanAccessStageFlag::FragmentShaderNonUniform;

		if(usage.IsSet(GpuResourceUseFlag::StageComputeShader))
			accessStageFlags |= VulkanAccessStageFlag::ComputeShaderNonUniform;

		// Assume all stages if none are set explicitly
		if(!usage.IsSetAny(GpuResourceUseFlag::AnyStage))
			accessStageFlags |= VulkanAccessStageFlag::VertexShaderNonUniform | VulkanAccessStageFlag::FragmentShaderNonUniform | VulkanAccessStageFlag::ComputeShaderNonUniform;
	}

	if(usage.IsSet(GpuResourceUseFlag::IndexBuffer))
		accessStageFlags |= VulkanAccessStageFlag::VertexInputIndices;

	if(usage.IsSet(GpuResourceUseFlag::VertexBuffer))
		accessStageFlags |= VulkanAccessStageFlag::VertexInputAttributes;

	if(usage.IsSet(GpuResourceUseFlag::UniformBuffer))
	{
		if(usage.IsSet(GpuResourceUseFlag::StageVertexShader))
			accessStageFlags |= VulkanAccessStageFlag::VertexShaderUniform;

		if(usage.IsSet(GpuResourceUseFlag::StageFragmentShader))
			accessStageFlags |= VulkanAccessStageFlag::FragmentShaderUniform;

		if(usage.IsSet(GpuResourceUseFlag::StageComputeShader))
			accessStageFlags |= VulkanAccessStageFlag::ComputeShaderUniform;

		// Assume all stages if none are set explicitly
		if(!usage.IsSetAny(GpuResourceUseFlag::AnyStage))
			accessStageFlags |= VulkanAccessStageFlag::VertexShaderUniform | VulkanAccessStageFlag::FragmentShaderUniform | VulkanAccessStageFlag::ComputeShaderUniform;
	}

	if(usage.IsSet(GpuResourceUseFlag::Transfer))
		accessStageFlags |= VulkanAccessStageFlag::Transfer;

	if(usage.IsSet(GpuResourceUseFlag::Host))
		accessStageFlags |= VulkanAccessStageFlag::Host;

	if(usage.IsSet(GpuResourceUseFlag::ColorAttachment))
		accessStageFlags |= VulkanAccessStageFlag::ColorAttachment;

	if(usage.IsSet(GpuResourceUseFlag::DepthStencilAttachment))
		accessStageFlags |= VulkanAccessStageFlag::EarlyFragmentTests | VulkanAccessStageFlag::LateFragmentTests;

	return accessStageFlags;
}

VkImageLayout VulkanUtility::GetImageLayout(ImageLayout layout)
{
	switch (layout)
	{
		case ImageLayout::Undefined:
			return VK_IMAGE_LAYOUT_UNDEFINED;

		case ImageLayout::General:
			return VK_IMAGE_LAYOUT_GENERAL;

		case ImageLayout::ColorAttachment:
			return VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		case ImageLayout::DepthStencilAttachment:
			return VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		case ImageLayout::DepthStencilReadOnly:
			return VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;

		case ImageLayout::ShaderReadOnly:
			return VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

		case ImageLayout::TransferSource:
			return VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;

		case ImageLayout::TransferDestination:
			return VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;

		case ImageLayout::Present:
			return VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

		default:
			B3D_LOG(Warning, LogRenderBackend, "Unknown ImageLayout enum value: {0}", (u32)layout);
			return VK_IMAGE_LAYOUT_UNDEFINED;
	}
}

