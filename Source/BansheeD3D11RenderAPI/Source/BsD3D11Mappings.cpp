//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsD3D11Mappings.h"
#include "BsDebug.h"
#include "BsException.h"

namespace bs { namespace ct
{
	D3D11_TEXTURE_ADDRESS_MODE D3D11Mappings::get(TextureAddressingMode tam)
	{
		switch(tam)
		{
		case TAM_WRAP:
			return D3D11_TEXTURE_ADDRESS_WRAP;
		case TAM_MIRROR:
			return D3D11_TEXTURE_ADDRESS_MIRROR;
		case TAM_CLAMP:
			return D3D11_TEXTURE_ADDRESS_CLAMP;
		case TAM_BORDER:
			return D3D11_TEXTURE_ADDRESS_BORDER;
		}

		return D3D11_TEXTURE_ADDRESS_MIRROR_ONCE;
	}

	D3D11_BLEND D3D11Mappings::get(BlendFactor bf)
	{
		switch(bf)
		{
		case BF_ONE:
			return D3D11_BLEND_ONE;
		case BF_ZERO:
			return D3D11_BLEND_ZERO;
		case BF_DEST_COLOR:
			return D3D11_BLEND_DEST_COLOR;
		case BF_SOURCE_COLOR:
			return D3D11_BLEND_SRC_COLOR;
		case BF_INV_DEST_COLOR:
			return D3D11_BLEND_INV_DEST_COLOR;
		case BF_INV_SOURCE_COLOR:
			return D3D11_BLEND_INV_SRC_COLOR;
		case BF_DEST_ALPHA:
			return D3D11_BLEND_DEST_ALPHA;
		case BF_SOURCE_ALPHA:
			return D3D11_BLEND_SRC_ALPHA;
		case BF_INV_DEST_ALPHA:
			return D3D11_BLEND_INV_DEST_ALPHA;
		case BF_INV_SOURCE_ALPHA:
			return D3D11_BLEND_INV_SRC_ALPHA;
		}

		// Unsupported type
		return D3D11_BLEND_ZERO;
	}

	D3D11_BLEND_OP D3D11Mappings::get(BlendOperation bo)
	{
		switch(bo)
		{
		case BO_ADD:
			return D3D11_BLEND_OP_ADD;
		case BO_SUBTRACT:
			return D3D11_BLEND_OP_SUBTRACT;
		case BO_REVERSE_SUBTRACT:
			return D3D11_BLEND_OP_REV_SUBTRACT;
		case BO_MIN:
			return D3D11_BLEND_OP_MIN;
		case BO_MAX:
			return D3D11_BLEND_OP_MAX;
		}

		// Unsupported type
		return D3D11_BLEND_OP_ADD;
	}

	D3D11_COMPARISON_FUNC D3D11Mappings::get(CompareFunction cf)
	{
		switch(cf)
		{
		case CMPF_ALWAYS_FAIL:
			return D3D11_COMPARISON_NEVER;
		case CMPF_ALWAYS_PASS:
			return D3D11_COMPARISON_ALWAYS;
		case CMPF_LESS:
			return D3D11_COMPARISON_LESS;
		case CMPF_LESS_EQUAL:
			return D3D11_COMPARISON_LESS_EQUAL;
		case CMPF_EQUAL:
			return D3D11_COMPARISON_EQUAL;
		case CMPF_NOT_EQUAL:
			return D3D11_COMPARISON_NOT_EQUAL;
		case CMPF_GREATER_EQUAL:
			return D3D11_COMPARISON_GREATER_EQUAL;
		case CMPF_GREATER:
			return D3D11_COMPARISON_GREATER;
		};

		// Unsupported type
		return D3D11_COMPARISON_ALWAYS;
	}

	D3D11_CULL_MODE D3D11Mappings::get(CullingMode cm)
	{
		switch(cm)
		{
		case CULL_NONE:
			return D3D11_CULL_NONE;
		case CULL_CLOCKWISE:
			return D3D11_CULL_FRONT;
		case CULL_COUNTERCLOCKWISE:
			return D3D11_CULL_BACK;
		}

		// Unsupported type
		return D3D11_CULL_NONE;
	}

	D3D11_FILL_MODE D3D11Mappings::get(PolygonMode mode)
	{
		switch(mode)
		{
		case PM_WIREFRAME:
			return D3D11_FILL_WIREFRAME;
		case PM_SOLID:
			return D3D11_FILL_SOLID;
		}

		return D3D11_FILL_SOLID;
	}

