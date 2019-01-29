//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptObject.h"
#include "../../../Foundation/bsfEngine/GUI/BsGUIContent.h"

namespace bs
{
	struct __GUIContentImagesInterop
	{
		MonoObject* normal;
		MonoObject* hover;
		MonoObject* active;
		MonoObject* focused;
		MonoObject* normalOn;
		MonoObject* hoverOn;
		MonoObject* activeOn;
		MonoObject* focusedOn;
	};

	class BS_SCR_BE_EXPORT ScriptGUIContentImages : public ScriptObject<ScriptGUIContentImages>
	{
	public:
		SCRIPT_OBJ(ENGINE_ASSEMBLY, ENGINE_NS, "GUIContentImages")

		static MonoObject* box(const __GUIContentImagesInterop& value);
		static __GUIContentImagesInterop unbox(MonoObject* value);
		static GUIContentImages fromInterop(const __GUIContentImagesInterop& value);
		static __GUIContentImagesInterop toInterop(const GUIContentImages& value);

	private:
		ScriptGUIContentImages(MonoObject* managedInstance);

	};
}
