//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "../../../Foundation/bsfEngine/GUI/BsGUIUtility.h"
#include "BsScriptTypeDefinition.h"
#include "GUI/BsGUIElement.h"
#include "../../../Foundation/bsfUtility/Math/BsVector2I.h"

namespace bs
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptGUIUtility : public TScriptTypeDefinition<ScriptGUIUtility>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "GUIUtility")

		ScriptGUIUtility();

		static void SetupScriptBindings();

	private:
		static void InternalCalculateTextBounds(MonoString* text, MonoObject* font, float fontSize, TVector2I<int32_t>* __output);
	};
}
