//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsScriptGUITexture.h"
#include "BsScriptMeta.h"
#include "BsMonoField.h"
#include "BsMonoClass.h"
#include "BsMonoManager.h"
#include "BsSpriteTexture.h"
#include "BsMonoUtil.h"
#include "BsGUILayout.h"
#include "BsGUITexture.h"
#include "BsGUIOptions.h"
#include "BsScriptSpriteTexture.h"
#include "BsScriptGUIElementStyle.h"
#include "BsScriptGUILayout.h"
#include "BsScriptHString.h"
#include "BsScriptGUIContent.h"

namespace BansheeEngine
{
	ScriptGUITexture::ScriptGUITexture(MonoObject* instance, GUITexture* texture)
		:TScriptGUIElement(instance, texture)
	{

	}

	void ScriptGUITexture::initRuntimeData()
	{
		metaData.scriptClass->addInternalCall("Internal_CreateInstance", &ScriptGUITexture::internal_createInstance);
		metaData.scriptClass->addInternalCall("Internal_SetTexture", &ScriptGUITexture::internal_setTexture);
		metaData.scriptClass->addInternalCall("Internal_SetTint", &ScriptGUITexture::internal_setTint);
	}

	void ScriptGUITexture::internal_createInstance(MonoObject* instance, MonoObject* texture, 
		TextureScaleMode scale, bool transparent, MonoString* style, MonoArray* guiOptions)
	{
		GUIOptions options;

		ScriptArray scriptArray(guiOptions);
		UINT32 arrayLen = scriptArray.size();
		for (UINT32 i = 0; i < arrayLen; i++)
			options.addOption(scriptArray.get<GUIOption>(i));

		HSpriteTexture nativeTexture;
		if(texture != nullptr)
			nativeTexture = ScriptSpriteTexture::toNative(texture)->getHandle();

		GUITexture* guiTexture = GUITexture::create(nativeTexture, scale, transparent, options, toString(MonoUtil::monoToWString(style)));

		new (bs_alloc<ScriptGUITexture>()) ScriptGUITexture(instance, guiTexture);
	}

	void ScriptGUITexture::internal_setTexture(ScriptGUITexture* nativeInstance, MonoObject* texture)
	{
		HSpriteTexture nativeTexture;
		if(texture != nullptr)
			nativeTexture = ScriptSpriteTexture::toNative(texture)->getHandle();

		GUITexture* guiTexture = (GUITexture*)nativeInstance->getGUIElement();
		guiTexture->setTexture(nativeTexture);
	}

	void ScriptGUITexture::internal_setTint(ScriptGUITexture* nativeInstance, Color* color)
	{
		GUITexture* guiTexture = (GUITexture*)nativeInstance->getGUIElement();
		guiTexture->setTint(*color);
	}
}