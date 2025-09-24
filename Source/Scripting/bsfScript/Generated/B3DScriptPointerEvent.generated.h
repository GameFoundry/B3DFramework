//********************************* B3D Framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DScriptEnginePrerequisites.h"
#include "B3DScriptObjectWrapper.h"
#include "../../../Foundation/Core/Input/B3DInputFwd.h"
#include "../../../Foundation/Utility/Math/B3DVector2.h"
#include "../../../Foundation/Core/Input/B3DInputFwd.h"
#include "../../../Foundation/Core/Input/B3DInputFwd.h"

namespace b3d
{
	struct __PointerEventInterop
	{
		TVector2<int32_t> ScreenPos;
		TVector2<int32_t> Delta;
		PointerEventButton Button;
		PointerEventType Type;
		bool Shift;
		bool Control;
		bool Alt;
		float MouseWheelScrollAmount;
		bool IsUsed;
	};

	class B3D_SCRIPT_INTEROP_EXPORT ScriptPointerEvent : public TScriptTypeDefinition<ScriptPointerEvent>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "PointerEvent")

		static MonoObject* Box(const __PointerEventInterop& value);
		static __PointerEventInterop Unbox(MonoObject* value);
		static PointerEvent FromInterop(const __PointerEventInterop& value);
		static __PointerEventInterop ToInterop(const PointerEvent& value);

	private:
		ScriptPointerEvent();

	};
}
