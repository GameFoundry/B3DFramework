//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptGUIElementStyle.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "BsScriptResourceManager.h"
#include "BsScriptFont.generated.h"
#include "BsScriptRectOffset.generated.h"
#include "BsScriptGUIElementStateStyle.generated.h"

using namespace bs;
ScriptGUIElementStyle::ScriptGUIElementStyle(MonoObject* managedInstance, const SPtr<GUIElementStyle>& value)
	: TScriptReflectable(managedInstance, value)
{
}

void ScriptGUIElementStyle::InitRuntimeData()
{
	metaData.ScriptClass->AddInternalCall("Internal_GUIElementStyle", (void*)&ScriptGUIElementStyle::InternalGUIElementStyle);
	metaData.ScriptClass->AddInternalCall("Internal_AddSubStyle", (void*)&ScriptGUIElementStyle::InternalAddSubStyle);
	metaData.ScriptClass->AddInternalCall("Internal_GetFont", (void*)&ScriptGUIElementStyle::InternalGetFont);
	metaData.ScriptClass->AddInternalCall("Internal_SetFont", (void*)&ScriptGUIElementStyle::InternalSetFont);
	metaData.ScriptClass->AddInternalCall("Internal_GetFontSize", (void*)&ScriptGUIElementStyle::InternalGetFontSize);
	metaData.ScriptClass->AddInternalCall("Internal_SetFontSize", (void*)&ScriptGUIElementStyle::InternalSetFontSize);
	metaData.ScriptClass->AddInternalCall("Internal_GetTextHorzAlign", (void*)&ScriptGUIElementStyle::InternalGetTextHorzAlign);
	metaData.ScriptClass->AddInternalCall("Internal_SetTextHorzAlign", (void*)&ScriptGUIElementStyle::InternalSetTextHorzAlign);
	metaData.ScriptClass->AddInternalCall("Internal_GetTextVertAlign", (void*)&ScriptGUIElementStyle::InternalGetTextVertAlign);
	metaData.ScriptClass->AddInternalCall("Internal_SetTextVertAlign", (void*)&ScriptGUIElementStyle::InternalSetTextVertAlign);
	metaData.ScriptClass->AddInternalCall("Internal_GetImagePosition", (void*)&ScriptGUIElementStyle::InternalGetImagePosition);
	metaData.ScriptClass->AddInternalCall("Internal_SetImagePosition", (void*)&ScriptGUIElementStyle::InternalSetImagePosition);
	metaData.ScriptClass->AddInternalCall("Internal_GetWordWrap", (void*)&ScriptGUIElementStyle::InternalGetWordWrap);
	metaData.ScriptClass->AddInternalCall("Internal_SetWordWrap", (void*)&ScriptGUIElementStyle::InternalSetWordWrap);
	metaData.ScriptClass->AddInternalCall("Internal_GetNormal", (void*)&ScriptGUIElementStyle::InternalGetNormal);
	metaData.ScriptClass->AddInternalCall("Internal_SetNormal", (void*)&ScriptGUIElementStyle::InternalSetNormal);
	metaData.ScriptClass->AddInternalCall("Internal_GetHover", (void*)&ScriptGUIElementStyle::InternalGetHover);
	metaData.ScriptClass->AddInternalCall("Internal_SetHover", (void*)&ScriptGUIElementStyle::InternalSetHover);
	metaData.ScriptClass->AddInternalCall("Internal_GetActive", (void*)&ScriptGUIElementStyle::InternalGetActive);
	metaData.ScriptClass->AddInternalCall("Internal_SetActive", (void*)&ScriptGUIElementStyle::InternalSetActive);
	metaData.ScriptClass->AddInternalCall("Internal_GetFocused", (void*)&ScriptGUIElementStyle::InternalGetFocused);
	metaData.ScriptClass->AddInternalCall("Internal_SetFocused", (void*)&ScriptGUIElementStyle::InternalSetFocused);
	metaData.ScriptClass->AddInternalCall("Internal_GetFocusedHover", (void*)&ScriptGUIElementStyle::InternalGetFocusedHover);
	metaData.ScriptClass->AddInternalCall("Internal_SetFocusedHover", (void*)&ScriptGUIElementStyle::InternalSetFocusedHover);
	metaData.ScriptClass->AddInternalCall("Internal_GetNormalOn", (void*)&ScriptGUIElementStyle::InternalGetNormalOn);
	metaData.ScriptClass->AddInternalCall("Internal_SetNormalOn", (void*)&ScriptGUIElementStyle::InternalSetNormalOn);
	metaData.ScriptClass->AddInternalCall("Internal_GetHoverOn", (void*)&ScriptGUIElementStyle::InternalGetHoverOn);
	metaData.ScriptClass->AddInternalCall("Internal_SetHoverOn", (void*)&ScriptGUIElementStyle::InternalSetHoverOn);
	metaData.ScriptClass->AddInternalCall("Internal_GetActiveOn", (void*)&ScriptGUIElementStyle::InternalGetActiveOn);
	metaData.ScriptClass->AddInternalCall("Internal_SetActiveOn", (void*)&ScriptGUIElementStyle::InternalSetActiveOn);
	metaData.ScriptClass->AddInternalCall("Internal_GetFocusedOn", (void*)&ScriptGUIElementStyle::InternalGetFocusedOn);
	metaData.ScriptClass->AddInternalCall("Internal_SetFocusedOn", (void*)&ScriptGUIElementStyle::InternalSetFocusedOn);
	metaData.ScriptClass->AddInternalCall("Internal_GetFocusedHoverOn", (void*)&ScriptGUIElementStyle::InternalGetFocusedHoverOn);
	metaData.ScriptClass->AddInternalCall("Internal_SetFocusedHoverOn", (void*)&ScriptGUIElementStyle::InternalSetFocusedHoverOn);
	metaData.ScriptClass->AddInternalCall("Internal_GetBorder", (void*)&ScriptGUIElementStyle::InternalGetBorder);
	metaData.ScriptClass->AddInternalCall("Internal_SetBorder", (void*)&ScriptGUIElementStyle::InternalSetBorder);
	metaData.ScriptClass->AddInternalCall("Internal_GetMargins", (void*)&ScriptGUIElementStyle::InternalGetMargins);
	metaData.ScriptClass->AddInternalCall("Internal_SetMargins", (void*)&ScriptGUIElementStyle::InternalSetMargins);
	metaData.ScriptClass->AddInternalCall("Internal_GetContentOffset", (void*)&ScriptGUIElementStyle::InternalGetContentOffset);
	metaData.ScriptClass->AddInternalCall("Internal_SetContentOffset", (void*)&ScriptGUIElementStyle::InternalSetContentOffset);
	metaData.ScriptClass->AddInternalCall("Internal_GetPadding", (void*)&ScriptGUIElementStyle::InternalGetPadding);
	metaData.ScriptClass->AddInternalCall("Internal_SetPadding", (void*)&ScriptGUIElementStyle::InternalSetPadding);
	metaData.ScriptClass->AddInternalCall("Internal_GetWidth", (void*)&ScriptGUIElementStyle::InternalGetWidth);
	metaData.ScriptClass->AddInternalCall("Internal_SetWidth", (void*)&ScriptGUIElementStyle::InternalSetWidth);
	metaData.ScriptClass->AddInternalCall("Internal_GetHeight", (void*)&ScriptGUIElementStyle::InternalGetHeight);
	metaData.ScriptClass->AddInternalCall("Internal_SetHeight", (void*)&ScriptGUIElementStyle::InternalSetHeight);
	metaData.ScriptClass->AddInternalCall("Internal_GetMinWidth", (void*)&ScriptGUIElementStyle::InternalGetMinWidth);
	metaData.ScriptClass->AddInternalCall("Internal_SetMinWidth", (void*)&ScriptGUIElementStyle::InternalSetMinWidth);
	metaData.ScriptClass->AddInternalCall("Internal_GetMaxWidth", (void*)&ScriptGUIElementStyle::InternalGetMaxWidth);
	metaData.ScriptClass->AddInternalCall("Internal_SetMaxWidth", (void*)&ScriptGUIElementStyle::InternalSetMaxWidth);
	metaData.ScriptClass->AddInternalCall("Internal_GetMinHeight", (void*)&ScriptGUIElementStyle::InternalGetMinHeight);
	metaData.ScriptClass->AddInternalCall("Internal_SetMinHeight", (void*)&ScriptGUIElementStyle::InternalSetMinHeight);
	metaData.ScriptClass->AddInternalCall("Internal_GetMaxHeight", (void*)&ScriptGUIElementStyle::InternalGetMaxHeight);
	metaData.ScriptClass->AddInternalCall("Internal_SetMaxHeight", (void*)&ScriptGUIElementStyle::InternalSetMaxHeight);
	metaData.ScriptClass->AddInternalCall("Internal_GetFixedWidth", (void*)&ScriptGUIElementStyle::InternalGetFixedWidth);
	metaData.ScriptClass->AddInternalCall("Internal_SetFixedWidth", (void*)&ScriptGUIElementStyle::InternalSetFixedWidth);
	metaData.ScriptClass->AddInternalCall("Internal_GetFixedHeight", (void*)&ScriptGUIElementStyle::InternalGetFixedHeight);
	metaData.ScriptClass->AddInternalCall("Internal_SetFixedHeight", (void*)&ScriptGUIElementStyle::InternalSetFixedHeight);
}

