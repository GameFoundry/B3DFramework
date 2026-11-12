//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DVulkanUtility.h"
#include "B3DVulkanGpuDevice.h"
#include "Image/B3DTexture.h"
#include "Math/B3DArea2.h"
#include "GpuBackend/B3DGpuParameterSet.h"
#include "GpuBackend/B3DGpuBackendUtility.h"
#include "GpuBackend/B3DGpuAccelerationStructure.h"
#include "B3DVulkanGpuBuffer.h"

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
			bool hasStencil = GpuBackendUtility::GetFormatAspects(format).IsSet(GpuTextureAspectFlag::Stencil);

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
		return VK_FORMAT_BC4_UNORM_BLOCK;
	case PF_BC5:
		return VK_FORMAT_BC5_UNORM_BLOCK;
	case PF_BC6H:
		return VK_FORMAT_BC6H_UFLOAT_BLOCK;
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
	case GPT_RAY_GENERATION_PROGRAM:
		return VK_SHADER_STAGE_RAYGEN_BIT_KHR;
	case GPT_MISS_PROGRAM:
		return VK_SHADER_STAGE_MISS_BIT_KHR;
	case GPT_CLOSEST_HIT_PROGRAM:
		return VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR;
	case GPT_ANY_HIT_PROGRAM:
		return VK_SHADER_STAGE_ANY_HIT_BIT_KHR;
	case GPT_INTERSECTION_PROGRAM:
		return VK_SHADER_STAGE_INTERSECTION_BIT_KHR;
	case GPT_CALLABLE_PROGRAM:
		return VK_SHADER_STAGE_CALLABLE_BIT_KHR;
	default:
		break;
	}

	B3D_ASSERT(false && "Unknown GPU program type");
	return VK_SHADER_STAGE_VERTEX_BIT;
}

