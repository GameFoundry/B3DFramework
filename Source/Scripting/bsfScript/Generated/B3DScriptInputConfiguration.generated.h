//********************************* B3D Framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DScriptEnginePrerequisites.h"
#include "../../../Foundation/Engine/Input/B3DInputConfiguration.h"
#include "B3DScriptNonReflectableWrapper.h"
#include "../../../Foundation/Core/Input/B3DInputFwd.h"
#include "../../../Foundation/Core/Input/B3DInputFwd.h"
#include "../../../Foundation/Engine/Input/B3DInputConfiguration.h"

namespace b3d
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptInputConfiguration : public TScriptNonReflectableWrapper<InputConfiguration, ScriptInputConfiguration>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "InputConfiguration")

		ScriptInputConfiguration(const SPtr<InputConfiguration>& nativeObject);
		~ScriptInputConfiguration();

		static void SetupScriptBindings();

		static MonoObject* CreateScriptObject(bool construct);

	private:
		static void InternalInputConfiguration(MonoObject* scriptObject);
		static void InternalRegisterButton(ScriptInputConfiguration* self, MonoString* name, ButtonCode buttonCode, ButtonModifier modifiers, bool repeatable);
		static void InternalUnregisterButton(ScriptInputConfiguration* self, MonoString* name);
		static void InternalRegisterAxis(ScriptInputConfiguration* self, MonoString* name, VirtualAxisCreateInformation* createInformation);
		static void InternalUnregisterAxis(ScriptInputConfiguration* self, MonoString* name);
		static void InternalSetRepeatInterval(ScriptInputConfiguration* self, uint64_t milliseconds);
		static uint64_t InternalGetRepeatInterval(ScriptInputConfiguration* self);
	};
}