MonoObject* ScriptGUIElementStyle::Create(const SPtr<GUIElementStyle>& value)
{
	if(value == nullptr) return nullptr;

	bool dummy = false;
	void* ctorParams[1] = { &dummy };

	MonoObject* managedInstance = metaData.ScriptClass->CreateInstance("bool", ctorParams);
	new(B3DAllocate<ScriptGUIElementStyle>()) ScriptGUIElementStyle(managedInstance, value);
	return managedInstance;
}

void ScriptGUIElementStyle::InternalGUIElementStyle(MonoObject* managedInstance)
{
	SPtr<GUIElementStyle> instance = B3DMakeShared<GUIElementStyle>();
	new(B3DAllocate<ScriptGUIElementStyle>()) ScriptGUIElementStyle(managedInstance, instance);
}

void ScriptGUIElementStyle::InternalAddSubStyle(ScriptGUIElementStyle* thisPtr, MonoString* guiType, MonoString* styleName)
{
	String tmpguiType;
	tmpguiType = MonoUtil::MonoToString(guiType);
	String tmpstyleName;
	tmpstyleName = MonoUtil::MonoToString(styleName);
	thisPtr->GetInternal()->AddSubStyle(tmpguiType, tmpstyleName);
}

MonoObject* ScriptGUIElementStyle::InternalGetFont(ScriptGUIElementStyle* thisPtr)
{
	ResourceHandle<Font> tmp__output;
	tmp__output = thisPtr->GetInternal()->Font;

	MonoObject* __output;
	ScriptResourceBase* script__output;
	script__output = ScriptResourceManager::Instance().GetScriptResource(tmp__output, true);
	if(script__output != nullptr)
		__output = script__output->GetManagedInstance();
	else
		__output = nullptr;

	return __output;
}

