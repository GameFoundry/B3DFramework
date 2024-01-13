//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "Wrappers/GUI/BsScriptGUIElement.h"

namespace bs { class GUISlider; }
namespace bs
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptGUISliderBase : public ScriptGUIInteractableBase
	{
	public:
		ScriptGUISliderBase(MonoObject* instance);
		virtual ~ScriptGUISliderBase() {}
		void OnChanged(float p0);

		typedef void(B3D_THUNKCALL *OnChangedThunkDef) (MonoObject*, float p0, MonoException**);
		static OnChangedThunkDef OnChangedThunk;

	};

	class B3D_SCRIPT_INTEROP_EXPORT ScriptGUISlider : public TScriptGUIInteractable<ScriptGUISlider, ScriptGUISliderBase>
	{
	public:
		SCRIPT_OBJ(kEngineAssembly, kEngineNs, "GUISlider")

		ScriptGUISlider(MonoObject* managedInstance, GUISlider* value);

	private:
		static void InternalSetHandlePositionInPercent(ScriptGUIElementBase* thisPtr, float percent);
		static float InternalGetHandlePositionInPercent(ScriptGUIElementBase* thisPtr);
		static void InternalSetHandlePositionInRange(ScriptGUIElementBase* thisPtr, float value);
		static float InternalGetHandlePositionInRange(ScriptGUIElementBase* thisPtr);
		static void InternalSetRange(ScriptGUIElementBase* thisPtr, float min, float max);
		static float InternalGetRangeMinimum(ScriptGUIElementBase* thisPtr);
		static float InternalGetRangeMaximum(ScriptGUIElementBase* thisPtr);
		static void InternalSetStep(ScriptGUIElementBase* thisPtr, float step);
		static float InternalGetStep(ScriptGUIElementBase* thisPtr);
	};
}
