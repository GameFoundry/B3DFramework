//************************************ B3D Framework - Copyright 2025 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DScriptEnginePrerequisites.h"
#include "../../../Foundation/Core/Input/B3DInput.h"
#include "B3DScriptTypeDefinition.h"
#include "../../../Foundation/Core/Input/B3DInputFwd.h"
#include "../../../Foundation/Utility/Math/B3DVector2.h"
#include "../../../Foundation/Core/Input/B3DInputFwd.h"
#include "../../../Foundation/Core/Input/B3DInputFwd.h"
#include "../../../Foundation/Core/Input/B3DInputFwd.h"
#include "../../../Foundation/Core/Input/B3DInputFwd.h"

namespace b3d { struct __PointerEventInterop; }
namespace b3d
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptInput : public TScriptTypeDefinition<ScriptInput>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "Input")

		ScriptInput();

		static void SetupScriptBindings();

		static void StartUp();
		static void ShutDown();

	private:
		static void OnButtonDown(const ButtonEvent& p0);
		static void OnButtonUp(const ButtonEvent& p0);
		static void OnCharInput(const TextInputEvent& p0);
		static void OnPointerMoved(const PointerEvent& p0);
		static void OnPointerPressed(const PointerEvent& p0);
		static void OnPointerReleased(const PointerEvent& p0);
		static void OnPointerDoubleClick(const PointerEvent& p0);

		typedef void(B3D_THUNKCALL *OnButtonDownThunkDefinition) (MonoObject* p0, MonoException**);
		static OnButtonDownThunkDefinition OnButtonDownThunk;
		typedef void(B3D_THUNKCALL *OnButtonUpThunkDefinition) (MonoObject* p0, MonoException**);
		static OnButtonUpThunkDefinition OnButtonUpThunk;
		typedef void(B3D_THUNKCALL *OnCharInputThunkDefinition) (MonoObject* p0, MonoException**);
		static OnCharInputThunkDefinition OnCharInputThunk;
		typedef void(B3D_THUNKCALL *OnPointerMovedThunkDefinition) (MonoObject* p0, MonoException**);
		static OnPointerMovedThunkDefinition OnPointerMovedThunk;
		typedef void(B3D_THUNKCALL *OnPointerPressedThunkDefinition) (MonoObject* p0, MonoException**);
		static OnPointerPressedThunkDefinition OnPointerPressedThunk;
		typedef void(B3D_THUNKCALL *OnPointerReleasedThunkDefinition) (MonoObject* p0, MonoException**);
		static OnPointerReleasedThunkDefinition OnPointerReleasedThunk;
		typedef void(B3D_THUNKCALL *OnPointerDoubleClickThunkDefinition) (MonoObject* p0, MonoException**);
		static OnPointerDoubleClickThunkDefinition OnPointerDoubleClickThunk;

		static HEvent OnButtonDownConnection;
		static HEvent OnButtonUpConnection;
		static HEvent OnCharInputConnection;
		static HEvent OnPointerMovedConnection;
		static HEvent OnPointerPressedConnection;
		static HEvent OnPointerReleasedConnection;
		static HEvent OnPointerDoubleClickConnection;

		static float InternalGetAxisValue(uint32_t type, uint32_t deviceIndex);
		static bool InternalIsButtonHeld(ButtonCode keyCode, uint32_t deviceIndex);
		static bool InternalIsButtonUp(ButtonCode keyCode, uint32_t deviceIndex);
		static bool InternalIsButtonDown(ButtonCode keyCode, uint32_t deviceIndex);
		static void InternalGetPointerPosition(TVector2<int32_t>* __output);
		static void InternalGetPointerDelta(TVector2<int32_t>* __output);
		static bool InternalIsPointerButtonHeld(PointerEventButton pointerButton);
		static bool InternalIsPointerButtonUp(PointerEventButton pointerButton);
		static bool InternalIsPointerButtonDown(PointerEventButton pointerButton);
		static bool InternalIsPointerDoubleClicked();
		static void InternalSetMouseSmoothing(bool enabled);
	};
}
