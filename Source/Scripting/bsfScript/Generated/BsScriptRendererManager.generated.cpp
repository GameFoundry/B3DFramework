//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptRendererManager.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "../../../Foundation/bsfCore/Renderer/BsRendererManager.h"

namespace bs
{
	ScriptRendererManager::ScriptRendererManager(MonoObject* managedInstance)
		:ScriptObject(managedInstance)
	{
	}

	void ScriptRendererManager::InitRuntimeData()
	{
		metaData.ScriptClass->AddInternalCall("Internal_RequestFrameCapture", (void*)&ScriptRendererManager::InternalRequestFrameCapture);

	}

	void ScriptRendererManager::InternalRequestFrameCapture()
	{
		RendererManager::Instance().RequestFrameCapture();
	}
}