void ScriptGUIElementStyle::InternalSetFont(ScriptGUIElementStyle* thisPtr, MonoObject* value)
{
	ResourceHandle<Font> tmpvalue;
	ScriptFont* scriptvalue;
	scriptvalue = ScriptFont::ToNative(value);
	if(scriptvalue != nullptr)
		tmpvalue = scriptvalue->GetHandle();
	thisPtr->GetInternal()->Font = tmpvalue;
}

uint32_t ScriptGUIElementStyle::InternalGetFontSize(ScriptGUIElementStyle* thisPtr)
{
	uint32_t tmp__output;
	tmp__output = thisPtr->GetInternal()->FontSize;

	uint32_t __output;
	__output = tmp__output;

	return __output;
}

void ScriptGUIElementStyle::InternalSetFontSize(ScriptGUIElementStyle* thisPtr, uint32_t value)
{
	thisPtr->GetInternal()->FontSize = value;
}

TextHorzAlign ScriptGUIElementStyle::InternalGetTextHorzAlign(ScriptGUIElementStyle* thisPtr)
{
	TextHorzAlign tmp__output;
	tmp__output = thisPtr->GetInternal()->TextHorzAlign;

	TextHorzAlign __output;
	__output = tmp__output;

	return __output;
}

void ScriptGUIElementStyle::InternalSetTextHorzAlign(ScriptGUIElementStyle* thisPtr, TextHorzAlign value)
{
	thisPtr->GetInternal()->TextHorzAlign = value;
}

