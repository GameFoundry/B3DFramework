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

	void ScriptGUIElementStyle::initRuntimeData()
	{
		metaData.scriptClass->addInternalCall("Internal_GUIElementStyle", (void*)&ScriptGUIElementStyle::Internal_GUIElementStyle);
		metaData.scriptClass->addInternalCall("Internal_addSubStyle", (void*)&ScriptGUIElementStyle::Internal_addSubStyle);
		metaData.scriptClass->addInternalCall("Internal_getfont", (void*)&ScriptGUIElementStyle::Internal_getfont);
		metaData.scriptClass->addInternalCall("Internal_setfont", (void*)&ScriptGUIElementStyle::Internal_setfont);
		metaData.scriptClass->addInternalCall("Internal_getfontSize", (void*)&ScriptGUIElementStyle::Internal_getfontSize);
		metaData.scriptClass->addInternalCall("Internal_setfontSize", (void*)&ScriptGUIElementStyle::Internal_setfontSize);
		metaData.scriptClass->addInternalCall("Internal_gettextHorzAlign", (void*)&ScriptGUIElementStyle::Internal_gettextHorzAlign);
		metaData.scriptClass->addInternalCall("Internal_settextHorzAlign", (void*)&ScriptGUIElementStyle::Internal_settextHorzAlign);
		metaData.scriptClass->addInternalCall("Internal_gettextVertAlign", (void*)&ScriptGUIElementStyle::Internal_gettextVertAlign);
		metaData.scriptClass->addInternalCall("Internal_settextVertAlign", (void*)&ScriptGUIElementStyle::Internal_settextVertAlign);
		metaData.scriptClass->addInternalCall("Internal_getimagePosition", (void*)&ScriptGUIElementStyle::Internal_getimagePosition);
		metaData.scriptClass->addInternalCall("Internal_setimagePosition", (void*)&ScriptGUIElementStyle::Internal_setimagePosition);
		metaData.scriptClass->addInternalCall("Internal_getwordWrap", (void*)&ScriptGUIElementStyle::Internal_getwordWrap);
		metaData.scriptClass->addInternalCall("Internal_setwordWrap", (void*)&ScriptGUIElementStyle::Internal_setwordWrap);
		metaData.scriptClass->addInternalCall("Internal_getnormal", (void*)&ScriptGUIElementStyle::Internal_getnormal);
		metaData.scriptClass->addInternalCall("Internal_setnormal", (void*)&ScriptGUIElementStyle::Internal_setnormal);
		metaData.scriptClass->addInternalCall("Internal_gethover", (void*)&ScriptGUIElementStyle::Internal_gethover);
		metaData.scriptClass->addInternalCall("Internal_sethover", (void*)&ScriptGUIElementStyle::Internal_sethover);
		metaData.scriptClass->addInternalCall("Internal_getactive", (void*)&ScriptGUIElementStyle::Internal_getactive);
		metaData.scriptClass->addInternalCall("Internal_setactive", (void*)&ScriptGUIElementStyle::Internal_setactive);
		metaData.scriptClass->addInternalCall("Internal_getfocused", (void*)&ScriptGUIElementStyle::Internal_getfocused);
		metaData.scriptClass->addInternalCall("Internal_setfocused", (void*)&ScriptGUIElementStyle::Internal_setfocused);
		metaData.scriptClass->addInternalCall("Internal_getfocusedHover", (void*)&ScriptGUIElementStyle::Internal_getfocusedHover);
		metaData.scriptClass->addInternalCall("Internal_setfocusedHover", (void*)&ScriptGUIElementStyle::Internal_setfocusedHover);
		metaData.scriptClass->addInternalCall("Internal_getnormalOn", (void*)&ScriptGUIElementStyle::Internal_getnormalOn);
		metaData.scriptClass->addInternalCall("Internal_setnormalOn", (void*)&ScriptGUIElementStyle::Internal_setnormalOn);
		metaData.scriptClass->addInternalCall("Internal_gethoverOn", (void*)&ScriptGUIElementStyle::Internal_gethoverOn);
		metaData.scriptClass->addInternalCall("Internal_sethoverOn", (void*)&ScriptGUIElementStyle::Internal_sethoverOn);
		metaData.scriptClass->addInternalCall("Internal_getactiveOn", (void*)&ScriptGUIElementStyle::Internal_getactiveOn);
		metaData.scriptClass->addInternalCall("Internal_setactiveOn", (void*)&ScriptGUIElementStyle::Internal_setactiveOn);
		metaData.scriptClass->addInternalCall("Internal_getfocusedOn", (void*)&ScriptGUIElementStyle::Internal_getfocusedOn);
		metaData.scriptClass->addInternalCall("Internal_setfocusedOn", (void*)&ScriptGUIElementStyle::Internal_setfocusedOn);
		metaData.scriptClass->addInternalCall("Internal_getfocusedHoverOn", (void*)&ScriptGUIElementStyle::Internal_getfocusedHoverOn);
		metaData.scriptClass->addInternalCall("Internal_setfocusedHoverOn", (void*)&ScriptGUIElementStyle::Internal_setfocusedHoverOn);
		metaData.scriptClass->addInternalCall("Internal_getborder", (void*)&ScriptGUIElementStyle::Internal_getborder);
		metaData.scriptClass->addInternalCall("Internal_setborder", (void*)&ScriptGUIElementStyle::Internal_setborder);
		metaData.scriptClass->addInternalCall("Internal_getmargins", (void*)&ScriptGUIElementStyle::Internal_getmargins);
		metaData.scriptClass->addInternalCall("Internal_setmargins", (void*)&ScriptGUIElementStyle::Internal_setmargins);
		metaData.scriptClass->addInternalCall("Internal_getcontentOffset", (void*)&ScriptGUIElementStyle::Internal_getcontentOffset);
		metaData.scriptClass->addInternalCall("Internal_setcontentOffset", (void*)&ScriptGUIElementStyle::Internal_setcontentOffset);
		metaData.scriptClass->addInternalCall("Internal_getpadding", (void*)&ScriptGUIElementStyle::Internal_getpadding);
		metaData.scriptClass->addInternalCall("Internal_setpadding", (void*)&ScriptGUIElementStyle::Internal_setpadding);
		metaData.scriptClass->addInternalCall("Internal_getwidth", (void*)&ScriptGUIElementStyle::Internal_getwidth);
		metaData.scriptClass->addInternalCall("Internal_setwidth", (void*)&ScriptGUIElementStyle::Internal_setwidth);
		metaData.scriptClass->addInternalCall("Internal_getheight", (void*)&ScriptGUIElementStyle::Internal_getheight);
		metaData.scriptClass->addInternalCall("Internal_setheight", (void*)&ScriptGUIElementStyle::Internal_setheight);
		metaData.scriptClass->addInternalCall("Internal_getminWidth", (void*)&ScriptGUIElementStyle::Internal_getminWidth);
		metaData.scriptClass->addInternalCall("Internal_setminWidth", (void*)&ScriptGUIElementStyle::Internal_setminWidth);
		metaData.scriptClass->addInternalCall("Internal_getmaxWidth", (void*)&ScriptGUIElementStyle::Internal_getmaxWidth);
		metaData.scriptClass->addInternalCall("Internal_setmaxWidth", (void*)&ScriptGUIElementStyle::Internal_setmaxWidth);
		metaData.scriptClass->addInternalCall("Internal_getminHeight", (void*)&ScriptGUIElementStyle::Internal_getminHeight);
		metaData.scriptClass->addInternalCall("Internal_setminHeight", (void*)&ScriptGUIElementStyle::Internal_setminHeight);
		metaData.scriptClass->addInternalCall("Internal_getmaxHeight", (void*)&ScriptGUIElementStyle::Internal_getmaxHeight);
		metaData.scriptClass->addInternalCall("Internal_setmaxHeight", (void*)&ScriptGUIElementStyle::Internal_setmaxHeight);
		metaData.scriptClass->addInternalCall("Internal_getfixedWidth", (void*)&ScriptGUIElementStyle::Internal_getfixedWidth);
		metaData.scriptClass->addInternalCall("Internal_setfixedWidth", (void*)&ScriptGUIElementStyle::Internal_setfixedWidth);
		metaData.scriptClass->addInternalCall("Internal_getfixedHeight", (void*)&ScriptGUIElementStyle::Internal_getfixedHeight);
		metaData.scriptClass->addInternalCall("Internal_setfixedHeight", (void*)&ScriptGUIElementStyle::Internal_setfixedHeight);

	}

	MonoObject* ScriptGUIElementStyle::create(const SPtr<GUIElementStyle>& value)
	{
		if(value == nullptr) return nullptr; 

		bool dummy = false;
		void* ctorParams[1] = { &dummy };

		MonoObject* managedInstance = metaData.scriptClass->createInstance("bool", ctorParams);
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
		thisPtr->getInternal()->addSubStyle(tmpguiType, tmpstyleName);
	}

	MonoObject* ScriptGUIElementStyle::Internal_getfont(ScriptGUIElementStyle* thisPtr)
	{
		ResourceHandle<Font> tmp__output;
		tmp__output = thisPtr->getInternal()->font;

		MonoObject* __output;
		ScriptResourceBase* script__output;
		script__output = ScriptResourceManager::instance().getScriptResource(tmp__output, true);
		if(script__output != nullptr)
			__output = script__output->getManagedInstance();
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
			tmpvalue = scriptvalue->getHandle();
		thisPtr->getInternal()->font = tmpvalue;
	}

	uint32_t ScriptGUIElementStyle::Internal_getfontSize(ScriptGUIElementStyle* thisPtr)
	{
		uint32_t tmp__output;
		tmp__output = thisPtr->getInternal()->fontSize;

		uint32_t __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptGUIElementStyle::Internal_setfontSize(ScriptGUIElementStyle* thisPtr, uint32_t value)
	{
		thisPtr->getInternal()->fontSize = value;
	}

	TextHorzAlign ScriptGUIElementStyle::Internal_gettextHorzAlign(ScriptGUIElementStyle* thisPtr)
	{
		TextHorzAlign tmp__output;
		tmp__output = thisPtr->getInternal()->textHorzAlign;

		TextHorzAlign __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptGUIElementStyle::Internal_settextHorzAlign(ScriptGUIElementStyle* thisPtr, TextHorzAlign value)
	{
		thisPtr->getInternal()->textHorzAlign = value;
	}

	TextVertAlign ScriptGUIElementStyle::Internal_gettextVertAlign(ScriptGUIElementStyle* thisPtr)
	{
		TextVertAlign tmp__output;
		tmp__output = thisPtr->getInternal()->textVertAlign;

		TextVertAlign __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptGUIElementStyle::Internal_settextVertAlign(ScriptGUIElementStyle* thisPtr, TextVertAlign value)
	{
		thisPtr->getInternal()->textVertAlign = value;
	}

	GUIImagePosition ScriptGUIElementStyle::Internal_getimagePosition(ScriptGUIElementStyle* thisPtr)
	{
		GUIImagePosition tmp__output;
		tmp__output = thisPtr->getInternal()->imagePosition;

		GUIImagePosition __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptGUIElementStyle::Internal_setimagePosition(ScriptGUIElementStyle* thisPtr, GUIImagePosition value)
	{
		thisPtr->getInternal()->imagePosition = value;
	}

	bool ScriptGUIElementStyle::Internal_getwordWrap(ScriptGUIElementStyle* thisPtr)
	{
		bool tmp__output;
		tmp__output = thisPtr->getInternal()->wordWrap;

		bool __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptGUIElementStyle::Internal_setwordWrap(ScriptGUIElementStyle* thisPtr, bool value)
	{
		thisPtr->getInternal()->wordWrap = value;
	}

	void ScriptGUIElementStyle::Internal_getnormal(ScriptGUIElementStyle* thisPtr, __GUIElementStateStyleInterop* __output)
	{
		GUIElementStateStyle tmp__output;
		tmp__output = thisPtr->getInternal()->normal;

		__GUIElementStateStyleInterop interop__output;
		interop__output = ScriptGUIElementStateStyle::toInterop(tmp__output);
		MonoUtil::valueCopy(__output, &interop__output, ScriptGUIElementStateStyle::getMetaData()->scriptClass->_getInternalClass());


	}

	void ScriptGUIElementStyle::Internal_setnormal(ScriptGUIElementStyle* thisPtr, __GUIElementStateStyleInterop* value)
	{
		GUIElementStateStyle tmpvalue;
		tmpvalue = ScriptGUIElementStateStyle::fromInterop(*value);
		thisPtr->getInternal()->normal = tmpvalue;
	}

	void ScriptGUIElementStyle::Internal_gethover(ScriptGUIElementStyle* thisPtr, __GUIElementStateStyleInterop* __output)
	{
		GUIElementStateStyle tmp__output;
		tmp__output = thisPtr->getInternal()->hover;

		__GUIElementStateStyleInterop interop__output;
		interop__output = ScriptGUIElementStateStyle::toInterop(tmp__output);
		MonoUtil::valueCopy(__output, &interop__output, ScriptGUIElementStateStyle::getMetaData()->scriptClass->_getInternalClass());


	}

	void ScriptGUIElementStyle::Internal_sethover(ScriptGUIElementStyle* thisPtr, __GUIElementStateStyleInterop* value)
	{
		GUIElementStateStyle tmpvalue;
		tmpvalue = ScriptGUIElementStateStyle::fromInterop(*value);
		thisPtr->getInternal()->hover = tmpvalue;
	}

	void ScriptGUIElementStyle::Internal_getactive(ScriptGUIElementStyle* thisPtr, __GUIElementStateStyleInterop* __output)
	{
		GUIElementStateStyle tmp__output;
		tmp__output = thisPtr->getInternal()->active;

		__GUIElementStateStyleInterop interop__output;
		interop__output = ScriptGUIElementStateStyle::toInterop(tmp__output);
		MonoUtil::valueCopy(__output, &interop__output, ScriptGUIElementStateStyle::getMetaData()->scriptClass->_getInternalClass());


	}

	void ScriptGUIElementStyle::Internal_setactive(ScriptGUIElementStyle* thisPtr, __GUIElementStateStyleInterop* value)
	{
		GUIElementStateStyle tmpvalue;
		tmpvalue = ScriptGUIElementStateStyle::fromInterop(*value);
		thisPtr->getInternal()->active = tmpvalue;
	}

	void ScriptGUIElementStyle::Internal_getfocused(ScriptGUIElementStyle* thisPtr, __GUIElementStateStyleInterop* __output)
	{
		GUIElementStateStyle tmp__output;
		tmp__output = thisPtr->getInternal()->focused;

		__GUIElementStateStyleInterop interop__output;
		interop__output = ScriptGUIElementStateStyle::toInterop(tmp__output);
		MonoUtil::valueCopy(__output, &interop__output, ScriptGUIElementStateStyle::getMetaData()->scriptClass->_getInternalClass());


	}

	void ScriptGUIElementStyle::Internal_setfocused(ScriptGUIElementStyle* thisPtr, __GUIElementStateStyleInterop* value)
	{
		GUIElementStateStyle tmpvalue;
		tmpvalue = ScriptGUIElementStateStyle::fromInterop(*value);
		thisPtr->getInternal()->focused = tmpvalue;
	}

	void ScriptGUIElementStyle::Internal_getfocusedHover(ScriptGUIElementStyle* thisPtr, __GUIElementStateStyleInterop* __output)
	{
		GUIElementStateStyle tmp__output;
		tmp__output = thisPtr->getInternal()->focusedHover;

		__GUIElementStateStyleInterop interop__output;
		interop__output = ScriptGUIElementStateStyle::toInterop(tmp__output);
		MonoUtil::valueCopy(__output, &interop__output, ScriptGUIElementStateStyle::getMetaData()->scriptClass->_getInternalClass());


	}

	void ScriptGUIElementStyle::Internal_setfocusedHover(ScriptGUIElementStyle* thisPtr, __GUIElementStateStyleInterop* value)
	{
		GUIElementStateStyle tmpvalue;
		tmpvalue = ScriptGUIElementStateStyle::fromInterop(*value);
		thisPtr->getInternal()->focusedHover = tmpvalue;
	}

	void ScriptGUIElementStyle::Internal_getnormalOn(ScriptGUIElementStyle* thisPtr, __GUIElementStateStyleInterop* __output)
	{
		GUIElementStateStyle tmp__output;
		tmp__output = thisPtr->getInternal()->normalOn;

		__GUIElementStateStyleInterop interop__output;
		interop__output = ScriptGUIElementStateStyle::toInterop(tmp__output);
		MonoUtil::valueCopy(__output, &interop__output, ScriptGUIElementStateStyle::getMetaData()->scriptClass->_getInternalClass());


	}

	void ScriptGUIElementStyle::Internal_setnormalOn(ScriptGUIElementStyle* thisPtr, __GUIElementStateStyleInterop* value)
	{
		GUIElementStateStyle tmpvalue;
		tmpvalue = ScriptGUIElementStateStyle::fromInterop(*value);
		thisPtr->getInternal()->normalOn = tmpvalue;
	}

	void ScriptGUIElementStyle::Internal_gethoverOn(ScriptGUIElementStyle* thisPtr, __GUIElementStateStyleInterop* __output)
	{
		GUIElementStateStyle tmp__output;
		tmp__output = thisPtr->getInternal()->hoverOn;

		__GUIElementStateStyleInterop interop__output;
		interop__output = ScriptGUIElementStateStyle::toInterop(tmp__output);
		MonoUtil::valueCopy(__output, &interop__output, ScriptGUIElementStateStyle::getMetaData()->scriptClass->_getInternalClass());


	}

	void ScriptGUIElementStyle::Internal_sethoverOn(ScriptGUIElementStyle* thisPtr, __GUIElementStateStyleInterop* value)
	{
		GUIElementStateStyle tmpvalue;
		tmpvalue = ScriptGUIElementStateStyle::fromInterop(*value);
		thisPtr->getInternal()->hoverOn = tmpvalue;
	}

	void ScriptGUIElementStyle::Internal_getactiveOn(ScriptGUIElementStyle* thisPtr, __GUIElementStateStyleInterop* __output)
	{
		GUIElementStateStyle tmp__output;
		tmp__output = thisPtr->getInternal()->activeOn;

		__GUIElementStateStyleInterop interop__output;
		interop__output = ScriptGUIElementStateStyle::toInterop(tmp__output);
		MonoUtil::valueCopy(__output, &interop__output, ScriptGUIElementStateStyle::getMetaData()->scriptClass->_getInternalClass());


	}

	void ScriptGUIElementStyle::Internal_setactiveOn(ScriptGUIElementStyle* thisPtr, __GUIElementStateStyleInterop* value)
	{
		GUIElementStateStyle tmpvalue;
		tmpvalue = ScriptGUIElementStateStyle::fromInterop(*value);
		thisPtr->getInternal()->activeOn = tmpvalue;
	}

	void ScriptGUIElementStyle::Internal_getfocusedOn(ScriptGUIElementStyle* thisPtr, __GUIElementStateStyleInterop* __output)
	{
		GUIElementStateStyle tmp__output;
		tmp__output = thisPtr->getInternal()->focusedOn;

		__GUIElementStateStyleInterop interop__output;
		interop__output = ScriptGUIElementStateStyle::toInterop(tmp__output);
		MonoUtil::valueCopy(__output, &interop__output, ScriptGUIElementStateStyle::getMetaData()->scriptClass->_getInternalClass());


	}

	void ScriptGUIElementStyle::Internal_setfocusedOn(ScriptGUIElementStyle* thisPtr, __GUIElementStateStyleInterop* value)
	{
		GUIElementStateStyle tmpvalue;
		tmpvalue = ScriptGUIElementStateStyle::fromInterop(*value);
		thisPtr->getInternal()->focusedOn = tmpvalue;
	}

	void ScriptGUIElementStyle::Internal_getfocusedHoverOn(ScriptGUIElementStyle* thisPtr, __GUIElementStateStyleInterop* __output)
	{
		GUIElementStateStyle tmp__output;
		tmp__output = thisPtr->getInternal()->focusedHoverOn;

		__GUIElementStateStyleInterop interop__output;
		interop__output = ScriptGUIElementStateStyle::toInterop(tmp__output);
		MonoUtil::valueCopy(__output, &interop__output, ScriptGUIElementStateStyle::getMetaData()->scriptClass->_getInternalClass());


	}

	void ScriptGUIElementStyle::Internal_setfocusedHoverOn(ScriptGUIElementStyle* thisPtr, __GUIElementStateStyleInterop* value)
	{
		GUIElementStateStyle tmpvalue;
		tmpvalue = ScriptGUIElementStateStyle::fromInterop(*value);
		thisPtr->getInternal()->focusedHoverOn = tmpvalue;
	}

	void ScriptGUIElementStyle::Internal_getborder(ScriptGUIElementStyle* thisPtr, RectOffset* __output)
	{
		RectOffset tmp__output;
		tmp__output = thisPtr->getInternal()->border;

		*__output = tmp__output;


	}

	void ScriptGUIElementStyle::Internal_setborder(ScriptGUIElementStyle* thisPtr, RectOffset* value)
	{
		thisPtr->getInternal()->border = *value;
	}

	void ScriptGUIElementStyle::Internal_getmargins(ScriptGUIElementStyle* thisPtr, RectOffset* __output)
	{
		RectOffset tmp__output;
		tmp__output = thisPtr->getInternal()->margins;

		*__output = tmp__output;


	}

	void ScriptGUIElementStyle::Internal_setmargins(ScriptGUIElementStyle* thisPtr, RectOffset* value)
	{
		thisPtr->getInternal()->margins = *value;
	}

	void ScriptGUIElementStyle::Internal_getcontentOffset(ScriptGUIElementStyle* thisPtr, RectOffset* __output)
	{
		RectOffset tmp__output;
		tmp__output = thisPtr->getInternal()->contentOffset;

		*__output = tmp__output;


	}

	void ScriptGUIElementStyle::Internal_setcontentOffset(ScriptGUIElementStyle* thisPtr, RectOffset* value)
	{
		thisPtr->getInternal()->contentOffset = *value;
	}

	void ScriptGUIElementStyle::Internal_getpadding(ScriptGUIElementStyle* thisPtr, RectOffset* __output)
	{
		RectOffset tmp__output;
		tmp__output = thisPtr->getInternal()->padding;

		*__output = tmp__output;


	}

	void ScriptGUIElementStyle::Internal_setpadding(ScriptGUIElementStyle* thisPtr, RectOffset* value)
	{
		thisPtr->getInternal()->padding = *value;
	}

	uint32_t ScriptGUIElementStyle::Internal_getwidth(ScriptGUIElementStyle* thisPtr)
	{
		uint32_t tmp__output;
		tmp__output = thisPtr->getInternal()->width;

		uint32_t __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptGUIElementStyle::Internal_setwidth(ScriptGUIElementStyle* thisPtr, uint32_t value)
	{
		thisPtr->getInternal()->width = value;
	}

	uint32_t ScriptGUIElementStyle::Internal_getheight(ScriptGUIElementStyle* thisPtr)
	{
		uint32_t tmp__output;
		tmp__output = thisPtr->getInternal()->height;

		uint32_t __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptGUIElementStyle::Internal_setheight(ScriptGUIElementStyle* thisPtr, uint32_t value)
	{
		thisPtr->getInternal()->height = value;
	}

	uint32_t ScriptGUIElementStyle::Internal_getminWidth(ScriptGUIElementStyle* thisPtr)
	{
		uint32_t tmp__output;
		tmp__output = thisPtr->getInternal()->minWidth;

		uint32_t __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptGUIElementStyle::Internal_setminWidth(ScriptGUIElementStyle* thisPtr, uint32_t value)
	{
		thisPtr->getInternal()->minWidth = value;
	}

	uint32_t ScriptGUIElementStyle::Internal_getmaxWidth(ScriptGUIElementStyle* thisPtr)
	{
		uint32_t tmp__output;
		tmp__output = thisPtr->getInternal()->maxWidth;

		uint32_t __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptGUIElementStyle::Internal_setmaxWidth(ScriptGUIElementStyle* thisPtr, uint32_t value)
	{
		thisPtr->getInternal()->maxWidth = value;
	}

	uint32_t ScriptGUIElementStyle::Internal_getminHeight(ScriptGUIElementStyle* thisPtr)
	{
		uint32_t tmp__output;
		tmp__output = thisPtr->getInternal()->minHeight;

		uint32_t __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptGUIElementStyle::Internal_setminHeight(ScriptGUIElementStyle* thisPtr, uint32_t value)
	{
		thisPtr->getInternal()->minHeight = value;
	}

	uint32_t ScriptGUIElementStyle::Internal_getmaxHeight(ScriptGUIElementStyle* thisPtr)
	{
		uint32_t tmp__output;
		tmp__output = thisPtr->getInternal()->maxHeight;

		uint32_t __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptGUIElementStyle::Internal_setmaxHeight(ScriptGUIElementStyle* thisPtr, uint32_t value)
	{
		thisPtr->getInternal()->maxHeight = value;
	}

	bool ScriptGUIElementStyle::Internal_getfixedWidth(ScriptGUIElementStyle* thisPtr)
	{
		bool tmp__output;
		tmp__output = thisPtr->getInternal()->fixedWidth;

		bool __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptGUIElementStyle::Internal_setfixedWidth(ScriptGUIElementStyle* thisPtr, bool value)
	{
		thisPtr->getInternal()->fixedWidth = value;
	}

	bool ScriptGUIElementStyle::Internal_getfixedHeight(ScriptGUIElementStyle* thisPtr)
	{
		bool tmp__output;
		tmp__output = thisPtr->getInternal()->fixedHeight;

		bool __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptGUIElementStyle::Internal_setfixedHeight(ScriptGUIElementStyle* thisPtr, bool value)
	{
		thisPtr->getInternal()->fixedHeight = value;
	}
}
