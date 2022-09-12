//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "RenderAPI/BsVertexDataDesc.h"
#include "Managers/BsHardwareBufferManager.h"
#include "Private/RTTI/BsVertexDataDescRTTI.h"

namespace bs
{
	void VertexDataDesc::addVertElem(VertexElementType type, VertexElementSemantic semantic, UINT32 semanticIdx,
		UINT32 streamIdx, UINT32 instanceStepRate)
	{
		clearIfItExists(type, semantic, semanticIdx, streamIdx);

		VertexElement NewElement(streamIdx, 0, type, semantic, semanticIdx, instanceStepRate);

		// Insert it so it is sorted by stream
		UINT32 insertToIndex = (UINT32)mVertexElements.size();
		UINT32 idx = 0;
		for(auto& elem : mVertexElements)
		{
			if(elem.GetStreamIdx() > streamIdx)
			{
				insertToIndex = idx;
				break;
			}

			idx++;
		}

		mVertexElements.Insert(mVertexElements.begin() + insertToIndex, newElement);
	}

	Vector<VertexElement> VertexDataDesc::CreateElements() const
	{
		UINT32 maxStreamIdx = getMaxStreamIdx();

		UINT32 numStreams = maxStreamIdx + 1;
		UINT32* streamOffsets = bs_newN<UINT32>(numStreams);
		for (UINT32 i = 0; i < numStreams; i++)
			streamOffsets[i] = 0;

		Vector<VertexElement> declarationElements;
		for (auto& vertElem : mVertexElements)
		{
			UINT32 streamIdx = vertElem.GetStreamIdx();

			declarationElements.push_back(VertexElement(streamIdx, streamOffsets[streamIdx], vertElem.GetType(),
				vertElem.GetSemantic(), vertElem.getSemanticIdx(), vertElem.getInstanceStepRate()));

			streamOffsets[streamIdx] += vertElem.GetSize();
		}

		bs_deleteN(streamOffsets, numStreams);

		return declarationElements;
	}

	UINT32 VertexDataDesc::GetMaxStreamIdx() const
	{
		UINT32 maxStreamIdx = 0;
		UINT32 numElems = (UINT32)mVertexElements.size();
		for(UINT32 i = 0; i < numElems; i++)
		{
			for(auto& vertElem : mVertexElements)
			{
				maxStreamIdx = std::max((UINT32)maxStreamIdx, (UINT32)vertElem.GetStreamIdx());
			}
		}

		return maxStreamIdx;
	}

	bool VertexDataDesc::HasStream(UINT32 streamIdx) const
	{
		for(auto& vertElem : mVertexElements)
		{
			if(vertElem.GetStreamIdx() == streamIdx)
				return true;
		}

		return false;
	}

	bool VertexDataDesc::HasElement(VertexElementSemantic semantic, UINT32 semanticIdx, UINT32 streamIdx) const
	{
		auto findIter = std::find_if(mVertexElements.begin(), mVertexElements.end(),
			[semantic, semanticIdx, streamIdx] (const VertexElement& x)
		{
			return x.GetSemantic() == semantic && x.getSemanticIdx() == semanticIdx && x.getStreamIdx() == streamIdx;
		});

		if(findIter != mVertexElements.end())
		{
			return true;
		}

		return false;
	}

	UINT32 VertexDataDesc::GetElementSize(VertexElementSemantic semantic, UINT32 semanticIdx, UINT32 streamIdx) const
	{
		for(auto& element : mVertexElements)
		{
			if(element.GetSemantic() == semantic && element.getSemanticIdx() == semanticIdx && element.getStreamIdx() == streamIdx)
				return element.GetSize();
		}

		return -1;
	}

	UINT32 VertexDataDesc::GetElementOffsetFromStream(VertexElementSemantic semantic, UINT32 semanticIdx, UINT32 streamIdx) const
	{
		UINT32 vertexOffset = 0;
		for(auto& element : mVertexElements)
		{
			if(element.GetStreamIdx() != streamIdx)
				continue;

			if(element.GetSemantic() == semantic && element.getSemanticIdx() == semanticIdx)
				break;

			vertexOffset += element.GetSize();
		}

		return vertexOffset;
	}

	UINT32 VertexDataDesc::GetVertexStride(UINT32 streamIdx) const
	{
		UINT32 vertexStride = 0;
		for(auto& element : mVertexElements)
		{
			if(element.GetStreamIdx() == streamIdx)
				vertexStride += element.GetSize();
		}

		return vertexStride;
	}

	UINT32 VertexDataDesc::GetVertexStride() const
	{
		UINT32 vertexStride = 0;
		for(auto& element : mVertexElements)
		{
			vertexStride += element.GetSize();
		}

		return vertexStride;
	}

	UINT32 VertexDataDesc::GetStreamOffset(UINT32 streamIdx) const
	{
		UINT32 streamOffset = 0;
		for(auto& element : mVertexElements)
		{
			if(element.GetStreamIdx() == streamIdx)
				break;

			streamOffset += element.GetSize();
		}

		return streamOffset;
	}

	const VertexElement* VertexDataDesc::getElement(VertexElementSemantic semantic, UINT32 semanticIdx, UINT32 streamIdx) const
	{
		auto findIter = std::find_if(mVertexElements.begin(), mVertexElements.end(),
									 [semantic, semanticIdx, streamIdx](const VertexElement& x)
		{
			return x.GetSemantic() == semantic && x.getSemanticIdx() == semanticIdx && x.getStreamIdx() == streamIdx;
		});

		if (findIter != mVertexElements.end())
			return &(*findIter);

		return nullptr;
	}

	void VertexDataDesc::ClearIfItExists(VertexElementType type, VertexElementSemantic semantic, UINT32 semanticIdx, UINT32 streamIdx)
	{
		auto findIter = std::find_if(mVertexElements.begin(), mVertexElements.end(),
			[semantic, semanticIdx, streamIdx] (const VertexElement& x)
		{
			return x.GetSemantic() == semantic && x.getSemanticIdx() == semanticIdx && x.getStreamIdx() == streamIdx;
		});

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

	RTTITypeBase* VertexDataDesc::getRTTIStatic()
	{
		return VertexDataDescRTTI::Instance();
	}

	RTTITypeBase* VertexDataDesc::getRTTI() const
	{
		return VertexDataDesc::GetRTTIStatic();
	}
}
