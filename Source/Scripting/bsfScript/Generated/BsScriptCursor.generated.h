//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "../../../Foundation/bsfEngine/Platform/BsCursor.h"
#include "BsScriptTypeDefinition.h"
#include "../../../Foundation/bsfUtility/Math/BsVector2I.h"
#include "Math/BsRect2I.h"
#include "../../../Foundation/bsfCore/Image/BsPixelData.h"
#include "../../../Foundation/bsfEngine/Utility/BsEnums.h"

namespace bs
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptCursor : public TScriptTypeDefinition<ScriptCursor>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "Cursor")

		ScriptCursor();

		static void SetupScriptBindings();

		static MonoObject* CreateScriptObject(bool construct);

	private:
		static void InternalSetScreenPosition(TVector2I<int32_t>* screenPos);
		static void InternalGetScreenPosition(TVector2I<int32_t>* __output);
		static void InternalHide();
		static void InternalShow();
		static void InternalClipToRect(Rect2I* screenRect);
		static void InternalClipDisable();
		static void InternalSetCursor(CursorType type);
		static void InternalSetCursor0(MonoString* name);
		static void InternalSetCursorIcon(MonoString* name, MonoObject* pixelData, TVector2I<int32_t>* hotSpot);
		static void InternalSetCursorIcon0(CursorType type, MonoObject* pixelData, TVector2I<int32_t>* hotSpot);
		static void InternalClearCursorIcon(MonoString* name);
		static void InternalClearCursorIcon0(CursorType type);
	};
}
