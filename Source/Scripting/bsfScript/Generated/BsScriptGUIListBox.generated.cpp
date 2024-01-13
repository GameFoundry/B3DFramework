//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptGUIListBox.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "../../../Foundation/bsfEngine/GUI/BsGUIListBox.h"
#include "BsScriptHString.generated.h"
#include "BsScriptGUIListBoxContent.generated.h"
#include "../../../Foundation/bsfEngine/GUI/BsGUIListBox.h"
#include "BsScriptGUIOption.generated.h"

namespace bs
{
	ScriptGUIListBox::OnSelectionToggledThunkDef ScriptGUIListBox::OnSelectionToggledThunk; 

	ScriptGUIListBox::ScriptGUIListBox(MonoObject* managedInstance, GUIListBox* value)
		:TScriptGUIInteractable(managedInstance, value)
	{
		RegisterEvents(value);
	}

	void ScriptGUIListBox::InitRuntimeData()
	{
		metaData.ScriptClass->AddInternalCall("Internal_IsMultiselect", (void*)&ScriptGUIListBox::InternalIsMultiselect);
		metaData.ScriptClass->AddInternalCall("Internal_SetElements", (void*)&ScriptGUIListBox::InternalSetElements);
		metaData.ScriptClass->AddInternalCall("Internal_SelectElement", (void*)&ScriptGUIListBox::InternalSelectElement);
		metaData.ScriptClass->AddInternalCall("Internal_DeselectElement", (void*)&ScriptGUIListBox::InternalDeselectElement);
		metaData.ScriptClass->AddInternalCall("Internal_GetElementStates", (void*)&ScriptGUIListBox::InternalGetElementStates);
		metaData.ScriptClass->AddInternalCall("Internal_SetElementStates", (void*)&ScriptGUIListBox::InternalSetElementStates);
		metaData.ScriptClass->AddInternalCall("Internal_Create", (void*)&ScriptGUIListBox::InternalCreate);
		metaData.ScriptClass->AddInternalCall("Internal_Create0", (void*)&ScriptGUIListBox::InternalCreate0);
		metaData.ScriptClass->AddInternalCall("Internal_Create1", (void*)&ScriptGUIListBox::InternalCreate1);
		metaData.ScriptClass->AddInternalCall("Internal_Create2", (void*)&ScriptGUIListBox::InternalCreate2);

		OnSelectionToggledThunk = (OnSelectionToggledThunkDef)metaData.ScriptClass->GetMethodExact("Internal_OnSelectionToggled", "int,bool")->GetThunk();
	}

	void ScriptGUIListBox::OnSelectionToggled(uint32_t p0, bool p1)
	{
		MonoUtil::InvokeThunk(OnSelectionToggledThunk, GetManagedInstance(), p0, p1);
	}

