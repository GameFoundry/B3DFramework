//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DD3D12Utility.h"

using namespace b3d;
using namespace b3d::render;

namespace
{
	/** Promotes a linear DXGI format to its sRGB variant, for formats that have one. */
	DXGI_FORMAT PromoteToSRGB(DXGI_FORMAT format)
	{
		switch (format)
		{
		case DXGI_FORMAT_R8G8B8A8_UNORM:
			return DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
		case DXGI_FORMAT_B8G8R8A8_UNORM:
			return DXGI_FORMAT_B8G8R8A8_UNORM_SRGB;
		case DXGI_FORMAT_BC1_UNORM:
			return DXGI_FORMAT_BC1_UNORM_SRGB;
		case DXGI_FORMAT_BC2_UNORM:
			return DXGI_FORMAT_BC2_UNORM_SRGB;
		case DXGI_FORMAT_BC3_UNORM:
			return DXGI_FORMAT_BC3_UNORM_SRGB;
		case DXGI_FORMAT_BC7_UNORM:
			return DXGI_FORMAT_BC7_UNORM_SRGB;
		default:
			return format;
		}
	}
}

DXGI_FORMAT D3D12Utility::GetDXGIFormat(PixelFormat format, bool sRGB)
{
	if (sRGB)
		return PromoteToSRGB(GetDXGIFormat(format, false));

	switch (format)
	{
	case PF_R8:
		return DXGI_FORMAT_R8_UNORM;
	case PF_RG8:
		return DXGI_FORMAT_R8G8_UNORM;
	case PF_RGB8: // No 24-bit format, use 32-bit
	case PF_RGBA8:
		return DXGI_FORMAT_R8G8B8A8_UNORM;
	case PF_BGR8: // No 24-bit format, use 32-bit
	case PF_BGRA8:
		return DXGI_FORMAT_B8G8R8A8_UNORM;
	case PF_R8S:
		return DXGI_FORMAT_R8_SNORM;
	case PF_RG8S:
		return DXGI_FORMAT_R8G8_SNORM;
	case PF_RGBA8S:
		return DXGI_FORMAT_R8G8B8A8_SNORM;
	case PF_R8I:
		return DXGI_FORMAT_R8_SINT;
	case PF_RG8I:
		return DXGI_FORMAT_R8G8_SINT;
	case PF_RGBA8I:
		return DXGI_FORMAT_R8G8B8A8_SINT;
	case PF_R8U:
		return DXGI_FORMAT_R8_UINT;
	case PF_RG8U:
		return DXGI_FORMAT_R8G8_UINT;
	case PF_RGBA8U:
		return DXGI_FORMAT_R8G8B8A8_UINT;
	case PF_R16:
		return DXGI_FORMAT_R16_UNORM;
	case PF_RG16:
		return DXGI_FORMAT_R16G16_UNORM;
	case PF_RGBA16:
		return DXGI_FORMAT_R16G16B16A16_UNORM;
	case PF_R16S:
		return DXGI_FORMAT_R16_SNORM;
	case PF_RG16S:
		return DXGI_FORMAT_R16G16_SNORM;
	case PF_RGBA16S:
		return DXGI_FORMAT_R16G16B16A16_SNORM;
	case PF_R16I:
		return DXGI_FORMAT_R16_SINT;
	case PF_RG16I:
		return DXGI_FORMAT_R16G16_SINT;
	case PF_RGBA16I:
		return DXGI_FORMAT_R16G16B16A16_SINT;
	case PF_R16U:
		return DXGI_FORMAT_R16_UINT;
	case PF_RG16U:
		return DXGI_FORMAT_R16G16_UINT;
	case PF_RGBA16U:
		return DXGI_FORMAT_R16G16B16A16_UINT;
	case PF_R16F:
		return DXGI_FORMAT_R16_FLOAT;
	case PF_RG16F:
		return DXGI_FORMAT_R16G16_FLOAT;
	case PF_RGBA16F:
		return DXGI_FORMAT_R16G16B16A16_FLOAT;
	case PF_R32I:
		return DXGI_FORMAT_R32_SINT;
	case PF_RG32I:
		return DXGI_FORMAT_R32G32_SINT;
	case PF_RGB32I:
		return DXGI_FORMAT_R32G32B32_SINT;
	case PF_RGBA32I:
		return DXGI_FORMAT_R32G32B32A32_SINT;
	case PF_R32U:
		return DXGI_FORMAT_R32_UINT;
	case PF_RG32U:
		return DXGI_FORMAT_R32G32_UINT;
	case PF_RGB32U:
		return DXGI_FORMAT_R32G32B32_UINT;
	case PF_RGBA32U:
		return DXGI_FORMAT_R32G32B32A32_UINT;
	case PF_R32F:
		return DXGI_FORMAT_R32_FLOAT;
	case PF_RG32F:
		return DXGI_FORMAT_R32G32_FLOAT;
	case PF_RGB32F:
		return DXGI_FORMAT_R32G32B32_FLOAT;
	case PF_RGBA32F:
		return DXGI_FORMAT_R32G32B32A32_FLOAT;
	case PF_BC1:
	case PF_BC1a:
		return DXGI_FORMAT_BC1_UNORM;
	case PF_BC2:
		return DXGI_FORMAT_BC2_UNORM;
	case PF_BC3:
		return DXGI_FORMAT_BC3_UNORM;
	case PF_BC4:
		return DXGI_FORMAT_BC4_UNORM;
	case PF_BC5:
		return DXGI_FORMAT_BC5_UNORM;
	case PF_BC6H:
		return DXGI_FORMAT_BC6H_UF16;
	case PF_BC7:
		return DXGI_FORMAT_BC7_UNORM;
	case PF_RG11B10F:
		return DXGI_FORMAT_R11G11B10_FLOAT;
	case PF_RGB10A2:
		return DXGI_FORMAT_R10G10B10A2_UNORM;
	case PF_D32_S8X24:
		return DXGI_FORMAT_D32_FLOAT_S8X24_UINT;
	case PF_D24S8:
		return DXGI_FORMAT_D24_UNORM_S8_UINT;
	case PF_D32:
		return DXGI_FORMAT_D32_FLOAT;
	case PF_D16:
		return DXGI_FORMAT_D16_UNORM;
	default:
		return DXGI_FORMAT_UNKNOWN;
	}
}

