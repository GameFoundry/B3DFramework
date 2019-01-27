//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Wrappers/GUI/BsScriptGUIElementStyle.h"
#include "BsScriptMeta.h"
#include "BsMonoField.h"
#include "BsMonoClass.h"
#include "BsMonoManager.h"
#include "Error/BsException.h"
#include "GUI/BsGUIElementStyle.h"
#include "Wrappers/GUI/BsScriptGUIElementStateStyle.h"
#include "BsMonoUtil.h"
#include "BsScriptResourceManager.h"

#include "BsScriptFont.generated.h"

namespace bs
{
	ScriptGUIElementStyle::ScriptGUIElementStyle(MonoObject* instance)
		:ScriptObject(instance)
	{

	}

	ScriptGUIElementStyle::ScriptGUIElementStyle(MonoObject* instance, const GUIElementStyle& externalStyle)
		:ScriptObject(instance), mElementStyle(externalStyle)
	{

	}

	ScriptGUIElementStyle::~ScriptGUIElementStyle()
	{

	}

	void ScriptGUIElementStyle::initRuntimeData()
	{
		metaData.scriptClass->addInternalCall("Internal_CreateInstance", (void*)&ScriptGUIElementStyle::internal_createInstance);
		metaData.scriptClass->addInternalCall("Internal_AddSubStyle", (void*)&ScriptGUIElementStyle::internal_addSubStyle);

		metaData.scriptClass->addInternalCall("Internal_GetFont", (void*)&ScriptGUIElementStyle::internal_GetFont);
		metaData.scriptClass->addInternalCall("Internal_SetFont", (void*)&ScriptGUIElementStyle::internal_SetFont);

		metaData.scriptClass->addInternalCall("Internal_GetFontSize", (void*)&ScriptGUIElementStyle::internal_GetFontSize);
		metaData.scriptClass->addInternalCall("Internal_SetFontSize", (void*)&ScriptGUIElementStyle::internal_SetFontSize);
		metaData.scriptClass->addInternalCall("Internal_GetTextHorzAlign", (void*)&ScriptGUIElementStyle::internal_GetTextHorzAlign);
		metaData.scriptClass->addInternalCall("Internal_SetTextHorzAlign", (void*)&ScriptGUIElementStyle::internal_SetTextHorzAlign);
		metaData.scriptClass->addInternalCall("Internal_GetTextVertAlign", (void*)&ScriptGUIElementStyle::internal_GetTextVertAlign);
		metaData.scriptClass->addInternalCall("Internal_SetTextVertAlign", (void*)&ScriptGUIElementStyle::internal_SetTextVertAlign);
		metaData.scriptClass->addInternalCall("Internal_GetImagePosition", (void*)&ScriptGUIElementStyle::internal_GetImagePosition);
		metaData.scriptClass->addInternalCall("Internal_SetImagePosition", (void*)&ScriptGUIElementStyle::internal_SetImagePosition);
		metaData.scriptClass->addInternalCall("Internal_GetWordWrap", (void*)&ScriptGUIElementStyle::internal_GetWordWrap);
		metaData.scriptClass->addInternalCall("Internal_SetWordWrap", (void*)&ScriptGUIElementStyle::internal_SetWordWrap);

		metaData.scriptClass->addInternalCall("Internal_GetNormal", (void*)&ScriptGUIElementStyle::internal_GetNormal);
		metaData.scriptClass->addInternalCall("Internal_SetNormal", (void*)&ScriptGUIElementStyle::internal_SetNormal);
		metaData.scriptClass->addInternalCall("Internal_GetHover", (void*)&ScriptGUIElementStyle::internal_GetHover);
		metaData.scriptClass->addInternalCall("Internal_SetHover", (void*)&ScriptGUIElementStyle::internal_SetHover);
		metaData.scriptClass->addInternalCall("Internal_GetActive", (void*)&ScriptGUIElementStyle::internal_GetActive);
		metaData.scriptClass->addInternalCall("Internal_SetActive", (void*)&ScriptGUIElementStyle::internal_SetActive);
		metaData.scriptClass->addInternalCall("Internal_GetFocused", (void*)&ScriptGUIElementStyle::internal_GetFocused);
		metaData.scriptClass->addInternalCall("Internal_SetFocused", (void*)&ScriptGUIElementStyle::internal_SetFocused);

		metaData.scriptClass->addInternalCall("Internal_GetNormalOn", (void*)&ScriptGUIElementStyle::internal_GetNormalOn);
		metaData.scriptClass->addInternalCall("Internal_SetNormalOn", (void*)&ScriptGUIElementStyle::internal_SetNormalOn);
		metaData.scriptClass->addInternalCall("Internal_GetHoverOn", (void*)&ScriptGUIElementStyle::internal_GetHoverOn);
		metaData.scriptClass->addInternalCall("Internal_SetHoverOn", (void*)&ScriptGUIElementStyle::internal_SetHoverOn);
		metaData.scriptClass->addInternalCall("Internal_GetActiveOn", (void*)&ScriptGUIElementStyle::internal_GetActiveOn);
		metaData.scriptClass->addInternalCall("Internal_SetActiveOn", (void*)&ScriptGUIElementStyle::internal_SetActiveOn);
		metaData.scriptClass->addInternalCall("Internal_GetFocusedOn", (void*)&ScriptGUIElementStyle::internal_GetFocusedOn);
		metaData.scriptClass->addInternalCall("Internal_SetFocusedOn", (void*)&ScriptGUIElementStyle::internal_SetFocusedOn);

		metaData.scriptClass->addInternalCall("Internal_GetBorder", (void*)&ScriptGUIElementStyle::internal_GetBorder);
		metaData.scriptClass->addInternalCall("Internal_SetBorder", (void*)&ScriptGUIElementStyle::internal_SetBorder);
		metaData.scriptClass->addInternalCall("Internal_GetMargins", (void*)&ScriptGUIElementStyle::internal_GetMargins);
		metaData.scriptClass->addInternalCall("Internal_SetMargins", (void*)&ScriptGUIElementStyle::internal_SetMargins);
		metaData.scriptClass->addInternalCall("Internal_GetContentOffset", (void*)&ScriptGUIElementStyle::internal_GetContentOffset);
		metaData.scriptClass->addInternalCall("Internal_SetContentOffset", (void*)&ScriptGUIElementStyle::internal_SetContentOffset);

		metaData.scriptClass->addInternalCall("Internal_GetWidth", (void*)&ScriptGUIElementStyle::internal_GetWidth);
		metaData.scriptClass->addInternalCall("Internal_SetWidth", (void*)&ScriptGUIElementStyle::internal_SetWidth);
		metaData.scriptClass->addInternalCall("Internal_GetHeight", (void*)&ScriptGUIElementStyle::internal_GetHeight);
		metaData.scriptClass->addInternalCall("Internal_SetHeight", (void*)&ScriptGUIElementStyle::internal_SetHeight);
		metaData.scriptClass->addInternalCall("Internal_GetMinWidth", (void*)&ScriptGUIElementStyle::internal_GetMinWidth);
		metaData.scriptClass->addInternalCall("Internal_SetMinWidth", (void*)&ScriptGUIElementStyle::internal_SetMinWidth);
		metaData.scriptClass->addInternalCall("Internal_GetMaxWidth", (void*)&ScriptGUIElementStyle::internal_GetMaxWidth);
		metaData.scriptClass->addInternalCall("Internal_SetMaxWidth", (void*)&ScriptGUIElementStyle::internal_SetMaxWidth);
		metaData.scriptClass->addInternalCall("Internal_GetMinHeight", (void*)&ScriptGUIElementStyle::internal_GetMinHeight);
		metaData.scriptClass->addInternalCall("Internal_SetMinHeight", (void*)&ScriptGUIElementStyle::internal_SetMinHeight);
		metaData.scriptClass->addInternalCall("Internal_GetMaxHeight", (void*)&ScriptGUIElementStyle::internal_GetMaxHeight);
		metaData.scriptClass->addInternalCall("Internal_SetMaxHeight", (void*)&ScriptGUIElementStyle::internal_SetMaxHeight);
		metaData.scriptClass->addInternalCall("Internal_GetFixedWidth", (void*)&ScriptGUIElementStyle::internal_GetFixedWidth);
		metaData.scriptClass->addInternalCall("Internal_SetFixedWidth", (void*)&ScriptGUIElementStyle::internal_SetFixedWidth);
		metaData.scriptClass->addInternalCall("Internal_GetFixedHeight", (void*)&ScriptGUIElementStyle::internal_GetFixedHeight);
		metaData.scriptClass->addInternalCall("Internal_SetFixedHeight", (void*)&ScriptGUIElementStyle::internal_SetFixedHeight);
	}