TextVertAlign ScriptGUIElementStyle::InternalGetTextVertAlign(ScriptGUIElementStyle* thisPtr)
{
	TextVertAlign tmp__output;
	tmp__output = thisPtr->GetInternal()->TextVertAlign;

	TextVertAlign __output;
	__output = tmp__output;

	return __output;
}

void ScriptGUIElementStyle::InternalSetTextVertAlign(ScriptGUIElementStyle* thisPtr, TextVertAlign value)
{
	thisPtr->GetInternal()->TextVertAlign = value;
}

GUIImagePosition ScriptGUIElementStyle::InternalGetImagePosition(ScriptGUIElementStyle* thisPtr)
{
	GUIImagePosition tmp__output;
	tmp__output = thisPtr->GetInternal()->ImagePosition;

	GUIImagePosition __output;
	__output = tmp__output;

	return __output;
}

void ScriptGUIElementStyle::InternalSetImagePosition(ScriptGUIElementStyle* thisPtr, GUIImagePosition value)
{
	thisPtr->GetInternal()->ImagePosition = value;
}

bool ScriptGUIElementStyle::InternalGetWordWrap(ScriptGUIElementStyle* thisPtr)
{
	bool tmp__output;
	tmp__output = thisPtr->GetInternal()->WordWrap;

	bool __output;
	__output = tmp__output;

	return __output;
}

void ScriptGUIElementStyle::InternalSetWordWrap(ScriptGUIElementStyle* thisPtr, bool value)
{
	thisPtr->GetInternal()->WordWrap = value;
}

void ScriptGUIElementStyle::InternalGetNormal(ScriptGUIElementStyle* thisPtr, __GUIElementStateStyleInterop* __output)
{
	GUIElementStateStyle tmp__output;
	tmp__output = thisPtr->GetInternal()->Normal;

	__GUIElementStateStyleInterop interop__output;
	interop__output = ScriptGUIElementStateStyle::ToInterop(tmp__output);
	MonoUtil::ValueCopy(__output, &interop__output, ScriptGUIElementStateStyle::GetMetaData()->ScriptClass->GetInternalClassInternal());
}

void ScriptGUIElementStyle::InternalSetNormal(ScriptGUIElementStyle* thisPtr, __GUIElementStateStyleInterop* value)
{
	GUIElementStateStyle tmpvalue;
	tmpvalue = ScriptGUIElementStateStyle::FromInterop(*value);
	thisPtr->GetInternal()->Normal = tmpvalue;
}

void ScriptGUIElementStyle::InternalGetHover(ScriptGUIElementStyle* thisPtr, __GUIElementStateStyleInterop* __output)
{
	GUIElementStateStyle tmp__output;
	tmp__output = thisPtr->GetInternal()->Hover;

	__GUIElementStateStyleInterop interop__output;
	interop__output = ScriptGUIElementStateStyle::ToInterop(tmp__output);
	MonoUtil::ValueCopy(__output, &interop__output, ScriptGUIElementStateStyle::GetMetaData()->ScriptClass->GetInternalClassInternal());
}

void ScriptGUIElementStyle::InternalSetHover(ScriptGUIElementStyle* thisPtr, __GUIElementStateStyleInterop* value)
{
	GUIElementStateStyle tmpvalue;
	tmpvalue = ScriptGUIElementStateStyle::FromInterop(*value);
	thisPtr->GetInternal()->Hover = tmpvalue;
}

void ScriptGUIElementStyle::InternalGetActive(ScriptGUIElementStyle* thisPtr, __GUIElementStateStyleInterop* __output)
{
	GUIElementStateStyle tmp__output;
	tmp__output = thisPtr->GetInternal()->Active;

	__GUIElementStateStyleInterop interop__output;
	interop__output = ScriptGUIElementStateStyle::ToInterop(tmp__output);
	MonoUtil::ValueCopy(__output, &interop__output, ScriptGUIElementStateStyle::GetMetaData()->ScriptClass->GetInternalClassInternal());
}

