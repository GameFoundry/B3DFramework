//************************************ B3D Framework - Copyright 2025 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DScriptCursor.generated.h"
#include "B3DMonoMethod.h"
#include "B3DMonoClass.h"
#include "B3DMonoUtil.h"
#include "../../../Engine/Core/Platform/B3DCursor.h"
#include "B3DScriptTArea2.generated.h"
#include "B3DScriptTVector2.generated.h"
#include "B3DScriptPixelData.generated.h"
#include "B3DScriptTVector2.generated.h"

namespace b3d
{
	ScriptCursor::ScriptCursor()
		:TScriptTypeDefinition()
	{
	}

	void ScriptCursor::SetupScriptBindings()
	{
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetScreenPosition", (void*)&ScriptCursor::InternalSetScreenPosition);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetScreenPosition", (void*)&ScriptCursor::InternalGetScreenPosition);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_Hide", (void*)&ScriptCursor::InternalHide);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_Show", (void*)&ScriptCursor::InternalShow);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_ClipToRect", (void*)&ScriptCursor::InternalClipToRect);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_ClipDisable", (void*)&ScriptCursor::InternalClipDisable);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetCursor", (void*)&ScriptCursor::InternalSetCursor);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetCursor0", (void*)&ScriptCursor::InternalSetCursor0);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetCursorIcon", (void*)&ScriptCursor::InternalSetCursorIcon);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetCursorIcon0", (void*)&ScriptCursor::InternalSetCursorIcon0);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_ClearCursorIcon", (void*)&ScriptCursor::InternalClearCursorIcon);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_ClearCursorIcon0", (void*)&ScriptCursor::InternalClearCursorIcon0);

	}

	void ScriptCursor::InternalSetScreenPosition(__TVector2_TUnitValue_int32_t__PhysicalPixel__Interop* screenPos)
	{
		TVector2<TUnitValue<int32_t, PhysicalPixel>> tmpscreenPos;
		tmpscreenPos = ScriptTVector2_TUnitValue_int32_t__PhysicalPixel__::FromInterop(*screenPos);
		Cursor::Instance().SetScreenPosition(tmpscreenPos);
	}

	void ScriptCursor::InternalGetScreenPosition(__TVector2_TUnitValue_int32_t__PhysicalPixel__Interop* __output)
	{
		TVector2<TUnitValue<int32_t, PhysicalPixel>> tmp__output;
		tmp__output = Cursor::Instance().GetScreenPosition();

		__TVector2_TUnitValue_int32_t__PhysicalPixel__Interop interop__output;
		interop__output = ScriptTVector2_TUnitValue_int32_t__PhysicalPixel__::ToInterop(tmp__output);
		MonoUtil::ValueCopy(__output, &interop__output, ScriptTVector2_TUnitValue_int32_t__PhysicalPixel__::GetMetaData()->ScriptClass->GetInternalClass());
	}

	void ScriptCursor::InternalHide()
	{
		Cursor::Instance().Hide();
	}

	void ScriptCursor::InternalShow()
	{
		Cursor::Instance().Show();
	}

	void ScriptCursor::InternalClipToRect(TArea2<int32_t, uint32_t>* screenRect)
	{
		Cursor::Instance().ClipToRect(*screenRect);
	}

	void ScriptCursor::InternalClipDisable()
	{
		Cursor::Instance().ClipDisable();
	}

	void ScriptCursor::InternalSetCursor(CursorType type)
	{
		Cursor::Instance().SetCursor(type);
	}

	void ScriptCursor::InternalSetCursor0(MonoString* name)
	{
		String tmpname;
		tmpname = MonoUtil::MonoToString(name);
		Cursor::Instance().SetCursor(tmpname);
	}

	void ScriptCursor::InternalSetCursorIcon(MonoString* name, MonoObject* pixelData, TVector2<int32_t>* hotSpot)
	{
		String tmpname;
		tmpname = MonoUtil::MonoToString(name);
		TShared<PixelData> tmppixelData;
		ScriptPixelData* scriptObjectWrapperpixelData;
		scriptObjectWrapperpixelData = ScriptPixelData::GetScriptObjectWrapper(pixelData);
		if(scriptObjectWrapperpixelData != nullptr)
			tmppixelData = std::static_pointer_cast<PixelData>(scriptObjectWrapperpixelData->GetBaseNativeObjectAsShared());
		Cursor::Instance().SetCursorIcon(tmpname, *tmppixelData, *hotSpot);
	}

	void ScriptCursor::InternalSetCursorIcon0(CursorType type, MonoObject* pixelData, TVector2<int32_t>* hotSpot)
	{
		TShared<PixelData> tmppixelData;
		ScriptPixelData* scriptObjectWrapperpixelData;
		scriptObjectWrapperpixelData = ScriptPixelData::GetScriptObjectWrapper(pixelData);
		if(scriptObjectWrapperpixelData != nullptr)
			tmppixelData = std::static_pointer_cast<PixelData>(scriptObjectWrapperpixelData->GetBaseNativeObjectAsShared());
		Cursor::Instance().SetCursorIcon(type, *tmppixelData, *hotSpot);
	}

	void ScriptCursor::InternalClearCursorIcon(MonoString* name)
	{
		String tmpname;
		tmpname = MonoUtil::MonoToString(name);
		Cursor::Instance().ClearCursorIcon(tmpname);
	}

	void ScriptCursor::InternalClearCursorIcon0(CursorType type)
	{
		Cursor::Instance().ClearCursorIcon(type);
	}
}