	MonoObject* ScriptGUIElementStyle::create(const GUIElementStyle& style)
	{
		bool dummy = false;

		void* params[1];
		params[0] = &dummy;;

		MonoObject* instance = metaData.scriptClass->createInstance(params, true);

		new (bs_alloc<ScriptGUIElementStyle>()) ScriptGUIElementStyle(instance, style);
		return instance;
	}

	void ScriptGUIElementStyle::internal_createInstance(MonoObject* instance)
	{
		new (bs_alloc<ScriptGUIElementStyle>()) ScriptGUIElementStyle(instance);
	}

	void ScriptGUIElementStyle::internal_addSubStyle(ScriptGUIElementStyle* nativeInstance, MonoString* guiType, MonoString* styleName)
	{
		String guiTypeStr = MonoUtil::monoToString(guiType);
		String styleNameStr = MonoUtil::monoToString(styleName);

		nativeInstance->getInternalValue().subStyles[guiTypeStr] = styleNameStr;
	}

	void ScriptGUIElementStyle::internal_GetFont(ScriptGUIElementStyle* nativeInstance, MonoObject** value)
	{
		const GUIElementStyle& style = nativeInstance->mElementStyle;
		if (style.font != nullptr)
		{
			ScriptResourceBase* scriptFont = ScriptResourceManager::instance().getScriptResource(style.font, true);
			MonoUtil::referenceCopy(value, scriptFont->getManagedInstance());
		}
		else
			*value = nullptr;
	}

