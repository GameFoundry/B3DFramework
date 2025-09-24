//********************************* B3D Framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DScriptEnginePrerequisites.h"
#include "B3DScriptObjectWrapper.h"
#include "../../../Foundation/Engine/GUI/B3DGUIContent.h"
#include "../../../Foundation/Core/Localization/B3DHString.h"
#include "../../../Foundation/Engine/GUI/B3DGUIContent.h"
#include "B3DScriptGUIContentImages.generated.h"

namespace b3d
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
