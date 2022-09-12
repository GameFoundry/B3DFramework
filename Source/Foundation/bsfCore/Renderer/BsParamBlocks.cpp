//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Renderer/BsParamBlocks.h"
#include "RenderAPI/BsGpuParam.h"

namespace bs { namespace ct
{
	ParamBlock::~ParamBlock()
	{
		ParamBlockManager::unregisterBlock(this);
	}

	Vector<ParamBlock*> ParamBlockManager::sToInitialize;

	ParamBlockManager::ParamBlockManager()
	{
		for (auto& entry : sToInitialize)
			entry->Initialize();

		sToInitialize.Clear();
	}

	void ParamBlockManager::RegisterBlock(ParamBlock* paramBlock)
	{
		if (isStarted())
			paramBlock->Initialize();
		else
			sToInitialize.push_back(paramBlock);
	}

	void ParamBlockManager::UnregisterBlock(ParamBlock* paramBlock)
	{
		auto iterFind = std::find(sToInitialize.begin(), sToInitialize.end(), paramBlock);
		if (iterFind != sToInitialize.end())
			sToInitialize.erase(iterFind);
	}
}}
