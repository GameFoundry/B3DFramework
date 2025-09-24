//********************************* B3D Framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DScriptEnginePrerequisites.h"
#include "B3DScriptObjectWrapper.h"
#include "../../../Foundation/Engine/GUI/B3DGUIScrollArea.h"

namespace b3d
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptGUIScrollAreaContent : public TScriptTypeDefinition<ScriptGUIScrollAreaContent>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "GUIScrollAreaContent")

		static MonoObject* Box(const GUIScrollAreaContent& value);
		static GUIScrollAreaContent Unbox(MonoObject* value);

	private:
		ScriptGUIScrollAreaContent();

	};
}