void ScriptGUIElementStyle::InternalSetActive(ScriptGUIElementStyle* thisPtr, __GUIElementStateStyleInterop* value)
{
	GUIElementStateStyle tmpvalue;
	tmpvalue = ScriptGUIElementStateStyle::FromInterop(*value);
	thisPtr->GetInternal()->Active = tmpvalue;
}

void ScriptGUIElementStyle::InternalGetFocused(ScriptGUIElementStyle* thisPtr, __GUIElementStateStyleInterop* __output)
{
	GUIElementStateStyle tmp__output;
	tmp__output = thisPtr->GetInternal()->Focused;

	__GUIElementStateStyleInterop interop__output;
	interop__output = ScriptGUIElementStateStyle::ToInterop(tmp__output);
	MonoUtil::ValueCopy(__output, &interop__output, ScriptGUIElementStateStyle::GetMetaData()->ScriptClass->GetInternalClassInternal());
}

void ScriptGUIElementStyle::InternalSetFocused(ScriptGUIElementStyle* thisPtr, __GUIElementStateStyleInterop* value)
{
	GUIElementStateStyle tmpvalue;
	tmpvalue = ScriptGUIElementStateStyle::FromInterop(*value);
	thisPtr->GetInternal()->Focused = tmpvalue;
}

void ScriptGUIElementStyle::InternalGetFocusedHover(ScriptGUIElementStyle* thisPtr, __GUIElementStateStyleInterop* __output)
{
	GUIElementStateStyle tmp__output;
	tmp__output = thisPtr->GetInternal()->FocusedHover;

	__GUIElementStateStyleInterop interop__output;
	interop__output = ScriptGUIElementStateStyle::ToInterop(tmp__output);
	MonoUtil::ValueCopy(__output, &interop__output, ScriptGUIElementStateStyle::GetMetaData()->ScriptClass->GetInternalClassInternal());
}

void ScriptGUIElementStyle::InternalSetFocusedHover(ScriptGUIElementStyle* thisPtr, __GUIElementStateStyleInterop* value)
{
	GUIElementStateStyle tmpvalue;
	tmpvalue = ScriptGUIElementStateStyle::FromInterop(*value);
	thisPtr->GetInternal()->FocusedHover = tmpvalue;
}

void ScriptGUIElementStyle::InternalGetNormalOn(ScriptGUIElementStyle* thisPtr, __GUIElementStateStyleInterop* __output)
{
	GUIElementStateStyle tmp__output;
	tmp__output = thisPtr->GetInternal()->NormalOn;

	__GUIElementStateStyleInterop interop__output;
	interop__output = ScriptGUIElementStateStyle::ToInterop(tmp__output);
	MonoUtil::ValueCopy(__output, &interop__output, ScriptGUIElementStateStyle::GetMetaData()->ScriptClass->GetInternalClassInternal());
}

void ScriptGUIElementStyle::InternalSetNormalOn(ScriptGUIElementStyle* thisPtr, __GUIElementStateStyleInterop* value)
{
	GUIElementStateStyle tmpvalue;
	tmpvalue = ScriptGUIElementStateStyle::FromInterop(*value);
	thisPtr->GetInternal()->NormalOn = tmpvalue;
}

void ScriptGUIElementStyle::InternalGetHoverOn(ScriptGUIElementStyle* thisPtr, __GUIElementStateStyleInterop* __output)
{
	GUIElementStateStyle tmp__output;
	tmp__output = thisPtr->GetInternal()->HoverOn;

	__GUIElementStateStyleInterop interop__output;
	interop__output = ScriptGUIElementStateStyle::ToInterop(tmp__output);
	MonoUtil::ValueCopy(__output, &interop__output, ScriptGUIElementStateStyle::GetMetaData()->ScriptClass->GetInternalClassInternal());
}

void ScriptGUIElementStyle::InternalSetHoverOn(ScriptGUIElementStyle* thisPtr, __GUIElementStateStyleInterop* value)
{
	GUIElementStateStyle tmpvalue;
	tmpvalue = ScriptGUIElementStateStyle::FromInterop(*value);
	thisPtr->GetInternal()->HoverOn = tmpvalue;
}

