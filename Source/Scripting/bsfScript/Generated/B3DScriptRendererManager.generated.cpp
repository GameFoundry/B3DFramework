//************************************ B3D Framework - Copyright 2025 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DScriptRendererManager.generated.h"
#include "B3DMonoMethod.h"
#include "B3DMonoClass.h"
#include "B3DMonoUtil.h"
#include "../../../Engine/Core/Renderer/B3DRendererManager.h"

namespace b3d
{
	ScriptRendererManager::ScriptRendererManager()
		:TScriptTypeDefinition()
	{
	}

	void ScriptRendererManager::SetupScriptBindings()
	{
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_RequestGPUCommandCapture", (void*)&ScriptRendererManager::InternalRequestGPUCommandCapture);

	}

	void ScriptRendererManager::InternalRequestGPUCommandCapture()
	{
		RendererManager::Instance().RequestGPUCommandCapture();
	}
}
