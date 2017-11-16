//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptObject.h"
#include "Input/BsInputConfiguration.h"

namespace bs
{
	/** @addtogroup ScriptInteropEngine
	 *  @{
	 */

	/** Interop class between C++ & CLR for InputConfiguration. */
	class BS_SCR_BE_EXPORT ScriptInputConfiguration : public ScriptObject<ScriptInputConfiguration>
	{
	public:
		SCRIPT_OBJ(ENGINE_ASSEMBLY, "BansheeEngine", "InputConfiguration")

		/** Returns the managed version of this object. */
		MonoObject* getManagedInstance() const;

		/**	Returns the internal wrapped InputConfiguration object. */
		SPtr<InputConfiguration> getInternalValue() const { return mInputConfig; }

		/**
		 * Attempts to find a existing interop object for the provided input configuration. Returns null if one cannot be
		 * found.
		 */
		static ScriptInputConfiguration* getScriptInputConfig(const SPtr<InputConfiguration>& inputConfig);

		/**
		 * Creates a new interop object for the provided input configuration. Caller should first call 
		 * getScriptInputConfig() to ensure one doesn't already exist.
		 */
		static ScriptInputConfiguration* createScriptInputConfig(const SPtr<InputConfiguration>& inputConfig);

	private:
		ScriptInputConfiguration(MonoObject* instance, const SPtr<InputConfiguration>& inputConfig);

		/** @copydoc ScriptObjectBase::_onManagedInstanceDeleted */
		void _onManagedInstanceDeleted() override;

		SPtr<InputConfiguration> mInputConfig;
		UINT32 mGCHandle = 0;
		static Map<UINT64, ScriptInputConfiguration*> ScriptInputConfigurations;

		/************************************************************************/
		/* 								CLR HOOKS						   		*/
		/************************************************************************/
		static void internal_CreateInstance(MonoObject* object);

		static void internal_RegisterButton(ScriptInputConfiguration* thisPtr, MonoString* name, ButtonCode buttonCode,
			ButtonModifier modifiers, bool repeatable);
		static void internal_UnregisterButton(ScriptInputConfiguration* thisPtr, MonoString* name);

		static void internal_RegisterAxis(ScriptInputConfiguration* thisPtr, MonoString* name, InputAxis type, float deadZone,
			float sensitivity, bool invert);
		static void internal_UnregisterAxis(ScriptInputConfiguration* thisPtr, MonoString* name);

		static void internal_SetRepeatInterval(ScriptInputConfiguration* thisPtr, UINT64 milliseconds);
		static UINT64 internal_GetRepeatInterval(ScriptInputConfiguration* thisPtr);
	};

	/**	Interop class between C++ & CLR for VirtualAxis. */
	class BS_SCR_BE_EXPORT ScriptVirtualAxis : public ScriptObject <ScriptVirtualAxis>
	{
	public:
		SCRIPT_OBJ(ENGINE_ASSEMBLY, "BansheeEngine", "VirtualAxis")

	private:
		ScriptVirtualAxis(MonoObject* instance);

		/************************************************************************/
		/* 								CLR HOOKS						   		*/
		/************************************************************************/
		static UINT32 internal_InitVirtualAxis(MonoString* name);
	};

	/** @} */
}