//************************************ B3D Framework - Copyright 2025 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DScriptVirtualInput.generated.h"
#include "B3DMonoMethod.h"
#include "B3DMonoClass.h"
#include "B3DMonoUtil.h"
#include "../../../Foundation/Core/Input/B3DVirtualInput.h"
#include "B3DScriptInputConfiguration.generated.h"
#include "B3DScriptVirtualButton.generated.h"
#include "B3DScriptVirtualAxis.generated.h"

namespace b3d
{
	ScriptVirtualInput::OnButtonDownThunkDefinition ScriptVirtualInput::OnButtonDownThunk; 
	ScriptVirtualInput::OnButtonUpThunkDefinition ScriptVirtualInput::OnButtonUpThunk; 
	ScriptVirtualInput::OnButtonHeldThunkDefinition ScriptVirtualInput::OnButtonHeldThunk; 

	HEvent ScriptVirtualInput::OnButtonDownConnection;
	HEvent ScriptVirtualInput::OnButtonUpConnection;
	HEvent ScriptVirtualInput::OnButtonHeldConnection;

	ScriptVirtualInput::ScriptVirtualInput()
		:TScriptTypeDefinition()
	{
	}

	void ScriptVirtualInput::SetupScriptBindings()
	{
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetConfiguration", (void*)&ScriptVirtualInput::InternalSetConfiguration);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetConfiguration", (void*)&ScriptVirtualInput::InternalGetConfiguration);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetOrCreateVirtualButton", (void*)&ScriptVirtualInput::InternalGetOrCreateVirtualButton);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetOrCreateVirtualAxis", (void*)&ScriptVirtualInput::InternalGetOrCreateVirtualAxis);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_IsButtonDown", (void*)&ScriptVirtualInput::InternalIsButtonDown);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_IsButtonUp", (void*)&ScriptVirtualInput::InternalIsButtonUp);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_IsButtonHeld", (void*)&ScriptVirtualInput::InternalIsButtonHeld);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetAxisValue", (void*)&ScriptVirtualInput::InternalGetAxisValue);

		OnButtonDownThunk = (OnButtonDownThunkDefinition)sInteropMetaData.ScriptClass->GetMethodExact("Internal_OnButtonDown", "VirtualButton&,int")->GetThunk();
		OnButtonUpThunk = (OnButtonUpThunkDefinition)sInteropMetaData.ScriptClass->GetMethodExact("Internal_OnButtonUp", "VirtualButton&,int")->GetThunk();
		OnButtonHeldThunk = (OnButtonHeldThunkDefinition)sInteropMetaData.ScriptClass->GetMethodExact("Internal_OnButtonHeld", "VirtualButton&,int")->GetThunk();
	}

	void ScriptVirtualInput::StartUp()
	{
		OnButtonDownConnection = VirtualInput::Instance().OnButtonDown.Connect(&ScriptVirtualInput::OnButtonDown);
		OnButtonUpConnection = VirtualInput::Instance().OnButtonUp.Connect(&ScriptVirtualInput::OnButtonUp);
		OnButtonHeldConnection = VirtualInput::Instance().OnButtonHeld.Connect(&ScriptVirtualInput::OnButtonHeld);
	}
	void ScriptVirtualInput::ShutDown()
	{
		OnButtonDownConnection.Disconnect();
		OnButtonUpConnection.Disconnect();
		OnButtonHeldConnection.Disconnect();
	}

	void ScriptVirtualInput::OnButtonDown(const VirtualButton& p0, uint32_t p1)
	{
		MonoObject* tmpp0;
		tmpp0 = ScriptVirtualButton::Box(p0);
		MonoUtil::InvokeThunk(OnButtonDownThunk, tmpp0, p1);
	}

	void ScriptVirtualInput::OnButtonUp(const VirtualButton& p0, uint32_t p1)
	{
		MonoObject* tmpp0;
		tmpp0 = ScriptVirtualButton::Box(p0);
		MonoUtil::InvokeThunk(OnButtonUpThunk, tmpp0, p1);
	}

	void ScriptVirtualInput::OnButtonHeld(const VirtualButton& p0, uint32_t p1)
	{
		MonoObject* tmpp0;
		tmpp0 = ScriptVirtualButton::Box(p0);
		MonoUtil::InvokeThunk(OnButtonHeldThunk, tmpp0, p1);
	}

	void ScriptVirtualInput::InternalSetConfiguration(MonoObject* input)
	{
		SPtr<InputConfiguration> tmpinput;
		ScriptInputConfiguration* scriptObjectWrapperinput;
		scriptObjectWrapperinput = ScriptInputConfiguration::GetScriptObjectWrapper(input);
		if(scriptObjectWrapperinput != nullptr)
			tmpinput = std::static_pointer_cast<InputConfiguration>(scriptObjectWrapperinput->GetBaseNativeObjectAsShared());
		VirtualInput::Instance().SetConfiguration(tmpinput);
	}

	MonoObject* ScriptVirtualInput::InternalGetConfiguration()
	{
		SPtr<InputConfiguration> tmp__output;
		tmp__output = VirtualInput::Instance().GetConfiguration();

		MonoObject* __output;
		__output = ScriptInputConfiguration::GetOrCreateScriptObject(tmp__output);

		return __output;
	}

	void ScriptVirtualInput::InternalGetOrCreateVirtualButton(MonoString* name, VirtualButton* __output)
	{
		String tmpname;
		tmpname = MonoUtil::MonoToString(name);
		VirtualButton tmp__output;
		tmp__output = VirtualInput::GetOrCreateVirtualButton(tmpname);

		*__output = tmp__output;
	}

	void ScriptVirtualInput::InternalGetOrCreateVirtualAxis(MonoString* name, VirtualAxis* __output)
	{
		String tmpname;
		tmpname = MonoUtil::MonoToString(name);
		VirtualAxis tmp__output;
		tmp__output = VirtualInput::GetOrCreateVirtualAxis(tmpname);

		*__output = tmp__output;
	}

	bool ScriptVirtualInput::InternalIsButtonDown(VirtualButton* button, uint32_t deviceIndex)
	{
		bool tmp__output;
		tmp__output = VirtualInput::Instance().IsButtonDown(*button, deviceIndex);

		bool __output;
		__output = tmp__output;

		return __output;
	}

	bool ScriptVirtualInput::InternalIsButtonUp(VirtualButton* button, uint32_t deviceIndex)
	{
		bool tmp__output;
		tmp__output = VirtualInput::Instance().IsButtonUp(*button, deviceIndex);

		bool __output;
		__output = tmp__output;

		return __output;
	}

	bool ScriptVirtualInput::InternalIsButtonHeld(VirtualButton* button, uint32_t deviceIndex)
	{
		bool tmp__output;
		tmp__output = VirtualInput::Instance().IsButtonHeld(*button, deviceIndex);

		bool __output;
		__output = tmp__output;

		return __output;
	}

	float ScriptVirtualInput::InternalGetAxisValue(VirtualAxis* axis, uint32_t deviceIndex)
	{
		float tmp__output;
		tmp__output = VirtualInput::Instance().GetAxisValue(*axis, deviceIndex);

		float __output;
		__output = tmp__output;

		return __output;
	}
}
