//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Renderer/B3DGpuUniformBuffer.h"
#include "RenderAPI/B3DGpuParameter.h"

using namespace b3d;
using namespace b3d::render;

GpuUniformBuffer::~GpuUniformBuffer()
{
	GpuUniformBufferManager::Instance().UnregisterBuffer(this);
}

GpuUniformBufferManager::GpuUniformBufferManager()
{
	for(auto& entry : GetToInitializeList())
	{
		entry->Initialize();
		mActiveBuffers.Add(entry);
	}

	GetToInitializeList().clear();
}

void GpuUniformBufferManager::AdvanceFrame()
{
	for(auto& entry : mActiveBuffers)
		entry->mTransientAllocationPool.AdvanceFrame();
}

void GpuUniformBufferManager::RegisterBuffer(GpuUniformBuffer* buffer)
{
	if(IsStarted())
		buffer->Initialize();
	else
		GetToInitializeList().Add(buffer);
}

void GpuUniformBufferManager::UnregisterBuffer(GpuUniformBuffer* buffer)
{
	auto found = std::find(GetToInitializeList().Begin(), GetToInitializeList().End(), buffer);
	if(found != GetToInitializeList().End())
		GetToInitializeList().Erase(found);
}
