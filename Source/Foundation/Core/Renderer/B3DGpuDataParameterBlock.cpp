//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Renderer/B3DGpuDataParameterBlock.h"
#include "RenderAPI/B3DGpuParam.h"

using namespace b3d;

namespace b3d { namespace render
{
GpuDataParameterBlock::~GpuDataParameterBlock()
{
	GpuDataParameterBlockManager::UnregisterBlock(this);
}

GpuDataParameterBlockManager::GpuDataParameterBlockManager()
{
	for(auto& entry : GetToInitializeList())
		entry->Initialize();

	GetToInitializeList().clear();
}

void GpuDataParameterBlockManager::RegisterBlock(GpuDataParameterBlock* parameterBlock)
{
	if(IsStarted())
		parameterBlock->Initialize();
	else
		GetToInitializeList().push_back(parameterBlock);
}

void GpuDataParameterBlockManager::UnregisterBlock(GpuDataParameterBlock* parameterBlock)
{
	auto found = std::find(GetToInitializeList().begin(), GetToInitializeList().end(), parameterBlock);
	if(found != GetToInitializeList().end())
		GetToInitializeList().erase(found);
}
}}