	void ScriptGUIListBox::RegisterEvents(GUIElement* value)
	{
		static_cast<GUIListBox*>(value)->OnSelectionToggled.Connect(std::bind(&ScriptGUIListBox::OnSelectionToggled, this, std::placeholders::_1, std::placeholders::_2));
		ScriptGUIClickableBase::RegisterEvents(value);
	}
	bool ScriptGUIListBox::InternalIsMultiselect(ScriptGUIListBox* thisPtr)
	{
		bool tmp__output;
		tmp__output = static_cast<GUIListBox*>(thisPtr->GetGuiElement())->IsMultiselect();

		bool __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptGUIListBox::InternalSetElements(ScriptGUIListBox* thisPtr, MonoArray* elements)
	{
		Vector<HString> vecelements;
		if(elements != nullptr)
		{
			ScriptArray arrayelements(elements);
			vecelements.resize(arrayelements.Size());
			for(int i = 0; i < (int)arrayelements.Size(); i++)
			{
				ScriptLocString* scriptelements;
				scriptelements = ScriptLocString::ToNative(arrayelements.Get<MonoObject*>(i));
				if(scriptelements != nullptr)
				{
					SPtr<HString> arrayElemPtrelements = scriptelements->GetInternal();
					if(arrayElemPtrelements)
						vecelements[i] = *arrayElemPtrelements;
				}
			}
		}
		static_cast<GUIListBox*>(thisPtr->GetGuiElement())->SetElements(vecelements);
	}

	void ScriptGUIListBox::InternalSelectElement(ScriptGUIListBox* thisPtr, uint32_t index)
	{
		static_cast<GUIListBox*>(thisPtr->GetGuiElement())->SelectElement(index);
	}

	void ScriptGUIListBox::InternalDeselectElement(ScriptGUIListBox* thisPtr, uint32_t index)
	{
		static_cast<GUIListBox*>(thisPtr->GetGuiElement())->DeselectElement(index);
	}

	MonoArray* ScriptGUIListBox::InternalGetElementStates(ScriptGUIListBox* thisPtr)
	{
		Vector<bool> vec__output;
		vec__output = static_cast<GUIListBox*>(thisPtr->GetGuiElement())->GetElementStates();

		MonoArray* __output;
		int arraySize__output = (int)vec__output.size();
		ScriptArray array__output = ScriptArray::Create<bool>(arraySize__output);
		for(int i = 0; i < arraySize__output; i++)
		{
			array__output.Set(i, vec__output[i]);
		}
		__output = array__output.GetInternal();

		return __output;
	}

	void ScriptGUIListBox::InternalSetElementStates(ScriptGUIListBox* thisPtr, MonoArray* states)
	{
		Vector<bool> vecstates;
		if(states != nullptr)
		{
			ScriptArray arraystates(states);
			vecstates.resize(arraystates.Size());
			for(int i = 0; i < (int)arraystates.Size(); i++)
			{
				vecstates[i] = arraystates.Get<bool>(i);
			}
		}
		static_cast<GUIListBox*>(thisPtr->GetGuiElement())->SetElementStates(vecstates);
	}

	void ScriptGUIListBox::InternalCreate(MonoObject* managedInstance, __GUIListBoxContentInterop* contents, MonoString* styleClass, MonoArray* options)
	{
		GUIListBoxContent tmpcontents;
		tmpcontents = ScriptGUIListBoxContent::FromInterop(*contents);
		String tmpstyleClass;
		tmpstyleClass = MonoUtil::MonoToString(styleClass);
		TInlineArray<GUIOption, 4> vecoptions;
		if(options != nullptr)
		{
			ScriptArray arrayoptions(options);
			vecoptions.resize(arrayoptions.Size());
			for(int i = 0; i < (int)arrayoptions.Size(); i++)
			{
				vecoptions[i] = arrayoptions.Get<GUIOption>(i);
			}
		}
		GUIListBox* instance = GUIListBox::Create(tmpcontents, tmpstyleClass, vecoptions);
		new (B3DAllocate<ScriptGUIListBox>())ScriptGUIListBox(managedInstance, instance);
	}

	void ScriptGUIListBox::InternalCreate0(MonoObject* managedInstance, __GUIListBoxContentInterop* contents, MonoArray* options)
	{
		GUIListBoxContent tmpcontents;
		tmpcontents = ScriptGUIListBoxContent::FromInterop(*contents);
		TInlineArray<GUIOption, 4> vecoptions;
		if(options != nullptr)
		{
			ScriptArray arrayoptions(options);
			vecoptions.resize(arrayoptions.Size());
			for(int i = 0; i < (int)arrayoptions.Size(); i++)
			{
				vecoptions[i] = arrayoptions.Get<GUIOption>(i);
			}
		}
		GUIListBox* instance = GUIListBox::Create(tmpcontents, vecoptions);
		new (B3DAllocate<ScriptGUIListBox>())ScriptGUIListBox(managedInstance, instance);
	}

	void ScriptGUIListBox::InternalCreate1(MonoObject* managedInstance, MonoString* styleClass, MonoArray* options)
	{
		String tmpstyleClass;
		tmpstyleClass = MonoUtil::MonoToString(styleClass);
		TInlineArray<GUIOption, 4> vecoptions;
		if(options != nullptr)
		{
			ScriptArray arrayoptions(options);
			vecoptions.resize(arrayoptions.Size());
			for(int i = 0; i < (int)arrayoptions.Size(); i++)
			{
				vecoptions[i] = arrayoptions.Get<GUIOption>(i);
			}
		}
		GUIListBox* instance = GUIListBox::Create(tmpstyleClass, vecoptions);
		new (B3DAllocate<ScriptGUIListBox>())ScriptGUIListBox(managedInstance, instance);
	}

	void ScriptGUIListBox::InternalCreate2(MonoObject* managedInstance, MonoArray* options)
	{
		TInlineArray<GUIOption, 4> vecoptions;
		if(options != nullptr)
		{
			ScriptArray arrayoptions(options);
			vecoptions.resize(arrayoptions.Size());
			for(int i = 0; i < (int)arrayoptions.Size(); i++)
			{
				vecoptions[i] = arrayoptions.Get<GUIOption>(i);
			}
		}
		GUIListBox* instance = GUIListBox::Create(vecoptions);
		new (B3DAllocate<ScriptGUIListBox>())ScriptGUIListBox(managedInstance, instance);
	}
}
