//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptObject.h"
#include "../../../../../EditorCore/GUI/BsGUITabButton.h"
#include "../../../Foundation/bsfEngine/GUI/BsGUIContent.h"
#include "BsScriptGUIContent.generated.h"

namespace bs
{
	struct __GUITabButtonContentInterop
	{
		uint32_t Index;
		__GUIContentInterop GeneralContent;
		MonoObject* ToggleGroup;
	};

	class B3D_SCRIPT_INTEROP_EXPORT ScriptGUITabButtonContent : public ScriptObject<ScriptGUITabButtonContent>
	{
	public:
		SCRIPT_OBJ(kEngineAssembly, kEngineNs, "GUITabButtonContent")

		static MonoObject* Box(const __GUITabButtonContentInterop& value);
		static __GUITabButtonContentInterop Unbox(MonoObject* value);
		static GUITabButtonContent FromInterop(const __GUITabButtonContentInterop& value);
		static __GUITabButtonContentInterop ToInterop(const GUITabButtonContent& value);

	private:
		ScriptGUITabButtonContent(MonoObject* managedInstance);

	};
}
