//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "../../../Foundation/bsfCore/Renderer/BsRendererManager.h"
#include "BsScriptObject.h"

namespace bs
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptRendererManager : public ScriptObject<ScriptRendererManager>
	{
	public:
		SCRIPT_OBJ(kEngineAssembly, kEngineNs, "RendererManager")

		ScriptRendererManager(MonoObject* managedInstance);

		static void SetupScriptBindings();

	private:
		static void InternalRequestFrameCapture();
	};
}
