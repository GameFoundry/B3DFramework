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

namespace bs
{
	ScriptGUIElementStyle::ScriptGUIElementStyle(MonoObject* managedInstance, const SPtr<GUIElementStyle>& value)
		:TScriptReflectable(managedInstance, value)
	{
	}

	void ScriptGUIElementStyle::InitRuntimeData()
	{
		metaData.scriptClass->AddInternalCall("Internal_GUIElementStyle", (void*)&ScriptGUIElementStyle::Internal_GUIElementStyle);
		metaData.scriptClass->AddInternalCall("Internal_addSubStyle", (void*)&ScriptGUIElementStyle::Internal_addSubStyle);
		metaData.scriptClass->AddInternalCall("Internal_getfont", (void*)&ScriptGUIElementStyle::Internal_getfont);
		metaData.scriptClass->AddInternalCall("Internal_setfont", (void*)&ScriptGUIElementStyle::Internal_setfont);
		metaData.scriptClass->AddInternalCall("Internal_getfontSize", (void*)&ScriptGUIElementStyle::Internal_getfontSize);
		metaData.scriptClass->AddInternalCall("Internal_setfontSize", (void*)&ScriptGUIElementStyle::Internal_setfontSize);
		metaData.scriptClass->AddInternalCall("Internal_gettextHorzAlign", (void*)&ScriptGUIElementStyle::Internal_gettextHorzAlign);
		metaData.scriptClass->AddInternalCall("Internal_settextHorzAlign", (void*)&ScriptGUIElementStyle::Internal_settextHorzAlign);
		metaData.scriptClass->AddInternalCall("Internal_gettextVertAlign", (void*)&ScriptGUIElementStyle::Internal_gettextVertAlign);
		metaData.scriptClass->AddInternalCall("Internal_settextVertAlign", (void*)&ScriptGUIElementStyle::Internal_settextVertAlign);
		metaData.scriptClass->AddInternalCall("Internal_getimagePosition", (void*)&ScriptGUIElementStyle::Internal_getimagePosition);
		metaData.scriptClass->AddInternalCall("Internal_setimagePosition", (void*)&ScriptGUIElementStyle::Internal_setimagePosition);
		metaData.scriptClass->AddInternalCall("Internal_getwordWrap", (void*)&ScriptGUIElementStyle::Internal_getwordWrap);
		metaData.scriptClass->AddInternalCall("Internal_setwordWrap", (void*)&ScriptGUIElementStyle::Internal_setwordWrap);
		metaData.scriptClass->AddInternalCall("Internal_getnormal", (void*)&ScriptGUIElementStyle::Internal_getnormal);
		metaData.scriptClass->AddInternalCall("Internal_setnormal", (void*)&ScriptGUIElementStyle::Internal_setnormal);
		metaData.scriptClass->AddInternalCall("Internal_gethover", (void*)&ScriptGUIElementStyle::Internal_gethover);
		metaData.scriptClass->AddInternalCall("Internal_sethover", (void*)&ScriptGUIElementStyle::Internal_sethover);
		metaData.scriptClass->AddInternalCall("Internal_getactive", (void*)&ScriptGUIElementStyle::Internal_getactive);
		metaData.scriptClass->AddInternalCall("Internal_setactive", (void*)&ScriptGUIElementStyle::Internal_setactive);
		metaData.scriptClass->AddInternalCall("Internal_getfocused", (void*)&ScriptGUIElementStyle::Internal_getfocused);
		metaData.scriptClass->AddInternalCall("Internal_setfocused", (void*)&ScriptGUIElementStyle::Internal_setfocused);
		metaData.scriptClass->AddInternalCall("Internal_getfocusedHover", (void*)&ScriptGUIElementStyle::Internal_getfocusedHover);
		metaData.scriptClass->AddInternalCall("Internal_setfocusedHover", (void*)&ScriptGUIElementStyle::Internal_setfocusedHover);
		metaData.scriptClass->AddInternalCall("Internal_getnormalOn", (void*)&ScriptGUIElementStyle::Internal_getnormalOn);
		metaData.scriptClass->AddInternalCall("Internal_setnormalOn", (void*)&ScriptGUIElementStyle::Internal_setnormalOn);
		metaData.scriptClass->AddInternalCall("Internal_gethoverOn", (void*)&ScriptGUIElementStyle::Internal_gethoverOn);
		metaData.scriptClass->AddInternalCall("Internal_sethoverOn", (void*)&ScriptGUIElementStyle::Internal_sethoverOn);
		metaData.scriptClass->AddInternalCall("Internal_getactiveOn", (void*)&ScriptGUIElementStyle::Internal_getactiveOn);
		metaData.scriptClass->AddInternalCall("Internal_setactiveOn", (void*)&ScriptGUIElementStyle::Internal_setactiveOn);
		metaData.scriptClass->AddInternalCall("Internal_getfocusedOn", (void*)&ScriptGUIElementStyle::Internal_getfocusedOn);
		metaData.scriptClass->AddInternalCall("Internal_setfocusedOn", (void*)&ScriptGUIElementStyle::Internal_setfocusedOn);
		metaData.scriptClass->AddInternalCall("Internal_getfocusedHoverOn", (void*)&ScriptGUIElementStyle::Internal_getfocusedHoverOn);
		metaData.scriptClass->AddInternalCall("Internal_setfocusedHoverOn", (void*)&ScriptGUIElementStyle::Internal_setfocusedHoverOn);
		metaData.scriptClass->AddInternalCall("Internal_getborder", (void*)&ScriptGUIElementStyle::Internal_getborder);
		metaData.scriptClass->AddInternalCall("Internal_setborder", (void*)&ScriptGUIElementStyle::Internal_setborder);
		metaData.scriptClass->AddInternalCall("Internal_getmargins", (void*)&ScriptGUIElementStyle::Internal_getmargins);
		metaData.scriptClass->AddInternalCall("Internal_setmargins", (void*)&ScriptGUIElementStyle::Internal_setmargins);
		metaData.scriptClass->AddInternalCall("Internal_getcontentOffset", (void*)&ScriptGUIElementStyle::Internal_getcontentOffset);
		metaData.scriptClass->AddInternalCall("Internal_setcontentOffset", (void*)&ScriptGUIElementStyle::Internal_setcontentOffset);
		metaData.scriptClass->AddInternalCall("Internal_getpadding", (void*)&ScriptGUIElementStyle::Internal_getpadding);
		metaData.scriptClass->AddInternalCall("Internal_setpadding", (void*)&ScriptGUIElementStyle::Internal_setpadding);
		metaData.scriptClass->AddInternalCall("Internal_getwidth", (void*)&ScriptGUIElementStyle::Internal_getwidth);
		metaData.scriptClass->AddInternalCall("Internal_setwidth", (void*)&ScriptGUIElementStyle::Internal_setwidth);
		metaData.scriptClass->AddInternalCall("Internal_getheight", (void*)&ScriptGUIElementStyle::Internal_getheight);
		metaData.scriptClass->AddInternalCall("Internal_setheight", (void*)&ScriptGUIElementStyle::Internal_setheight);
		metaData.scriptClass->AddInternalCall("Internal_getminWidth", (void*)&ScriptGUIElementStyle::Internal_getminWidth);
		metaData.scriptClass->AddInternalCall("Internal_setminWidth", (void*)&ScriptGUIElementStyle::Internal_setminWidth);
		metaData.scriptClass->AddInternalCall("Internal_getmaxWidth", (void*)&ScriptGUIElementStyle::Internal_getmaxWidth);
		metaData.scriptClass->AddInternalCall("Internal_setmaxWidth", (void*)&ScriptGUIElementStyle::Internal_setmaxWidth);
		metaData.scriptClass->AddInternalCall("Internal_getminHeight", (void*)&ScriptGUIElementStyle::Internal_getminHeight);
		metaData.scriptClass->AddInternalCall("Internal_setminHeight", (void*)&ScriptGUIElementStyle::Internal_setminHeight);
		metaData.scriptClass->AddInternalCall("Internal_getmaxHeight", (void*)&ScriptGUIElementStyle::Internal_getmaxHeight);
		metaData.scriptClass->AddInternalCall("Internal_setmaxHeight", (void*)&ScriptGUIElementStyle::Internal_setmaxHeight);
		metaData.scriptClass->AddInternalCall("Internal_getfixedWidth", (void*)&ScriptGUIElementStyle::Internal_getfixedWidth);
		metaData.scriptClass->AddInternalCall("Internal_setfixedWidth", (void*)&ScriptGUIElementStyle::Internal_setfixedWidth);
		metaData.scriptClass->AddInternalCall("Internal_getfixedHeight", (void*)&ScriptGUIElementStyle::Internal_getfixedHeight);
		metaData.scriptClass->AddInternalCall("Internal_setfixedHeight", (void*)&ScriptGUIElementStyle::Internal_setfixedHeight);

	}

