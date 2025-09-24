//********************************* B3D Framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DScriptEnginePrerequisites.h"
#include "B3DScriptObjectWrapper.h"
#include "../../../Foundation/Engine/GUI/B3DGUIListBox.h"
#include "../../../Foundation/Core/Localization/B3DHString.h"

namespace b3d
{
	struct __GUIListBoxContentInterop
	{
		MonoArray* Elements;
		bool AllowMultiselect;
	};

	class B3D_SCRIPT_INTEROP_EXPORT ScriptGUIListBoxContent : public TScriptTypeDefinition<ScriptGUIListBoxContent>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "GUIListBoxContent")

		static MonoObject* Box(const __GUIListBoxContentInterop& value);
		static __GUIListBoxContentInterop Unbox(MonoObject* value);
		static GUIListBoxContent FromInterop(const __GUIListBoxContentInterop& value);
		static __GUIListBoxContentInterop ToInterop(const GUIListBoxContent& value);

	private:
		ScriptGUIListBoxContent();

	};
}
