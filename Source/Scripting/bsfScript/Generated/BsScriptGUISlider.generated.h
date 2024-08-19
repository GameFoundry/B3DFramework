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
		void RegisterEvents(GUIElement* value) override;
		void OnChanged(float p0);

		typedef void(B3D_THUNKCALL *OnChangedThunkDefinition) (MonoObject*, float p0, MonoException**);
		static OnChangedThunkDefinition OnChangedThunk;

	};

	class B3D_SCRIPT_INTEROP_EXPORT ScriptGUISlider : public TScriptGUIInteractable<ScriptGUISlider, ScriptGUISliderBase>
	{
	public:
		SCRIPT_OBJ(kEngineAssembly, kEngineNs, "GUISlider")

		ScriptGUISlider(MonoObject* managedInstance, GUISlider* value);

	private:
		static void InternalSetHandlePositionInPercent(ScriptGUIElementBase* self, float percent);
		static float InternalGetHandlePositionInPercent(ScriptGUIElementBase* self);
		static void InternalSetHandlePositionInRange(ScriptGUIElementBase* self, float value);
		static float InternalGetHandlePositionInRange(ScriptGUIElementBase* self);
		static void InternalSetRange(ScriptGUIElementBase* self, float min, float max);
		static float InternalGetRangeMinimum(ScriptGUIElementBase* self);
		static float InternalGetRangeMaximum(ScriptGUIElementBase* self);
		static void InternalSetStep(ScriptGUIElementBase* self, float step);
		static float InternalGetStep(ScriptGUIElementBase* self);
	};
}
