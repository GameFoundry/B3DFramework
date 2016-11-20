//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsVertexData.h"
#include "BsHardwareBufferManager.h"
#include "BsVertexBuffer.h"
#include "BsVector3.h"
#include "BsException.h"
#include "BsRenderAPI.h"

namespace bs 
{
	VertexData::VertexData()
	{
		vertexCount = 0;

	}

	VertexData::~VertexData()
	{
	}

	void VertexData::setBuffer(UINT32 index, SPtr<VertexBufferCore> buffer)
	{
		mVertexBuffers[index] = buffer;

		recalculateMaxIndex();
	}

	SPtr<VertexBufferCore> VertexData::getBuffer(UINT32 index) const
	{
		auto iterFind = mVertexBuffers.find(index);
		if(iterFind != mVertexBuffers.end())
		{
			return iterFind->second;
		}

		return nullptr;
	}

	bool VertexData::isBufferBound(UINT32 index) const
	{
		auto iterFind = mVertexBuffers.find(index);
		if(iterFind != mVertexBuffers.end())
		{
			if(iterFind->second != nullptr)
				return true;
		}

		return false;
	}

	void VertexData::recalculateMaxIndex()
	{
		mMaxBufferIdx = 0;
		for (auto& bufferData : mVertexBuffers)
			mMaxBufferIdx = std::max(bufferData.first, mMaxBufferIdx);
	}
}
