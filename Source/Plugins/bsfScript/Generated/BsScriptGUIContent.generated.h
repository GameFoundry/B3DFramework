//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptObject.h"
#include "../../../Foundation/bsfEngine/GUI/BsGUIContent.h"
#include "../../../Foundation/bsfCore/Localization/BsHString.h"
#include "../../../Foundation/bsfEngine/GUI/BsGUIContent.h"
#include "BsScriptGUIContentImages.generated.h"

namespace bs
{
	struct __GUIContentInterop
	{
		MonoObject* text;
		__GUIContentImagesInterop images;
		MonoObject* tooltip;
	};

	class BS_SCR_BE_EXPORT ScriptGUIContent : public ScriptObject<ScriptGUIContent>
	{
	public:
		SCRIPT_OBJ(ENGINE_ASSEMBLY, ENGINE_NS, "GUIContent")

		static MonoObject* box(const __GUIContentInterop& value);
		static __GUIContentInterop unbox(MonoObject* value);
		static GUIContent fromInterop(const __GUIContentInterop& value);
		static __GUIContentInterop toInterop(const GUIContent& value);

	private:
		ScriptGUIContent(MonoObject* managedInstance);

	};
}