	D3D11_STENCIL_OP D3D11Mappings::get(StencilOperation op, bool invert)
	{
		switch(op)
		{
		case SOP_KEEP:
			return D3D11_STENCIL_OP_KEEP;
		case SOP_ZERO:
			return D3D11_STENCIL_OP_ZERO;
		case SOP_REPLACE:
			return D3D11_STENCIL_OP_REPLACE;
		case SOP_INCREMENT:
			return invert ? D3D11_STENCIL_OP_DECR_SAT : D3D11_STENCIL_OP_INCR_SAT;
		case SOP_DECREMENT:
			return invert ? D3D11_STENCIL_OP_INCR_SAT : D3D11_STENCIL_OP_DECR_SAT;
		case SOP_INCREMENT_WRAP:
			return invert ? D3D11_STENCIL_OP_DECR : D3D11_STENCIL_OP_INCR;
		case SOP_DECREMENT_WRAP:
			return invert ? D3D11_STENCIL_OP_INCR : D3D11_STENCIL_OP_DECR;
		case SOP_INVERT:
			return D3D11_STENCIL_OP_INVERT;
		}

		// Unsupported type
		return D3D11_STENCIL_OP_KEEP;
	}

	DWORD D3D11Mappings::get(FilterType ft)
	{
		switch (ft)
		{
		case FT_MIN:
			return D3D11_MIN_FILTER_SHIFT;
		case FT_MAG:
			return D3D11_MAG_FILTER_SHIFT;
		case FT_MIP:
			return D3D11_MIP_FILTER_SHIFT;
		}

		// Unsupported type
		return D3D11_MIP_FILTER_SHIFT;
	}

	D3D11_FILTER D3D11Mappings::get(const FilterOptions min, const FilterOptions mag, const FilterOptions mip, const bool comparison)
	{
		D3D11_FILTER res;
#define MERGE_FOR_SWITCH(_comparison_, _min_ , _mag_, _mip_ ) ((_comparison_ << 16) | (_min_ << 8) | (_mag_ << 4) | (_mip_))

		switch((MERGE_FOR_SWITCH(comparison, min, mag, mip)))
		{
		case MERGE_FOR_SWITCH(true, FO_POINT, FO_POINT, FO_POINT):
			res = D3D11_FILTER_COMPARISON_MIN_MAG_MIP_POINT;
			break;
		case MERGE_FOR_SWITCH(true, FO_POINT, FO_POINT, FO_LINEAR):
			res = D3D11_FILTER_COMPARISON_MIN_MAG_POINT_MIP_LINEAR;
			break;
		case MERGE_FOR_SWITCH(true, FO_POINT, FO_LINEAR, FO_POINT):
			res = D3D11_FILTER_COMPARISON_MIN_POINT_MAG_LINEAR_MIP_POINT;
			break;
		case MERGE_FOR_SWITCH(true, FO_POINT, FO_LINEAR, FO_LINEAR):
			res = D3D11_FILTER_COMPARISON_MIN_POINT_MAG_MIP_LINEAR;
			break;
		case MERGE_FOR_SWITCH(true, FO_LINEAR, FO_POINT, FO_POINT):
			res = D3D11_FILTER_COMPARISON_MIN_LINEAR_MAG_MIP_POINT;
			break;
		case MERGE_FOR_SWITCH(true, FO_LINEAR, FO_POINT, FO_LINEAR):
			res = D3D11_FILTER_COMPARISON_MIN_LINEAR_MAG_POINT_MIP_LINEAR;
			break;		
		case MERGE_FOR_SWITCH(true, FO_LINEAR, FO_LINEAR, FO_POINT):
			res = D3D11_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT;
			break;
		case MERGE_FOR_SWITCH(true, FO_LINEAR, FO_LINEAR, FO_LINEAR):
			res = D3D11_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR;
			break;
		case MERGE_FOR_SWITCH(true, FO_ANISOTROPIC, FO_ANISOTROPIC, FO_ANISOTROPIC):
			res = D3D11_FILTER_COMPARISON_ANISOTROPIC;
			break;
		case MERGE_FOR_SWITCH(false, FO_POINT, FO_POINT, FO_POINT):
			res = D3D11_FILTER_MIN_MAG_MIP_POINT;
			break;
		case MERGE_FOR_SWITCH(false, FO_POINT, FO_POINT, FO_LINEAR):
			res = D3D11_FILTER_MIN_MAG_POINT_MIP_LINEAR;
			break;
		case MERGE_FOR_SWITCH(false, FO_POINT, FO_LINEAR, FO_POINT):
			res = D3D11_FILTER_MIN_POINT_MAG_LINEAR_MIP_POINT;
			break;
		case MERGE_FOR_SWITCH(false, FO_POINT, FO_LINEAR, FO_LINEAR):
			res = D3D11_FILTER_MIN_POINT_MAG_MIP_LINEAR;
			break;
		case MERGE_FOR_SWITCH(false, FO_LINEAR, FO_POINT, FO_POINT):
			res = D3D11_FILTER_MIN_LINEAR_MAG_MIP_POINT;
			break;
		case MERGE_FOR_SWITCH(false, FO_LINEAR, FO_POINT, FO_LINEAR):
			res = D3D11_FILTER_MIN_LINEAR_MAG_POINT_MIP_LINEAR;
			break;		
		case MERGE_FOR_SWITCH(false, FO_LINEAR, FO_LINEAR, FO_POINT):
			res = D3D11_FILTER_MIN_MAG_LINEAR_MIP_POINT;
			break;
		case MERGE_FOR_SWITCH(false, FO_LINEAR, FO_LINEAR, FO_LINEAR):
			res = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
			break;
		case MERGE_FOR_SWITCH(false, FO_ANISOTROPIC, FO_ANISOTROPIC, FO_ANISOTROPIC):
			res = D3D11_FILTER_ANISOTROPIC;
			break;
		default:
			res = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
		}	

		return res;
	}

