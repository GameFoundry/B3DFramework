//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsScriptEditorPrerequisites.h"
#include "BsScriptGUIElement.h"

namespace BansheeEngine
{
	/** @addtogroup ScriptInteropEditor
	 *  @{
	 */

	/**	Interop class between C++ & CLR for GUIFloatField. */
	class BS_SCR_BED_EXPORT ScriptGUIFloatField : public TScriptGUIElement<ScriptGUIFloatField>
	{
	public:
		SCRIPT_OBJ(EDITOR_ASSEMBLY, "BansheeEditor", "GUIFloatField")

	private:
		ScriptGUIFloatField(MonoObject* instance, GUIFloatField* floatField);

		/**
		 * Triggered when the value in the native float field changes.
		 *
		 * @param[in]	instance	Managed GUIFloatField instance.
		 * @param[in]	newValue	New field value.
		 */
		static void onChanged(MonoObject* instance, float newValue);

		/**
		 * Triggered when the user confirms input in the native float field.
		 *
		 * @param[in]	instance	Managed GUIFloatField instance.
		 */
		static void onConfirmed(MonoObject* instance);

		/************************************************************************/
		/* 								CLR HOOKS						   		*/
		/************************************************************************/
		static void internal_createInstance(MonoObject* instance, MonoObject* title, UINT32 titleWidth,
			MonoString* style, MonoArray* guiOptions, bool withTitle);

		static void internal_getValue(ScriptGUIFloatField* nativeInstance, float* output);
		static float internal_setValue(ScriptGUIFloatField* nativeInstance, float value);
		static void internal_hasInputFocus(ScriptGUIFloatField* nativeInstance, bool* output);
		static void internal_setTint(ScriptGUIFloatField* nativeInstance, Color* color);
		static void internal_setRange(ScriptGUIFloatField* nativeInstance, float min, float max);
		static void internal_setStep(ScriptGUIFloatField* nativeInstance, float step);
		static float internal_getStep(ScriptGUIFloatField* nativeInstance);

		typedef void(__stdcall *OnChangedThunkDef) (MonoObject*, float, MonoException**);
		typedef void(__stdcall *OnConfirmedThunkDef) (MonoObject*, MonoException**);

		static OnChangedThunkDef onChangedThunk;
		static OnConfirmedThunkDef onConfirmedThunk;
	};

	/** @} */
}