void ScriptGUIElementStyle::InternalGetActiveOn(ScriptGUIElementStyle* thisPtr, __GUIElementStateStyleInterop* __output)
{
	GUIElementStateStyle tmp__output;
	tmp__output = thisPtr->GetInternal()->ActiveOn;

	__GUIElementStateStyleInterop interop__output;
	interop__output = ScriptGUIElementStateStyle::ToInterop(tmp__output);
	MonoUtil::ValueCopy(__output, &interop__output, ScriptGUIElementStateStyle::GetMetaData()->ScriptClass->GetInternalClassInternal());
}

void ScriptGUIElementStyle::InternalSetActiveOn(ScriptGUIElementStyle* thisPtr, __GUIElementStateStyleInterop* value)
{
	GUIElementStateStyle tmpvalue;
	tmpvalue = ScriptGUIElementStateStyle::FromInterop(*value);
	thisPtr->GetInternal()->ActiveOn = tmpvalue;
}

void ScriptGUIElementStyle::InternalGetFocusedOn(ScriptGUIElementStyle* thisPtr, __GUIElementStateStyleInterop* __output)
{
	GUIElementStateStyle tmp__output;
	tmp__output = thisPtr->GetInternal()->FocusedOn;

	__GUIElementStateStyleInterop interop__output;
	interop__output = ScriptGUIElementStateStyle::ToInterop(tmp__output);
	MonoUtil::ValueCopy(__output, &interop__output, ScriptGUIElementStateStyle::GetMetaData()->ScriptClass->GetInternalClassInternal());
}

void ScriptGUIElementStyle::InternalSetFocusedOn(ScriptGUIElementStyle* thisPtr, __GUIElementStateStyleInterop* value)
{
	GUIElementStateStyle tmpvalue;
	tmpvalue = ScriptGUIElementStateStyle::FromInterop(*value);
	thisPtr->GetInternal()->FocusedOn = tmpvalue;
}

void ScriptGUIElementStyle::InternalGetFocusedHoverOn(ScriptGUIElementStyle* thisPtr, __GUIElementStateStyleInterop* __output)
{
	GUIElementStateStyle tmp__output;
	tmp__output = thisPtr->GetInternal()->FocusedHoverOn;

	__GUIElementStateStyleInterop interop__output;
	interop__output = ScriptGUIElementStateStyle::ToInterop(tmp__output);
	MonoUtil::ValueCopy(__output, &interop__output, ScriptGUIElementStateStyle::GetMetaData()->ScriptClass->GetInternalClassInternal());
}

void ScriptGUIElementStyle::InternalSetFocusedHoverOn(ScriptGUIElementStyle* thisPtr, __GUIElementStateStyleInterop* value)
{
	GUIElementStateStyle tmpvalue;
	tmpvalue = ScriptGUIElementStateStyle::FromInterop(*value);
	thisPtr->GetInternal()->FocusedHoverOn = tmpvalue;
}

void ScriptGUIElementStyle::InternalGetBorder(ScriptGUIElementStyle* thisPtr, RectOffset* __output)
{
	RectOffset tmp__output;
	tmp__output = thisPtr->GetInternal()->Border;

	*__output = tmp__output;
}

void ScriptGUIElementStyle::InternalSetBorder(ScriptGUIElementStyle* thisPtr, RectOffset* value)
{
	thisPtr->GetInternal()->Border = *value;
}

void ScriptGUIElementStyle::InternalGetMargins(ScriptGUIElementStyle* thisPtr, RectOffset* __output)
{
	RectOffset tmp__output;
	tmp__output = thisPtr->GetInternal()->Margins;

	*__output = tmp__output;
}

void ScriptGUIElementStyle::InternalSetMargins(ScriptGUIElementStyle* thisPtr, RectOffset* value)
{
	thisPtr->GetInternal()->Margins = *value;
}

void ScriptGUIElementStyle::InternalGetContentOffset(ScriptGUIElementStyle* thisPtr, RectOffset* __output)
{
	RectOffset tmp__output;
	tmp__output = thisPtr->GetInternal()->ContentOffset;

	*__output = tmp__output;
}

void ScriptGUIElementStyle::InternalSetContentOffset(ScriptGUIElementStyle* thisPtr, RectOffset* value)
{
	thisPtr->GetInternal()->ContentOffset = *value;
}

void ScriptGUIElementStyle::InternalGetPadding(ScriptGUIElementStyle* thisPtr, RectOffset* __output)
{
	RectOffset tmp__output;
	tmp__output = thisPtr->GetInternal()->Padding;

	*__output = tmp__output;
}

