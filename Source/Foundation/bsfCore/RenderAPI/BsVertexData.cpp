//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "RenderAPI/BsVertexData.h"
#include "Managers/BsHardwareBufferManager.h"
#include "RenderAPI/BsVertexBuffer.h"
#include "Math/BsVector3.h"
#include "Error/BsException.h"
#include "RenderAPI/BsRenderAPI.h"

using namespace bs;

namespace bs { namespace ct
{
void VertexData::SetBuffer(u32 index, SPtr<VertexBuffer> buffer)
{
	mVertexBuffers[index] = buffer;

	RecalculateMaxIndex();
}

SPtr<VertexBuffer> VertexData::GetBuffer(u32 index) const
{
	auto iterFind = mVertexBuffers.find(index);
	if(iterFind != mVertexBuffers.end())
	{
		return iterFind->second;
	}

	return nullptr;
}

bool VertexData::IsBufferBound(u32 index) const
{
	auto iterFind = mVertexBuffers.find(index);
	if(iterFind != mVertexBuffers.end())
	{
		if(iterFind->second != nullptr)
			return true;
	}

	return false;
}

void VertexData::RecalculateMaxIndex()
{
	mMaxBufferIdx = 0;
	for(auto& bufferData : mVertexBuffers)
		mMaxBufferIdx = std::max(bufferData.first, mMaxBufferIdx);
}
}}