	DWORD D3D11Mappings::get(GpuBufferUsage usage)
	{
		DWORD ret = D3D11_USAGE_DEFAULT;

		if (usage & GBU_DYNAMIC)
			ret = D3D11_USAGE_DYNAMIC;

		return ret;
	}

	D3D11_MAP D3D11Mappings::get(GpuLockOptions options, GpuBufferUsage usage)
	{
		D3D11_MAP ret = D3D11_MAP_READ_WRITE;
		if (options == GBL_WRITE_ONLY_DISCARD)
		{
			// D3D doesn't like discard on non-dynamic buffers
			if (usage & GBU_DYNAMIC)
				ret = D3D11_MAP_WRITE_DISCARD;
			else
				ret = D3D11_MAP_WRITE;
		}
		else if (options == GBL_READ_ONLY)
		{
			ret = D3D11_MAP_READ;
		}
		else if (options == GBL_WRITE_ONLY_NO_OVERWRITE)
		{
			ret = D3D11_MAP_WRITE_NO_OVERWRITE; // Only allowed for vertex/index buffers
		}

		return ret;
	}

	DXGI_FORMAT D3D11Mappings::get(VertexElementType type)
	{
		switch (type)
		{
		case VET_COLOR:
		case VET_COLOR_ABGR:
		case VET_COLOR_ARGB:
		case VET_UBYTE4_NORM:
			return DXGI_FORMAT_R8G8B8A8_UNORM;
		case VET_FLOAT1:
			return DXGI_FORMAT_R32_FLOAT;
		case VET_FLOAT2:
			return DXGI_FORMAT_R32G32_FLOAT;
		case VET_FLOAT3:
			return DXGI_FORMAT_R32G32B32_FLOAT;
		case VET_FLOAT4:
			return DXGI_FORMAT_R32G32B32A32_FLOAT;
		case VET_USHORT1:
			return DXGI_FORMAT_R16_UINT;
		case VET_USHORT2:
			return DXGI_FORMAT_R16G16_UINT;
		case VET_USHORT4:
			return DXGI_FORMAT_R16G16B16A16_UINT;
		case VET_SHORT1:
			return DXGI_FORMAT_R16_SINT;
		case VET_SHORT2:
			return DXGI_FORMAT_R16G16_SINT;
		case VET_SHORT4:
			return DXGI_FORMAT_R16G16B16A16_SINT;
		case VET_UINT1:
			return DXGI_FORMAT_R32_UINT;
		case VET_UINT2:
			return DXGI_FORMAT_R32G32_UINT;
		case VET_UINT3:
			return DXGI_FORMAT_R32G32B32_UINT;
		case VET_UINT4:
			return DXGI_FORMAT_R32G32B32A32_UINT;
		case VET_INT1:
			return DXGI_FORMAT_R32_SINT;
		case VET_INT2:
			return DXGI_FORMAT_R32G32_SINT;
		case VET_INT3:
			return DXGI_FORMAT_R32G32B32_SINT;
		case VET_INT4:
			return DXGI_FORMAT_R32G32B32A32_SINT;
		case VET_UBYTE4:
			return DXGI_FORMAT_R8G8B8A8_UINT;
		}

		// Unsupported type
		return DXGI_FORMAT_R32G32B32A32_FLOAT;
	}

	VertexElementSemantic D3D11Mappings::get(LPCSTR sem)
	{
		if(strcmp(sem, "BLENDINDICES") == 0)
			return VES_BLEND_INDICES;
		if(strcmp(sem, "BLENDWEIGHT") == 0)
			return VES_BLEND_WEIGHTS;
		if(strcmp(sem, "COLOR") == 0)
			return VES_COLOR;
		if(strcmp(sem, "NORMAL") == 0)
			return VES_NORMAL;
		if(strcmp(sem, "POSITION") == 0)
			return VES_POSITION;
		if(strcmp(sem, "TEXCOORD") == 0)
			return VES_TEXCOORD;
		if(strcmp(sem, "BINORMAL") == 0)
			return VES_BITANGENT;
		if(strcmp(sem, "TANGENT") == 0)
			return VES_TANGENT;
		if(strcmp(sem, "POSITIONT") == 0)
			return VES_POSITIONT;
		if(strcmp(sem, "PSIZE") == 0) 
			return VES_PSIZE;

		// Unsupported type
		return VES_POSITION;
	}

