//********************************* B3D Framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "../../../Foundation/bsfCore/Renderer/BsRendererManager.h"
#include "BsScriptTypeDefinition.h"

namespace b3d
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptRendererManager : public TScriptTypeDefinition<ScriptRendererManager>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "RendererManager")

		ScriptRendererManager();

		static void SetupScriptBindings();

	private:
		static void InternalRequestFrameCapture();
	};
}
