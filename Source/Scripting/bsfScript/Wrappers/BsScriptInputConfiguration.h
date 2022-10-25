//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
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
		SCRIPT_OBJ(ENGINE_ASSEMBLY, ENGINE_NS, "InputConfiguration")

		/** Returns the managed version of this object. */
		MonoObject* GetManagedInstance() const;

		/**	Returns the internal wrapped InputConfiguration object. */
		SPtr<InputConfiguration> GetInternalValue() const { return mInputConfig; }

		/**
		 * Attempts to find a existing interop object for the provided input configuration. Returns null if one cannot be
		 * found.
		 */
		static ScriptInputConfiguration* GetScriptInputConfig(const SPtr<InputConfiguration>& inputConfig);

		/**
		 * Creates a new interop object for the provided input configuration. Caller should first call
		 * getScriptInputConfig() to ensure one doesn't already exist.
		 */
		static ScriptInputConfiguration* CreateScriptInputConfig(const SPtr<InputConfiguration>& inputConfig);

	private:
		ScriptInputConfiguration(MonoObject* instance, const SPtr<InputConfiguration>& inputConfig);

		/** @copydoc ScriptObjectBase::_onManagedInstanceDeleted */
		void OnManagedInstanceDeletedInternal(bool assemblyRefresh) override;

		SPtr<InputConfiguration> mInputConfig;
		u32 mGCHandle = 0;
		static Map<u64, ScriptInputConfiguration*> ScriptInputConfigurations;

		/************************************************************************/
		/* 								CLR HOOKS						   		*/
		/************************************************************************/
		static void InternalCreateInstance(MonoObject* object);

		static void InternalRegisterButton(ScriptInputConfiguration* thisPtr, MonoString* name, ButtonCode buttonCode, ButtonModifier modifiers, bool repeatable);
		static void InternalUnregisterButton(ScriptInputConfiguration* thisPtr, MonoString* name);

		static void InternalRegisterAxis(ScriptInputConfiguration* thisPtr, MonoString* name, InputAxis type, float deadZone, float sensitivity, bool invert);
		static void InternalUnregisterAxis(ScriptInputConfiguration* thisPtr, MonoString* name);

		static void InternalSetRepeatInterval(ScriptInputConfiguration* thisPtr, u64 milliseconds);
		static u64 InternalGetRepeatInterval(ScriptInputConfiguration* thisPtr);
	};

	/**	Interop class between C++ & CLR for VirtualAxis. */
	class BS_SCR_BE_EXPORT ScriptVirtualAxis : public ScriptObject<ScriptVirtualAxis>
	{
	public:
		SCRIPT_OBJ(ENGINE_ASSEMBLY, ENGINE_NS, "VirtualAxis")

	private:
		ScriptVirtualAxis(MonoObject* instance);

		/************************************************************************/
		/* 								CLR HOOKS						   		*/
		/************************************************************************/
		static u32 InternalInitVirtualAxis(MonoString* name);
	};

	/** @} */
} // namespace bs
