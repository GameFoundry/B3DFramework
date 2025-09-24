//********************************* B3D Framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DScriptEnginePrerequisites.h"
#include "B3DScriptObjectWrapper.h"
#include "../../../Foundation/Engine/GUI/B3DGUIToggleable.h"
#include "../../../Foundation/Engine/GUI/B3DGUIContent.h"
#include "B3DScriptGUIContent.generated.h"

namespace b3d
{
	struct __GUIToggleContentInterop
	{
		__GUIContentInterop GeneralContent;
		MonoObject* ToggleGroup;
	};

	class B3D_SCRIPT_INTEROP_EXPORT ScriptGUIToggleContent : public TScriptTypeDefinition<ScriptGUIToggleContent>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "GUIToggleContent")

		static MonoObject* Box(const __GUIToggleContentInterop& value);
		static __GUIToggleContentInterop Unbox(MonoObject* value);
		static GUIToggleContent FromInterop(const __GUIToggleContentInterop& value);
		static __GUIToggleContentInterop ToInterop(const GUIToggleContent& value);

	private:
		ScriptGUIToggleContent();

	};
}
