//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsD3D9Mappings.h"
#include "BsException.h"
#include "BsMatrix4.h"

namespace BansheeEngine 
{
	D3DTEXTUREADDRESS D3D9Mappings::get(TextureAddressingMode tam, const D3DCAPS9& devCaps)
	{
		switch(tam)
		{
		case TAM_WRAP:
			return D3DTADDRESS_WRAP;
		case TAM_MIRROR:
			return D3DTADDRESS_MIRROR;
		case TAM_CLAMP:
			return D3DTADDRESS_CLAMP;
        case TAM_BORDER:
            if (devCaps.TextureAddressCaps & D3DPTADDRESSCAPS_BORDER)
                return D3DTADDRESS_BORDER;
            else
                return D3DTADDRESS_CLAMP;
		}

		return D3DTADDRESS_FORCE_DWORD;
	}

	D3DBLEND D3D9Mappings::get(BlendFactor sbf)
	{
		switch(sbf)
		{
		case BF_ONE:
			return D3DBLEND_ONE;
		case BF_ZERO:
			return D3DBLEND_ZERO;
		case BF_DEST_COLOR:
			return D3DBLEND_DESTCOLOR;
		case BF_SOURCE_COLOR:
			return D3DBLEND_SRCCOLOR;
		case BF_INV_DEST_COLOR:
			return D3DBLEND_INVDESTCOLOR;
		case BF_INV_SOURCE_COLOR:
			return D3DBLEND_INVSRCCOLOR;
		case BF_DEST_ALPHA:
			return D3DBLEND_DESTALPHA;
		case BF_SOURCE_ALPHA:
			return D3DBLEND_SRCALPHA;
		case BF_INV_DEST_ALPHA:
			return D3DBLEND_INVDESTALPHA;
		case BF_INV_SOURCE_ALPHA:
			return D3DBLEND_INVSRCALPHA;
		}

		return D3DBLEND_FORCE_DWORD;
	}

	D3DBLENDOP D3D9Mappings::get(BlendOperation sbo)
	{
		switch(sbo)
		{
		case BO_ADD:
			return D3DBLENDOP_ADD;
		case BO_SUBTRACT:
			return D3DBLENDOP_SUBTRACT;
		case BO_REVERSE_SUBTRACT:
			return D3DBLENDOP_REVSUBTRACT;
		case BO_MIN:
			return D3DBLENDOP_MIN;
		case BO_MAX:
			return D3DBLENDOP_MAX;
		}

		return D3DBLENDOP_FORCE_DWORD;
	}

	DWORD D3D9Mappings::get(CompareFunction cf)
	{
		switch(cf)
		{
		case CMPF_ALWAYS_FAIL:
			return D3DCMP_NEVER;
		case CMPF_ALWAYS_PASS:
			return D3DCMP_ALWAYS;
		case CMPF_LESS:
			return D3DCMP_LESS;
		case CMPF_LESS_EQUAL:
			return D3DCMP_LESSEQUAL;
		case CMPF_EQUAL:
			return D3DCMP_EQUAL;
		case CMPF_NOT_EQUAL:
			return D3DCMP_NOTEQUAL;
		case CMPF_GREATER_EQUAL:
			return D3DCMP_GREATEREQUAL;
		case CMPF_GREATER:
			return D3DCMP_GREATER;
		};

		return 0;
	}

	DWORD D3D9Mappings::get(CullingMode cm, bool flip)
	{
		switch(cm)
		{
		case CULL_NONE:
			return D3DCULL_NONE;
		case CULL_CLOCKWISE:
			if(flip)
				return D3DCULL_CCW;
			else
				return D3DCULL_CW;
		case CULL_COUNTERCLOCKWISE:
			if(flip)
				return D3DCULL_CW;
			else
				return D3DCULL_CCW;
		}

		return 0;
	}

