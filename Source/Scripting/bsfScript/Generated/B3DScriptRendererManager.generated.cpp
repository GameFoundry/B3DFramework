//********************************* B3D Framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DScriptRendererManager.generated.h"
#include "B3DMonoMethod.h"
#include "B3DMonoClass.h"
#include "B3DMonoUtil.h"
#include "../../../Foundation/Core/Renderer/B3DRendererManager.h"

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
