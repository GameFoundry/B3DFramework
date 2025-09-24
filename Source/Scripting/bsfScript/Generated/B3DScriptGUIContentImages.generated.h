//********************************* B3D Framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DScriptEnginePrerequisites.h"
#include "B3DScriptObjectWrapper.h"
#include "../../../Foundation/Engine/GUI/B3DGUIContent.h"

namespace b3d
{
	struct __GUIContentImagesInterop
	{
		MonoObject* Normal;
		MonoObject* Hover;
		MonoObject* Active;
		MonoObject* Focused;
		MonoObject* NormalOn;
		MonoObject* HoverOn;
		MonoObject* ActiveOn;
		MonoObject* FocusedOn;
	};

	class B3D_SCRIPT_INTEROP_EXPORT ScriptGUIContentImages : public TScriptTypeDefinition<ScriptGUIContentImages>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "GUIContentImages")

		static MonoObject* Box(const __GUIContentImagesInterop& value);
		static __GUIContentImagesInterop Unbox(MonoObject* value);
		static GUIContentImages FromInterop(const __GUIContentImagesInterop& value);
		static __GUIContentImagesInterop ToInterop(const GUIContentImages& value);

	private:
		ScriptGUIContentImages();

	};
}