	MonoObject* ScriptGUIElementStyle::create(const SPtr<GUIElementStyle>& value)
	{
		if(value == nullptr) return nullptr; 

		bool dummy = false;
		void* ctorParams[1] = { &dummy };

		MonoObject* managedInstance = metaData.scriptClass->CreateInstance("bool", ctorParams);
		new (bs_alloc<ScriptGUIElementStyle>()) ScriptGUIElementStyle(managedInstance, value);
		return managedInstance;
	}
	void ScriptGUIElementStyle::Internal_GUIElementStyle(MonoObject* managedInstance)
	{
		SPtr<GUIElementStyle> instance = bs_shared_ptr_new<GUIElementStyle>();
		new (bs_alloc<ScriptGUIElementStyle>())ScriptGUIElementStyle(managedInstance, instance);
	}

	void ScriptGUIElementStyle::Internal_addSubStyle(ScriptGUIElementStyle* thisPtr, MonoString* guiType, MonoString* styleName)
	{
		String tmpguiType;
		tmpguiType = MonoUtil::monoToString(guiType);
		String tmpstyleName;
		tmpstyleName = MonoUtil::monoToString(styleName);
		thisPtr->GetInternal()->addSubStyle(tmpguiType, tmpstyleName);
	}

	MonoObject* ScriptGUIElementStyle::Internal_getfont(ScriptGUIElementStyle* thisPtr)
	{
		ResourceHandle<Font> tmp__output;
		tmp__output = thisPtr->GetInternal()->font;

		MonoObject* __output;
		ScriptResourceBase* script__output;
		script__output = ScriptResourceManager::instance().GetScriptResource(tmp__output, true);
		if(script__output != nullptr)
			__output = script__output->GetManagedInstance();
		else
			__output = nullptr;

		return __output;
	}

