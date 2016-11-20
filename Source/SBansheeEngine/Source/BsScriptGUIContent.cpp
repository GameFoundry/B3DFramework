//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsScriptGUIContent.h"
#include "BsScriptMeta.h"
#include "BsMonoField.h"
#include "BsMonoClass.h"
#include "BsMonoManager.h"
#include "BsMonoUtil.h"
#include "BsScriptHString.h"
#include "BsScriptSpriteTexture.h"
#include "BsSpriteTexture.h"
#include "BsScriptGUIContentImages.h"

namespace bs
{
	MonoField* ScriptGUIContent::mTextField;
	MonoField* ScriptGUIContent::mTooltipField;
	MonoField* ScriptGUIContent::mImagesField;

	ScriptGUIContent::ScriptGUIContent(MonoObject* instance)
		:ScriptObject(instance)
	{ }

	void ScriptGUIContent::initRuntimeData()
	{
		mTextField = metaData.scriptClass->getField("text");
		mTooltipField = metaData.scriptClass->getField("tooltip");
		mImagesField = metaData.scriptClass->getField("images");
	}

	const HString& ScriptGUIContent::getText(MonoObject* instance)
	{
		MonoObject* textManaged = nullptr;
		mTextField->getValue(instance, &textManaged);

		if(textManaged == nullptr)
			return HString::dummy();

		ScriptHString* textScript = ScriptHString::toNative(textManaged);
		return textScript->getInternalValue();
	}

	const HString& ScriptGUIContent::getTooltip(MonoObject* instance)
	{
		MonoObject* tooltipManaged = nullptr;
		mTooltipField->getValue(instance, &tooltipManaged);

		if(tooltipManaged == nullptr)
			return HString::dummy();

		ScriptHString* tooltipScript = ScriptHString::toNative(tooltipManaged);
		return tooltipScript->getInternalValue();
	}

	GUIContentImages ScriptGUIContent::getImage(MonoObject* instance)
	{
		MonoObject* imagesManaged = nullptr;
		mImagesField->getValue(instance, &imagesManaged);

		if(imagesManaged == nullptr)
			return GUIContentImages();

		return ScriptGUIContentImages::getNative(imagesManaged);
	}
}