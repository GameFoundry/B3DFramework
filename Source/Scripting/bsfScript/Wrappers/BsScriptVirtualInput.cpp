//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Wrappers/BsScriptVirtualInput.h"
#include "BsMonoManager.h"
#include "BsMonoClass.h"
#include "BsMonoMethod.h"
#include "BsMonoUtil.h"
#include "Input/BsVirtualInput.h"
#include "Wrappers/BsScriptVirtualButton.h"
#include "Wrappers/BsScriptInputConfiguration.h"
#include "BsPlayInEditor.h"
#include "BsScriptInputConfiguration.generated.h"

using namespace bs;
ScriptVirtualInput::OnButtonEventThunkDef ScriptVirtualInput::OnButtonUpThunk;
ScriptVirtualInput::OnButtonEventThunkDef ScriptVirtualInput::OnButtonDownThunk;
ScriptVirtualInput::OnButtonEventThunkDef ScriptVirtualInput::OnButtonHeldThunk;

HEvent ScriptVirtualInput::OnButtonPressedConn;
HEvent ScriptVirtualInput::OnButtonReleasedConn;
HEvent ScriptVirtualInput::OnButtonHeldConn;

ScriptVirtualInput::ScriptVirtualInput(MonoObject* instance)
	: ScriptObject(instance)
{}

void ScriptVirtualInput::InitRuntimeData()
{
	metaData.ScriptClass->AddInternalCall("Internal_GetKeyConfig", (void*)&ScriptVirtualInput::InternalGetKeyConfig);
	metaData.ScriptClass->AddInternalCall("Internal_SetKeyConfig", (void*)&ScriptVirtualInput::InternalSetKeyConfig);
	metaData.ScriptClass->AddInternalCall("Internal_IsButtonHeld", (void*)&ScriptVirtualInput::InternalIsButtonHeld);
	metaData.ScriptClass->AddInternalCall("Internal_IsButtonDown", (void*)&ScriptVirtualInput::InternalIsButtonDown);
	metaData.ScriptClass->AddInternalCall("Internal_IsButtonUp", (void*)&ScriptVirtualInput::InternalIsButtonUp);
	metaData.ScriptClass->AddInternalCall("Internal_GetAxisValue", (void*)&ScriptVirtualInput::InternalGetAxisValue);

	OnButtonUpThunk = (OnButtonEventThunkDef)metaData.ScriptClass->GetMethodExact("Internal_TriggerButtonDown", "VirtualButton,int")->GetThunk();
	OnButtonDownThunk = (OnButtonEventThunkDef)metaData.ScriptClass->GetMethodExact("Internal_TriggerButtonUp", "VirtualButton,int")->GetThunk();
	OnButtonHeldThunk = (OnButtonEventThunkDef)metaData.ScriptClass->GetMethodExact("Internal_TriggerButtonHeld", "VirtualButton,int")->GetThunk();
}

void ScriptVirtualInput::StartUp()
{
	VirtualInput& input = VirtualInput::Instance();

	OnButtonPressedConn = input.OnButtonDown.Connect(&ScriptVirtualInput::OnButtonDown);
	OnButtonReleasedConn = input.OnButtonUp.Connect(&ScriptVirtualInput::OnButtonUp);
	OnButtonHeldConn = input.OnButtonHeld.Connect(&ScriptVirtualInput::OnButtonHeld);
}

void ScriptVirtualInput::ShutDown()
{
	OnButtonPressedConn.Disconnect();
	OnButtonReleasedConn.Disconnect();
	OnButtonHeldConn.Disconnect();
}

void ScriptVirtualInput::OnButtonDown(const VirtualButton& btn, u32 deviceIdx)
{
	if(PlayInEditor::Instance().GetState() != PlayInEditorState::Playing)
		return;

	MonoObject* virtualButton = ScriptVirtualButton::Box(btn);
	MonoUtil::InvokeThunk(OnButtonDownThunk, virtualButton, deviceIdx);
}

void ScriptVirtualInput::OnButtonUp(const VirtualButton& btn, u32 deviceIdx)
{
	if(PlayInEditor::Instance().GetState() != PlayInEditorState::Playing)
		return;

	MonoObject* virtualButton = ScriptVirtualButton::Box(btn);
	MonoUtil::InvokeThunk(OnButtonUpThunk, virtualButton, deviceIdx);
}

void ScriptVirtualInput::OnButtonHeld(const VirtualButton& btn, u32 deviceIdx)
{
	if(PlayInEditor::Instance().GetState() != PlayInEditorState::Playing)
		return;

	MonoObject* virtualButton = ScriptVirtualButton::Box(btn);
	MonoUtil::InvokeThunk(OnButtonHeldThunk, virtualButton, deviceIdx);
}

MonoObject* ScriptVirtualInput::InternalGetKeyConfig()
{
	SPtr<InputConfiguration> inputConfig = VirtualInput::Instance().GetConfiguration();
	return ScriptInputConfiguration::GetOrCreateScriptObject(inputConfig);
}

void ScriptVirtualInput::InternalSetKeyConfig(MonoObject* keyConfig)
{
	ScriptInputConfiguration* inputConfig = ScriptInputConfiguration::GetScriptObjectWrapper(keyConfig);

	VirtualInput::Instance().SetConfiguration(inputConfig->GetNativeObjectAsShared());
}

bool ScriptVirtualInput::InternalIsButtonHeld(VirtualButton* btn, u32 deviceIdx)
{
	return VirtualInput::Instance().IsButtonHeld(*btn, deviceIdx);
}

bool ScriptVirtualInput::InternalIsButtonDown(VirtualButton* btn, u32 deviceIdx)
{
	return VirtualInput::Instance().IsButtonDown(*btn, deviceIdx);
}

bool ScriptVirtualInput::InternalIsButtonUp(VirtualButton* btn, u32 deviceIdx)
{
	return VirtualInput::Instance().IsButtonUp(*btn, deviceIdx);
}

float ScriptVirtualInput::InternalGetAxisValue(VirtualAxis* axis, u32 deviceIdx)
{
	return VirtualInput::Instance().GetAxisValue(*axis, deviceIdx);
}
