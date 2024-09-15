//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptCursor.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "../../../Foundation/bsfEngine/Platform/BsCursor.h"
#include "BsScriptTVector2I.generated.h"
#include "BsScriptPixelData.generated.h"

namespace bs
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

	void ScriptCursor::InternalSetScreenPosition(TVector2I<int32_t>* screenPos)
	{
		Cursor::Instance().SetScreenPosition(*screenPos);
	}

	void ScriptCursor::InternalGetScreenPosition(TVector2I<int32_t>* __output)
	{
		TVector2I<int32_t> tmp__output;
		tmp__output = Cursor::Instance().GetScreenPosition();

		*__output = tmp__output;
	}

	void ScriptCursor::InternalHide()
	{
		Cursor::Instance().Hide();
	}

	void ScriptCursor::InternalShow()
	{
		Cursor::Instance().Show();
	}

	void ScriptCursor::InternalClipToRect(Rect2I* screenRect)
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

	void ScriptCursor::InternalSetCursorIcon(MonoString* name, MonoObject* pixelData, TVector2I<int32_t>* hotSpot)
	{
		String tmpname;
		tmpname = MonoUtil::MonoToString(name);
		SPtr<PixelData> tmppixelData;
		ScriptPixelData* scriptObjectWrapperpixelData;
		scriptObjectWrapperpixelData = ScriptPixelData::GetScriptObjectWrapper(pixelData);
		if(scriptObjectWrapperpixelData != nullptr)
			tmppixelData = std::static_pointer_cast<PixelData>(scriptObjectWrapperpixelData->GetBaseNativeObjectAsShared());
		Cursor::Instance().SetCursorIcon(tmpname, *tmppixelData, *hotSpot);
	}

	void ScriptCursor::InternalSetCursorIcon0(CursorType type, MonoObject* pixelData, TVector2I<int32_t>* hotSpot)
	{
		SPtr<PixelData> tmppixelData;
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
