//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptObjectWrapper.h"
#include "../../../Foundation/bsfEngine/GUI/BsGUIContent.h"
#include "../../../Foundation/bsfCore/Localization/BsHString.h"
#include "../../../Foundation/bsfEngine/GUI/BsGUIContent.h"
#include "BsScriptGUIContentImages.generated.h"

namespace bs
{
	struct __GUIContentInterop
	{
		MonoObject* Text;
		__GUIContentImagesInterop Images;
		MonoObject* Tooltip;
	};

	class B3D_SCRIPT_INTEROP_EXPORT ScriptGUIContent : public TScriptTypeDefinition<ScriptGUIContent>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "GUIContent")

		static MonoObject* Box(const __GUIContentInterop& value);
		static __GUIContentInterop Unbox(MonoObject* value);
		static GUIContent FromInterop(const __GUIContentInterop& value);
		static __GUIContentInterop ToInterop(const GUIContent& value);

	private:
		ScriptGUIContent();

	};
}
