//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsVulkanUtility.h"
#include "BsException.h"

namespace BansheeEngine
{
	VkFormat VulkanUtility::getPixelFormat(PixelFormat format, bool sRGB)
	{
		switch (format)
		{
		case PF_R8:
			if(sRGB)
				return VK_FORMAT_R8_SRGB;

			return VK_FORMAT_R8_UNORM;
		case PF_R8G8:
			if (sRGB)
				return VK_FORMAT_R8G8_SRGB;

			return VK_FORMAT_R8G8_UNORM;
		case PF_R8G8B8:
			if (sRGB)
				return VK_FORMAT_R8G8B8_SRGB;

			return VK_FORMAT_R8G8B8_UNORM;
		case PF_R8G8B8A8:
			if (sRGB)
				return VK_FORMAT_R8G8B8A8_SRGB;

			return VK_FORMAT_R8G8B8A8_UNORM;
		case PF_B8G8R8A8:
		case PF_B8G8R8X8:
			if (sRGB)
				return VK_FORMAT_B8G8R8A8_SRGB;

			return VK_FORMAT_B8G8R8A8_UNORM;
		case PF_FLOAT16_R:
			return VK_FORMAT_R16_SFLOAT;
		case PF_FLOAT16_RG:
			return VK_FORMAT_R16G16_SFLOAT;
		case PF_FLOAT16_RGB:
			return VK_FORMAT_R16G16B16_SFLOAT;
		case PF_FLOAT16_RGBA:
			return VK_FORMAT_R16G16B16A16_SFLOAT;
		case PF_FLOAT32_R:
			return VK_FORMAT_R32_SFLOAT;
		case PF_FLOAT32_RG:
			return VK_FORMAT_R32G32_SFLOAT;
		case PF_FLOAT32_RGB:
			return VK_FORMAT_R32G32B32_SFLOAT;
		case PF_FLOAT32_RGBA:
			return VK_FORMAT_R32G32B32A32_SFLOAT;
		case PF_BC1:
			if (sRGB)
				return VK_FORMAT_BC1_RGB_SRGB_BLOCK;

			return VK_FORMAT_BC1_RGB_UNORM_BLOCK;
		case PF_BC1a:
			if (sRGB)
				return VK_FORMAT_BC1_RGBA_SRGB_BLOCK;

			return VK_FORMAT_BC1_RGBA_UNORM_BLOCK;
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
		case PF_FLOAT_R11G11B10:
			return VK_FORMAT_B10G11R11_UFLOAT_PACK32;
		case PF_UNORM_R10G10B10A2:
			return VK_FORMAT_A2B10G10R10_UNORM_PACK32;
		case PF_A8R8G8B8:
		case PF_A8B8G8R8:
		case PF_X8R8G8B8:
		case PF_X8B8G8R8:
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

		BS_EXCEPT(RenderingAPIException, "Unsupported sample count: " + toString(numSamples));
		return VK_SAMPLE_COUNT_1_BIT;
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
			return VK_CULL_MODE_BACK_BIT;
		case CULL_COUNTERCLOCKWISE:
			return VK_CULL_MODE_FRONT_BIT;
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

	VulkanQueueType VulkanUtility::getQueueType(CommandBufferType type)
	{
		switch(type)
		{
		case CBT_GRAPHICS:
			return VQT_GRAPHICS;
		case CBT_COMPUTE:
			return VQT_COMPUTE;
		case CBT_UPLOAD:
			return VQT_UPLOAD;
		}

		// Unsupported type
		return VQT_GRAPHICS;
	}
}