	D3DFILLMODE D3D9Mappings::get(PolygonMode level)
	{
		switch(level)
		{
		case PM_WIREFRAME:
			return D3DFILL_WIREFRAME;
		case PM_SOLID:
			return D3DFILL_SOLID;
		}

		return D3DFILL_FORCE_DWORD;
	}

	DWORD D3D9Mappings::get(StencilOperation op, bool invert)
	{
		switch(op)
		{
		case SOP_KEEP:
			return D3DSTENCILOP_KEEP;
		case SOP_ZERO:
			return D3DSTENCILOP_ZERO;
		case SOP_REPLACE:
			return D3DSTENCILOP_REPLACE;
		case SOP_INCREMENT:
            return invert ? D3DSTENCILOP_DECRSAT : D3DSTENCILOP_INCRSAT;
		case SOP_DECREMENT:
            return invert ? D3DSTENCILOP_INCRSAT : D3DSTENCILOP_DECRSAT;
		case SOP_INCREMENT_WRAP:
            return invert ? D3DSTENCILOP_DECR : D3DSTENCILOP_INCR;
		case SOP_DECREMENT_WRAP:
            return invert ? D3DSTENCILOP_INCR : D3DSTENCILOP_DECR;
		case SOP_INVERT:
			return D3DSTENCILOP_INVERT;
		}

		return 0;
	}

	D3DSAMPLERSTATETYPE D3D9Mappings::get(FilterType ft)
    {
        switch (ft)
        {
        case FT_MIN:
            return D3DSAMP_MINFILTER;
            break;
        case FT_MAG:
            return D3DSAMP_MAGFILTER;
            break;
        case FT_MIP:
            return D3DSAMP_MIPFILTER;
            break;
        }

        return D3DSAMP_MINFILTER;
    }

	DWORD D3D9Mappings::get(FilterType ft, FilterOptions fo, const D3DCAPS9& devCaps, D3DTexType texType)
	{
		// Assume normal
		DWORD capsType = devCaps.TextureFilterCaps;

		switch( texType )
		{
		case D3D_TEX_TYPE_NORMAL:
			capsType = devCaps.TextureFilterCaps;
			break;
		case D3D_TEX_TYPE_CUBE:
			capsType = devCaps.CubeTextureFilterCaps;
			break;
		case D3D_TEX_TYPE_VOLUME:
			capsType = devCaps.VolumeTextureFilterCaps;
			break;
		}

        switch (ft)
        {
        case FT_MIN:
            switch(fo)
            {
                // NOTE: Fall through if device doesn't support requested type
            case FO_ANISOTROPIC:
                if(capsType & D3DPTFILTERCAPS_MINFANISOTROPIC)
                {
                    return D3DTEXF_ANISOTROPIC;
                    break;
                }
            case FO_LINEAR:
                if(capsType & D3DPTFILTERCAPS_MINFLINEAR)
                {
                    return D3DTEXF_LINEAR;
                    break;
                }
            case FO_POINT:
            case FO_NONE:
                return D3DTEXF_POINT;
                break;
            }
            break;
        case FT_MAG:
            switch( fo )
            {
            // NOTE: Fall through if device doesn't support requested type
            case FO_ANISOTROPIC:
                if(capsType & D3DPTFILTERCAPS_MAGFANISOTROPIC)
                {
                    return D3DTEXF_ANISOTROPIC;
                    break;
                }
            case FO_LINEAR:
                if(capsType & D3DPTFILTERCAPS_MAGFLINEAR)
                {
                    return D3DTEXF_LINEAR;
                    break;
                }
            case FO_POINT:
            case FO_NONE:
                return D3DTEXF_POINT;
                break;
            }
            break;
        case FT_MIP:
            switch( fo )
            {
            case FO_ANISOTROPIC:
            case FO_LINEAR:
                if(capsType & D3DPTFILTERCAPS_MIPFLINEAR)
                {
                    return D3DTEXF_LINEAR;
                    break;
                }
            case FO_POINT:
                if(capsType & D3DPTFILTERCAPS_MIPFPOINT)
                {
                    return D3DTEXF_POINT;
                    break;
                }
            case FO_NONE:
                return D3DTEXF_NONE;
                break;
            }
            break;
        }

        return 0;
	}