	void ScriptGUIElementStyle::internal_SetFont(ScriptGUIElementStyle* nativeInstance, MonoObject* value)
	{
		if (value != nullptr)
		{
			ScriptFont* nativeValue = ScriptFont::toNative(value);
			nativeInstance->mElementStyle.font = nativeValue->getHandle();
		}
		else
			nativeInstance->mElementStyle.font = nullptr;
	}

	void ScriptGUIElementStyle::internal_GetFontSize(ScriptGUIElementStyle* nativeInstance, UINT32* value)
	{
		*value = nativeInstance->mElementStyle.fontSize;
	}

	void ScriptGUIElementStyle::internal_SetFontSize(ScriptGUIElementStyle* nativeInstance, UINT32 value)
	{
		nativeInstance->mElementStyle.fontSize = value;
	}

	void ScriptGUIElementStyle::internal_GetTextHorzAlign(ScriptGUIElementStyle* nativeInstance, TextHorzAlign*value)
	{
		*value = nativeInstance->mElementStyle.textHorzAlign;
	}

	void ScriptGUIElementStyle::internal_SetTextHorzAlign(ScriptGUIElementStyle* nativeInstance, TextHorzAlign value)
	{
		nativeInstance->mElementStyle.textHorzAlign = value;
	}

	void ScriptGUIElementStyle::internal_GetTextVertAlign(ScriptGUIElementStyle* nativeInstance, TextVertAlign* value)
	{
		*value = nativeInstance->mElementStyle.textVertAlign;
	}

