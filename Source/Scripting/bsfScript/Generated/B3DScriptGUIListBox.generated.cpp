//************************************ B3D Framework - Copyright 2025 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DScriptGUIListBox.generated.h"
#include "B3DMonoMethod.h"
#include "B3DMonoClass.h"
#include "B3DMonoUtil.h"
#include "../../../Engine/Core/GUI/B3DGUIListBox.h"
#include "B3DScriptHString.generated.h"
#include "B3DScriptGUIOption.generated.h"
#include "B3DScriptGUIListBox.generated.h"
#include "B3DScriptGUIListBoxContent.generated.h"

namespace b3d
{
	ScriptGUIListBox::OnSelectionToggledThunkDefinition ScriptGUIListBox::OnSelectionToggledThunk; 

	ScriptGUIListBox::ScriptGUIListBox(GUIListBox* nativeObject)
		:TScriptGUIElementWrapper(nativeObject)
	{
		RegisterEvents();
	}

	ScriptGUIListBox::~ScriptGUIListBox()
	{
		UnregisterEvents();
	}

	void ScriptGUIListBox::SetupScriptBindings()
	{
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_IsMultiselect", (void*)&ScriptGUIListBox::InternalIsMultiselect);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetElements", (void*)&ScriptGUIListBox::InternalSetElements);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SelectElement", (void*)&ScriptGUIListBox::InternalSelectElement);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_DeselectElement", (void*)&ScriptGUIListBox::InternalDeselectElement);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetSelectedElementIndex", (void*)&ScriptGUIListBox::InternalGetSelectedElementIndex);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetElementStates", (void*)&ScriptGUIListBox::InternalGetElementStates);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetElementStates", (void*)&ScriptGUIListBox::InternalSetElementStates);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_Create", (void*)&ScriptGUIListBox::InternalCreate);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_Create0", (void*)&ScriptGUIListBox::InternalCreate0);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_Create1", (void*)&ScriptGUIListBox::InternalCreate1);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_Create2", (void*)&ScriptGUIListBox::InternalCreate2);

		OnSelectionToggledThunk = (OnSelectionToggledThunkDefinition)sInteropMetaData.ScriptClass->GetMethodExact("Internal_OnSelectionToggled", "int,bool")->GetThunk();
	}

	MonoObject* ScriptGUIListBox::CreateScriptObject(bool construct)
	{
		bool dummy = false;
		void* ctorParams[1] = { &dummy };

		if(construct)
			return sInteropMetaData.ScriptClass->CreateInstance("bool", ctorParams);

		return sInteropMetaData.ScriptClass->CreateInstance(false);
	}
	void ScriptGUIListBox::OnSelectionToggled(uint32_t p0, bool p1)
	{
		MonoUtil::InvokeThunk(OnSelectionToggledThunk, GetScriptObject(), p0, p1);
	}

	void ScriptGUIListBox::RegisterEvents()
	{
		OnSelectionToggledConnection = static_cast<GUIListBox*>(GetNativeObject())->OnSelectionToggled.Connect([this](uint32_t p0, bool p1) { OnSelectionToggled(p0, p1); }));
		ScriptGUIClickableWrapperBase::RegisterEvents();
	}
	void ScriptGUIListBox::UnregisterEvents()
	{
		OnSelectionToggledConnection.Disconnect();
		ScriptGUIClickableWrapperBase::UnregisterEvents();
	}
	bool ScriptGUIListBox::InternalIsMultiselect(ScriptGUIListBox* self)
	{
		bool tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<GUIListBox*>(self->GetNativeObject())->IsMultiselect();

		bool __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptGUIListBox::InternalSetElements(ScriptGUIListBox* self, MonoArray* elements)
	{
		if(!self->IsNativeObjectValid())
			return;

		Vector<HString> nativeArrayelements;
		if(elements != nullptr)
		{
			ScriptArray scriptArrayelements(elements);
			nativeArrayelements.resize(scriptArrayelements.Size());
			for(int elementIndex = 0; elementIndex < (int)scriptArrayelements.Size(); elementIndex++)
			{
				SPtr<HString> arrayElementPointerelements;
				ScriptLocString* scriptObjectWrapperelements;
				scriptObjectWrapperelements = ScriptLocString::GetScriptObjectWrapper(scriptArrayelements.Get<MonoObject*>(elementIndex));
				if(scriptObjectWrapperelements != nullptr)
				{
					arrayElementPointerelements = std::static_pointer_cast<HString>(scriptObjectWrapperelements->GetBaseNativeObjectAsShared());
					if(arrayElementPointerelements)
						nativeArrayelements[elementIndex] = *arrayElementPointerelements;
				}
			}
		}
		static_cast<GUIListBox*>(self->GetNativeObject())->SetElements(nativeArrayelements);
	}

	void ScriptGUIListBox::InternalSelectElement(ScriptGUIListBox* self, uint32_t index)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<GUIListBox*>(self->GetNativeObject())->SelectElement(index);
	}

	void ScriptGUIListBox::InternalDeselectElement(ScriptGUIListBox* self, uint32_t index)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<GUIListBox*>(self->GetNativeObject())->DeselectElement(index);
	}

	uint32_t ScriptGUIListBox::InternalGetSelectedElementIndex(ScriptGUIListBox* self)
	{
		uint32_t tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<GUIListBox*>(self->GetNativeObject())->GetSelectedElementIndex();

		uint32_t __output;
		__output = tmp__output;

		return __output;
	}

	MonoArray* ScriptGUIListBox::InternalGetElementStates(ScriptGUIListBox* self)
	{
		Vector<bool> nativeArray__output;
		if(!self->IsNativeObjectValid())
			return {};

		nativeArray__output = static_cast<GUIListBox*>(self->GetNativeObject())->GetElementStates();

		MonoArray* __output;
		int elementCount__output = (int)nativeArray__output.size();
		ScriptArray scriptArray__output = ScriptArray::Create<bool>(elementCount__output);
		for(int elementIndex = 0; elementIndex < elementCount__output; elementIndex++)
		{
			scriptArray__output.Set(elementIndex, nativeArray__output[elementIndex]);
		}
		__output = scriptArray__output.GetInternal();

		return __output;
	}

	void ScriptGUIListBox::InternalSetElementStates(ScriptGUIListBox* self, MonoArray* states)
	{
		if(!self->IsNativeObjectValid())
			return;

		Vector<bool> nativeArraystates;
		if(states != nullptr)
		{
			ScriptArray scriptArraystates(states);
			nativeArraystates.resize(scriptArraystates.Size());
			for(int elementIndex = 0; elementIndex < (int)scriptArraystates.Size(); elementIndex++)
			{
				nativeArraystates[elementIndex] = scriptArraystates.Get<bool>(elementIndex);
			}
		}
		static_cast<GUIListBox*>(self->GetNativeObject())->SetElementStates(nativeArraystates);
	}

	void ScriptGUIListBox::InternalCreate(MonoObject* scriptObject, __GUIListBoxContentInterop* contents, MonoString* styleClass, MonoArray* options)
	{
		GUIListBoxContent tmpcontents;
		tmpcontents = ScriptGUIListBoxContent::FromInterop(*contents);
		String tmpstyleClass;
		tmpstyleClass = MonoUtil::MonoToString(styleClass);
		TInlineArray<GUIOption, 4> nativeArrayoptions;
		if(options != nullptr)
		{
			ScriptArray scriptArrayoptions(options);
			nativeArrayoptions.resize(scriptArrayoptions.Size());
			for(int elementIndex = 0; elementIndex < (int)scriptArrayoptions.Size(); elementIndex++)
			{
				nativeArrayoptions[elementIndex] = ScriptGUIOption::FromInterop(scriptArrayoptions.Get<__GUIOptionInterop>(elementIndex));
			}
		}
		GUIListBox* nativeObject = GUIListBox::Create(tmpcontents, tmpstyleClass, nativeArrayoptions);
		ScriptObjectWrapper::Create<ScriptGUIListBox>(nativeObject, scriptObject);
	}

	void ScriptGUIListBox::InternalCreate0(MonoObject* scriptObject, __GUIListBoxContentInterop* contents, MonoArray* options)
	{
		GUIListBoxContent tmpcontents;
		tmpcontents = ScriptGUIListBoxContent::FromInterop(*contents);
		TInlineArray<GUIOption, 4> nativeArrayoptions;
		if(options != nullptr)
		{
			ScriptArray scriptArrayoptions(options);
			nativeArrayoptions.resize(scriptArrayoptions.Size());
			for(int elementIndex = 0; elementIndex < (int)scriptArrayoptions.Size(); elementIndex++)
			{
				nativeArrayoptions[elementIndex] = ScriptGUIOption::FromInterop(scriptArrayoptions.Get<__GUIOptionInterop>(elementIndex));
			}
		}
		GUIListBox* nativeObject = GUIListBox::Create(tmpcontents, nativeArrayoptions);
		ScriptObjectWrapper::Create<ScriptGUIListBox>(nativeObject, scriptObject);
	}

	void ScriptGUIListBox::InternalCreate1(MonoObject* scriptObject, MonoString* styleClass, MonoArray* options)
	{
		String tmpstyleClass;
		tmpstyleClass = MonoUtil::MonoToString(styleClass);
		TInlineArray<GUIOption, 4> nativeArrayoptions;
		if(options != nullptr)
		{
			ScriptArray scriptArrayoptions(options);
			nativeArrayoptions.resize(scriptArrayoptions.Size());
			for(int elementIndex = 0; elementIndex < (int)scriptArrayoptions.Size(); elementIndex++)
			{
				nativeArrayoptions[elementIndex] = ScriptGUIOption::FromInterop(scriptArrayoptions.Get<__GUIOptionInterop>(elementIndex));
			}
		}
		GUIListBox* nativeObject = GUIListBox::Create(tmpstyleClass, nativeArrayoptions);
		ScriptObjectWrapper::Create<ScriptGUIListBox>(nativeObject, scriptObject);
	}

	void ScriptGUIListBox::InternalCreate2(MonoObject* scriptObject, MonoArray* options)
	{
		TInlineArray<GUIOption, 4> nativeArrayoptions;
		if(options != nullptr)
		{
			ScriptArray scriptArrayoptions(options);
			nativeArrayoptions.resize(scriptArrayoptions.Size());
			for(int elementIndex = 0; elementIndex < (int)scriptArrayoptions.Size(); elementIndex++)
			{
				nativeArrayoptions[elementIndex] = ScriptGUIOption::FromInterop(scriptArrayoptions.Get<__GUIOptionInterop>(elementIndex));
			}
		}
		GUIListBox* nativeObject = GUIListBox::Create(nativeArrayoptions);
		ScriptObjectWrapper::Create<ScriptGUIListBox>(nativeObject, scriptObject);
	}
}
