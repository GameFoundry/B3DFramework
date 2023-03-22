//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "RenderAPI/BsVertexDataDesc.h"
#include "Managers/BsHardwareBufferManager.h"
#include "Private/RTTI/BsVertexDataDescRTTI.h"

using namespace bs;

VertexDataDesc::VertexDataDesc(const SmallVector<VertexElement, 8>& elements)
	:mVertexElements(elements)
{
	// Sort by stream, but preserve remaining ordering
	std::stable_sort(mVertexElements.begin(), mVertexElements.end(), [](const VertexElement& lhs, const VertexElement& rhs)
	{
		return lhs.GetStreamIndex() < rhs.GetStreamIndex();
	});

	CalculateOffsets();
}

void VertexDataDesc::CalculateOffsets()
{
	const u32 largestStreamIndex = GetLargestStreamIndex();
	const u32 streamCount = largestStreamIndex + 1;

	u32* streamOffsets = B3DStackAllocate<u32>(streamCount);
	B3DZeroOut(streamOffsets, streamCount);

	for(auto& element : mVertexElements)
	{
		const u32 streamIndex = element.GetStreamIndex();

		element.mOffset = streamOffsets[streamIndex];
		streamOffsets[streamIndex] += element.GetSize();
	}

	B3DStackFree(streamOffsets);
}

u32 VertexDataDesc::GetLargestStreamIndex() const
{
	u32 maxStreamIdx = 0;
	u32 numElems = (u32)mVertexElements.size();
	for(u32 i = 0; i < numElems; i++)
	{
		for(auto& vertElem : mVertexElements)
		{
			maxStreamIdx = std::max((u32)maxStreamIdx, (u32)vertElem.GetStreamIndex());
		}
	}

	return maxStreamIdx;
}

bool VertexDataDesc::HasStream(u32 streamIndex) const
{
	for(auto& vertElem : mVertexElements)
	{
		if(vertElem.GetStreamIndex() == streamIndex)
			return true;
	}

	return false;
}

bool VertexDataDesc::HasElement(VertexElementSemantic semantic, u32 semanticIdx, u32 streamIdx) const
{
	auto findIter = std::find_if(mVertexElements.begin(), mVertexElements.end(), [semantic, semanticIdx, streamIdx](const VertexElement& x)
								 { return x.GetSemantic() == semantic && x.GetSemanticIndex() == semanticIdx && x.GetStreamIndex() == streamIdx; });

	if(findIter != mVertexElements.end())
	{
		return true;
	}

	return false;
}

u32 VertexDataDesc::GetElementSize(VertexElementSemantic semantic, u32 semanticIdx, u32 streamIdx) const
{
	for(auto& element : mVertexElements)
	{
		if(element.GetSemantic() == semantic && element.GetSemanticIndex() == semanticIdx && element.GetStreamIndex() == streamIdx)
			return element.GetSize();
	}

	return -1;
}

u32 VertexDataDesc::GetElementOffsetFromStream(VertexElementSemantic semantic, u32 semanticIdx, u32 streamIdx) const
{
	u32 vertexOffset = 0;
	for(auto& element : mVertexElements)
	{
		if(element.GetStreamIndex() != streamIdx)
			continue;

		if(element.GetSemantic() == semantic && element.GetSemanticIndex() == semanticIdx)
			break;

		vertexOffset += element.GetSize();
	}

	return vertexOffset;
}

u32 VertexDataDesc::GetVertexStride(u32 streamIdx) const
{
	u32 vertexStride = 0;
	for(auto& element : mVertexElements)
	{
		if(element.GetStreamIndex() == streamIdx)
			vertexStride += element.GetSize();
	}

	return vertexStride;
}

u32 VertexDataDesc::GetVertexStride() const
{
	u32 vertexStride = 0;
	for(auto& element : mVertexElements)
	{
		vertexStride += element.GetSize();
	}

	return vertexStride;
}

u32 VertexDataDesc::GetStreamOffset(u32 streamIdx) const
{
	u32 streamOffset = 0;
	for(auto& element : mVertexElements)
	{
		if(element.GetStreamIndex() == streamIdx)
			break;

		streamOffset += element.GetSize();
	}

	return streamOffset;
}

const VertexElement* VertexDataDesc::GetElement(VertexElementSemantic semantic, u32 semanticIndex, u32 streamIndex) const
{
	auto findIter = std::find_if(mVertexElements.begin(), mVertexElements.end(), [semantic, semanticIndex, streamIndex](const VertexElement& x)
								 { return x.GetSemantic() == semantic && x.GetSemanticIndex() == semanticIndex && x.GetStreamIndex() == streamIndex; });

	if(findIter != mVertexElements.end())
		return &(*findIter);

	return nullptr;
}

/************************************************************************/
/* 								SERIALIZATION                      		*/
/************************************************************************/

RTTITypeBase* VertexDataDesc::GetRttiStatic()
{
	return VertexDataDescRTTI::Instance();
}

RTTITypeBase* VertexDataDesc::GetRtti() const
{
	return VertexDataDesc::GetRttiStatic();
}