	LPCSTR D3D11Mappings::get(VertexElementSemantic sem)
	{
		switch (sem)	
		{
		case VES_BLEND_INDICES:
			return "BLENDINDICES";
		case VES_BLEND_WEIGHTS:
			return "BLENDWEIGHT";
		case VES_COLOR:
			return "COLOR";
		case VES_NORMAL:
			return "NORMAL";
		case VES_POSITION:
			return "POSITION";
		case VES_TEXCOORD:
			return "TEXCOORD";
		case VES_BITANGENT:
			return "BINORMAL";
		case VES_TANGENT:
			return "TANGENT";
		case VES_POSITIONT:
			return "POSITIONT";
		case VES_PSIZE:
			return "PSIZE";
		}

		// Unsupported type
		return "";
	}

	VertexElementType D3D11Mappings::getInputType(D3D_REGISTER_COMPONENT_TYPE type)
	{
		switch(type)
		{
		case D3D_REGISTER_COMPONENT_FLOAT32:
			return VET_FLOAT4;
		case D3D_REGISTER_COMPONENT_SINT32:
			return VET_INT4;
		case D3D_REGISTER_COMPONENT_UINT32:
			return VET_UINT4;
		default:
			return VET_FLOAT4;
		}
	}

	void D3D11Mappings::get(const Color& inColor, float* outColor)
	{
		outColor[0] = inColor.r;
		outColor[1] = inColor.g;
		outColor[2] = inColor.b;
		outColor[3] = inColor.a;	
	}