PixelFormat D3D12Utility::GetPixelFormat(DXGI_FORMAT format)
{
	switch (format)
	{
	case DXGI_FORMAT_R8_UNORM:
		return PF_R8;
	case DXGI_FORMAT_R8G8_UNORM:
		return PF_RG8;
	case DXGI_FORMAT_R8G8B8A8_UNORM:
		return PF_RGBA8;
	case DXGI_FORMAT_B8G8R8A8_UNORM:
		return PF_BGRA8;
	case DXGI_FORMAT_R8_SNORM:
		return PF_R8S;
	case DXGI_FORMAT_R8G8_SNORM:
		return PF_RG8S;
	case DXGI_FORMAT_R8G8B8A8_SNORM:
		return PF_RGBA8S;
	case DXGI_FORMAT_R8_SINT:
		return PF_R8I;
	case DXGI_FORMAT_R8G8_SINT:
		return PF_RG8I;
	case DXGI_FORMAT_R8G8B8A8_SINT:
		return PF_RGBA8I;
	case DXGI_FORMAT_R8_UINT:
		return PF_R8U;
	case DXGI_FORMAT_R8G8_UINT:
		return PF_RG8U;
	case DXGI_FORMAT_R8G8B8A8_UINT:
		return PF_RGBA8U;
	case DXGI_FORMAT_R16_UNORM:
		return PF_R16;
	case DXGI_FORMAT_R16G16_UNORM:
		return PF_RG16;
	case DXGI_FORMAT_R16G16B16A16_UNORM:
		return PF_RGBA16;
	case DXGI_FORMAT_R16_SNORM:
		return PF_R16S;
	case DXGI_FORMAT_R16G16_SNORM:
		return PF_RG16S;
	case DXGI_FORMAT_R16G16B16A16_SNORM:
		return PF_RGBA16S;
	case DXGI_FORMAT_R16_SINT:
		return PF_R16I;
	case DXGI_FORMAT_R16G16_SINT:
		return PF_RG16I;
	case DXGI_FORMAT_R16G16B16A16_SINT:
		return PF_RGBA16I;
	case DXGI_FORMAT_R16_UINT:
		return PF_R16U;
	case DXGI_FORMAT_R16G16_UINT:
		return PF_RG16U;
	case DXGI_FORMAT_R16G16B16A16_UINT:
		return PF_RGBA16U;
	case DXGI_FORMAT_R16_FLOAT:
		return PF_R16F;
	case DXGI_FORMAT_R16G16_FLOAT:
		return PF_RG16F;
	case DXGI_FORMAT_R16G16B16A16_FLOAT:
		return PF_RGBA16F;
	case DXGI_FORMAT_R32_SINT:
		return PF_R32I;
	case DXGI_FORMAT_R32G32_SINT:
		return PF_RG32I;
	case DXGI_FORMAT_R32G32B32_SINT:
		return PF_RGB32I;
	case DXGI_FORMAT_R32G32B32A32_SINT:
		return PF_RGBA32I;
	case DXGI_FORMAT_R32_UINT:
		return PF_R32U;
	case DXGI_FORMAT_R32G32_UINT:
		return PF_RG32U;
	case DXGI_FORMAT_R32G32B32_UINT:
		return PF_RGB32U;
	case DXGI_FORMAT_R32G32B32A32_UINT:
		return PF_RGBA32U;
	case DXGI_FORMAT_R32_FLOAT:
		return PF_R32F;
	case DXGI_FORMAT_R32G32_FLOAT:
		return PF_RG32F;
	case DXGI_FORMAT_R32G32B32_FLOAT:
		return PF_RGB32F;
	case DXGI_FORMAT_R32G32B32A32_FLOAT:
		return PF_RGBA32F;
	case DXGI_FORMAT_BC1_UNORM:
		return PF_BC1;
	case DXGI_FORMAT_BC2_UNORM:
		return PF_BC2;
	case DXGI_FORMAT_BC3_UNORM:
		return PF_BC3;
	case DXGI_FORMAT_BC4_UNORM:
		return PF_BC4;
	case DXGI_FORMAT_BC5_UNORM:
		return PF_BC5;
	case DXGI_FORMAT_BC6H_UF16:
		return PF_BC6H;
	case DXGI_FORMAT_BC7_UNORM:
		return PF_BC7;
	case DXGI_FORMAT_R11G11B10_FLOAT:
		return PF_RG11B10F;
	case DXGI_FORMAT_R10G10B10A2_UNORM:
		return PF_RGB10A2;
	case DXGI_FORMAT_D32_FLOAT_S8X24_UINT:
		return PF_D32_S8X24;
	case DXGI_FORMAT_D24_UNORM_S8_UINT:
		return PF_D24S8;
	case DXGI_FORMAT_D32_FLOAT:
		return PF_D32;
	case DXGI_FORMAT_D16_UNORM:
		return PF_D16;
	default:
		return PF_UNKNOWN;
	}
}

