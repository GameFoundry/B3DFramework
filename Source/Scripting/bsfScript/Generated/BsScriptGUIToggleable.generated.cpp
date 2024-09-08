//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptGUIToggleable.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "../../../Foundation/bsfEngine/GUI/BsGUIToggleable.h"

namespace bs
{
	ScriptGUIToggleableWrapperBase::OnToggledThunkDefinition ScriptGUIToggleableWrapperBase::OnToggledThunk; 

	void ScriptGUIToggleableWrapperBase::OnToggled(bool p0)
	{
		MonoUtil::InvokeThunk(OnToggledThunk, GetScriptObject(), p0);
	}

	void ScriptGUIToggleableWrapperBase::RegisterEvents()
	{
		static_cast<GUIToggleable*>(GetNativeObject())->OnToggled.Connect(std::bind(&ScriptGUIToggleableWrapperBase::OnToggled, this, std::placeholders::_1));
		ScriptGUIClickableWrapperBase::RegisterEvents();
	}
	ScriptGUIToggleable::ScriptGUIToggleable(GUIToggleable* nativeObject)
		:TScriptGUIElementWrapper(nativeObject)
	{
		RegisterEvents();
	}

	void ScriptGUIToggleable::SetupScriptBindings()
	{
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetIsToggled", (void*)&ScriptGUIToggleable::InternalSetIsToggled);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_IsToggled", (void*)&ScriptGUIToggleable::InternalIsToggled);

		OnToggledThunk = (OnToggledThunkDefinition)sInteropMetaData.ScriptClass->GetMethodExact("Internal_OnToggled", "bool")->GetThunk();
	}

	MonoObject* ScriptGUIToggleable::CreateScriptObject(bool construct)
	{
		bool dummy = false;
		void* ctorParams[1] = { &dummy };

		if(construct)
			return sInteropMetaData.ScriptClass->CreateInstance("bool", ctorParams);

		return sInteropMetaData.ScriptClass->CreateInstance(false);
	}
	void ScriptGUIToggleable::InternalSetIsToggled(ScriptGUIToggleableWrapperBase* self, bool isToggled)
	{
		static_cast<GUIToggleable*>(self->GetNativeObject())->SetIsToggled(isToggled);
	}

	bool ScriptGUIToggleable::InternalIsToggled(ScriptGUIToggleableWrapperBase* self)
	{
		bool tmp__output;
		tmp__output = static_cast<GUIToggleable*>(self->GetNativeObject())->IsToggled();

		bool __output;
		__output = tmp__output;

		return __output;
	}
}
