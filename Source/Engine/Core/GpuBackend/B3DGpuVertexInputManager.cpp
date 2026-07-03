//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "GpuBackend/B3DGpuVertexInputManager.h"
#include "Math/B3DMath.h"

using namespace b3d;
using namespace b3d::render;

void GpuVertexInputLayout::Resolve(const VertexDescription& vertexBufferDescription, const VertexDescription& shaderInputDescription)
{
	const TInlineArray<VertexElement, 8>& vertexBufferElements = vertexBufferDescription.GetElements();
	const TInlineArray<VertexElement, 8>& shaderInputElements = shaderInputDescription.GetElements();

	Attributes.Clear();
	StreamCount = 0;
	UsedStreamMask = 0;
	NullStreamIndex = kNoNullStream;

	bool areAnyShaderInputsMissing = false;
	for(const VertexElement& shaderInputElement : shaderInputElements)
	{
		GpuVertexInputAttribute attribute;
		attribute.ShaderInput = &shaderInputElement;

		for(const VertexElement& vertexBufferElement : vertexBufferElements)
		{
			if(shaderInputElement.GetSemantic() == vertexBufferElement.GetSemantic() && shaderInputElement.GetSemanticIndex() == vertexBufferElement.GetSemanticIndex())
			{
				attribute.BufferElement = &vertexBufferElement;
				break;
			}
		}

		if(attribute.BufferElement != nullptr)
		{
			attribute.StreamIndex = attribute.BufferElement->GetStreamIndex();
			attribute.SteppedPerInstance = attribute.BufferElement->GetInstanceStepRate() > 0;

			if(attribute.StreamIndex < 32)
				UsedStreamMask |= 1u << attribute.StreamIndex;

			StreamCount = Math::Max(StreamCount, attribute.StreamIndex + 1);
		}
		else
			areAnyShaderInputsMissing = true;

		Attributes.Add(attribute);
	}

	// Reserve an extra stream for unmatched shader inputs; backends are expected to bind an empty vertex buffer
	// there so such inputs read zero
	if(areAnyShaderInputsMissing)
	{
		NullStreamIndex = StreamCount++;

		for(GpuVertexInputAttribute& attribute : Attributes)
		{
			if(attribute.BufferElement == nullptr)
				attribute.StreamIndex = NullStreamIndex;
		}
	}
}
