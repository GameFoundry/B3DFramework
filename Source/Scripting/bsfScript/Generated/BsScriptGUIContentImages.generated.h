//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptObjectWrapper.h"
#include "../../../Foundation/bsfEngine/GUI/BsGUIContent.h"

namespace bs
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

	class B3D_SCRIPT_INTEROP_EXPORT ScriptGUIContentImages : public TScriptStructWrapper<ScriptGUIContentImages>
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
