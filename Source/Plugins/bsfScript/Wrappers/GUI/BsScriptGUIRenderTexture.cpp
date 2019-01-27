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
#include "Wrappers/GUI/BsScriptGUIElementStyle.h"
#include "Wrappers/GUI/BsScriptGUILayout.h"

#include "Generated/BsScriptHString.generated.h"
#include "Generated/BsScriptGUIContent.generated.h"
#include "Generated/BsScriptRenderTexture.generated.h"

namespace bs
{
	ScriptGUIRenderTexture::ScriptGUIRenderTexture(MonoObject* instance, GUIRenderTexture* texture)
		:TScriptGUIElement(instance, texture)
	{

	}

	void ScriptGUIRenderTexture::initRuntimeData()
	{
		metaData.scriptClass->addInternalCall("Internal_CreateInstance", (void*)&ScriptGUIRenderTexture::internal_createInstance);
		metaData.scriptClass->addInternalCall("Internal_SetTexture", (void*)&ScriptGUIRenderTexture::internal_setTexture);
		metaData.scriptClass->addInternalCall("Internal_SetTint", (void*)&ScriptGUIRenderTexture::internal_setTint);
	}

	void ScriptGUIRenderTexture::internal_createInstance(MonoObject* instance, 
		ScriptRenderTexture* texture, bool transparent, MonoString* style, MonoArray* guiOptions)
	{
		GUIOptions options;

		ScriptArray scriptArray(guiOptions);
		UINT32 arrayLen = scriptArray.size();
		for (UINT32 i = 0; i < arrayLen; i++)
			options.addOption(scriptArray.get<GUIOption>(i));

		SPtr<RenderTexture> renderTexture;
		if (texture != nullptr)
			renderTexture = texture->getInternal();

		GUIRenderTexture* guiTexture = GUIRenderTexture::create(renderTexture, transparent, options, MonoUtil::monoToString(style));

		new (bs_alloc<ScriptGUIRenderTexture>()) ScriptGUIRenderTexture(instance, guiTexture);
	}

	void ScriptGUIRenderTexture::internal_setTexture(ScriptGUIRenderTexture* nativeInstance, ScriptRenderTexture* texture)
	{
		SPtr<RenderTexture> renderTexture;
		if (texture != nullptr)
			renderTexture = texture->getInternal();

		GUIRenderTexture* guiTexture = (GUIRenderTexture*)nativeInstance->getGUIElement();
		guiTexture->setRenderTexture(renderTexture);
	}

	void ScriptGUIRenderTexture::internal_setTint(ScriptGUIRenderTexture* nativeInstance, Color* color)
	{
		GUIRenderTexture* guiTexture = (GUIRenderTexture*)nativeInstance->getGUIElement();
		guiTexture->setTint(*color);
	}
}