	void ScriptGUIElementStyle::internal_SetTextVertAlign(ScriptGUIElementStyle* nativeInstance, TextVertAlign value)
	{
		nativeInstance->mElementStyle.textVertAlign = value;
	}

	void ScriptGUIElementStyle::internal_GetImagePosition(ScriptGUIElementStyle* nativeInstance, GUIImagePosition* value)
	{
		*value = nativeInstance->mElementStyle.imagePosition;
	}

	void ScriptGUIElementStyle::internal_SetImagePosition(ScriptGUIElementStyle* nativeInstance, GUIImagePosition value)
	{
		nativeInstance->mElementStyle.imagePosition = value;
	}

	void ScriptGUIElementStyle::internal_GetWordWrap(ScriptGUIElementStyle* nativeInstance, bool* value)
	{
		*value = nativeInstance->mElementStyle.wordWrap;
	}

	void ScriptGUIElementStyle::internal_SetWordWrap(ScriptGUIElementStyle* nativeInstance, bool value)
	{
		nativeInstance->mElementStyle.wordWrap = value;
	}

	void ScriptGUIElementStyle::internal_GetNormal(ScriptGUIElementStyle* nativeInstance, ScriptGUIElementStateStyleStruct* value)
	{
		auto temp = ScriptGUIElementStateStyle::toManaged(nativeInstance->mElementStyle.normal);
		MonoUtil::valueCopy(value, &temp, ScriptGUIElementStateStyle::getMetaData()->scriptClass->_getInternalClass());
	}

	void ScriptGUIElementStyle::internal_SetNormal(ScriptGUIElementStyle* nativeInstance, ScriptGUIElementStateStyleStruct* value)
	{
		nativeInstance->mElementStyle.normal = ScriptGUIElementStateStyle::toNative(*value);
	}

	void ScriptGUIElementStyle::internal_GetHover(ScriptGUIElementStyle* nativeInstance, ScriptGUIElementStateStyleStruct* value)
	{
		auto temp = ScriptGUIElementStateStyle::toManaged(nativeInstance->mElementStyle.hover);
		MonoUtil::valueCopy(value, &temp, ScriptGUIElementStateStyle::getMetaData()->scriptClass->_getInternalClass());
	}

	void ScriptGUIElementStyle::internal_SetHover(ScriptGUIElementStyle* nativeInstance, ScriptGUIElementStateStyleStruct* value)
	{
		nativeInstance->mElementStyle.hover = ScriptGUIElementStateStyle::toNative(*value);
	}

	void ScriptGUIElementStyle::internal_GetActive(ScriptGUIElementStyle* nativeInstance, ScriptGUIElementStateStyleStruct* value)
	{
		auto temp = ScriptGUIElementStateStyle::toManaged(nativeInstance->mElementStyle.active);
		MonoUtil::valueCopy(value, &temp, ScriptGUIElementStateStyle::getMetaData()->scriptClass->_getInternalClass());
	}

	void ScriptGUIElementStyle::internal_SetActive(ScriptGUIElementStyle* nativeInstance, ScriptGUIElementStateStyleStruct* value)
	{
		nativeInstance->mElementStyle.active = ScriptGUIElementStateStyle::toNative(*value);
	}

	void ScriptGUIElementStyle::internal_GetFocused(ScriptGUIElementStyle* nativeInstance, ScriptGUIElementStateStyleStruct* value)
	{
		auto temp = ScriptGUIElementStateStyle::toManaged(nativeInstance->mElementStyle.focused);
		MonoUtil::valueCopy(value, &temp, ScriptGUIElementStateStyle::getMetaData()->scriptClass->_getInternalClass());
	}

