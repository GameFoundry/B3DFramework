//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "RenderAPI/BsVertexDataDesc.h"
#include "Managers/BsHardwareBufferManager.h"
#include "Private/RTTI/BsVertexDataDescRTTI.h"

using namespace bs;

void VertexDataDesc::AddVertElem(VertexElementType type, VertexElementSemantic semantic, u32 semanticIdx, u32 streamIdx, u32 instanceStepRate)
{
	ClearIfItExists(type, semantic, semanticIdx, streamIdx);

	VertexElement newElement(streamIdx, 0, type, semantic, semanticIdx, instanceStepRate);

	// Insert it so it is sorted by stream
	u32 insertToIndex = (u32)mVertexElements.size();
	u32 idx = 0;
	for(auto& elem : mVertexElements)
	{
		if(elem.GetStreamIdx() > streamIdx)
		{
			insertToIndex = idx;
			break;
		}

		idx++;
	}

	mVertexElements.insert(mVertexElements.begin() + insertToIndex, newElement);
}

Vector<VertexElement> VertexDataDesc::CreateElements() const
{
	u32 maxStreamIdx = GetMaxStreamIdx();

	u32 numStreams = maxStreamIdx + 1;
	u32* streamOffsets = bs_newN<u32>(numStreams);
	for(u32 i = 0; i < numStreams; i++)
		streamOffsets[i] = 0;

	Vector<VertexElement> declarationElements;
	for(auto& vertElem : mVertexElements)
	{
		u32 streamIdx = vertElem.GetStreamIdx();

		declarationElements.push_back(VertexElement(streamIdx, streamOffsets[streamIdx], vertElem.GetType(), vertElem.GetSemantic(), vertElem.GetSemanticIdx(), vertElem.GetInstanceStepRate()));

		streamOffsets[streamIdx] += vertElem.GetSize();
	}

	bs_deleteN(streamOffsets, numStreams);

	return declarationElements;
}

u32 VertexDataDesc::GetMaxStreamIdx() const
{
	u32 maxStreamIdx = 0;
	u32 numElems = (u32)mVertexElements.size();
	for(u32 i = 0; i < numElems; i++)
	{
		for(auto& vertElem : mVertexElements)
		{
			maxStreamIdx = std::max((u32)maxStreamIdx, (u32)vertElem.GetStreamIdx());
		}
	}

	return maxStreamIdx;
}

bool VertexDataDesc::HasStream(u32 streamIdx) const
{
	for(auto& vertElem : mVertexElements)
	{
		if(vertElem.GetStreamIdx() == streamIdx)
			return true;
	}

	return false;
}

bool VertexDataDesc::HasElement(VertexElementSemantic semantic, u32 semanticIdx, u32 streamIdx) const
{
	auto findIter = std::find_if(mVertexElements.begin(), mVertexElements.end(), [semantic, semanticIdx, streamIdx](const VertexElement& x)
								 { return x.GetSemantic() == semantic && x.GetSemanticIdx() == semanticIdx && x.GetStreamIdx() == streamIdx; });

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
		if(element.GetSemantic() == semantic && element.GetSemanticIdx() == semanticIdx && element.GetStreamIdx() == streamIdx)
			return element.GetSize();
	}

	return -1;
}

u32 VertexDataDesc::GetElementOffsetFromStream(VertexElementSemantic semantic, u32 semanticIdx, u32 streamIdx) const
{
	u32 vertexOffset = 0;
	for(auto& element : mVertexElements)
	{
		if(element.GetStreamIdx() != streamIdx)
			continue;

		if(element.GetSemantic() == semantic && element.GetSemanticIdx() == semanticIdx)
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
		if(element.GetStreamIdx() == streamIdx)
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
		if(element.GetStreamIdx() == streamIdx)
			break;

		streamOffset += element.GetSize();
	}

	return streamOffset;
}

const VertexElement* VertexDataDesc::GetElement(VertexElementSemantic semantic, u32 semanticIdx, u32 streamIdx) const
{
	auto findIter = std::find_if(mVertexElements.begin(), mVertexElements.end(), [semantic, semanticIdx, streamIdx](const VertexElement& x)
								 { return x.GetSemantic() == semantic && x.GetSemanticIdx() == semanticIdx && x.GetStreamIdx() == streamIdx; });

	if(findIter != mVertexElements.end())
		return &(*findIter);

	return nullptr;
}

void VertexDataDesc::ClearIfItExists(VertexElementType type, VertexElementSemantic semantic, u32 semanticIdx, u32 streamIdx)
{
	auto findIter = std::find_if(mVertexElements.begin(), mVertexElements.end(), [semantic, semanticIdx, streamIdx](const VertexElement& x)
								 { return x.GetSemantic() == semantic && x.GetSemanticIdx() == semanticIdx && x.GetStreamIdx() == streamIdx; });

	if(findIter != mVertexElements.end())
	{
		mVertexElements.erase(findIter);
	}
}

SPtr<VertexDataDesc> VertexDataDesc::Create()
{
	return bs_shared_ptr_new<VertexDataDesc>();
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
