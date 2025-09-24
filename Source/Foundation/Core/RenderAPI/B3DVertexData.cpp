//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "RenderAPI/B3DVertexData.h"
#include "Math/B3DVector3.h"

using namespace b3d;

namespace b3d { namespace render
{
void VertexData::SetBuffer(u32 index, SPtr<GpuBuffer> buffer)
{
	mVertexBuffers[index] = buffer;

	RecalculateMaxIndex();
}

SPtr<GpuBuffer> VertexData::GetBuffer(u32 index) const
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
