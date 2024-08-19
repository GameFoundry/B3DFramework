//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "Wrappers/GUI/BsScriptGUIElement.h"

namespace bs { class GUIScrollBar; }
namespace bs
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptGUIScrollBarBase : public ScriptGUIInteractableBase
	{
	public:
		ScriptGUIScrollBarBase(MonoObject* instance);
		virtual ~ScriptGUIScrollBarBase() {}
		void RegisterEvents(GUIElement* value) override;
		void OnScrollOrResize(float p0, float p1);

		typedef void(B3D_THUNKCALL *OnScrollOrResizeThunkDefinition) (MonoObject*, float p0, float p1, MonoException**);
		static OnScrollOrResizeThunkDefinition OnScrollOrResizeThunk;

	};

	class B3D_SCRIPT_INTEROP_EXPORT ScriptGUIScrollBar : public TScriptGUIInteractable<ScriptGUIScrollBar, ScriptGUIScrollBarBase>
	{
	public:
		SCRIPT_OBJ(kEngineAssembly, kEngineNs, "GUIScrollBar")

		ScriptGUIScrollBar(MonoObject* managedInstance, GUIScrollBar* value);

	private:
		static void InternalSetScrollHandlePosition(ScriptGUIElementBase* self, float pct);
		static float InternalGetScrollHandlePosition(ScriptGUIElementBase* self);
		static void InternalSetScrollHandleSize(ScriptGUIElementBase* self, float pct);
		static float InternalGetScrollHandleSize(ScriptGUIElementBase* self);
	};
}
