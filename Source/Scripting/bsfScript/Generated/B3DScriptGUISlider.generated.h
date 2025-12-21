//************************************ B3D Framework - Copyright 2025 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DScriptEnginePrerequisites.h"
#include "B3DScriptGUIElementWrapper.h"
#include "B3DScriptGUIInteractable.generated.h"

namespace b3d { class GUISlider; }
namespace b3d
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptGUISliderWrapperBase : public ScriptGUIInteractableWrapperBase
	{
	public:
		using ScriptGUIInteractableWrapperBase::ScriptGUIInteractableWrapperBase;

		virtual void RegisterEvents();
		virtual void UnregisterEvents();
		void OnChanged(float p0);

		typedef void(B3D_THUNKCALL *OnChangedThunkDefinition) (MonoObject*, float p0, MonoException**);
		static OnChangedThunkDefinition OnChangedThunk;

		HEvent OnChangedConnection;
	};

	class B3D_SCRIPT_INTEROP_EXPORT ScriptGUISlider : public TScriptGUIElementWrapper<GUISlider, ScriptGUISlider, ScriptGUISliderWrapperBase>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "GUISlider")

		ScriptGUISlider(GUISlider* nativeObject);
		~ScriptGUISlider();

		static void SetupScriptBindings();

		static MonoObject* CreateScriptObject(bool construct);

	private:
		static void InternalSetHandlePositionInPercent(ScriptGUISliderWrapperBase* self, float percent);
		static float InternalGetHandlePositionInPercent(ScriptGUISliderWrapperBase* self);
		static void InternalSetHandlePositionInRange(ScriptGUISliderWrapperBase* self, float value);
		static float InternalGetHandlePositionInRange(ScriptGUISliderWrapperBase* self);
		static void InternalSetRange(ScriptGUISliderWrapperBase* self, float minimum, float maximum);
		static float InternalGetRangeMinimum(ScriptGUISliderWrapperBase* self);
		static float InternalGetRangeMaximum(ScriptGUISliderWrapperBase* self);
		static void InternalSetStep(ScriptGUISliderWrapperBase* self, float step);
		static float InternalGetStep(ScriptGUISliderWrapperBase* self);
	};
}
