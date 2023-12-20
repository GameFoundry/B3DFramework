//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptGUIToggleable.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "../../../Foundation/bsfEngine/GUI/BsGUIToggle.h"

namespace bs
{
	ScriptGUIToggleable::OnToggledThunkDef ScriptGUIToggleable::OnToggledThunk; 

	ScriptGUIToggleable::ScriptGUIToggleable(MonoObject* managedInstance, GUIToggleable* value)
		:TScriptGUIElement(managedInstance, value)
	{
		value->OnToggled.Connect(std::bind(&ScriptGUIToggleable::OnToggled, this, std::placeholders::_1));
	}

	void ScriptGUIToggleable::InitRuntimeData()
	{
		metaData.ScriptClass->AddInternalCall("Internal_SetIsToggled", (void*)&ScriptGUIToggleable::InternalSetIsToggled);
		metaData.ScriptClass->AddInternalCall("Internal_IsToggled", (void*)&ScriptGUIToggleable::InternalIsToggled);

		OnToggledThunk = (OnToggledThunkDef)metaData.ScriptClass->GetMethodExact("Internal_OnToggled", "bool")->GetThunk();
	}

	void ScriptGUIToggleable::OnToggled(bool p0)
	{
		MonoUtil::InvokeThunk(OnToggledThunk, GetManagedInstance(), p0);
	}
	void ScriptGUIToggleable::InternalSetIsToggled(ScriptGUIElementBaseTBase* thisPtr, bool isToggled)
	{
		static_cast<GUIToggleable*>(thisPtr->GetGuiElement())->SetIsToggled(isToggled);
	}

	bool ScriptGUIToggleable::InternalIsToggled(ScriptGUIElementBaseTBase* thisPtr)
	{
		bool tmp__output;
		tmp__output = static_cast<GUIToggleable*>(thisPtr->GetGuiElement())->IsToggled();

		bool __output;
		__output = tmp__output;

		return __output;
	}
}