void ScriptGUIElementStyle::InternalSetPadding(ScriptGUIElementStyle* thisPtr, RectOffset* value)
{
	thisPtr->GetInternal()->Padding = *value;
}

uint32_t ScriptGUIElementStyle::InternalGetWidth(ScriptGUIElementStyle* thisPtr)
{
	uint32_t tmp__output;
	tmp__output = thisPtr->GetInternal()->Width;

	uint32_t __output;
	__output = tmp__output;

	return __output;
}

void ScriptGUIElementStyle::InternalSetWidth(ScriptGUIElementStyle* thisPtr, uint32_t value)
{
	thisPtr->GetInternal()->Width = value;
}

uint32_t ScriptGUIElementStyle::InternalGetHeight(ScriptGUIElementStyle* thisPtr)
{
	uint32_t tmp__output;
	tmp__output = thisPtr->GetInternal()->Height;

	uint32_t __output;
	__output = tmp__output;

	return __output;
}

void ScriptGUIElementStyle::InternalSetHeight(ScriptGUIElementStyle* thisPtr, uint32_t value)
{
	thisPtr->GetInternal()->Height = value;
}

uint32_t ScriptGUIElementStyle::InternalGetMinWidth(ScriptGUIElementStyle* thisPtr)
{
	uint32_t tmp__output;
	tmp__output = thisPtr->GetInternal()->MinWidth;

	uint32_t __output;
	__output = tmp__output;

	return __output;
}

void ScriptGUIElementStyle::InternalSetMinWidth(ScriptGUIElementStyle* thisPtr, uint32_t value)
{
	thisPtr->GetInternal()->MinWidth = value;
}

uint32_t ScriptGUIElementStyle::InternalGetMaxWidth(ScriptGUIElementStyle* thisPtr)
{
	uint32_t tmp__output;
	tmp__output = thisPtr->GetInternal()->MaxWidth;

	uint32_t __output;
	__output = tmp__output;

	return __output;
}

void ScriptGUIElementStyle::InternalSetMaxWidth(ScriptGUIElementStyle* thisPtr, uint32_t value)
{
	thisPtr->GetInternal()->MaxWidth = value;
}

uint32_t ScriptGUIElementStyle::InternalGetMinHeight(ScriptGUIElementStyle* thisPtr)
{
	uint32_t tmp__output;
	tmp__output = thisPtr->GetInternal()->MinHeight;

	uint32_t __output;
	__output = tmp__output;

	return __output;
}

void ScriptGUIElementStyle::InternalSetMinHeight(ScriptGUIElementStyle* thisPtr, uint32_t value)
{
	thisPtr->GetInternal()->MinHeight = value;
}

uint32_t ScriptGUIElementStyle::InternalGetMaxHeight(ScriptGUIElementStyle* thisPtr)
{
	uint32_t tmp__output;
	tmp__output = thisPtr->GetInternal()->MaxHeight;

	uint32_t __output;
	__output = tmp__output;

	return __output;
}

void ScriptGUIElementStyle::InternalSetMaxHeight(ScriptGUIElementStyle* thisPtr, uint32_t value)
{
	thisPtr->GetInternal()->MaxHeight = value;
}

bool ScriptGUIElementStyle::InternalGetFixedWidth(ScriptGUIElementStyle* thisPtr)
{
	bool tmp__output;
	tmp__output = thisPtr->GetInternal()->FixedWidth;

	bool __output;
	__output = tmp__output;

	return __output;
}

void ScriptGUIElementStyle::InternalSetFixedWidth(ScriptGUIElementStyle* thisPtr, bool value)
{
	thisPtr->GetInternal()->FixedWidth = value;
}

bool ScriptGUIElementStyle::InternalGetFixedHeight(ScriptGUIElementStyle* thisPtr)
{
	bool tmp__output;
	tmp__output = thisPtr->GetInternal()->FixedHeight;

	bool __output;
	__output = tmp__output;

	return __output;
}

void ScriptGUIElementStyle::InternalSetFixedHeight(ScriptGUIElementStyle* thisPtr, bool value)
{
	thisPtr->GetInternal()->FixedHeight = value;
}
