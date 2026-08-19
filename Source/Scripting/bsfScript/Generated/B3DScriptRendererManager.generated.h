//************************************ B3D Framework - Copyright 2025 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DScriptEnginePrerequisites.h"
#include "../../../Engine/Core/Renderer/B3DRendererManager.h"
#include "B3DScriptTypeDefinition.h"

namespace b3d
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptRendererManager : public TScriptTypeDefinition<ScriptRendererManager>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "RendererManager")

		ScriptRendererManager();

		static void SetupScriptBindings();

	private:
		static void InternalRequestGPUCommandCapture();
	};
}