	void ScriptGUIElementStyle::Internal_setfont(ScriptGUIElementStyle* thisPtr, MonoObject* value)
	{
		ResourceHandle<Font> tmpvalue;
		ScriptFont* scriptvalue;
		scriptvalue = ScriptFont::toNative(value);
		if(scriptvalue != nullptr)
			tmpvalue = scriptvalue->GetHandle();
		thisPtr->GetInternal()->font = tmpvalue;
	}

	uint32_t ScriptGUIElementStyle::Internal_getfontSize(ScriptGUIElementStyle* thisPtr)
	{
		uint32_t tmp__output;
		tmp__output = thisPtr->GetInternal()->fontSize;

		uint32_t __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptGUIElementStyle::Internal_setfontSize(ScriptGUIElementStyle* thisPtr, uint32_t value)
	{
		thisPtr->GetInternal()->fontSize = value;
	}

	TextHorzAlign ScriptGUIElementStyle::Internal_gettextHorzAlign(ScriptGUIElementStyle* thisPtr)
	{
		TextHorzAlign tmp__output;
		tmp__output = thisPtr->GetInternal()->textHorzAlign;

		TextHorzAlign __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptGUIElementStyle::Internal_settextHorzAlign(ScriptGUIElementStyle* thisPtr, TextHorzAlign value)
	{
		thisPtr->GetInternal()->textHorzAlign = value;
	}

	TextVertAlign ScriptGUIElementStyle::Internal_gettextVertAlign(ScriptGUIElementStyle* thisPtr)
	{
		TextVertAlign tmp__output;
		tmp__output = thisPtr->GetInternal()->textVertAlign;

		TextVertAlign __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptGUIElementStyle::Internal_settextVertAlign(ScriptGUIElementStyle* thisPtr, TextVertAlign value)
	{
		thisPtr->GetInternal()->textVertAlign = value;
	}

	GUIImagePosition ScriptGUIElementStyle::Internal_getimagePosition(ScriptGUIElementStyle* thisPtr)
	{
		GUIImagePosition tmp__output;
		tmp__output = thisPtr->GetInternal()->imagePosition;

		GUIImagePosition __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptGUIElementStyle::Internal_setimagePosition(ScriptGUIElementStyle* thisPtr, GUIImagePosition value)
	{
		thisPtr->GetInternal()->imagePosition = value;
	}

	bool ScriptGUIElementStyle::Internal_getwordWrap(ScriptGUIElementStyle* thisPtr)
	{
		bool tmp__output;
		tmp__output = thisPtr->GetInternal()->wordWrap;

		bool __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptGUIElementStyle::Internal_setwordWrap(ScriptGUIElementStyle* thisPtr, bool value)
	{
		thisPtr->GetInternal()->wordWrap = value;
	}

	void ScriptGUIElementStyle::Internal_getnormal(ScriptGUIElementStyle* thisPtr, __GUIElementStateStyleInterop* __output)
	{
		GUIElementStateStyle tmp__output;
		tmp__output = thisPtr->GetInternal()->normal;

		__GUIElementStateStyleInterop interop__output;
		interop__output = ScriptGUIElementStateStyle::toInterop(tmp__output);
		MonoUtil::valueCopy(__output, &interop__output, ScriptGUIElementStateStyle::getMetaData()->scriptClass->_getInternalClass());


	}

	void ScriptGUIElementStyle::Internal_setnormal(ScriptGUIElementStyle* thisPtr, __GUIElementStateStyleInterop* value)
	{
		GUIElementStateStyle tmpvalue;
		tmpvalue = ScriptGUIElementStateStyle::fromInterop(*value);
		thisPtr->GetInternal()->normal = tmpvalue;
	}

	void ScriptGUIElementStyle::Internal_gethover(ScriptGUIElementStyle* thisPtr, __GUIElementStateStyleInterop* __output)
	{
		GUIElementStateStyle tmp__output;
		tmp__output = thisPtr->GetInternal()->hover;

		__GUIElementStateStyleInterop interop__output;
		interop__output = ScriptGUIElementStateStyle::toInterop(tmp__output);
		MonoUtil::valueCopy(__output, &interop__output, ScriptGUIElementStateStyle::getMetaData()->scriptClass->_getInternalClass());


	}

	void ScriptGUIElementStyle::Internal_sethover(ScriptGUIElementStyle* thisPtr, __GUIElementStateStyleInterop* value)
	{
		GUIElementStateStyle tmpvalue;
		tmpvalue = ScriptGUIElementStateStyle::fromInterop(*value);
		thisPtr->GetInternal()->hover = tmpvalue;
	}

	void ScriptGUIElementStyle::Internal_getactive(ScriptGUIElementStyle* thisPtr, __GUIElementStateStyleInterop* __output)
	{
		GUIElementStateStyle tmp__output;
		tmp__output = thisPtr->GetInternal()->active;

		__GUIElementStateStyleInterop interop__output;
		interop__output = ScriptGUIElementStateStyle::toInterop(tmp__output);
		MonoUtil::valueCopy(__output, &interop__output, ScriptGUIElementStateStyle::getMetaData()->scriptClass->_getInternalClass());


	}

	void ScriptGUIElementStyle::Internal_setactive(ScriptGUIElementStyle* thisPtr, __GUIElementStateStyleInterop* value)
	{
		GUIElementStateStyle tmpvalue;
		tmpvalue = ScriptGUIElementStateStyle::fromInterop(*value);
		thisPtr->GetInternal()->active = tmpvalue;
	}

	void ScriptGUIElementStyle::Internal_getfocused(ScriptGUIElementStyle* thisPtr, __GUIElementStateStyleInterop* __output)
	{
		GUIElementStateStyle tmp__output;
		tmp__output = thisPtr->GetInternal()->focused;

		__GUIElementStateStyleInterop interop__output;
		interop__output = ScriptGUIElementStateStyle::toInterop(tmp__output);
		MonoUtil::valueCopy(__output, &interop__output, ScriptGUIElementStateStyle::getMetaData()->scriptClass->_getInternalClass());


	}

	void ScriptGUIElementStyle::Internal_setfocused(ScriptGUIElementStyle* thisPtr, __GUIElementStateStyleInterop* value)
	{
		GUIElementStateStyle tmpvalue;
		tmpvalue = ScriptGUIElementStateStyle::fromInterop(*value);
		thisPtr->GetInternal()->focused = tmpvalue;
	}

	void ScriptGUIElementStyle::Internal_getfocusedHover(ScriptGUIElementStyle* thisPtr, __GUIElementStateStyleInterop* __output)
	{
		GUIElementStateStyle tmp__output;
		tmp__output = thisPtr->GetInternal()->focusedHover;

		__GUIElementStateStyleInterop interop__output;
		interop__output = ScriptGUIElementStateStyle::toInterop(tmp__output);
		MonoUtil::valueCopy(__output, &interop__output, ScriptGUIElementStateStyle::getMetaData()->scriptClass->_getInternalClass());


	}

	void ScriptGUIElementStyle::Internal_setfocusedHover(ScriptGUIElementStyle* thisPtr, __GUIElementStateStyleInterop* value)
	{
		GUIElementStateStyle tmpvalue;
		tmpvalue = ScriptGUIElementStateStyle::fromInterop(*value);
		thisPtr->GetInternal()->focusedHover = tmpvalue;
	}

	void ScriptGUIElementStyle::Internal_getnormalOn(ScriptGUIElementStyle* thisPtr, __GUIElementStateStyleInterop* __output)
	{
		GUIElementStateStyle tmp__output;
		tmp__output = thisPtr->GetInternal()->normalOn;

		__GUIElementStateStyleInterop interop__output;
		interop__output = ScriptGUIElementStateStyle::toInterop(tmp__output);
		MonoUtil::valueCopy(__output, &interop__output, ScriptGUIElementStateStyle::getMetaData()->scriptClass->_getInternalClass());


	}

	void ScriptGUIElementStyle::Internal_setnormalOn(ScriptGUIElementStyle* thisPtr, __GUIElementStateStyleInterop* value)
	{
		GUIElementStateStyle tmpvalue;
		tmpvalue = ScriptGUIElementStateStyle::fromInterop(*value);
		thisPtr->GetInternal()->normalOn = tmpvalue;
	}

	void ScriptGUIElementStyle::Internal_gethoverOn(ScriptGUIElementStyle* thisPtr, __GUIElementStateStyleInterop* __output)
	{
		GUIElementStateStyle tmp__output;
		tmp__output = thisPtr->GetInternal()->hoverOn;

		__GUIElementStateStyleInterop interop__output;
		interop__output = ScriptGUIElementStateStyle::toInterop(tmp__output);
		MonoUtil::valueCopy(__output, &interop__output, ScriptGUIElementStateStyle::getMetaData()->scriptClass->_getInternalClass());


	}

	void ScriptGUIElementStyle::Internal_sethoverOn(ScriptGUIElementStyle* thisPtr, __GUIElementStateStyleInterop* value)
	{
		GUIElementStateStyle tmpvalue;
		tmpvalue = ScriptGUIElementStateStyle::fromInterop(*value);
		thisPtr->GetInternal()->hoverOn = tmpvalue;
	}

	void ScriptGUIElementStyle::Internal_getactiveOn(ScriptGUIElementStyle* thisPtr, __GUIElementStateStyleInterop* __output)
	{
		GUIElementStateStyle tmp__output;
		tmp__output = thisPtr->GetInternal()->activeOn;

		__GUIElementStateStyleInterop interop__output;
		interop__output = ScriptGUIElementStateStyle::toInterop(tmp__output);
		MonoUtil::valueCopy(__output, &interop__output, ScriptGUIElementStateStyle::getMetaData()->scriptClass->_getInternalClass());


	}

	void ScriptGUIElementStyle::Internal_setactiveOn(ScriptGUIElementStyle* thisPtr, __GUIElementStateStyleInterop* value)
	{
		GUIElementStateStyle tmpvalue;
		tmpvalue = ScriptGUIElementStateStyle::fromInterop(*value);
		thisPtr->GetInternal()->activeOn = tmpvalue;
	}

	void ScriptGUIElementStyle::Internal_getfocusedOn(ScriptGUIElementStyle* thisPtr, __GUIElementStateStyleInterop* __output)
	{
		GUIElementStateStyle tmp__output;
		tmp__output = thisPtr->GetInternal()->focusedOn;

		__GUIElementStateStyleInterop interop__output;
		interop__output = ScriptGUIElementStateStyle::toInterop(tmp__output);
		MonoUtil::valueCopy(__output, &interop__output, ScriptGUIElementStateStyle::getMetaData()->scriptClass->_getInternalClass());


	}

	void ScriptGUIElementStyle::Internal_setfocusedOn(ScriptGUIElementStyle* thisPtr, __GUIElementStateStyleInterop* value)
	{
		GUIElementStateStyle tmpvalue;
		tmpvalue = ScriptGUIElementStateStyle::fromInterop(*value);
		thisPtr->GetInternal()->focusedOn = tmpvalue;
	}

	void ScriptGUIElementStyle::Internal_getfocusedHoverOn(ScriptGUIElementStyle* thisPtr, __GUIElementStateStyleInterop* __output)
	{
		GUIElementStateStyle tmp__output;
		tmp__output = thisPtr->GetInternal()->focusedHoverOn;

		__GUIElementStateStyleInterop interop__output;
		interop__output = ScriptGUIElementStateStyle::toInterop(tmp__output);
		MonoUtil::valueCopy(__output, &interop__output, ScriptGUIElementStateStyle::getMetaData()->scriptClass->_getInternalClass());


	}

	void ScriptGUIElementStyle::Internal_setfocusedHoverOn(ScriptGUIElementStyle* thisPtr, __GUIElementStateStyleInterop* value)
	{
		GUIElementStateStyle tmpvalue;
		tmpvalue = ScriptGUIElementStateStyle::fromInterop(*value);
		thisPtr->GetInternal()->focusedHoverOn = tmpvalue;
	}

	void ScriptGUIElementStyle::Internal_getborder(ScriptGUIElementStyle* thisPtr, RectOffset* __output)
	{
		RectOffset tmp__output;
		tmp__output = thisPtr->GetInternal()->border;

		*__output = tmp__output;


	}

	void ScriptGUIElementStyle::Internal_setborder(ScriptGUIElementStyle* thisPtr, RectOffset* value)
	{
		thisPtr->GetInternal()->border = *value;
	}

	void ScriptGUIElementStyle::Internal_getmargins(ScriptGUIElementStyle* thisPtr, RectOffset* __output)
	{
		RectOffset tmp__output;
		tmp__output = thisPtr->GetInternal()->margins;

		*__output = tmp__output;


	}

	void ScriptGUIElementStyle::Internal_setmargins(ScriptGUIElementStyle* thisPtr, RectOffset* value)
	{
		thisPtr->GetInternal()->margins = *value;
	}

	void ScriptGUIElementStyle::Internal_getcontentOffset(ScriptGUIElementStyle* thisPtr, RectOffset* __output)
	{
		RectOffset tmp__output;
		tmp__output = thisPtr->GetInternal()->contentOffset;

		*__output = tmp__output;


	}

	void ScriptGUIElementStyle::Internal_setcontentOffset(ScriptGUIElementStyle* thisPtr, RectOffset* value)
	{
		thisPtr->GetInternal()->contentOffset = *value;
	}

	void ScriptGUIElementStyle::Internal_getpadding(ScriptGUIElementStyle* thisPtr, RectOffset* __output)
	{
		RectOffset tmp__output;
		tmp__output = thisPtr->GetInternal()->padding;

		*__output = tmp__output;


	}

	void ScriptGUIElementStyle::Internal_setpadding(ScriptGUIElementStyle* thisPtr, RectOffset* value)
	{
		thisPtr->GetInternal()->padding = *value;
	}

	uint32_t ScriptGUIElementStyle::Internal_getwidth(ScriptGUIElementStyle* thisPtr)
	{
		uint32_t tmp__output;
		tmp__output = thisPtr->GetInternal()->width;

		uint32_t __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptGUIElementStyle::Internal_setwidth(ScriptGUIElementStyle* thisPtr, uint32_t value)
	{
		thisPtr->GetInternal()->width = value;
	}

	uint32_t ScriptGUIElementStyle::Internal_getheight(ScriptGUIElementStyle* thisPtr)
	{
		uint32_t tmp__output;
		tmp__output = thisPtr->GetInternal()->height;

		uint32_t __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptGUIElementStyle::Internal_setheight(ScriptGUIElementStyle* thisPtr, uint32_t value)
	{
		thisPtr->GetInternal()->height = value;
	}

	uint32_t ScriptGUIElementStyle::Internal_getminWidth(ScriptGUIElementStyle* thisPtr)
	{
		uint32_t tmp__output;
		tmp__output = thisPtr->GetInternal()->minWidth;

		uint32_t __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptGUIElementStyle::Internal_setminWidth(ScriptGUIElementStyle* thisPtr, uint32_t value)
	{
		thisPtr->GetInternal()->minWidth = value;
	}

	uint32_t ScriptGUIElementStyle::Internal_getmaxWidth(ScriptGUIElementStyle* thisPtr)
	{
		uint32_t tmp__output;
		tmp__output = thisPtr->GetInternal()->maxWidth;

		uint32_t __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptGUIElementStyle::Internal_setmaxWidth(ScriptGUIElementStyle* thisPtr, uint32_t value)
	{
		thisPtr->GetInternal()->maxWidth = value;
	}

	uint32_t ScriptGUIElementStyle::Internal_getminHeight(ScriptGUIElementStyle* thisPtr)
	{
		uint32_t tmp__output;
		tmp__output = thisPtr->GetInternal()->minHeight;

		uint32_t __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptGUIElementStyle::Internal_setminHeight(ScriptGUIElementStyle* thisPtr, uint32_t value)
	{
		thisPtr->GetInternal()->minHeight = value;
	}

	uint32_t ScriptGUIElementStyle::Internal_getmaxHeight(ScriptGUIElementStyle* thisPtr)
	{
		uint32_t tmp__output;
		tmp__output = thisPtr->GetInternal()->maxHeight;

		uint32_t __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptGUIElementStyle::Internal_setmaxHeight(ScriptGUIElementStyle* thisPtr, uint32_t value)
	{
		thisPtr->GetInternal()->maxHeight = value;
	}

	bool ScriptGUIElementStyle::Internal_getfixedWidth(ScriptGUIElementStyle* thisPtr)
	{
		bool tmp__output;
		tmp__output = thisPtr->GetInternal()->fixedWidth;

		bool __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptGUIElementStyle::Internal_setfixedWidth(ScriptGUIElementStyle* thisPtr, bool value)
	{
		thisPtr->GetInternal()->fixedWidth = value;
	}

	bool ScriptGUIElementStyle::Internal_getfixedHeight(ScriptGUIElementStyle* thisPtr)
	{
		bool tmp__output;
		tmp__output = thisPtr->GetInternal()->fixedHeight;

		bool __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptGUIElementStyle::Internal_setfixedHeight(ScriptGUIElementStyle* thisPtr, bool value)
	{
		thisPtr->GetInternal()->fixedHeight = value;
	}
}