VkShaderStageFlags VulkanUtility::GetShaderStages(const GpuProgramStageBits& stages)
{
	VkShaderStageFlags flags = 0;
	if(stages.IsSet(GpuProgramStageBit::Vertex))
		flags |= VK_SHADER_STAGE_VERTEX_BIT;

	if(stages.IsSet(GpuProgramStageBit::Fragment))
		flags |= VK_SHADER_STAGE_FRAGMENT_BIT;

	if(stages.IsSet(GpuProgramStageBit::Hull))
		flags |= VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;

	if(stages.IsSet(GpuProgramStageBit::Domain))
		flags |= VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;

	if(stages.IsSet(GpuProgramStageBit::Geometry))
		flags |= VK_SHADER_STAGE_GEOMETRY_BIT;

	if(stages.IsSet(GpuProgramStageBit::Compute))
		flags |= VK_SHADER_STAGE_COMPUTE_BIT;

	if(stages.IsSet(GpuProgramStageBit::RayGeneration))
		flags |= VK_SHADER_STAGE_RAYGEN_BIT_KHR;

	if(stages.IsSet(GpuProgramStageBit::Miss))
		flags |= VK_SHADER_STAGE_MISS_BIT_KHR;

	if(stages.IsSet(GpuProgramStageBit::ClosestHit))
		flags |= VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR;

	if(stages.IsSet(GpuProgramStageBit::AnyHit))
		flags |= VK_SHADER_STAGE_ANY_HIT_BIT_KHR;

	if(stages.IsSet(GpuProgramStageBit::Intersection))
		flags |= VK_SHADER_STAGE_INTERSECTION_BIT_KHR;

	if(stages.IsSet(GpuProgramStageBit::Callable))
		flags |= VK_SHADER_STAGE_CALLABLE_BIT_KHR;

	return flags;
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

VkImageSubresourceRange VulkanUtility::ToVkImageSubresourceRange(const GpuTextureSubresourceRange& subresourceRange)
{
	VkImageSubresourceRange output;
	output.aspectMask = GetAspectMask(subresourceRange.AspectMask);
	output.baseMipLevel = subresourceRange.BaseMipLevel;
	output.levelCount = subresourceRange.MipLevelCount == ~0u ? VK_REMAINING_MIP_LEVELS : subresourceRange.MipLevelCount;
	output.baseArrayLayer = subresourceRange.BaseArrayLayer;
	output.layerCount = subresourceRange.ArrayLayerCount == ~0u ? VK_REMAINING_ARRAY_LAYERS : subresourceRange.ArrayLayerCount;

	return output;
}

bool VulkanUtility::RangeEquals(const VkImageSubresourceRange& a, const VkImageSubresourceRange& b)
{
	return a.baseArrayLayer == b.baseArrayLayer && a.baseMipLevel == b.baseMipLevel && a.layerCount == b.layerCount && a.levelCount == b.levelCount && a.aspectMask == b.aspectMask;
}

void VulkanUtility::GetPipelineStageAndAccessMask(const VulkanGpuDevice& device, GpuStageFlags accessStage, GpuAccessFlags access, VkPipelineStageFlags& outStages, VkAccessFlags& outAccessMask)
{
	outStages = 0;
	outAccessMask = 0;

	// Resting states and stage masks such as GpuStageFlag::All name stages the device may not have
	accessStage &= device.GetSupportedStageFlags();

	const bool isRead = access.IsSet(GpuAccessFlag::Read);
	const bool isWrite = access.IsSet(GpuAccessFlag::Write);

	if(accessStage.IsSet(GpuStageFlag::DrawIndirect))
	{
		outStages |= VK_PIPELINE_STAGE_DRAW_INDIRECT_BIT;
		if(isRead) outAccessMask |= VK_ACCESS_INDIRECT_COMMAND_READ_BIT;
	}

	if(accessStage.IsSet(GpuStageFlag::VertexInputAttributes))
	{
		outStages |= VK_PIPELINE_STAGE_VERTEX_INPUT_BIT;
		if(isRead) outAccessMask |= VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT;
	}

	if(accessStage.IsSet(GpuStageFlag::VertexInputIndices))
	{
		outStages |= VK_PIPELINE_STAGE_VERTEX_INPUT_BIT;
		if(isRead) outAccessMask |= VK_ACCESS_INDEX_READ_BIT;
	}

	if(accessStage.IsSet(GpuStageFlag::VertexShaderNonUniform))
	{
		outStages |= VK_PIPELINE_STAGE_VERTEX_SHADER_BIT;
		if(isRead) outAccessMask |= VK_ACCESS_SHADER_READ_BIT;
		if(isWrite) outAccessMask |= VK_ACCESS_SHADER_WRITE_BIT;
	}

	if(accessStage.IsSet(GpuStageFlag::FragmentShaderNonUniform))
	{
		outStages |= VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		if(isRead) outAccessMask |= VK_ACCESS_SHADER_READ_BIT;
		if(isWrite) outAccessMask |= VK_ACCESS_SHADER_WRITE_BIT;
	}

	if(accessStage.IsSet(GpuStageFlag::ComputeShaderNonUniform))
	{
		outStages |= VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
		if(isRead) outAccessMask |= VK_ACCESS_SHADER_READ_BIT;
		if(isWrite) outAccessMask |= VK_ACCESS_SHADER_WRITE_BIT;
	}

	if(accessStage.IsSet(GpuStageFlag::VertexShaderUniform))
	{
		outStages |= VK_PIPELINE_STAGE_VERTEX_SHADER_BIT;
		if(isRead) outAccessMask |= VK_ACCESS_UNIFORM_READ_BIT;
	}

	if(accessStage.IsSet(GpuStageFlag::FragmentShaderUniform))
	{
		outStages |= VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		if(isRead) outAccessMask |= VK_ACCESS_UNIFORM_READ_BIT;
	}

	if(accessStage.IsSet(GpuStageFlag::ComputeShaderUniform))
	{
		outStages |= VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
		if(isRead) outAccessMask |= VK_ACCESS_UNIFORM_READ_BIT;
	}

	if(accessStage.IsSet(GpuStageFlag::RayTracingShaderNonUniform))
	{
		outStages |= VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR;
		if(isRead) outAccessMask |= VK_ACCESS_SHADER_READ_BIT;
		if(isWrite) outAccessMask |= VK_ACCESS_SHADER_WRITE_BIT;
	}

	if(accessStage.IsSet(GpuStageFlag::RayTracingShaderUniform))
	{
		outStages |= VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR;
		if(isRead) outAccessMask |= VK_ACCESS_UNIFORM_READ_BIT;
	}

	// Rays traced against an acceleration structure read it as an acceleration structure, in the stage of the program that traces them
	if(accessStage.IsSet(GpuStageFlag::VertexShaderAccelerationStructure))
	{
		outStages |= VK_PIPELINE_STAGE_VERTEX_SHADER_BIT;
		if(isRead) outAccessMask |= VK_ACCESS_ACCELERATION_STRUCTURE_READ_BIT_KHR;
	}

	if(accessStage.IsSet(GpuStageFlag::FragmentShaderAccelerationStructure))
	{
		outStages |= VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		if(isRead) outAccessMask |= VK_ACCESS_ACCELERATION_STRUCTURE_READ_BIT_KHR;
	}

	if(accessStage.IsSet(GpuStageFlag::ComputeShaderAccelerationStructure))
	{
		outStages |= VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
		if(isRead) outAccessMask |= VK_ACCESS_ACCELERATION_STRUCTURE_READ_BIT_KHR;
	}

	if(accessStage.IsSet(GpuStageFlag::RayTracingShaderAccelerationStructure))
	{
		outStages |= VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR;
		if(isRead) outAccessMask |= VK_ACCESS_ACCELERATION_STRUCTURE_READ_BIT_KHR;
	}

	// Builds access acceleration structures, and the scratch memory they both read and write, as acceleration structures
	if(accessStage.IsSet(GpuStageFlag::AccelerationStructureBuild))
	{
		outStages |= VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_KHR;
		if(isRead) outAccessMask |= VK_ACCESS_ACCELERATION_STRUCTURE_READ_BIT_KHR;
		if(isWrite) outAccessMask |= VK_ACCESS_ACCELERATION_STRUCTURE_WRITE_BIT_KHR;
	}

	// Builds read their geometry and instances like shaders do
	if(accessStage.IsSet(GpuStageFlag::AccelerationStructureBuildInput))
	{
		outStages |= VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_KHR;
		if(isRead) outAccessMask |= VK_ACCESS_SHADER_READ_BIT;
	}

	if(accessStage.IsSet(GpuStageFlag::EarlyFragmentTests))
	{
		outStages |= VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
		if(isRead) outAccessMask |= VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT;
		if(isWrite) outAccessMask |= VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
	}

	if(accessStage.IsSet(GpuStageFlag::LateFragmentTests))
	{
		outStages |= VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
		if(isRead) outAccessMask |= VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT;
		if(isWrite) outAccessMask |= VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
	}

	if(accessStage.IsSet(GpuStageFlag::ColorAttachment))
	{
		outStages |= VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		if(isRead) outAccessMask |= VK_ACCESS_COLOR_ATTACHMENT_READ_BIT;
		if(isWrite) outAccessMask |= VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	}

	if(accessStage.IsSetAny(GpuStageFlag::Transfer | GpuStageFlag::Resolve))
	{
		outStages |= VK_PIPELINE_STAGE_TRANSFER_BIT;
		if(isRead) outAccessMask |= VK_ACCESS_TRANSFER_READ_BIT;
		if(isWrite) outAccessMask |= VK_ACCESS_TRANSFER_WRITE_BIT;
	}

	if(accessStage.IsSet(GpuStageFlag::Host))
	{
		outStages |= VK_PIPELINE_STAGE_HOST_BIT;
		if(isRead) outAccessMask |= VK_ACCESS_HOST_READ_BIT;
		if(isWrite) outAccessMask |= VK_ACCESS_HOST_WRITE_BIT;
	}
}


VkImageLayout VulkanUtility::ToVkImageLayout(GpuImageLayout layout)
{
	switch (layout)
	{
		case GpuImageLayout::Undefined:
			return VK_IMAGE_LAYOUT_UNDEFINED;

		case GpuImageLayout::General:
			return VK_IMAGE_LAYOUT_GENERAL;

		case GpuImageLayout::ColorAttachment:
			return VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		case GpuImageLayout::DepthStencilAttachment:
			return VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		case GpuImageLayout::DepthStencilReadOnly:
			return VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;

		case GpuImageLayout::DepthReadOnlyStencilAttachment:
			return VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL_KHR;

		case GpuImageLayout::DepthAttachmentStencilReadOnly:
			return VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL_KHR;

		case GpuImageLayout::ShaderReadOnly:
			return VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

		case GpuImageLayout::TransferSource:
			return VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;

		case GpuImageLayout::TransferDestination:
			return VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;

		case GpuImageLayout::ResolveSource:
			return VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;

		case GpuImageLayout::ResolveDestination:
			return VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;

		case GpuImageLayout::Present:
			return VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

		default:
			B3D_LOG(Warning, LogRenderBackend, "Unknown GpuImageLayout enum value: {0}", (u32)layout);
			return VK_IMAGE_LAYOUT_UNDEFINED;
	}
}

GpuImageLayout VulkanUtility::ToGpuImageLayout(VkImageLayout layout)
{
	switch (layout)
	{
		case VK_IMAGE_LAYOUT_UNDEFINED:
			return GpuImageLayout::Undefined;

		case VK_IMAGE_LAYOUT_GENERAL:
			return GpuImageLayout::General;

		case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
			return GpuImageLayout::ColorAttachment;

		case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
			return GpuImageLayout::DepthStencilAttachment;

		case VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL:
			return GpuImageLayout::DepthStencilReadOnly;

		case VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL_KHR:
			return GpuImageLayout::DepthReadOnlyStencilAttachment;

		case VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL_KHR:
			return GpuImageLayout::DepthAttachmentStencilReadOnly;

		case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
			return GpuImageLayout::ShaderReadOnly;

		case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
			return GpuImageLayout::TransferSource;

		case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
			return GpuImageLayout::TransferDestination;

		case VK_IMAGE_LAYOUT_PRESENT_SRC_KHR:
			return GpuImageLayout::Present;

		default:
			B3D_LOG(Warning, LogRenderBackend, "Unsupported VkImageLayout enum value: {0}", (u32)layout);
			return GpuImageLayout::Undefined;
	}
}

bool VulkanUtility::GetAccelerationStructureBuildInformation(const GpuAccelerationStructureInputs& inputs, bool includeAddresses, VkAccelerationStructureBuildGeometryInfoKHR& outInformation,
	FrameVector<u32>& outPrimitiveCounts)
{
	// Offsets go into the addresses, so every build range starts at primitive offset 0 and first vertex 0
	const auto fnGetAddress = [includeAddresses](const TShared<GpuBuffer>& buffer, u32 offset)
	{
		VkDeviceOrHostAddressConstKHR address;
		address.deviceAddress = 0;
		if(includeAddresses && buffer != nullptr)
			address.deviceAddress = static_cast<VulkanGpuBuffer&>(*buffer).GetVulkanResource()->GetDeviceAddress() + offset;

		return address;
	};

	VkBuildAccelerationStructureFlagsKHR buildFlags = 0;
	if(inputs.Flags.IsSet(GpuAccelerationStructureBuildFlag::PreferFastTrace))
		buildFlags |= VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR;

	if(inputs.Flags.IsSet(GpuAccelerationStructureBuildFlag::PreferFastBuild))
		buildFlags |= VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_BUILD_BIT_KHR;

	if(inputs.Flags.IsSet(GpuAccelerationStructureBuildFlag::MinimizeMemory))
		buildFlags |= VK_BUILD_ACCELERATION_STRUCTURE_LOW_MEMORY_BIT_KHR;

	outInformation.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR;
	outInformation.pNext = nullptr;
	outInformation.flags = buildFlags;
	outInformation.mode = VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR;
	outInformation.srcAccelerationStructure = VK_NULL_HANDLE;
	outInformation.dstAccelerationStructure = VK_NULL_HANDLE;
	outInformation.ppGeometries = nullptr;
	outInformation.scratchData.deviceAddress = 0;

	if(inputs.Type == GpuAccelerationStructureType::TopLevel)
	{
		auto* const geometry = B3DFrameAllocate<VkAccelerationStructureGeometryKHR>(1);
		geometry->sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR;
		geometry->pNext = nullptr;
		geometry->geometryType = VK_GEOMETRY_TYPE_INSTANCES_KHR;
		geometry->flags = 0;
		geometry->geometry.instances.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_INSTANCES_DATA_KHR;
		geometry->geometry.instances.pNext = nullptr;
		geometry->geometry.instances.arrayOfPointers = VK_FALSE;
		geometry->geometry.instances.data = fnGetAddress(inputs.InstanceBuffer, inputs.InstanceOffset);

		outInformation.type = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR;
		outInformation.geometryCount = 1;
		outInformation.pGeometries = geometry;

		outPrimitiveCounts.assign(1, inputs.InstanceCount);
		return true;
	}

	const u32 geometryCount = (u32)inputs.Geometries.size();
	auto* const geometries = B3DFrameAllocate<VkAccelerationStructureGeometryKHR>(geometryCount);
	outPrimitiveCounts.resize(geometryCount);

	for(u32 geometryIndex = 0; geometryIndex < geometryCount; geometryIndex++)
	{
		const GpuRayTracingGeometry& source = inputs.Geometries[geometryIndex];
		if(source.VertexType != VET_FLOAT3)
		{
			B3D_LOG(Error, LogRenderBackend, "Geometry {0} of an acceleration structure must have VET_FLOAT3 vertex positions.", geometryIndex);
			return false;
		}

		VkGeometryFlagsKHR geometryFlags = 0;
		if(source.Flags.IsSet(GpuRayTracingGeometryFlag::Opaque))
			geometryFlags |= VK_GEOMETRY_OPAQUE_BIT_KHR;

		if(source.Flags.IsSet(GpuRayTracingGeometryFlag::NoDuplicateAnyHitInvocation))
			geometryFlags |= VK_GEOMETRY_NO_DUPLICATE_ANY_HIT_INVOCATION_BIT_KHR;

		const bool isIndexed = source.IndexBuffer != nullptr;

		VkAccelerationStructureGeometryKHR& geometry = geometries[geometryIndex];
		geometry.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR;
		geometry.pNext = nullptr;
		geometry.geometryType = VK_GEOMETRY_TYPE_TRIANGLES_KHR;
		geometry.flags = geometryFlags;

		VkAccelerationStructureGeometryTrianglesDataKHR& triangles = geometry.geometry.triangles;
		triangles.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_TRIANGLES_DATA_KHR;
		triangles.pNext = nullptr;
		triangles.vertexFormat = VK_FORMAT_R32G32B32_SFLOAT;
		triangles.vertexData = fnGetAddress(source.VertexBuffer, source.VertexOffset);
		triangles.vertexStride = source.VertexStride;
		triangles.maxVertex = source.VertexCount > 0 ? source.VertexCount - 1 : 0;
		triangles.indexType = isIndexed ? GetIndexType(source.IndexType) : VK_INDEX_TYPE_NONE_KHR;
		triangles.indexData = fnGetAddress(source.IndexBuffer, source.IndexOffset);
		triangles.transformData.deviceAddress = 0;

		outPrimitiveCounts[geometryIndex] = (isIndexed ? source.IndexCount : source.VertexCount) / 3;
	}

	outInformation.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
	outInformation.geometryCount = geometryCount;
	outInformation.pGeometries = geometries;
	return true;
}
