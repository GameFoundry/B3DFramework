//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Wrappers/GUI/BsScriptGUIButton.h"
#include "BsScriptMeta.h"
#include "BsMonoField.h"
#include "BsMonoClass.h"
#include "BsMonoManager.h"
#include "BsMonoMethod.h"
#include "Image/BsSpriteTexture.h"
#include "BsMonoUtil.h"
#include "GUI/BsGUILayout.h"
#include "GUI/BsGUIButton.h"
#include "GUI/BsGUIOptions.h"
#include "Wrappers/GUI/BsScriptGUIElementStyle.h"
#include "Wrappers/GUI/BsScriptGUILayout.h"

#include "Generated/BsScriptHString.generated.h"
#include "Generated/BsScriptGUIContent.generated.h"

namespace bs
{
	ScriptGUIButton::OnClickThunkDef ScriptGUIButton::onClickThunk;
	ScriptGUIButton::OnHoverThunkDef ScriptGUIButton::onHoverThunk;
	ScriptGUIButton::OnOutThunkDef ScriptGUIButton::onOutThunk;
	ScriptGUIButton::OnDoubleClickThunkDef ScriptGUIButton::onDoubleClickThunk;

	ScriptGUIButton::ScriptGUIButton(MonoObject* instance, GUIButton* button)
		:TScriptGUIElement(instance, button)
	{

	}

	void ScriptGUIButton::initRuntimeData()
	{
		metaData.scriptClass->addInternalCall("Internal_CreateInstance", (void*)&ScriptGUIButton::internal_createInstance);
		metaData.scriptClass->addInternalCall("Internal_SetContent", (void*)&ScriptGUIButton::internal_setContent);
		metaData.scriptClass->addInternalCall("Internal_SetTint", (void*)&ScriptGUIButton::internal_setTint);

		onClickThunk = (OnClickThunkDef)metaData.scriptClass->getMethod("DoOnClick")->getThunk();
		onDoubleClickThunk = (OnDoubleClickThunkDef)metaData.scriptClass->getMethod("DoOnDoubleClick")->getThunk();
		onHoverThunk = (OnHoverThunkDef)metaData.scriptClass->getMethod("DoOnHover")->getThunk();
		onOutThunk = (OnOutThunkDef)metaData.scriptClass->getMethod("DoOnOut")->getThunk();
	}

	void ScriptGUIButton::internal_createInstance(MonoObject* instance, __GUIContentInterop* content,
		MonoString* style, MonoArray* guiOptions)
	{
		GUIOptions options;

		ScriptArray scriptArray(guiOptions);
		UINT32 arrayLen = scriptArray.size();
		for(UINT32 i = 0; i < arrayLen; i++)
			options.addOption(scriptArray.get<GUIOption>(i));

		GUIContent nativeContent = ScriptGUIContent::fromInterop(*content);
		GUIButton* guiButton = GUIButton::create(nativeContent, options, MonoUtil::monoToString(style));

		auto nativeInstance = new (bs_alloc<ScriptGUIButton>()) ScriptGUIButton(instance, guiButton);

		guiButton->onClick.connect(std::bind(&ScriptGUIButton::onClick, nativeInstance));
		guiButton->onDoubleClick.connect(std::bind(&ScriptGUIButton::onDoubleClick, nativeInstance));
		guiButton->onHover.connect(std::bind(&ScriptGUIButton::onHover, nativeInstance));
		guiButton->onOut.connect(std::bind(&ScriptGUIButton::onOut, nativeInstance));
	}

	void ScriptGUIButton::internal_setContent(ScriptGUIButton* nativeInstance, __GUIContentInterop* content)
	{
		GUIContent nativeContent = ScriptGUIContent::fromInterop(*content);

		GUIButton* button = (GUIButton*)nativeInstance->getGUIElement();
		button->setContent(nativeContent);
	}

	void ScriptGUIButton::internal_setTint(ScriptGUIButton* nativeInstance, Color* color)
	{
		GUIButton* button = (GUIButton*)nativeInstance->getGUIElement();
		button->setTint(*color);
	}

	void ScriptGUIButton::onClick()
	{
		MonoUtil::invokeThunk(onClickThunk, getManagedInstance());
	}

	void ScriptGUIButton::onDoubleClick()
	{
		MonoUtil::invokeThunk(onDoubleClickThunk, getManagedInstance());
	}

	void ScriptGUIButton::onHover()
	{
		MonoUtil::invokeThunk(onHoverThunk, getManagedInstance());
	}

	void ScriptGUIButton::onOut()
	{
		MonoUtil::invokeThunk(onOutThunk, getManagedInstance());
	}
}