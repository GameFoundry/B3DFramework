//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "Wrappers/BsScriptComponent.h"
#include "GUI/BsGUIPanel.h"
#include "../../../Foundation/bsfUtility/Math/BsVector2.h"
#include "Math/BsRect2I.h"

namespace bs { class CGUIWidget; }
namespace bs
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptGUIWidget : public TScriptGameObjectWrapper<CGUIWidget, ScriptGUIWidget>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "GUIWidget")

		ScriptGUIWidget(const GameObjectHandle<CGUIWidget>& nativeObject);

		static void SetupScriptBindings();

		static MonoObject* CreateScriptObject(bool construct);

	private:
		static MonoObject* InternalGetPanel(ScriptGUIWidget* self);
		static uint8_t InternalGetDepth(ScriptGUIWidget* self);
		static void InternalSetDepth(ScriptGUIWidget* self, uint8_t depth);
		static bool InternalInBounds(ScriptGUIWidget* self, TVector2<int32_t>* position);
		static void InternalGetBounds(ScriptGUIWidget* self, Rect2I* __output);
	};
}
