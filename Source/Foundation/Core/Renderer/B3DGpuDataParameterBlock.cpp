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

Vector<GpuDataParameterBlock*> GpuDataParameterBlockManager::sToInitialize;

GpuDataParameterBlockManager::GpuDataParameterBlockManager()
{
	for(auto& entry : sToInitialize)
		entry->Initialize();

	sToInitialize.clear();
}

void GpuDataParameterBlockManager::RegisterBlock(GpuDataParameterBlock* parameterBlock)
{
	if(IsStarted())
		parameterBlock->Initialize();
	else
		sToInitialize.push_back(parameterBlock);
}

void GpuDataParameterBlockManager::UnregisterBlock(GpuDataParameterBlock* parameterBlock)
{
	auto iterFind = std::find(sToInitialize.begin(), sToInitialize.end(), parameterBlock);
	if(iterFind != sToInitialize.end())
		sToInitialize.erase(iterFind);
}
}}
