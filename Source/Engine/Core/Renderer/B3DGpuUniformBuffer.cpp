//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Renderer/B3DGpuUniformBuffer.h"
#include "RenderAPI/B3DGpuParameter.h"

using namespace b3d;
using namespace b3d::render;

GpuUniformBuffer::~GpuUniformBuffer()
{
	if(GpuUniformBufferManager::IsStarted())
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

GpuUniformBufferManager::~GpuUniformBufferManager()
{
	for(auto& entry : mActiveBuffers)
		entry->Destroy();
}

void GpuUniformBufferManager::AdvanceFrame()
{
	for(auto& entry : mActiveBuffers)
		entry->mTransientAllocationPool.AdvanceFrame();
}

void GpuUniformBufferManager::RegisterBuffer(GpuUniformBuffer* buffer)
{
	if(IsStarted())
	{
		buffer->Initialize();
		Instance().mActiveBuffers.Add(buffer);
	}
	else
		GetToInitializeList().Add(buffer);
}

void GpuUniformBufferManager::UnregisterBuffer(GpuUniformBuffer* buffer)
{
	auto found = std::find(mActiveBuffers.begin(), mActiveBuffers.end(), buffer);
	if(found != mActiveBuffers.end())
		mActiveBuffers.erase(found);
}
