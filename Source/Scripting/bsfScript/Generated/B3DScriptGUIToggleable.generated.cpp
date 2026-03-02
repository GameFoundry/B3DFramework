//************************************ B3D Framework - Copyright 2025 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DScriptGUIToggleable.generated.h"
#include "B3DMonoMethod.h"
#include "B3DMonoClass.h"
#include "B3DMonoUtil.h"
#include "../../../Engine/Core/GUI/B3DGUIToggleable.h"

namespace b3d
{
	ScriptGUIToggleableWrapperBase::OnToggledThunkDefinition ScriptGUIToggleableWrapperBase::OnToggledThunk; 

	void ScriptGUIToggleableWrapperBase::OnToggled(bool p0)
	{
		MonoUtil::InvokeThunk(OnToggledThunk, GetScriptObject(), p0);
	}

	void ScriptGUIToggleableWrapperBase::RegisterEvents()
	{
		OnToggledConnection = static_cast<GUIToggleable*>(GetNativeObject())->OnToggled.Connect([this](bool p0) { OnToggled(p0); });
		ScriptGUIClickableWrapperBase::RegisterEvents();
	}
	void ScriptGUIToggleableWrapperBase::UnregisterEvents()
	{
		OnToggledConnection.Disconnect();
		ScriptGUIClickableWrapperBase::UnregisterEvents();
	}
	ScriptGUIToggleable::ScriptGUIToggleable(GUIToggleable* nativeObject)
		:TScriptGUIElementWrapper(nativeObject)
	{
		RegisterEvents();
	}

	ScriptGUIToggleable::~ScriptGUIToggleable()
	{
		UnregisterEvents();
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
		if(!self->IsNativeObjectValid())
			return;

		static_cast<GUIToggleable*>(self->GetNativeObject())->SetIsToggled(isToggled);
	}

	bool ScriptGUIToggleable::InternalIsToggled(ScriptGUIToggleableWrapperBase* self)
	{
		bool tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<GUIToggleable*>(self->GetNativeObject())->IsToggled();

		bool __output;
		__output = tmp__output;

		return __output;
	}
}
