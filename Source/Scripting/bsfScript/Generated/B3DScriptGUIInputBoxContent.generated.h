//********************************* B3D Framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DScriptEnginePrerequisites.h"
#include "B3DScriptObjectWrapper.h"
#include "../../../Foundation/Engine/GUI/B3DGUIInputBox.h"

namespace b3d
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptGUIInputBoxContent : public TScriptTypeDefinition<ScriptGUIInputBoxContent>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "GUIInputBoxContent")

		static MonoObject* Box(const GUIInputBoxContent& value);
		static GUIInputBoxContent Unbox(MonoObject* value);

	private:
		ScriptGUIInputBoxContent();

	};
}
