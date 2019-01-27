//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Wrappers/GUI/BsScriptGUIToggle.h"
#include "BsScriptMeta.h"
#include "BsMonoField.h"
#include "BsMonoClass.h"
#include "BsMonoManager.h"
#include "BsMonoMethod.h"
#include "Image/BsSpriteTexture.h"
#include "BsMonoUtil.h"
#include "GUI/BsGUILayout.h"
#include "GUI/BsGUIToggle.h"
#include "GUI/BsGUIToggleGroup.h"
#include "GUI/BsGUIOptions.h"
#include "Wrappers/GUI/BsScriptGUIToggleGroup.h"
#include "Wrappers/GUI/BsScriptGUIElementStyle.h"
#include "Wrappers/GUI/BsScriptGUILayout.h"

#include "Generated/BsScriptHString.generated.h"
#include "Generated/BsScriptGUIContent.generated.h"

using namespace std::placeholders;

namespace bs
{
	ScriptGUIToggle::OnClickThunkDef ScriptGUIToggle::onClickThunk;
	ScriptGUIToggle::OnHoverThunkDef ScriptGUIToggle::onHoverThunk;
	ScriptGUIToggle::OnOutThunkDef ScriptGUIToggle::onOutThunk;
	ScriptGUIToggle::OnToggledThunkDef ScriptGUIToggle::onToggledThunk;
	ScriptGUIToggle::OnDoubleClickThunkDef ScriptGUIToggle::onDoubleClickThunk;

	ScriptGUIToggle::ScriptGUIToggle(MonoObject* instance, GUIToggle* toggle)
		:TScriptGUIElement(instance, toggle)
	{

	}

	void ScriptGUIToggle::initRuntimeData()
	{
		metaData.scriptClass->addInternalCall("Internal_CreateInstance", (void*)&ScriptGUIToggle::internal_createInstance);
		metaData.scriptClass->addInternalCall("Internal_SetContent", (void*)&ScriptGUIToggle::internal_setContent);
		metaData.scriptClass->addInternalCall("Internal_GetValue", (void*)&ScriptGUIToggle::internal_getValue);
		metaData.scriptClass->addInternalCall("Internal_SetValue", (void*)&ScriptGUIToggle::internal_setValue);
		metaData.scriptClass->addInternalCall("Internal_SetTint", (void*)&ScriptGUIToggle::internal_setTint);

		onClickThunk = (OnClickThunkDef)metaData.scriptClass->getMethod("DoOnClick")->getThunk();
		onHoverThunk = (OnHoverThunkDef)metaData.scriptClass->getMethod("DoOnHover")->getThunk();
		onOutThunk = (OnOutThunkDef)metaData.scriptClass->getMethod("DoOnOut")->getThunk();
		onToggledThunk = (OnToggledThunkDef)metaData.scriptClass->getMethod("DoOnToggled", 1)->getThunk();
		onDoubleClickThunk = (OnDoubleClickThunkDef)metaData.scriptClass->getMethod("DoOnDoubleClick")->getThunk();
	}

	void ScriptGUIToggle::internal_createInstance(MonoObject* instance, __GUIContentInterop* content, 
		MonoObject* monoToggleGroup, MonoString* style, MonoArray* guiOptions)
	{
		GUIOptions options;

		ScriptArray scriptArray(guiOptions);
		UINT32 arrayLen = scriptArray.size();
		for (UINT32 i = 0; i < arrayLen; i++)
			options.addOption(scriptArray.get<GUIOption>(i));

		ScriptGUIToggleGroup* scriptToggleGroup = nullptr;
		SPtr<GUIToggleGroup> toggleGroup;
		if (monoToggleGroup != nullptr)
		{
			scriptToggleGroup = ScriptGUIToggleGroup::toNative(monoToggleGroup);
			toggleGroup = scriptToggleGroup->getInternalValue();
		}

		GUIContent nativeContent = ScriptGUIContent::fromInterop(*content);
		GUIToggle* guiToggle = GUIToggle::create(nativeContent, toggleGroup, options, MonoUtil::monoToString(style));

		auto nativeInstance = new (bs_alloc<ScriptGUIToggle>()) ScriptGUIToggle(instance, guiToggle);

		guiToggle->onClick.connect(std::bind(&ScriptGUIToggle::onClick, nativeInstance));
		guiToggle->onHover.connect(std::bind(&ScriptGUIToggle::onHover, nativeInstance));
		guiToggle->onOut.connect(std::bind(&ScriptGUIToggle::onOut, nativeInstance));
		guiToggle->onToggled.connect(std::bind(&ScriptGUIToggle::onToggled, nativeInstance, std::placeholders::_1));
		guiToggle->onDoubleClick.connect(std::bind(&ScriptGUIToggle::onDoubleClick, nativeInstance));
	}

	void ScriptGUIToggle::internal_setContent(ScriptGUIToggle* nativeInstance, __GUIContentInterop* content)
	{
		GUIContent nativeContent = ScriptGUIContent::fromInterop(*content);

		GUIToggle* toggle = (GUIToggle*)nativeInstance->getGUIElement();
		toggle->setContent(nativeContent);
	}

	bool ScriptGUIToggle::internal_getValue(ScriptGUIToggle* nativeInstance)
	{
		GUIToggle* toggle = (GUIToggle*)nativeInstance->getGUIElement();
		return toggle->isToggled();
	}

	void ScriptGUIToggle::internal_setValue(ScriptGUIToggle* nativeInstance, bool value)
	{
		GUIToggle* toggle = (GUIToggle*)nativeInstance->getGUIElement();

		if (value)
			toggle->toggleOn();
		else
			toggle->toggleOff();
	}

	void ScriptGUIToggle::internal_setTint(ScriptGUIToggle* nativeInstance, Color* color)
	{
		GUIToggle* toggle = (GUIToggle*)nativeInstance->getGUIElement();
		toggle->setTint(*color);
	}

	void ScriptGUIToggle::onClick()
	{
		MonoUtil::invokeThunk(onClickThunk, getManagedInstance());
	}

	void ScriptGUIToggle::onHover()
	{
		MonoUtil::invokeThunk(onHoverThunk, getManagedInstance());
	}

	void ScriptGUIToggle::onOut()
	{
		MonoUtil::invokeThunk(onOutThunk, getManagedInstance());
	}

	void ScriptGUIToggle::onToggled(bool toggled)
	{
		MonoUtil::invokeThunk(onToggledThunk, getManagedInstance(), toggled);
	}

	void ScriptGUIToggle::onDoubleClick()
	{
		MonoUtil::invokeThunk(onDoubleClickThunk, getManagedInstance());
	}
}