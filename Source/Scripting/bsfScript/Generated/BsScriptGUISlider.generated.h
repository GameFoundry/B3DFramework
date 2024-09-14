//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptGUIElementWrapper.h"

namespace bs { class GUISlider; }
namespace bs
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptGUISliderWrapperBase : public ScriptGUIElementWrapper
	{
	public:
		using ScriptGUIElementWrapper::ScriptGUIElementWrapper;

		virtual void RegisterEvents();
		void OnChanged(float p0);

		typedef void(B3D_THUNKCALL *OnChangedThunkDefinition) (MonoObject*, float p0, MonoException**);
		static OnChangedThunkDefinition OnChangedThunk;

	};

	class B3D_SCRIPT_INTEROP_EXPORT ScriptGUISlider : public TScriptGUIElementWrapper<GUISlider, ScriptGUISlider, ScriptGUISliderWrapperBase>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "GUISlider")

		ScriptGUISlider(GUISlider* nativeObject);

		static void SetupScriptBindings();

		static MonoObject* CreateScriptObject(bool construct);

	private:
		static void InternalSetHandlePositionInPercent(ScriptGUISliderWrapperBase* self, float percent);
		static float InternalGetHandlePositionInPercent(ScriptGUISliderWrapperBase* self);
		static void InternalSetHandlePositionInRange(ScriptGUISliderWrapperBase* self, float value);
		static float InternalGetHandlePositionInRange(ScriptGUISliderWrapperBase* self);
		static void InternalSetRange(ScriptGUISliderWrapperBase* self, float min, float max);
		static float InternalGetRangeMinimum(ScriptGUISliderWrapperBase* self);
		static float InternalGetRangeMaximum(ScriptGUISliderWrapperBase* self);
		static void InternalSetStep(ScriptGUISliderWrapperBase* self, float step);
		static float InternalGetStep(ScriptGUISliderWrapperBase* self);
	};
}
