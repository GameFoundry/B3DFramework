//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Managers/B3DD3D12VertexInputManager.h"
#include "GpuBackend/B3DVertexDescription.h"

// Generic manager method definitions, followed by the explicit instantiation for the D3D12 vertex input types.
// Included here so the single instantiation lives in this translation unit. The header carries a matching
// `extern template` to suppress implicit instantiation elsewhere.
#include "GpuBackend/B3DGpuVertexInputManager.inl"

template class b3d::render::TGpuVertexInputManager<b3d::render::D3D12VertexInputManager, b3d::TShared<b3d::render::D3D12VertexInput>>;

using namespace b3d;
using namespace b3d::render;

namespace
{
	/** Maps an engine vertex element semantic to an HLSL semantic name. */
	const char* GetSemanticName(VertexElementSemantic semantic)
	{
		switch (semantic)
		{
		case VES_POSITION:
			return "POSITION";
		case VES_BLEND_WEIGHTS:
			return "BLENDWEIGHT";
		case VES_BLEND_INDICES:
			return "BLENDINDICES";
		case VES_NORMAL:
			return "NORMAL";
		case VES_COLOR:
			return "COLOR";
		case VES_TEXCOORD:
			return "TEXCOORD";
		case VES_BITANGENT:
			return "BINORMAL";
		case VES_TANGENT:
			return "TANGENT";
		default:
			return "TEXCOORD";
		}
	}

	/**
	 * Maps an engine vertex element type to a DXGI format for use in an input layout.
	 *
	 * Note: D3D12Utility::GetDXGIFormat operates on PixelFormat, so vertex element types require their own mapping.
	 */
	DXGI_FORMAT GetVertexElementDXGIFormat(VertexElementType type)
	{
		switch (type)
		{
		case VET_FLOAT1:		return DXGI_FORMAT_R32_FLOAT;
		case VET_FLOAT2:		return DXGI_FORMAT_R32G32_FLOAT;
		case VET_FLOAT3:		return DXGI_FORMAT_R32G32B32_FLOAT;
		case VET_FLOAT4:		return DXGI_FORMAT_R32G32B32A32_FLOAT;
		case VET_COLOR:
		case VET_COLOR_ARGB:
		case VET_COLOR_ABGR:
		case VET_UBYTE4_NORM:	return DXGI_FORMAT_R8G8B8A8_UNORM;
		case VET_UBYTE4:		return DXGI_FORMAT_R8G8B8A8_UINT;
		case VET_SHORT1:		return DXGI_FORMAT_R16_SINT;
		case VET_SHORT2:		return DXGI_FORMAT_R16G16_SINT;
		case VET_SHORT4:		return DXGI_FORMAT_R16G16B16A16_SINT;
		case VET_USHORT1:		return DXGI_FORMAT_R16_UINT;
		case VET_USHORT2:		return DXGI_FORMAT_R16G16_UINT;
		case VET_USHORT4:		return DXGI_FORMAT_R16G16B16A16_UINT;
		case VET_INT1:			return DXGI_FORMAT_R32_SINT;
		case VET_INT2:			return DXGI_FORMAT_R32G32_SINT;
		case VET_INT3:			return DXGI_FORMAT_R32G32B32_SINT;
		case VET_INT4:			return DXGI_FORMAT_R32G32B32A32_SINT;
		case VET_UINT1:			return DXGI_FORMAT_R32_UINT;
		case VET_UINT2:			return DXGI_FORMAT_R32G32_UINT;
		case VET_UINT3:			return DXGI_FORMAT_R32G32B32_UINT;
		case VET_UINT4:			return DXGI_FORMAT_R32G32B32A32_UINT;
		case VET_HALF1:			return DXGI_FORMAT_R16_FLOAT;
		case VET_HALF2:			return DXGI_FORMAT_R16G16_FLOAT;
		case VET_HALF4:			return DXGI_FORMAT_R16G16B16A16_FLOAT;
		default:
			// VET_HALF3 has no direct 48-bit DXGI equivalent; fall back to the closest larger format.
			return DXGI_FORMAT_R32G32B32A32_FLOAT;
		}
	}
}

D3D12VertexInput::D3D12VertexInput(u32 id, const GpuVertexInputLayout& layout)
	: mId(id), mVertexBufferBindingCount(layout.StreamCount)
{
	mElements.reserve(layout.Attributes.Size());

	for (const GpuVertexInputAttribute& attribute : layout.Attributes)
	{
		D3D12_INPUT_ELEMENT_DESC element = {};
		element.SemanticName = GetSemanticName(attribute.ShaderInput->GetSemantic());
		element.SemanticIndex = attribute.ShaderInput->GetSemanticIndex();
		element.InputSlot = attribute.StreamIndex;

		if (attribute.BufferElement != nullptr)
		{
			element.Format = GetVertexElementDXGIFormat(attribute.BufferElement->GetType());
			element.AlignedByteOffset = attribute.BufferElement->GetOffset();
		}
		else
		{
			// Shader input with no matching vertex buffer element; fetches from the null stream, where the command
			// buffer binds an empty vertex buffer view so the input reads zero
			element.Format = GetVertexElementDXGIFormat(attribute.ShaderInput->GetType());
			element.AlignedByteOffset = 0;
		}

		if (attribute.SteppedPerInstance)
		{
			element.InputSlotClass = D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA;
			element.InstanceDataStepRate = attribute.BufferElement->GetInstanceStepRate();
		}
		else
		{
			element.InputSlotClass = D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA;
			element.InstanceDataStepRate = 0;
		}

		mElements.push_back(element);
	}

	mLayoutDesc.pInputElementDescs = mElements.data();
	mLayoutDesc.NumElements = (UINT)mElements.size();
}

D3D12VertexInputManager::~D3D12VertexInputManager()
{
	ReleaseAll();
}

TShared<D3D12VertexInput> D3D12VertexInputManager::CreateVertexInput(const GpuVertexInputLayout& layout)
{
	return B3DMakeShared<D3D12VertexInput>(mNextId++, layout);
}

void D3D12VertexInputManager::DestroyVertexInput(TShared<D3D12VertexInput>& vertexInput)
{
	vertexInput = nullptr;
}