	void ScriptGUIElementStyle::internal_SetFocused(ScriptGUIElementStyle* nativeInstance, ScriptGUIElementStateStyleStruct* value)
	{
		nativeInstance->mElementStyle.focused = ScriptGUIElementStateStyle::toNative(*value);
	}

	void ScriptGUIElementStyle::internal_GetNormalOn(ScriptGUIElementStyle* nativeInstance, ScriptGUIElementStateStyleStruct* value)
	{
		auto temp = ScriptGUIElementStateStyle::toManaged(nativeInstance->mElementStyle.normalOn);
		MonoUtil::valueCopy(value, &temp, ScriptGUIElementStateStyle::getMetaData()->scriptClass->_getInternalClass());
	}

	void ScriptGUIElementStyle::internal_SetNormalOn(ScriptGUIElementStyle* nativeInstance, ScriptGUIElementStateStyleStruct* value)
	{
		nativeInstance->mElementStyle.normalOn = ScriptGUIElementStateStyle::toNative(*value);
	}

	void ScriptGUIElementStyle::internal_GetHoverOn(ScriptGUIElementStyle* nativeInstance, ScriptGUIElementStateStyleStruct* value)
	{
		auto temp = ScriptGUIElementStateStyle::toManaged(nativeInstance->mElementStyle.hoverOn);
		MonoUtil::valueCopy(value, &temp, ScriptGUIElementStateStyle::getMetaData()->scriptClass->_getInternalClass());
	}

	void ScriptGUIElementStyle::internal_SetHoverOn(ScriptGUIElementStyle* nativeInstance, ScriptGUIElementStateStyleStruct* value)
	{
		nativeInstance->mElementStyle.hoverOn = ScriptGUIElementStateStyle::toNative(*value);
	}

	void ScriptGUIElementStyle::internal_GetActiveOn(ScriptGUIElementStyle* nativeInstance, ScriptGUIElementStateStyleStruct* value)
	{
		auto temp = ScriptGUIElementStateStyle::toManaged(nativeInstance->mElementStyle.activeOn);
		MonoUtil::valueCopy(value, &temp, ScriptGUIElementStateStyle::getMetaData()->scriptClass->_getInternalClass());
	}

	void ScriptGUIElementStyle::internal_SetActiveOn(ScriptGUIElementStyle* nativeInstance, ScriptGUIElementStateStyleStruct* value)
	{
		nativeInstance->mElementStyle.activeOn = ScriptGUIElementStateStyle::toNative(*value);
	}

	void ScriptGUIElementStyle::internal_GetFocusedOn(ScriptGUIElementStyle* nativeInstance, ScriptGUIElementStateStyleStruct* value)
	{
		auto temp = ScriptGUIElementStateStyle::toManaged(nativeInstance->mElementStyle.focusedOn);
		MonoUtil::valueCopy(value, &temp, ScriptGUIElementStateStyle::getMetaData()->scriptClass->_getInternalClass());
	}

	void ScriptGUIElementStyle::internal_SetFocusedOn(ScriptGUIElementStyle* nativeInstance, ScriptGUIElementStateStyleStruct* value)
	{
		nativeInstance->mElementStyle.focusedOn = ScriptGUIElementStateStyle::toNative(*value);
	}

	void ScriptGUIElementStyle::internal_GetBorder(ScriptGUIElementStyle* nativeInstance, RectOffset* value)
	{
		*value = nativeInstance->mElementStyle.border;
	}

	void ScriptGUIElementStyle::internal_SetBorder(ScriptGUIElementStyle* nativeInstance, RectOffset* value)
	{
		nativeInstance->mElementStyle.border = *value;
	}

	void ScriptGUIElementStyle::internal_GetMargins(ScriptGUIElementStyle* nativeInstance, RectOffset* value)
	{
		*value = nativeInstance->mElementStyle.margins;
	}

