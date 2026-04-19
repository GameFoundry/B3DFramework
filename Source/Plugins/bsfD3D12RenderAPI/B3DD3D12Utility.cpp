//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DD3D12Utility.h"

using namespace b3d;
using namespace b3d::render;

DXGI_FORMAT D3D12Utility::GetDXGIFormat(PixelFormat format)
{
	switch (format)
	{
	case PF_R8G8B8A8:
		return DXGI_FORMAT_R8G8B8A8_UNORM;
	case PF_B8G8R8A8:
		return DXGI_FORMAT_B8G8R8A8_UNORM;
	case PF_R8G8B8:
		return DXGI_FORMAT_R8G8B8A8_UNORM; // No 24-bit format, use 32-bit
	case PF_R32G32B32A32F:
		return DXGI_FORMAT_R32G32B32A32_FLOAT;
	case PF_R32G32B32F:
		return DXGI_FORMAT_R32G32B32_FLOAT;
	case PF_R32G32F:
		return DXGI_FORMAT_R32G32_FLOAT;
	case PF_R32F:
		return DXGI_FORMAT_R32_FLOAT;
	case PF_R16G16B16A16F:
		return DXGI_FORMAT_R16G16B16A16_FLOAT;
	case PF_R16G16F:
		return DXGI_FORMAT_R16G16_FLOAT;
	case PF_R16F:
		return DXGI_FORMAT_R16_FLOAT;
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

D3D12_COMPARISON_FUNC D3D12Utility::GetComparisonFunc(ComparisonFunction func)
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

u32 D3D12Utility::CalcConstantBufferElementSizeAndOffset(GpuDataParameterType type, u32 arraySize, u32& offset)
{
	// HLSL constant buffer packing rules (similar to std140)
	// Each element is aligned to 16-byte (float4) boundaries

	u32 size = 0;
	u32 alignment = 0;

	switch (type)
	{
	case GPDT_FLOAT1:
		size = 1;
		alignment = 1;
		break;
	case GPDT_FLOAT2:
		size = 2;
		alignment = 2;
		break;
	case GPDT_FLOAT3:
		size = 3;
		alignment = 4; // float3 aligns to float4 boundary
		break;
	case GPDT_FLOAT4:
		size = 4;
		alignment = 4;
		break;
	case GPDT_MATRIX_3X3:
		size = 12; // 3 float4s
		alignment = 4;
		break;
	case GPDT_MATRIX_4X4:
		size = 16; // 4 float4s
		alignment = 4;
		break;
	case GPDT_INT1:
		size = 1;
		alignment = 1;
		break;
	case GPDT_INT2:
		size = 2;
		alignment = 2;
		break;
	case GPDT_INT3:
		size = 3;
		alignment = 4;
		break;
	case GPDT_INT4:
		size = 4;
		alignment = 4;
		break;
	case GPDT_BOOL:
		size = 1;
		alignment = 1;
		break;
	case GPDT_STRUCT:
		// Structs handled separately
		size = 0;
		alignment = 4;
		break;
	default:
		size = 0;
		alignment = 1;
		break;
	}

	// Align offset to the element's alignment requirement
	if (alignment > 0 && offset % alignment != 0)
	{
		offset = (offset / alignment + 1) * alignment;
	}

	return size;
}