D3D12_RESOURCE_FLAGS D3D12Utility::GetBufferResourceFlags(GpuBufferFlags flags)
{
	D3D12_RESOURCE_FLAGS output = D3D12_RESOURCE_FLAG_NONE;

	if (flags.IsSet(GpuBufferFlag::AllowUnorderedAccessOnTheGPU))
		output |= D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;

	return output;
}

D3D12_RESOURCE_FLAGS D3D12Utility::GetTextureResourceFlags(TextureUsageFlags usage)
{
	D3D12_RESOURCE_FLAGS output = D3D12_RESOURCE_FLAG_NONE;

	if (usage.IsSet(TextureUsageFlag::RenderTarget))
		output |= D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;

	if (usage.IsSet(TextureUsageFlag::DepthStencil))
		output |= D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

	if (usage.IsSet(TextureUsageFlag::AllowUnorderedAccessOnTheGPU))
		output |= D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;

	return output;
}

D3D12_HEAP_TYPE D3D12Utility::GetHeapType(GpuBufferType type, GpuBufferFlags flags)
{
	switch (type)
	{
	case GpuBufferType::StagingWrite:
		return D3D12_HEAP_TYPE_UPLOAD;
	case GpuBufferType::StagingRead:
		return D3D12_HEAP_TYPE_READBACK;
	default:
		break;
	}

	if (flags.IsSet(GpuBufferFlag::StoreOnCPUWithGPUAccess))
		return D3D12_HEAP_TYPE_UPLOAD;

	return D3D12_HEAP_TYPE_DEFAULT;
}

