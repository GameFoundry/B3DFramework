//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptGUIToggleable.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "../../../Foundation/bsfEngine/GUI/BsGUIToggleable.h"

namespace bs
{
	ScriptGUIToggleableBase::OnToggledThunkDef ScriptGUIToggleableBase::OnToggledThunk; 

	ScriptGUIToggleableBase::ScriptGUIToggleableBase(MonoObject* managedInstance)
		:ScriptGUIClickableBase(managedInstance)
	 { }

	void ScriptGUIToggleableBase::OnToggled(bool p0)
	{
		MonoUtil::InvokeThunk(OnToggledThunk, GetManagedInstance(), p0);
	}

	void ScriptGUIToggleableBase::RegisterEvents(GUIElement* value)
	{
		static_cast<GUIToggleable*>(value)->OnToggled.Connect(std::bind(&ScriptGUIToggleableBase::OnToggled, this, std::placeholders::_1));
		ScriptGUIClickableBase::RegisterEvents(value);
	}
	ScriptGUIToggleable::ScriptGUIToggleable(MonoObject* managedInstance, GUIToggleable* value)
		:TScriptGUIInteractable(managedInstance, value)
	{
	}

	void ScriptGUIToggleable::InitRuntimeData()
	{
		metaData.ScriptClass->AddInternalCall("Internal_SetIsToggled", (void*)&ScriptGUIToggleable::InternalSetIsToggled);
		metaData.ScriptClass->AddInternalCall("Internal_IsToggled", (void*)&ScriptGUIToggleable::InternalIsToggled);

		OnToggledThunk = (OnToggledThunkDef)metaData.ScriptClass->GetMethodExact("Internal_OnToggled", "bool")->GetThunk();
	}

	void ScriptGUIToggleable::InternalSetIsToggled(ScriptGUIElementBase* self, bool isToggled)
	{
		static_cast<GUIToggleable*>(self->GetGuiElement())->SetIsToggled(isToggled);
	}

	bool ScriptGUIToggleable::InternalIsToggled(ScriptGUIElementBase* self)
	{
		bool tmp__output;
		tmp__output = static_cast<GUIToggleable*>(self->GetGuiElement())->IsToggled();

		bool __output;
		__output = tmp__output;

		return __output;
	}
}
