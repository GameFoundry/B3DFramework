//************************************ B3D Framework - Copyright 2025 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DScriptGUIInteractable.generated.h"
#include "B3DMonoMethod.h"
#include "B3DMonoClass.h"
#include "B3DMonoUtil.h"
#include "../../../Engine/Core/GUI/B3DGUIInteractable.h"
#include "Wrappers/B3DScriptContextMenu.h"

namespace b3d
{
	ScriptGUIInteractableWrapperBase::OnFocusGainedThunkDefinition ScriptGUIInteractableWrapperBase::OnFocusGainedThunk; 
	ScriptGUIInteractableWrapperBase::OnFocusLostThunkDefinition ScriptGUIInteractableWrapperBase::OnFocusLostThunk; 

	void ScriptGUIInteractableWrapperBase::OnFocusGained()
	{
		MonoUtil::InvokeThunk(OnFocusGainedThunk, GetScriptObject());
	}

	void ScriptGUIInteractableWrapperBase::OnFocusLost()
	{
		MonoUtil::InvokeThunk(OnFocusLostThunk, GetScriptObject());
	}

	void ScriptGUIInteractableWrapperBase::RegisterEvents()
	{
		OnFocusGainedConnection = static_cast<GUIInteractable*>(GetNativeObject())->OnFocusGained.Connect([this]() { OnFocusGained(); });
		OnFocusLostConnection = static_cast<GUIInteractable*>(GetNativeObject())->OnFocusLost.Connect([this]() { OnFocusLost(); });
		ScriptGUIRenderableWrapperBase::RegisterEvents();
	}
	void ScriptGUIInteractableWrapperBase::UnregisterEvents()
	{
		OnFocusGainedConnection.Disconnect();
		OnFocusLostConnection.Disconnect();
		ScriptGUIRenderableWrapperBase::UnregisterEvents();
	}
	ScriptGUIInteractable::ScriptGUIInteractable(GUIInteractable* nativeObject)
		:TScriptGUIElementWrapper(nativeObject)
	{
		RegisterEvents();
	}

	ScriptGUIInteractable::~ScriptGUIInteractable()
	{
		UnregisterEvents();
	}

	void ScriptGUIInteractable::SetupScriptBindings()
	{
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetFocus", (void*)&ScriptGUIInteractable::InternalSetFocus);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetOptionFlags", (void*)&ScriptGUIInteractable::InternalSetOptionFlags);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetOptionFlags", (void*)&ScriptGUIInteractable::InternalGetOptionFlags);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetContextMenu", (void*)&ScriptGUIInteractable::InternalSetContextMenu);

		OnFocusGainedThunk = (OnFocusGainedThunkDefinition)sInteropMetaData.ScriptClass->GetMethodExact("Internal_OnFocusGained", "")->GetThunk();
		OnFocusLostThunk = (OnFocusLostThunkDefinition)sInteropMetaData.ScriptClass->GetMethodExact("Internal_OnFocusLost", "")->GetThunk();
	}

	MonoObject* ScriptGUIInteractable::CreateScriptObject(bool construct)
	{
		bool dummy = false;
		void* ctorParams[1] = { &dummy };

		if(construct)
			return sInteropMetaData.ScriptClass->CreateInstance("bool", ctorParams);

		return sInteropMetaData.ScriptClass->CreateInstance(false);
	}
	void ScriptGUIInteractable::InternalSetFocus(ScriptGUIInteractableWrapperBase* self, bool enabled, bool clear)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<GUIInteractable*>(self->GetNativeObject())->SetFocus(enabled, clear);
	}

	void ScriptGUIInteractable::InternalSetOptionFlags(ScriptGUIInteractableWrapperBase* self, GUIElementOption options)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<GUIInteractable*>(self->GetNativeObject())->SetOptionFlags(options);
	}

	GUIElementOption ScriptGUIInteractable::InternalGetOptionFlags(ScriptGUIInteractableWrapperBase* self)
	{
		Flags<GUIElementOption> tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<GUIInteractable*>(self->GetNativeObject())->GetOptionFlags();

		GUIElementOption __output;
		__output = (GUIElementOption)(uint32_t)tmp__output;

		return __output;
	}

	void ScriptGUIInteractable::InternalSetContextMenu(ScriptGUIInteractableWrapperBase* self, MonoObject* menu)
	{
		if(!self->IsNativeObjectValid())
			return;

		TShared<GUIContextMenu> tmpmenu;
		ScriptContextMenu* scriptObjectWrappermenu;
		scriptObjectWrappermenu = ScriptContextMenu::GetScriptObjectWrapper(menu);
		if(scriptObjectWrappermenu != nullptr)
			tmpmenu = std::static_pointer_cast<GUIContextMenu>(scriptObjectWrappermenu->GetBaseNativeObjectAsShared());
		static_cast<GUIInteractable*>(self->GetNativeObject())->SetContextMenu(tmpmenu);
	}
}