D3D12_COMPARISON_FUNC D3D12Utility::GetComparisonFunc(CompareFunction func)
{
	switch (func)
	{
	case CMPF_ALWAYS_FAIL:
		return D3D12_COMPARISON_FUNC_NEVER;
	case CMPF_ALWAYS_PASS:
		return D3D12_COMPARISON_FUNC_ALWAYS;
	case CMPF_LESS:
		return D3D12_COMPARISON_FUNC_LESS;
	case CMPF_LESS_EQUAL:
		return D3D12_COMPARISON_FUNC_LESS_EQUAL;
	case CMPF_EQUAL:
		return D3D12_COMPARISON_FUNC_EQUAL;
	case CMPF_NOT_EQUAL:
		return D3D12_COMPARISON_FUNC_NOT_EQUAL;
	case CMPF_GREATER_EQUAL:
		return D3D12_COMPARISON_FUNC_GREATER_EQUAL;
	case CMPF_GREATER:
		return D3D12_COMPARISON_FUNC_GREATER;
	default:
		return D3D12_COMPARISON_FUNC_ALWAYS;
	}
}

D3D12_TEXTURE_ADDRESS_MODE D3D12Utility::GetTextureAddressMode(TextureAddressingMode mode)
{
	switch (mode)
	{
	case TAM_WRAP:
		return D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	case TAM_MIRROR:
		return D3D12_TEXTURE_ADDRESS_MODE_MIRROR;
	case TAM_CLAMP:
		return D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	case TAM_BORDER:
		return D3D12_TEXTURE_ADDRESS_MODE_BORDER;
	default:
		return D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	}
}

D3D12_FILTER D3D12Utility::GetFilter(FilterOptions minFilter, FilterOptions magFilter, FilterOptions mipFilter)
{
	if (minFilter == FO_ANISOTROPIC || magFilter == FO_ANISOTROPIC || mipFilter == FO_ANISOTROPIC)
		return D3D12_FILTER_ANISOTROPIC;

	const bool minLinear = minFilter == FO_LINEAR;
	const bool magLinear = magFilter == FO_LINEAR;
	const bool mipLinear = mipFilter == FO_LINEAR;

	if (minLinear)
	{
		if (magLinear)
			return mipLinear ? D3D12_FILTER_MIN_MAG_MIP_LINEAR : D3D12_FILTER_MIN_MAG_LINEAR_MIP_POINT;

		return mipLinear ? D3D12_FILTER_MIN_LINEAR_MAG_POINT_MIP_LINEAR : D3D12_FILTER_MIN_LINEAR_MAG_MIP_POINT;
	}

	if (magLinear)
		return mipLinear ? D3D12_FILTER_MIN_POINT_MAG_MIP_LINEAR : D3D12_FILTER_MIN_POINT_MAG_LINEAR_MIP_POINT;

	return mipLinear ? D3D12_FILTER_MIN_MAG_POINT_MIP_LINEAR : D3D12_FILTER_MIN_MAG_MIP_POINT;
}

D3D12_BLEND D3D12Utility::GetBlend(BlendFactor factor)
{
	switch (factor)
	{
	case BF_ONE:
		return D3D12_BLEND_ONE;
	case BF_ZERO:
		return D3D12_BLEND_ZERO;
	case BF_DEST_COLOR:
		return D3D12_BLEND_DEST_COLOR;
	case BF_SOURCE_COLOR:
		return D3D12_BLEND_SRC_COLOR;
	case BF_INV_DEST_COLOR:
		return D3D12_BLEND_INV_DEST_COLOR;
	case BF_INV_SOURCE_COLOR:
		return D3D12_BLEND_INV_SRC_COLOR;
	case BF_DEST_ALPHA:
		return D3D12_BLEND_DEST_ALPHA;
	case BF_SOURCE_ALPHA:
		return D3D12_BLEND_SRC_ALPHA;
	case BF_INV_DEST_ALPHA:
		return D3D12_BLEND_INV_DEST_ALPHA;
	case BF_INV_SOURCE_ALPHA:
		return D3D12_BLEND_INV_SRC_ALPHA;
	default:
		return D3D12_BLEND_ONE;
	}
}