	PixelFormat D3D11Mappings::getPF(DXGI_FORMAT pf)
	{
		switch(pf)
		{
		case DXGI_FORMAT_UNKNOWN:
			return PF_UNKNOWN;
		case DXGI_FORMAT_R32G32B32A32_TYPELESS:
			return PF_UNKNOWN;
		case DXGI_FORMAT_R32G32B32A32_FLOAT:
			return PF_RGBA32F;
		case DXGI_FORMAT_R32G32B32A32_UINT:
			return PF_UNKNOWN;
		case DXGI_FORMAT_R32G32B32A32_SINT:
			return PF_RGBA32I;
		case DXGI_FORMAT_R32G32B32_TYPELESS:
			return PF_UNKNOWN;
		case DXGI_FORMAT_R32G32B32_FLOAT:
			return PF_RGB32F;
		case DXGI_FORMAT_R32G32B32_UINT:
			return PF_UNKNOWN;
		case DXGI_FORMAT_R32G32B32_SINT:
			return PF_RGB32I;
		case DXGI_FORMAT_R16G16B16A16_TYPELESS:
			return PF_UNKNOWN;
		case DXGI_FORMAT_R16G16B16A16_FLOAT:
			return PF_RGBA16F;
		case DXGI_FORMAT_R16G16B16A16_UNORM:
			return PF_RGBA16;
		case DXGI_FORMAT_R16G16B16A16_UINT:
			return PF_UNKNOWN;
		case DXGI_FORMAT_R16G16B16A16_SNORM:
			return PF_RGBA16S;
		case DXGI_FORMAT_R16G16B16A16_SINT:
			return PF_RGBA16I;
		case DXGI_FORMAT_R32G32_TYPELESS:
			return PF_UNKNOWN;
		case DXGI_FORMAT_R32G32_FLOAT:
			return PF_RG32F;
		case DXGI_FORMAT_R32G32_UINT:
			return PF_UNKNOWN;
		case DXGI_FORMAT_R32G32_SINT:
			return PF_RG32I;
		case DXGI_FORMAT_R32G8X24_TYPELESS:
			return PF_UNKNOWN;
		case DXGI_FORMAT_D32_FLOAT_S8X24_UINT:
			return PF_D32_S8X24;
		case DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS:
			return PF_UNKNOWN;
		case DXGI_FORMAT_X32_TYPELESS_G8X24_UINT:
			return PF_UNKNOWN;
		case DXGI_FORMAT_R10G10B10A2_TYPELESS:
			return PF_UNKNOWN;
		case DXGI_FORMAT_R10G10B10A2_UNORM:
			return PF_RGB10A2;
		case DXGI_FORMAT_R10G10B10A2_UINT:
			return PF_UNKNOWN;
		case DXGI_FORMAT_R11G11B10_FLOAT:
			return PF_RG11B10F;
		case DXGI_FORMAT_R8G8B8A8_TYPELESS:
			return PF_UNKNOWN;
		case DXGI_FORMAT_R8G8B8A8_UNORM:
		case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB:
			return PF_RGBA8;
		case DXGI_FORMAT_R8G8B8A8_UINT:
			return PF_UNKNOWN;
		case DXGI_FORMAT_R8G8B8A8_SNORM:
			return PF_RGBA8S;
		case DXGI_FORMAT_R8G8B8A8_SINT:
			return PF_RGBA8I;
		case DXGI_FORMAT_R16G16_TYPELESS:
			return PF_UNKNOWN;
		case DXGI_FORMAT_R16G16_FLOAT:
			return PF_RG16F;
		case DXGI_FORMAT_R16G16_UNORM:
			return PF_RG16;
		case DXGI_FORMAT_R16G16_UINT:
			return PF_UNKNOWN;
		case DXGI_FORMAT_R16G16_SNORM:
			return PF_RG16S;
		case DXGI_FORMAT_R16G16_SINT:
			return PF_RG16I;
		case DXGI_FORMAT_R32_TYPELESS:
			return PF_UNKNOWN;
		case DXGI_FORMAT_D32_FLOAT:
			return PF_D32;
		case DXGI_FORMAT_R32_FLOAT:
			return PF_R32F;
		case DXGI_FORMAT_R32_UINT:
			return PF_UNKNOWN;
		case DXGI_FORMAT_R32_SINT:
			return PF_R32I;
		case DXGI_FORMAT_R24G8_TYPELESS:
			return PF_UNKNOWN;
		case DXGI_FORMAT_D24_UNORM_S8_UINT:
			return PF_D24S8;
		case DXGI_FORMAT_R24_UNORM_X8_TYPELESS:
			return PF_UNKNOWN;
		case DXGI_FORMAT_X24_TYPELESS_G8_UINT:
			return PF_UNKNOWN;
		case DXGI_FORMAT_R8G8_TYPELESS:
			return PF_UNKNOWN;
		case DXGI_FORMAT_R8G8_UNORM:
			return PF_RG8;
		case DXGI_FORMAT_R8G8_UINT:
			return PF_UNKNOWN;
		case DXGI_FORMAT_R8G8_SNORM:
			return PF_RG8S;
		case DXGI_FORMAT_R8G8_SINT:
			return PF_RG8I;
		case DXGI_FORMAT_R16_TYPELESS:
			return PF_UNKNOWN;
		case DXGI_FORMAT_R16_FLOAT:
			return PF_R16F;
		case DXGI_FORMAT_D16_UNORM:
			return PF_D16;
		case DXGI_FORMAT_R16_UNORM:
			return PF_R16;
		case DXGI_FORMAT_R16_UINT:
			return PF_UNKNOWN;
		case DXGI_FORMAT_R16_SNORM:
			return PF_R16S;
		case DXGI_FORMAT_R16_SINT:
			return PF_R16I;
		case DXGI_FORMAT_R8_TYPELESS:
			return PF_UNKNOWN;
		case DXGI_FORMAT_R8_UNORM:
			return PF_R8;
		case DXGI_FORMAT_R8_UINT:
			return PF_UNKNOWN;
		case DXGI_FORMAT_R8_SNORM:
			return PF_R8S;
		case DXGI_FORMAT_R8_SINT:
			return PF_R8I;
		case DXGI_FORMAT_A8_UNORM:
			return PF_UNKNOWN;
		case DXGI_FORMAT_R1_UNORM:
			return PF_UNKNOWN;
		case DXGI_FORMAT_R9G9B9E5_SHAREDEXP:
			return PF_UNKNOWN;
		case DXGI_FORMAT_R8G8_B8G8_UNORM:
			return PF_UNKNOWN;
		case DXGI_FORMAT_G8R8_G8B8_UNORM:
			return PF_UNKNOWN;
		case DXGI_FORMAT_BC1_TYPELESS:
			return PF_UNKNOWN;
		case DXGI_FORMAT_BC1_UNORM:
		case DXGI_FORMAT_BC1_UNORM_SRGB:
			return PF_BC1;
		case DXGI_FORMAT_BC2_TYPELESS:
			return PF_BC2;
		case DXGI_FORMAT_BC2_UNORM:
		case DXGI_FORMAT_BC2_UNORM_SRGB:
			return PF_BC2;
		case DXGI_FORMAT_BC3_TYPELESS:
			return PF_BC3;
		case DXGI_FORMAT_BC3_UNORM:
		case DXGI_FORMAT_BC3_UNORM_SRGB:
			return PF_BC3;
		case DXGI_FORMAT_BC4_TYPELESS:
			return PF_BC4;
		case DXGI_FORMAT_BC4_UNORM:
			return PF_BC4;
		case DXGI_FORMAT_BC4_SNORM:
			return PF_BC4;
		case DXGI_FORMAT_BC5_TYPELESS:
			return PF_BC5;
		case DXGI_FORMAT_BC5_UNORM:
			return PF_BC5;
		case DXGI_FORMAT_BC5_SNORM:
			return PF_BC5;
		case DXGI_FORMAT_BC6H_UF16:
			return PF_BC6H;
		case DXGI_FORMAT_BC6H_SF16:
			return PF_BC6H;
		case DXGI_FORMAT_BC6H_TYPELESS:
			return PF_BC6H;
		case DXGI_FORMAT_BC7_UNORM:
		case DXGI_FORMAT_BC7_UNORM_SRGB:
			return PF_BC7;
		case DXGI_FORMAT_BC7_TYPELESS:
			return PF_BC7;
		case DXGI_FORMAT_B5G6R5_UNORM:
			return PF_UNKNOWN;
		case DXGI_FORMAT_B5G5R5A1_UNORM:
			return PF_UNKNOWN;
		case DXGI_FORMAT_B8G8R8X8_UNORM_SRGB:
		case DXGI_FORMAT_B8G8R8X8_UNORM:
			return PF_BGR8;
		case DXGI_FORMAT_B8G8R8A8_UNORM_SRGB:
		case DXGI_FORMAT_B8G8R8A8_UNORM:
			return PF_BGRA8;
		default:
			return PF_UNKNOWN;
		}
	}

