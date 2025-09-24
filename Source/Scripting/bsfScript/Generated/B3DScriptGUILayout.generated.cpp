//********************************* B3D Framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptGUILayout.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "../../../Foundation/bsfEngine/GUI/BsGUILayout.h"
#include "BsScriptGUIElement.generated.h"

namespace b3d
{
	ScriptGUILayout::ScriptGUILayout(GUILayout* nativeObject)
		:TScriptGUIElementWrapper(nativeObject)
	{
		RegisterEvents();
	}

	ScriptGUILayout::~ScriptGUILayout()
	{
		UnregisterEvents();
	}

	void ScriptGUILayout::SetupScriptBindings()
	{
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_AddElement", (void*)&ScriptGUILayout::InternalAddElement);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_RemoveElement", (void*)&ScriptGUILayout::InternalRemoveElement);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_RemoveElementAt", (void*)&ScriptGUILayout::InternalRemoveElementAt);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_InsertElement", (void*)&ScriptGUILayout::InternalInsertElement);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetChildCount", (void*)&ScriptGUILayout::InternalGetChildCount);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetChild", (void*)&ScriptGUILayout::InternalGetChild);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_Clear", (void*)&ScriptGUILayout::InternalClear);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetEnableCulling", (void*)&ScriptGUILayout::InternalSetEnableCulling);

	}

	MonoObject* ScriptGUILayout::CreateScriptObject(bool construct)
	{
		bool dummy = false;
		void* ctorParams[1] = { &dummy };

		if(construct)
			return sInteropMetaData.ScriptClass->CreateInstance("bool", ctorParams);

		return sInteropMetaData.ScriptClass->CreateInstance(false);
	}
	void ScriptGUILayout::InternalAddElement(ScriptGUILayoutWrapperBase* self, MonoObject* element)
	{
		if(!self->IsNativeObjectValid())
			return;

		GUIElement* tmpelement = nullptr;
		ScriptGUIElement* scriptObjectWrapperelement;
		scriptObjectWrapperelement = ScriptGUIElement::GetScriptObjectWrapper(element);
		if(scriptObjectWrapperelement != nullptr)
			tmpelement = static_cast<GUIElement*>(scriptObjectWrapperelement->GetNativeObject());
		static_cast<GUILayout*>(self->GetNativeObject())->AddElement(tmpelement);
	}

	void ScriptGUILayout::InternalRemoveElement(ScriptGUILayoutWrapperBase* self, MonoObject* element)
	{
		if(!self->IsNativeObjectValid())
			return;

		GUIElement* tmpelement = nullptr;
		ScriptGUIElement* scriptObjectWrapperelement;
		scriptObjectWrapperelement = ScriptGUIElement::GetScriptObjectWrapper(element);
		if(scriptObjectWrapperelement != nullptr)
			tmpelement = static_cast<GUIElement*>(scriptObjectWrapperelement->GetNativeObject());
		static_cast<GUILayout*>(self->GetNativeObject())->RemoveElement(tmpelement);
	}

	void ScriptGUILayout::InternalRemoveElementAt(ScriptGUILayoutWrapperBase* self, uint32_t index)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<GUILayout*>(self->GetNativeObject())->RemoveElementAt(index);
	}

	void ScriptGUILayout::InternalInsertElement(ScriptGUILayoutWrapperBase* self, uint32_t index, MonoObject* element)
	{
		if(!self->IsNativeObjectValid())
			return;

		GUIElement* tmpelement = nullptr;
		ScriptGUIElement* scriptObjectWrapperelement;
		scriptObjectWrapperelement = ScriptGUIElement::GetScriptObjectWrapper(element);
		if(scriptObjectWrapperelement != nullptr)
			tmpelement = static_cast<GUIElement*>(scriptObjectWrapperelement->GetNativeObject());
		static_cast<GUILayout*>(self->GetNativeObject())->InsertElement(index, tmpelement);
	}

	uint32_t ScriptGUILayout::InternalGetChildCount(ScriptGUILayoutWrapperBase* self)
	{
		uint32_t tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<GUILayout*>(self->GetNativeObject())->GetChildCount();

		uint32_t __output;
		__output = tmp__output;

		return __output;
	}

	MonoObject* ScriptGUILayout::InternalGetChild(ScriptGUILayoutWrapperBase* self, uint32_t index)
	{
		GUIElement* tmp__output = nullptr;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<GUILayout*>(self->GetNativeObject())->GetChild(index);

		MonoObject* __output;
		__output = ScriptGUIElement::GetOrCreateScriptObject(tmp__output);

		return __output;
	}

	void ScriptGUILayout::InternalClear(ScriptGUILayoutWrapperBase* self)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<GUILayout*>(self->GetNativeObject())->Clear();
	}

	void ScriptGUILayout::InternalSetEnableCulling(ScriptGUILayoutWrapperBase* self, bool enable)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<GUILayout*>(self->GetNativeObject())->SetEnableCulling(enable);
	}
}