D3D12_BLEND_OP D3D12Utility::GetBlendOp(BlendOperation op)
{
	switch (op)
	{
	case BO_ADD:
		return D3D12_BLEND_OP_ADD;
	case BO_SUBTRACT:
		return D3D12_BLEND_OP_SUBTRACT;
	case BO_REVERSE_SUBTRACT:
		return D3D12_BLEND_OP_REV_SUBTRACT;
	case BO_MIN:
		return D3D12_BLEND_OP_MIN;
	case BO_MAX:
		return D3D12_BLEND_OP_MAX;
	default:
		return D3D12_BLEND_OP_ADD;
	}
}

D3D12_CULL_MODE D3D12Utility::GetCullMode(CullingMode mode)
{
	switch (mode)
	{
	case CULL_NONE:
		return D3D12_CULL_MODE_NONE;
	case CULL_CLOCKWISE:
		return D3D12_CULL_MODE_FRONT;
	case CULL_COUNTERCLOCKWISE:
		return D3D12_CULL_MODE_BACK;
	default:
		return D3D12_CULL_MODE_NONE;
	}
}

D3D12_FILL_MODE D3D12Utility::GetFillMode(PolygonMode mode)
{
	switch (mode)
	{
	case PM_WIREFRAME:
		return D3D12_FILL_MODE_WIREFRAME;
	case PM_SOLID:
		return D3D12_FILL_MODE_SOLID;
	default:
		return D3D12_FILL_MODE_SOLID;
	}
}

D3D12_STENCIL_OP D3D12Utility::GetStencilOp(StencilOperation op)
{
	switch (op)
	{
	case SOP_KEEP:
		return D3D12_STENCIL_OP_KEEP;
	case SOP_ZERO:
		return D3D12_STENCIL_OP_ZERO;
	case SOP_REPLACE:
		return D3D12_STENCIL_OP_REPLACE;
	case SOP_INCREMENT:
		return D3D12_STENCIL_OP_INCR_SAT;
	case SOP_DECREMENT:
		return D3D12_STENCIL_OP_DECR_SAT;
	case SOP_INCREMENT_WRAP:
		return D3D12_STENCIL_OP_INCR;
	case SOP_DECREMENT_WRAP:
		return D3D12_STENCIL_OP_DECR;
	case SOP_INVERT:
		return D3D12_STENCIL_OP_INVERT;
	default:
		return D3D12_STENCIL_OP_KEEP;
	}
}

D3D_PRIMITIVE_TOPOLOGY D3D12Utility::GetPrimitiveTopology(DrawOperationType drawOp)
{
	switch (drawOp)
	{
	case DOT_POINT_LIST:
		return D3D_PRIMITIVE_TOPOLOGY_POINTLIST;
	case DOT_LINE_LIST:
		return D3D_PRIMITIVE_TOPOLOGY_LINELIST;
	case DOT_LINE_STRIP:
		return D3D_PRIMITIVE_TOPOLOGY_LINESTRIP;
	case DOT_TRIANGLE_LIST:
		return D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	case DOT_TRIANGLE_STRIP:
		return D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP;
	case DOT_TRIANGLE_FAN:
		return D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST; // Not supported, use triangle list
	default:
		return D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	}
}

D3D12_PRIMITIVE_TOPOLOGY_TYPE D3D12Utility::GetPrimitiveTopologyType(DrawOperationType drawOp)
{
	switch (drawOp)
	{
	case DOT_POINT_LIST:
		return D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT;
	case DOT_LINE_LIST:
	case DOT_LINE_STRIP:
		return D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE;
	case DOT_TRIANGLE_LIST:
	case DOT_TRIANGLE_STRIP:
	case DOT_TRIANGLE_FAN:
		return D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	default:
		return D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	}
}

u32 D3D12Utility::GetBytesPerPixel(DXGI_FORMAT format)
{
	return static_cast<u32>(PixelUtility::GetMemorySize(1, 1, 1, GetPixelFormat(format)));
}

bool D3D12Utility::IsDepthStencilFormat(DXGI_FORMAT format)
{
	switch (format)
	{
	case DXGI_FORMAT_D32_FLOAT_S8X24_UINT:
	case DXGI_FORMAT_D24_UNORM_S8_UINT:
	case DXGI_FORMAT_D32_FLOAT:
	case DXGI_FORMAT_D16_UNORM:
		return true;
	default:
		return false;
	}
}