	DXGI_FORMAT D3D11Mappings::getPF(PixelFormat pf, bool gamma)
	{
		switch(pf)
		{
		case PF_R8:
			return DXGI_FORMAT_R8_UNORM;
		case PF_R8S:
			return DXGI_FORMAT_R8_SNORM;
		case PF_R8I:
			return DXGI_FORMAT_R8_SINT;
		case PF_RG8:
			return DXGI_FORMAT_R8G8_UNORM; 
		case PF_RG8S:
			return DXGI_FORMAT_R8G8_SNORM; 
		case PF_RG8I:
			return DXGI_FORMAT_R8G8_SINT; 
		case PF_BGR8:
			if (gamma)
				return DXGI_FORMAT_B8G8R8X8_UNORM_SRGB;
			return DXGI_FORMAT_B8G8R8X8_UNORM;
		case PF_RGB8:
		case PF_RGBA8:
			if (gamma)
				return DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
			return DXGI_FORMAT_R8G8B8A8_UNORM;
		case PF_RGBA8I:
			return DXGI_FORMAT_R8G8B8A8_SINT;
		case PF_RGBA8S:
			return DXGI_FORMAT_R8G8B8A8_SNORM;
		case PF_BGRA8:
			if (gamma)
				return DXGI_FORMAT_B8G8R8A8_UNORM_SRGB;
			return DXGI_FORMAT_B8G8R8A8_UNORM;
		case PF_R16F:
			return DXGI_FORMAT_R16_FLOAT;
		case PF_RG16F:
			return DXGI_FORMAT_R16G16_FLOAT;
		case PF_RGBA16F:
			return DXGI_FORMAT_R16G16B16A16_FLOAT;
		case PF_R32F:
			return DXGI_FORMAT_R32_FLOAT;
		case PF_RG32F:
			return DXGI_FORMAT_R32G32_FLOAT;
		case PF_RGB32F:
			return DXGI_FORMAT_R32G32B32_FLOAT;
		case PF_RGBA32F:
			return DXGI_FORMAT_R32G32B32A32_FLOAT;
		case PF_R16I:
			return DXGI_FORMAT_R16_SINT;
		case PF_RG16I:
			return DXGI_FORMAT_R16G16_SINT;
		case PF_RGBA16I:
			return DXGI_FORMAT_R16G16B16A16_SINT;
		case PF_R32I:
			return DXGI_FORMAT_R32_SINT;
		case PF_RG32I:
			return DXGI_FORMAT_R32G32_SINT;
		case PF_RGB32I:
			return DXGI_FORMAT_R32G32B32_SINT;
		case PF_RGBA32I:
			return DXGI_FORMAT_R32G32B32A32_SINT;
		case PF_R16S:
			return DXGI_FORMAT_R16_SNORM;
		case PF_RG16S:
			return DXGI_FORMAT_R16G16_SNORM;
		case PF_RGBA16S:
			return DXGI_FORMAT_R16G16B16A16_SNORM;
		case PF_R16:
			return DXGI_FORMAT_R16_UNORM;
		case PF_RG16:
			return DXGI_FORMAT_R16G16_UNORM;
		case PF_RGBA16:
			return DXGI_FORMAT_R16G16B16A16_UNORM;
		case PF_BC1:
		case PF_BC1a:
			if(gamma)
				return DXGI_FORMAT_BC1_UNORM_SRGB;
			return DXGI_FORMAT_BC1_UNORM;
		case PF_BC2:
			if (gamma)
				return DXGI_FORMAT_BC2_UNORM_SRGB;
			return DXGI_FORMAT_BC2_UNORM;
		case PF_BC3:
			if (gamma)
				return DXGI_FORMAT_BC3_UNORM_SRGB;
			return DXGI_FORMAT_BC3_UNORM;
		case PF_BC4:
			return DXGI_FORMAT_BC4_UNORM;
		case PF_BC5:
			return DXGI_FORMAT_BC5_UNORM;
		case PF_BC6H:
			return DXGI_FORMAT_BC6H_UF16;
		case PF_BC7:
			if (gamma)
				return DXGI_FORMAT_BC7_UNORM_SRGB;
			return DXGI_FORMAT_BC7_UNORM;
		case PF_D32_S8X24:
			return DXGI_FORMAT_D32_FLOAT_S8X24_UINT;
		case PF_D24S8:
			return DXGI_FORMAT_D24_UNORM_S8_UINT;
		case PF_D32:
			return DXGI_FORMAT_D32_FLOAT;
		case PF_D16:
			return DXGI_FORMAT_D16_UNORM;
		case PF_RG11B10F:
			return DXGI_FORMAT_R11G11B10_FLOAT;
		case PF_RGB10A2:
			return DXGI_FORMAT_R10G10B10A2_UNORM;
		case PF_UNKNOWN:
		default:
			return DXGI_FORMAT_UNKNOWN;
		}
	}