	D3D9Mappings::D3DTexType D3D9Mappings::get(TextureType texType)
	{
		switch( texType )
		{
		case TEX_TYPE_1D :
		case TEX_TYPE_2D :
			return D3D9Mappings::D3D_TEX_TYPE_NORMAL;
		case TEX_TYPE_CUBE_MAP :
			return D3D9Mappings::D3D_TEX_TYPE_CUBE;
		case TEX_TYPE_3D :
            return D3D9Mappings::D3D_TEX_TYPE_VOLUME;
		}
		return D3D9Mappings::D3D_TEX_TYPE_NONE;
	}

    DWORD D3D9Mappings::get(GpuBufferUsage usage)
    {
        DWORD ret = 0;
        if (usage & GBU_DYNAMIC)
        {
            ret |= D3DUSAGE_DYNAMIC;
        }

        return ret;
    }

    DWORD D3D9Mappings::get(GpuLockOptions options, GpuBufferUsage usage)
    {
        DWORD ret = 0;
        if (options == GBL_WRITE_ONLY_DISCARD)
        {
            // D3D doesn't like discard or no_overwrite on non-dynamic buffers
            if (usage & GBU_DYNAMIC)
                ret |= D3DLOCK_DISCARD;
        }
        
		if (options == GBL_READ_ONLY)
        {
			ret |= D3DLOCK_READONLY;
        }

        if (options == GBL_WRITE_ONLY_NO_OVERWRITE)
        {
            // D3D doesn't like discard or no_overwrite on non-dynamic buffers
            if (usage & GBU_DYNAMIC)
                ret |= D3DLOCK_NOOVERWRITE;
        }

        return ret;
    }

    D3DFORMAT D3D9Mappings::get(IndexType itype)
    {
        if (itype == IT_32BIT)
        {
            return D3DFMT_INDEX32;
        }
        else
        {
            return D3DFMT_INDEX16;
        }
    }

	D3DDECLTYPE D3D9Mappings::get(VertexElementType vType)
	{
		switch (vType)
		{
		case VET_COLOR:
		case VET_COLOR_ABGR:
		case VET_COLOR_ARGB:
		case VET_UBYTE4_NORM:
			return D3DDECLTYPE_D3DCOLOR;
		case VET_FLOAT1:
			return D3DDECLTYPE_FLOAT1;
		case VET_FLOAT2:
			return D3DDECLTYPE_FLOAT2;
		case VET_FLOAT3:
			return D3DDECLTYPE_FLOAT3;
		case VET_FLOAT4:
			return D3DDECLTYPE_FLOAT4;
        case VET_SHORT2:
			return D3DDECLTYPE_SHORT2;
        case VET_SHORT4:
			return D3DDECLTYPE_SHORT4;
        case VET_UBYTE4:
            return D3DDECLTYPE_UBYTE4;
		default:
			LOGWRN("Invalid vertex element type for DX9.");
			break;
		}

		return D3DDECLTYPE_FLOAT3;
	}

	D3DDECLUSAGE D3D9Mappings::get(VertexElementSemantic sem)
	{
		switch (sem)
		{
		case VES_BLEND_INDICES:
			return D3DDECLUSAGE_BLENDINDICES;
		case VES_BLEND_WEIGHTS:
			return D3DDECLUSAGE_BLENDWEIGHT;
		case VES_COLOR:
			return D3DDECLUSAGE_COLOR;
		case VES_NORMAL:
			return D3DDECLUSAGE_NORMAL;
		case VES_POSITION:
			return D3DDECLUSAGE_POSITION;
		case VES_TEXCOORD:
			return D3DDECLUSAGE_TEXCOORD;
		case VES_BITANGENT:
			return D3DDECLUSAGE_BINORMAL;
		case VES_TANGENT:
			return D3DDECLUSAGE_TANGENT;
		default:
			BS_EXCEPT(RenderingAPIException, "Invalid semantic for D3D9 render system: " + toString(sem));
		}

		return D3DDECLUSAGE_POSITION;
	}

