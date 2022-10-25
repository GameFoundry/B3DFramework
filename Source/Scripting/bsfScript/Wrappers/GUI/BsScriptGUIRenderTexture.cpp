//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Wrappers/GUI/BsScriptGUIRenderTexture.h"
#include "BsScriptMeta.h"
#include "BsMonoField.h"
#include "BsMonoClass.h"
#include "BsMonoManager.h"
#include "Image/BsSpriteTexture.h"
#include "BsMonoUtil.h"
#include "GUI/BsGUILayout.h"
#include "GUI/BsGUIRenderTexture.h"
#include "GUI/BsGUIOptions.h"
#include "Wrappers/GUI/BsScriptGUILayout.h"

#include "Generated/BsScriptHString.generated.h"
#include "Generated/BsScriptGUIContent.generated.h"
#include "Generated/BsScriptRenderTexture.generated.h"
#include "Generated/BsScriptGUIElementStyle.generated.h"

namespace bs
{
	ScriptGUIRenderTexture::ScriptGUIRenderTexture(MonoObject* instance, GUIRenderTexture* texture)
		: TScriptGUIElement(instance, texture)
	{
	}

	void ScriptGUIRenderTexture::InitRuntimeData()
	{
		metaData.ScriptClass->AddInternalCall("Internal_CreateInstance", (void*)&ScriptGUIRenderTexture::InternalCreateInstance);
		metaData.ScriptClass->AddInternalCall("Internal_SetTexture", (void*)&ScriptGUIRenderTexture::InternalSetTexture);
		metaData.ScriptClass->AddInternalCall("Internal_SetTint", (void*)&ScriptGUIRenderTexture::InternalSetTint);
	}

	void ScriptGUIRenderTexture::InternalCreateInstance(MonoObject* instance, ScriptRenderTexture* texture, bool transparent, MonoString* style, MonoArray* guiOptions)
	{
		GUIOptions options;

		ScriptArray scriptArray(guiOptions);
		u32 arrayLen = scriptArray.Size();
		for(u32 i = 0; i < arrayLen; i++)
			options.AddOption(scriptArray.Get<GUIOption>(i));

		SPtr<RenderTexture> renderTexture;
		if(texture != nullptr)
			renderTexture = texture->GetInternal();

		GUIRenderTexture* guiTexture = GUIRenderTexture::Create(renderTexture, transparent, options, MonoUtil::MonoToString(style));

		new(bs_alloc<ScriptGUIRenderTexture>()) ScriptGUIRenderTexture(instance, guiTexture);
	}

	void ScriptGUIRenderTexture::InternalSetTexture(ScriptGUIRenderTexture* nativeInstance, ScriptRenderTexture* texture)
	{
		SPtr<RenderTexture> renderTexture;
		if(texture != nullptr)
			renderTexture = texture->GetInternal();

		GUIRenderTexture* guiTexture = (GUIRenderTexture*)nativeInstance->GetGuiElement();
		guiTexture->SetRenderTexture(renderTexture);
	}

	void ScriptGUIRenderTexture::InternalSetTint(ScriptGUIRenderTexture* nativeInstance, Color* color)
	{
		GUIRenderTexture* guiTexture = (GUIRenderTexture*)nativeInstance->GetGuiElement();
		guiTexture->SetTint(*color);
	}
} // namespace bs
