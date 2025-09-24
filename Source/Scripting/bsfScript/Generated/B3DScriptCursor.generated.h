//********************************* B3D Framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DScriptEnginePrerequisites.h"
#include "../../../Foundation/Engine/Platform/B3DCursor.h"
#include "B3DScriptTypeDefinition.h"
#include "../../../Foundation/Utility/Math/B3DArea2.h"
#include "../../../Foundation/Engine/GUI/B3DGUIUnits.h"
#include "../../../Foundation/Core/Image/B3DPixelData.h"
#include "../../../Foundation/Engine/Utility/B3DEnums.h"
#include "../../../Foundation/Utility/Math/B3DVector2.h"

namespace b3d { struct __TVector2_TUnitValue_int32_t__PhysicalPixel__Interop; }
namespace b3d
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptCursor : public TScriptTypeDefinition<ScriptCursor>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "Cursor")

		ScriptCursor();

		static void SetupScriptBindings();

	private:
		static void InternalSetScreenPosition(__TVector2_TUnitValue_int32_t__PhysicalPixel__Interop* screenPos);
		static void InternalGetScreenPosition(__TVector2_TUnitValue_int32_t__PhysicalPixel__Interop* __output);
		static void InternalHide();
		static void InternalShow();
		static void InternalClipToRect(TArea2<int32_t, uint32_t>* screenRect);
		static void InternalClipDisable();
		static void InternalSetCursor(CursorType type);
		static void InternalSetCursor0(MonoString* name);
		static void InternalSetCursorIcon(MonoString* name, MonoObject* pixelData, TVector2<int32_t>* hotSpot);
		static void InternalSetCursorIcon0(CursorType type, MonoObject* pixelData, TVector2<int32_t>* hotSpot);
		static void InternalClearCursorIcon(MonoString* name);
		static void InternalClearCursorIcon0(CursorType type);
	};
}
