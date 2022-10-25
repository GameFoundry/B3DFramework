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
		: TScriptGUIElement(instance, texture)
	{
	}

	void ScriptGUITexture::InitRuntimeData()
	{
		metaData.ScriptClass->AddInternalCall("Internal_CreateInstance", (void*)&ScriptGUITexture::InternalCreateInstance);
		metaData.ScriptClass->AddInternalCall("Internal_SetTexture", (void*)&ScriptGUITexture::InternalSetTexture);
		metaData.ScriptClass->AddInternalCall("Internal_SetTint", (void*)&ScriptGUITexture::InternalSetTint);
	}

	void ScriptGUITexture::InternalCreateInstance(MonoObject* instance, MonoObject* texture, TextureScaleMode scale, bool transparent, MonoString* style, MonoArray* guiOptions)
	{
		GUIOptions options;

		ScriptArray scriptArray(guiOptions);
		u32 arrayLen = scriptArray.Size();
		for(u32 i = 0; i < arrayLen; i++)
			options.AddOption(scriptArray.Get<GUIOption>(i));

		HSpriteTexture nativeTexture;
		if(texture != nullptr)
			nativeTexture = ScriptSpriteTexture::ToNative(texture)->GetHandle();

		GUITexture* guiTexture = GUITexture::Create(nativeTexture, scale, transparent, options, MonoUtil::MonoToString(style));

		new(bs_alloc<ScriptGUITexture>()) ScriptGUITexture(instance, guiTexture);
	}

	void ScriptGUITexture::InternalSetTexture(ScriptGUITexture* nativeInstance, MonoObject* texture)
	{
		HSpriteTexture nativeTexture;
		if(texture != nullptr)
			nativeTexture = ScriptSpriteTexture::ToNative(texture)->GetHandle();

		GUITexture* guiTexture = (GUITexture*)nativeInstance->GetGuiElement();
		guiTexture->SetTexture(nativeTexture);
	}

	void ScriptGUITexture::InternalSetTint(ScriptGUITexture* nativeInstance, Color* color)
	{
		GUITexture* guiTexture = (GUITexture*)nativeInstance->GetGuiElement();
		guiTexture->SetTint(*color);
	}
} // namespace bs
