//************************************ B3D Framework - Copyright 2025 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DScriptEnginePrerequisites.h"
#include "../../../Foundation/Core/Input/B3DVirtualInput.h"
#include "B3DScriptTypeDefinition.h"
#include "../../../Foundation/Core/Input/B3DInputConfiguration.h"
#include "../../../Foundation/Core/Input/B3DInputConfiguration.h"

namespace b3d
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptVirtualInput : public TScriptTypeDefinition<ScriptVirtualInput>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "VirtualInput")

		ScriptVirtualInput();

		static void SetupScriptBindings();

		static void StartUp();
		static void ShutDown();

	private:
		static void OnButtonDown(const VirtualButton& p0, uint32_t p1);
		static void OnButtonUp(const VirtualButton& p0, uint32_t p1);
		static void OnButtonHeld(const VirtualButton& p0, uint32_t p1);

		typedef void(B3D_THUNKCALL *OnButtonDownThunkDefinition) (MonoObject* p0, uint32_t p1, MonoException**);
		static OnButtonDownThunkDefinition OnButtonDownThunk;
		typedef void(B3D_THUNKCALL *OnButtonUpThunkDefinition) (MonoObject* p0, uint32_t p1, MonoException**);
		static OnButtonUpThunkDefinition OnButtonUpThunk;
		typedef void(B3D_THUNKCALL *OnButtonHeldThunkDefinition) (MonoObject* p0, uint32_t p1, MonoException**);
		static OnButtonHeldThunkDefinition OnButtonHeldThunk;

		static HEvent OnButtonDownConnection;
		static HEvent OnButtonUpConnection;
		static HEvent OnButtonHeldConnection;

		static void InternalSetConfiguration(MonoObject* input);
		static MonoObject* InternalGetConfiguration();
		static void InternalGetOrCreateVirtualButton(MonoString* name, VirtualButton* __output);
		static void InternalGetOrCreateVirtualAxis(MonoString* name, VirtualAxis* __output);
		static bool InternalIsButtonDown(VirtualButton* button, uint32_t deviceIndex);
		static bool InternalIsButtonUp(VirtualButton* button, uint32_t deviceIndex);
		static bool InternalIsButtonHeld(VirtualButton* button, uint32_t deviceIndex);
		static float InternalGetAxisValue(VirtualAxis* axis, uint32_t deviceIndex);
	};
}
