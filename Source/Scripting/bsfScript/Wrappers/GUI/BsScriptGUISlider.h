//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "Wrappers/GUI/BsScriptGUIElement.h"

namespace bs
{
	/** @addtogroup ScriptInteropEngine
	 *  @{
	 */

	/**	Interop class between C++ & CLR for GUISliderH.  */
	class B3D_SCRIPT_INTEROP_EXPORT ScriptGUISliderH : public TScriptGUIInteractable<ScriptGUISliderH>
	{
	public:
		SCRIPT_OBJ(kEngineAssembly, kEngineNs, "GUIHorizontalSlider")

	private:
		ScriptGUISliderH(MonoObject* instance, GUIHorizontalSlider* slider);

		/**	Triggered when the native slider is moved. */
		void OnChanged(float percent);

		/************************************************************************/
		/* 								CLR HOOKS						   		*/
		/************************************************************************/
		static void InternalCreateInstance(MonoObject* instance, MonoString* style, MonoArray* guiOptions);
		static void InternalSetPercent(ScriptGUISliderH* nativeInstance, float percent);
		static float InternalGetPercent(ScriptGUISliderH* nativeInstance);
		static float InternalGetValue(ScriptGUISliderH* nativeInstance);
		static void InternalSetValue(ScriptGUISliderH* nativeInstance, float percent);
		static void InternalSetRange(ScriptGUISliderH* nativeInstance, float min, float max);
		static float InternalGetRangeMaximum(ScriptGUISliderH* nativeInstance);
		static float InternalGetRangeMinimum(ScriptGUISliderH* nativeInstance);
		static void InternalSetStep(ScriptGUISliderH* nativeInstance, float step);
		static float InternalGetStep(ScriptGUISliderH* nativeInstance);

		typedef void(B3D_THUNKCALL* OnChangedThunkDef)(MonoObject*, float, MonoException**);
		static OnChangedThunkDef onChangedThunk;
	};

	/**	Interop class between C++ & CLR for GUISliderV. */
	class B3D_SCRIPT_INTEROP_EXPORT ScriptGUISliderV : public TScriptGUIInteractable<ScriptGUISliderV>
	{
	public:
		SCRIPT_OBJ(kEngineAssembly, kEngineNs, "GUIVerticalSlider")

	private:
		ScriptGUISliderV(MonoObject* instance, GUIVerticalSlider* slider);

		/**	Triggered when the native slider is moved. */
		void OnChanged(float percent);

		/************************************************************************/
		/* 								CLR HOOKS						   		*/
		/************************************************************************/
		static void InternalCreateInstance(MonoObject* instance, MonoString* style, MonoArray* guiOptions);
		static void InternalSetPercent(ScriptGUISliderV* nativeInstance, float percent);
		static float InternalGetPercent(ScriptGUISliderV* nativeInstance);
		static float InternalGetValue(ScriptGUISliderV* nativeInstance);
		static void InternalSetValue(ScriptGUISliderV* nativeInstance, float percent);
		static void InternalSetRange(ScriptGUISliderV* nativeInstance, float min, float max);
		static float InternalGetRangeMaximum(ScriptGUISliderV* nativeInstance);
		static float InternalGetRangeMinimum(ScriptGUISliderV* nativeInstance);
		static void InternalSetStep(ScriptGUISliderV* nativeInstance, float step);
		static float InternalGetStep(ScriptGUISliderV* nativeInstance);

		typedef void(B3D_THUNKCALL* OnChangedThunkDef)(MonoObject*, float, MonoException**);
		static OnChangedThunkDef onChangedThunk;
	};

	/** @} */
} // namespace bs