	VertexElementSemantic D3D9Mappings::get(D3DDECLUSAGE sem)
	{
		switch (sem)
		{
		case D3DDECLUSAGE_BLENDINDICES:
			return VES_BLEND_INDICES;
		case D3DDECLUSAGE_BLENDWEIGHT:
			return VES_BLEND_WEIGHTS;
		case D3DDECLUSAGE_COLOR:
			return VES_COLOR;
		case D3DDECLUSAGE_NORMAL:
			return VES_NORMAL;
		case D3DDECLUSAGE_POSITION:
			return VES_POSITION;
		case D3DDECLUSAGE_TEXCOORD:
			return VES_TEXCOORD;
		case D3DDECLUSAGE_BINORMAL:
			return VES_BITANGENT;
		case D3DDECLUSAGE_TANGENT:
			return VES_TANGENT;
		default:
			BS_EXCEPT(RenderingAPIException, "Invalid semantic for D3D9 render system: " + toString(sem));
		}

		return VES_POSITION;
	}

	D3DXMATRIX D3D9Mappings::makeD3DXMatrix(const Matrix4& mat)
	{
		// Transpose matrix
		// D3D9 uses row vectors i.e. V*M
		return D3DXMATRIX(
            mat[0][0], mat[1][0], mat[2][0], mat[3][0],
            mat[0][1], mat[1][1], mat[2][1], mat[3][1],
            mat[0][2], mat[1][2], mat[2][2], mat[3][2],
            mat[0][3], mat[1][3], mat[2][3], mat[3][3]);
	}

	Matrix4 D3D9Mappings::convertD3DXMatrix(const D3DXMATRIX& mat)
	{
		return Matrix4(
            mat.m[0][0], mat.m[1][0], mat.m[2][0], mat.m[3][0],
            mat.m[0][1], mat.m[1][1], mat.m[2][1], mat.m[3][1],
            mat.m[0][2], mat.m[1][2], mat.m[2][2], mat.m[3][2],
            mat.m[0][3], mat.m[1][3], mat.m[2][3], mat.m[3][3]);
	}

	PixelFormat D3D9Mappings::_getPF(D3DFORMAT d3dPF)
	{
		switch(d3dPF)
		{
		case D3DFMT_R8G8B8:
			return PF_B8G8R8;
		case D3DFMT_A8R8G8B8:
			return PF_B8G8R8A8;
		case D3DFMT_A8B8G8R8:
			return PF_R8G8B8A8;
		case D3DFMT_X8B8G8R8:
			return PF_R8G8B8X8;
		case D3DFMT_X8R8G8B8:
			return PF_B8G8R8X8;
		case D3DFMT_R16F:
			return PF_FLOAT16_R;
		case D3DFMT_A16B16G16R16F:
			return PF_FLOAT16_RGBA;
		case D3DFMT_R32F:
			return PF_FLOAT32_R;
		case D3DFMT_G32R32F: // TODO - For some reason G and R are flipped in DX9 which might not be compatible with other render systems directly
			return PF_FLOAT32_RG;
		case D3DFMT_A32B32G32R32F:
			return PF_FLOAT32_RGBA;
		case D3DFMT_G16R16F: // TODO - For some reason G and R are flipped in DX9 which might not be compatible with other render systems directly
			return PF_FLOAT16_RG;
		case D3DFMT_DXT1:
			return PF_BC1;
		case D3DFMT_DXT3:
			return PF_BC2;
		case D3DFMT_DXT5:
			return PF_BC3;
		case D3DFMT_D24S8:
			return PF_D24S8;
		case D3DFMT_D32F_LOCKABLE:
			return PF_D32;
		case D3DFMT_D16:
			return PF_D16;
		case D3DFMT_A2B10G10R10:
			return PF_UNORM_R10G10B10A2;
		default:
			return PF_UNKNOWN;
		}
	}

