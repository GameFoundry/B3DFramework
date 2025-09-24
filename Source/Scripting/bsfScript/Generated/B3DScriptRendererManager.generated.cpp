//********************************* B3D Framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptRendererManager.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "../../../Foundation/bsfCore/Renderer/BsRendererManager.h"

namespace b3d
{
	ScriptRendererManager::ScriptRendererManager()
		:TScriptTypeDefinition()
	{
	}

	void ScriptRendererManager::SetupScriptBindings()
	{
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_RequestFrameCapture", (void*)&ScriptRendererManager::InternalRequestFrameCapture);

	}

	void ScriptRendererManager::InternalRequestFrameCapture()
	{
		RendererManager::Instance().RequestFrameCapture();
	}
}
