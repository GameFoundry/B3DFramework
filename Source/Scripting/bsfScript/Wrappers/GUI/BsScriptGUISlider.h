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
	class BS_SCR_BE_EXPORT ScriptGUISliderH : public TScriptGUIElement<ScriptGUISliderH>
	{
	public:
		SCRIPT_OBJ(ENGINE_ASSEMBLY, ENGINE_NS, "GUISliderH")

	private:
		ScriptGUISliderH(MonoObject* instance, GUISliderHorz* slider);

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
		static void InternalSetTint(ScriptGUISliderH* nativeInstance, Color* color);

		typedef void(BS_THUNKCALL* OnChangedThunkDef)(MonoObject*, float, MonoException**);
		static OnChangedThunkDef onChangedThunk;
	};

	/**	Interop class between C++ & CLR for GUISliderV. */
	class BS_SCR_BE_EXPORT ScriptGUISliderV : public TScriptGUIElement<ScriptGUISliderV>
	{
	public:
		SCRIPT_OBJ(ENGINE_ASSEMBLY, ENGINE_NS, "GUISliderV")

	private:
		ScriptGUISliderV(MonoObject* instance, GUISliderVert* slider);

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
		static void InternalSetTint(ScriptGUISliderV* nativeInstance, Color* color);

		typedef void(BS_THUNKCALL* OnChangedThunkDef)(MonoObject*, float, MonoException**);
		static OnChangedThunkDef onChangedThunk;
	};

	/** @} */
} // namespace bs
