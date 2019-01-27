//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptGUIContent.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "../../../../../Foundation/bsfCore/Localization/BsHString.h"
#include "BsScriptHString.generated.h"
#include "../../../../../Foundation/bsfEngine/GUI/BsGUIContent.h"
#include "BsScriptGUIContentImages.generated.h"

namespace bs
{
	ScriptGUIContent::ScriptGUIContent(MonoObject* managedInstance)
		:ScriptObject(managedInstance)
	{ }

	void ScriptGUIContent::initRuntimeData()
	{ }

	MonoObject*ScriptGUIContent::box(const __GUIContentInterop& value)
	{
		return MonoUtil::box(metaData.scriptClass->_getInternalClass(), (void*)&value);
	}

	__GUIContentInterop ScriptGUIContent::unbox(MonoObject* value)
	{
		return *(__GUIContentInterop*)MonoUtil::unbox(value);
	}

	GUIContent ScriptGUIContent::fromInterop(const __GUIContentInterop& value)
	{
		GUIContent output;
		SPtr<HString> tmptext;
		ScriptHString* scripttext;
		scripttext = ScriptHString::toNative(value.text);
		if(scripttext != nullptr)
			tmptext = scripttext->getInternal();
		if(tmptext != nullptr)
		output.text = *tmptext;
		GUIContentImages tmpimages;
		tmpimages = ScriptGUIContentImages::fromInterop(value.images);
		output.images = tmpimages;
		SPtr<HString> tmptooltip;
		ScriptHString* scripttooltip;
		scripttooltip = ScriptHString::toNative(value.tooltip);
		if(scripttooltip != nullptr)
			tmptooltip = scripttooltip->getInternal();
		if(tmptooltip != nullptr)
		output.tooltip = *tmptooltip;

		return output;
	}

	__GUIContentInterop ScriptGUIContent::toInterop(const GUIContent& value)
	{
		__GUIContentInterop output;
		MonoObject* tmptext;
		SPtr<HString> tmptextcopy;
		tmptextcopy = bs_shared_ptr_new<HString>(value.text);
		tmptext = ScriptHString::create(tmptextcopy);
		output.text = tmptext;
		__GUIContentImagesInterop tmpimages;
		tmpimages = ScriptGUIContentImages::toInterop(value.images);
		output.images = tmpimages;
		MonoObject* tmptooltip;
		SPtr<HString> tmptooltipcopy;
		tmptooltipcopy = bs_shared_ptr_new<HString>(value.tooltip);
		tmptooltip = ScriptHString::create(tmptooltipcopy);
		output.tooltip = tmptooltip;

		return output;
	}

}