	DXGI_FORMAT D3D11Mappings::getBF(GpuBufferFormat format)
	{
		static bool lookupInitialized = false;

		static DXGI_FORMAT lookup[BF_COUNT];
		if (!lookupInitialized)
		{
			lookup[BF_16X1F] = DXGI_FORMAT_R16_FLOAT;
			lookup[BF_16X2F] = DXGI_FORMAT_R16G16_FLOAT;
			lookup[BF_16X4F] = DXGI_FORMAT_R16G16B16A16_FLOAT;
			lookup[BF_32X1F] = DXGI_FORMAT_R32_FLOAT;
			lookup[BF_32X2F] = DXGI_FORMAT_R32G32_FLOAT;
			lookup[BF_32X3F] = DXGI_FORMAT_R32G32B32_FLOAT;
			lookup[BF_32X4F] = DXGI_FORMAT_R32G32B32A32_FLOAT;
			lookup[BF_8X1] = DXGI_FORMAT_R8_UNORM;
			lookup[BF_8X2] = DXGI_FORMAT_R8G8_UNORM;
			lookup[BF_8X4] = DXGI_FORMAT_R8G8B8A8_UNORM;
			lookup[BF_16X1] = DXGI_FORMAT_R16_UNORM;
			lookup[BF_16X2] = DXGI_FORMAT_R16G16_UNORM;
			lookup[BF_16X4] = DXGI_FORMAT_R16G16B16A16_UNORM;
			lookup[BF_8X1S] = DXGI_FORMAT_R8_SINT;
			lookup[BF_8X2S] = DXGI_FORMAT_R8G8_SINT;
			lookup[BF_8X4S] = DXGI_FORMAT_R8G8B8A8_SINT;
			lookup[BF_16X1S] = DXGI_FORMAT_R16_SINT;
			lookup[BF_16X2S] = DXGI_FORMAT_R16G16_SINT;
			lookup[BF_16X4S] = DXGI_FORMAT_R16G16B16A16_SINT;
			lookup[BF_32X1S] = DXGI_FORMAT_R32_SINT;
			lookup[BF_32X2S] = DXGI_FORMAT_R32G32_SINT;
			lookup[BF_32X3S] = DXGI_FORMAT_R32G32B32_SINT;
			lookup[BF_32X4S] = DXGI_FORMAT_R32G32B32A32_SINT;
			lookup[BF_8X1U] = DXGI_FORMAT_R8_UINT;
			lookup[BF_8X2U] = DXGI_FORMAT_R8G8_UINT;
			lookup[BF_8X4U] = DXGI_FORMAT_R8G8B8A8_UINT;
			lookup[BF_16X1U] = DXGI_FORMAT_R16_UINT;
			lookup[BF_16X2U] = DXGI_FORMAT_R16G16_UINT;
			lookup[BF_16X4U] = DXGI_FORMAT_R16G16B16A16_UINT;
			lookup[BF_32X1U] = DXGI_FORMAT_R32_UINT;
			lookup[BF_32X2U] = DXGI_FORMAT_R32G32_UINT;
			lookup[BF_32X3U] = DXGI_FORMAT_R32G32B32_UINT;
			lookup[BF_32X4U] = DXGI_FORMAT_R32G32B32A32_UINT;

			lookupInitialized = true;
		}
		
		if (format >= BF_COUNT)
			return DXGI_FORMAT_UNKNOWN;

		return lookup[(UINT32)format];
	}

	DXGI_FORMAT D3D11Mappings::getTypelessDepthStencilPF(PixelFormat format)
	{
		switch(format)
		{
		case PF_D32_S8X24:
			return DXGI_FORMAT_R32G8X24_TYPELESS;
		case PF_D24S8:
			return DXGI_FORMAT_R24G8_TYPELESS;
		case PF_D32:
			return DXGI_FORMAT_R32_TYPELESS;
		case PF_D16:
			return DXGI_FORMAT_R16_TYPELESS;
		default:
			return DXGI_FORMAT_UNKNOWN;
		}
	}

