//********************************* B3D Framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DScriptEnginePrerequisites.h"
#include "B3DScriptObjectWrapper.h"
#include "../../../Foundation/Engine/GUI/B3DGUISliderHandle.h"

namespace b3d
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptGUISliderHandleContent : public TScriptTypeDefinition<ScriptGUISliderHandleContent>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "GUISliderHandleContent")

		static MonoObject* Box(const GUISliderHandleContent& value);
		static GUISliderHandleContent Unbox(MonoObject* value);

	private:
		ScriptGUISliderHandleContent();

	};
}