	void ScriptGUIElementStyle::internal_SetMargins(ScriptGUIElementStyle* nativeInstance, RectOffset* value)
	{
		nativeInstance->mElementStyle.margins = *value;
	}

	void ScriptGUIElementStyle::internal_GetContentOffset(ScriptGUIElementStyle* nativeInstance, RectOffset* value)
	{
		*value = nativeInstance->mElementStyle.contentOffset;
	}

	void ScriptGUIElementStyle::internal_SetContentOffset(ScriptGUIElementStyle* nativeInstance, RectOffset* value)
	{
		nativeInstance->mElementStyle.contentOffset = *value;
	}

	void ScriptGUIElementStyle::internal_GetWidth(ScriptGUIElementStyle* nativeInstance, UINT32* value)
	{
		*value = nativeInstance->mElementStyle.width;
	}

	void ScriptGUIElementStyle::internal_SetWidth(ScriptGUIElementStyle* nativeInstance, UINT32 value)
	{
		nativeInstance->mElementStyle.width = value;
	}

	void ScriptGUIElementStyle::internal_GetHeight(ScriptGUIElementStyle* nativeInstance, UINT32* value)
	{
		*value = nativeInstance->mElementStyle.height;
	}

	void ScriptGUIElementStyle::internal_SetHeight(ScriptGUIElementStyle* nativeInstance, UINT32 value)
	{
		nativeInstance->mElementStyle.height = value;
	}

	void ScriptGUIElementStyle::internal_GetMinWidth(ScriptGUIElementStyle* nativeInstance, UINT32* value)
	{
		*value = nativeInstance->mElementStyle.minWidth;
	}

	void ScriptGUIElementStyle::internal_SetMinWidth(ScriptGUIElementStyle* nativeInstance, UINT32 value)
	{
		nativeInstance->mElementStyle.minWidth = value;
	}

	void ScriptGUIElementStyle::internal_GetMaxWidth(ScriptGUIElementStyle* nativeInstance, UINT32* value)
	{
		*value = nativeInstance->mElementStyle.maxWidth;
	}

	void ScriptGUIElementStyle::internal_SetMaxWidth(ScriptGUIElementStyle* nativeInstance, UINT32 value)
	{
		nativeInstance->mElementStyle.maxWidth = value;
	}

	void ScriptGUIElementStyle::internal_GetMinHeight(ScriptGUIElementStyle* nativeInstance, UINT32* value)
	{
		*value = nativeInstance->mElementStyle.minHeight;
	}

	void ScriptGUIElementStyle::internal_SetMinHeight(ScriptGUIElementStyle* nativeInstance, UINT32 value)
	{
		nativeInstance->mElementStyle.minHeight = value;
	}

	void ScriptGUIElementStyle::internal_GetMaxHeight(ScriptGUIElementStyle* nativeInstance, UINT32* value)
	{
		*value = nativeInstance->mElementStyle.maxHeight;
	}

	void ScriptGUIElementStyle::internal_SetMaxHeight(ScriptGUIElementStyle* nativeInstance, UINT32 value)
	{
		nativeInstance->mElementStyle.maxHeight = value;
	}

	void ScriptGUIElementStyle::internal_GetFixedWidth(ScriptGUIElementStyle* nativeInstance, bool* value)
	{
		*value = nativeInstance->mElementStyle.fixedWidth;
	}

	void ScriptGUIElementStyle::internal_SetFixedWidth(ScriptGUIElementStyle* nativeInstance, bool value)
	{
		nativeInstance->mElementStyle.fixedWidth = value;
	}

	void ScriptGUIElementStyle::internal_GetFixedHeight(ScriptGUIElementStyle* nativeInstance, bool* value)
	{
		*value = nativeInstance->mElementStyle.fixedHeight;
	}

	void ScriptGUIElementStyle::internal_SetFixedHeight(ScriptGUIElementStyle* nativeInstance, bool value)
	{
		nativeInstance->mElementStyle.fixedHeight = value;
	}
}