	D3DFORMAT D3D9Mappings::_getPF(PixelFormat pf)
	{
		// DX9 uses different format semantics than most render systems, where most significant bit is signified by the
		// first channel letter. For example in RGBA8 format, R is most-significant byte. Which is why most formats map to their
		// reverse version in-engine as seen below.
		// (This is not the case for floating point formats, EXCEPT for green-red one)
		switch(pf)
		{
		case PF_B8G8R8A8:
			return D3DFMT_A8R8G8B8;
		case PF_R8G8B8X8:
			return D3DFMT_X8B8G8R8;
		case PF_B8G8R8X8:
			return D3DFMT_X8R8G8B8;
		case PF_FLOAT16_R:
			return D3DFMT_R16F;
		case PF_FLOAT16_RG:
			return D3DFMT_G16R16F;
		case PF_FLOAT16_RGBA:
			return D3DFMT_A16B16G16R16F;
		case PF_FLOAT32_R:
			return D3DFMT_R32F;
		case PF_FLOAT32_RG:
			return D3DFMT_G32R32F;
		case PF_FLOAT32_RGBA:
			return D3DFMT_A32B32G32R32F;
		case PF_BC1:
		case PF_BC1a:
			return D3DFMT_DXT1;
		case PF_BC2:
			return D3DFMT_DXT3;
		case PF_BC3:
			return D3DFMT_DXT5;
		case PF_D24S8:
			return D3DFMT_D24S8;
		case PF_D32:
			return D3DFMT_D32F_LOCKABLE;
		case PF_D16:
			return D3DFMT_D16;
		case PF_UNORM_R10G10B10A2:
			return D3DFMT_A2B10G10R10;
		case PF_UNKNOWN:
		default:
			return D3DFMT_UNKNOWN;
		}
	}

	PixelFormat D3D9Mappings::_getClosestSupportedPF(PixelFormat enginePF)
	{
		if (_getPF(enginePF) != D3DFMT_UNKNOWN)
			return enginePF;

		switch(enginePF)
		{
		case PF_R8:
			return PF_B8G8R8X8;
		case PF_R8G8:
			return PF_B8G8R8X8;
		case PF_B8G8R8:
			return PF_B8G8R8X8;
		case PF_A8R8G8B8:
			return PF_B8G8R8A8;
		case PF_A8B8G8R8:
			return PF_R8G8B8A8;
		case PF_R8G8B8:
			return PF_B8G8R8;
		case PF_FLOAT16_RGB:
			return PF_FLOAT16_RGBA;
		case PF_FLOAT32_RGB:
			return PF_FLOAT32_RGBA;
		case PF_D32_S8X24:
			return PF_D24S8;
		case PF_UNKNOWN:
		default:
			return PF_B8G8R8A8;
		}
	}

	PixelFormat D3D9Mappings::_getClosestSupportedRenderTargetPF(PixelFormat enginePF)
	{
		switch(enginePF)
		{
		case PF_B8G8R8A8:
			return PF_B8G8R8A8;
		case PF_B8G8R8X8:
			return PF_B8G8R8X8;
		default:
			return PF_B8G8R8A8;
		}
	}

	PixelFormat D3D9Mappings::_getClosestSupportedDepthStencilPF(PixelFormat enginePF)
	{
		switch(enginePF)
		{
		case PF_D24S8:
			return PF_D24S8;
		case PF_D32:
			return PF_D32;
		case PF_D16:
			return PF_D16;
		default:
			return PF_D24S8;
		}
	}
}