	DXGI_FORMAT D3D11Mappings::getShaderResourceDepthStencilPF(PixelFormat format)
	{
		switch (format)
		{
		case PF_D32_S8X24:
			return DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS;
		case PF_D24S8:
			return DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
		case PF_D32:
			return DXGI_FORMAT_R32_FLOAT;
		case PF_D16:
			return DXGI_FORMAT_R16_UNORM;
		default:
			return DXGI_FORMAT_UNKNOWN;
		}
	}

	PixelFormat D3D11Mappings::getClosestSupportedPF(PixelFormat pf, TextureType texType, int usage)
	{
		// Check for any obvious issues first
		PixelUtil::checkFormat(pf, texType, usage);

		// Check for formats that are not supported at all by DX11
		switch(pf)
		{
		case PF_RGB8:
			pf = PF_RGBA8;
			break;
		default:
			break;
		}

		// Check for usage specific format restrictions
		if((usage & TU_LOADSTORE) != 0)
		{
			switch (pf)
			{
			case PF_BGRA8:
				pf = PF_RGBA8;
				break;
			default:
				break;
			}
		}

		return pf;
	}

	D3D11_USAGE D3D11Mappings::getUsage(GpuBufferUsage usage)
	{
		if (isDynamic(usage))
			return D3D11_USAGE_DYNAMIC;
		else
			return D3D11_USAGE_DEFAULT;
	}

	bool D3D11Mappings::isDynamic(GpuBufferUsage usage)
	{
		return (usage & GBU_DYNAMIC) != 0;
	}

	bool D3D11Mappings::isMappingWrite(D3D11_MAP map)
	{
		if(map == D3D11_MAP_READ)
			return false;

		return true;
	}

	bool D3D11Mappings::isMappingRead(D3D11_MAP map)
	{
		if(map == D3D11_MAP_READ || map == D3D11_MAP_READ_WRITE)
			return true;

		return false;
	}

	UINT D3D11Mappings::getAccessFlags(GpuBufferUsage usage)
	{
		if(isDynamic(usage))
			return D3D11_CPU_ACCESS_WRITE;
		else
			return 0;
	}

	D3D11_PRIMITIVE_TOPOLOGY D3D11Mappings::getPrimitiveType(DrawOperationType type)
	{
		switch(type)
		{
		case DOT_POINT_LIST:
			return D3D11_PRIMITIVE_TOPOLOGY_POINTLIST;
		case DOT_LINE_LIST:
			return D3D11_PRIMITIVE_TOPOLOGY_LINELIST;
		case DOT_LINE_STRIP:
			return D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP;
		case DOT_TRIANGLE_LIST:
			return D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
		case DOT_TRIANGLE_STRIP:
			return D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP;
		case DOT_TRIANGLE_FAN:
			BS_EXCEPT(InvalidParametersException, "D3D11 doesn't support triangle fan primitive type.");
		}

		return D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	}

	UINT32 D3D11Mappings::getSizeInBytes(PixelFormat pf, UINT32 width, UINT32 height)
	{
		if(width == 0 || height == 0)
			return 0;

		if(PixelUtil::isCompressed(pf))
		{
			// D3D wants the width of one row of cells in bytes
			if (pf == PF_BC1 || pf == PF_BC4)
			{
				// 64 bits (8 bytes) per 4x4 block
				return std::max<UINT32>(1, width / 4) * std::max<UINT32>(1, height / 4) * 8;
			}
			else
			{
				// 128 bits (16 bytes) per 4x4 block
				return std::max<UINT32>(1, width / 4) * std::max<UINT32>(1, height / 4) * 16;
			}
		}
		else
		{
			return width * height * PixelUtil::getNumElemBytes(pf);
		}
	}

	D3D11_MAP D3D11Mappings::getLockOptions(GpuLockOptions lockOptions)
	{
		switch(lockOptions)
		{
		case GBL_WRITE_ONLY_NO_OVERWRITE:
			return D3D11_MAP_WRITE_NO_OVERWRITE;
			break;
		case GBL_READ_WRITE:
			return D3D11_MAP_READ_WRITE;
			break;
		case GBL_WRITE_ONLY_DISCARD:
			return D3D11_MAP_WRITE_DISCARD;
			break;
		case GBL_READ_ONLY:
			return D3D11_MAP_READ;
			break;
		case GBL_WRITE_ONLY:
			return D3D11_MAP_WRITE;
			break;
		default: 
			break;
		};

		BS_EXCEPT(RenderingAPIException, "Invalid lock option. No DX11 equivalent of: " + toString(lockOptions));
		return D3D11_MAP_WRITE;
	}
}}