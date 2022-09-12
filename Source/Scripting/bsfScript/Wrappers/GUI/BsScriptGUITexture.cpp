//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Wrappers/GUI/BsScriptGUITexture.h"
#include "BsScriptMeta.h"
#include "BsMonoField.h"
#include "BsMonoClass.h"
#include "BsMonoManager.h"
#include "Image/BsSpriteTexture.h"
#include "BsMonoUtil.h"
#include "GUI/BsGUILayout.h"
#include "GUI/BsGUITexture.h"
#include "GUI/BsGUIOptions.h"
#include "Wrappers/GUI/BsScriptGUILayout.h"

#include "Generated/BsScriptHString.generated.h"
#include "Generated/BsScriptGUIContent.generated.h"
#include "Generated/BsScriptSpriteTexture.generated.h"
#include "Generated/BsScriptGUIElementStyle.generated.h"

namespace bs
{
	ScriptGUITexture::ScriptGUITexture(MonoObject* instance, GUITexture* texture)
		:TScriptGUIElement(instance, texture)
	{

	}

	void ScriptGUITexture::InitRuntimeData()
	{
		metaData.scriptClass->AddInternalCall("Internal_CreateInstance", (void*)&ScriptGUITexture::internal_createInstance);
		metaData.scriptClass->AddInternalCall("Internal_SetTexture", (void*)&ScriptGUITexture::internal_setTexture);
		metaData.scriptClass->AddInternalCall("Internal_SetTint", (void*)&ScriptGUITexture::internal_setTint);
	}

	void ScriptGUITexture::internal_createInstance(MonoObject* instance, MonoObject* texture,
		TextureScaleMode scale, bool transparent, MonoString* style, MonoArray* guiOptions)
	{
		GUIOptions options;

		ScriptArray ScriptArray(guiOptions);
		UINT32 arrayLen = scriptArray.size();
		for (UINT32 i = 0; i < arrayLen; i++)
			options.AddOption(scriptArray.get<GUIOption>(i));

		HSpriteTexture nativeTexture;
		if(texture != nullptr)
			nativeTexture = ScriptSpriteTexture::toNative(texture)->GetHandle();

		GUITexture* guiTexture = GUITexture::create(nativeTexture, scale, transparent, options, MonoUtil::monoToString(style));

		new (bs_alloc<ScriptGUITexture>()) ScriptGUITexture(instance, guiTexture);
	}

	void ScriptGUITexture::internal_setTexture(ScriptGUITexture* nativeInstance, MonoObject* texture)
	{
		HSpriteTexture nativeTexture;
		if(texture != nullptr)
			nativeTexture = ScriptSpriteTexture::toNative(texture)->GetHandle();

		GUITexture* guiTexture = (GUITexture*)nativeInstance->GetGUIElement();
		guiTexture->SetTexture(nativeTexture);
	}

	void ScriptGUITexture::internal_setTint(ScriptGUITexture* nativeInstance, Color* color)
	{
		GUITexture* guiTexture = (GUITexture*)nativeInstance->GetGUIElement();
		guiTexture->SetTint(*color);
	}
}
