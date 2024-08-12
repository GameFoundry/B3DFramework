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
		metaData.ScriptClass->AddInternalCall("Internal_GetSelectedElementIndex", (void*)&ScriptGUIListBox::InternalGetSelectedElementIndex);
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
		Vector<HString> nativeArrayelements;
		if(elements != nullptr)
		{
			ScriptArray scriptArrayelements(elements);
			nativeArrayelements.resize(scriptArrayelements.Size());
			for(int elementIndex = 0; elementIndex < (int)scriptArrayelements.Size(); elementIndex++)
			{
				ScriptLocString* scriptObjectWrapperelements;
				scriptObjectWrapperelements = ScriptLocString::ToNative(scriptArrayelements.Get<MonoObject*>(elementIndex));
				if(scriptObjectWrapperelements != nullptr)
				{
					SPtr<HString> arrayElementPointerelements = scriptObjectWrapperelements->GetInternal();
					if(arrayElementPointerelements)
						nativeArrayelements[elementIndex] = *arrayElementPointerelements;
				}
			}
		}
		static_cast<GUIListBox*>(thisPtr->GetGuiElement())->SetElements(nativeArrayelements);
	}

	void ScriptGUIListBox::InternalSelectElement(ScriptGUIListBox* thisPtr, uint32_t index)
	{
		static_cast<GUIListBox*>(thisPtr->GetGuiElement())->SelectElement(index);
	}

	void ScriptGUIListBox::InternalDeselectElement(ScriptGUIListBox* thisPtr, uint32_t index)
	{
		static_cast<GUIListBox*>(thisPtr->GetGuiElement())->DeselectElement(index);
	}

	uint32_t ScriptGUIListBox::InternalGetSelectedElementIndex(ScriptGUIListBox* thisPtr)
	{
		uint32_t tmp__output;
		tmp__output = static_cast<GUIListBox*>(thisPtr->GetGuiElement())->GetSelectedElementIndex();

		uint32_t __output;
		__output = tmp__output;

		return __output;
	}

	MonoArray* ScriptGUIListBox::InternalGetElementStates(ScriptGUIListBox* thisPtr)
	{
		Vector<bool> nativeArray__output;
		nativeArray__output = static_cast<GUIListBox*>(thisPtr->GetGuiElement())->GetElementStates();

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

	void ScriptGUIListBox::InternalSetElementStates(ScriptGUIListBox* thisPtr, MonoArray* states)
	{
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
		static_cast<GUIListBox*>(thisPtr->GetGuiElement())->SetElementStates(nativeArraystates);
	}

	void ScriptGUIListBox::InternalCreate(MonoObject* managedInstance, __GUIListBoxContentInterop* contents, MonoString* styleClass, MonoArray* options)
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
				nativeArrayoptions[elementIndex] = scriptArrayoptions.Get<GUIOption>(elementIndex);
			}
		}
		GUIListBox* instance = GUIListBox::Create(tmpcontents, tmpstyleClass, nativeArrayoptions);
		new (B3DAllocate<ScriptGUIListBox>())ScriptGUIListBox(managedInstance, instance);
	}

	void ScriptGUIListBox::InternalCreate0(MonoObject* managedInstance, __GUIListBoxContentInterop* contents, MonoArray* options)
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
				nativeArrayoptions[elementIndex] = scriptArrayoptions.Get<GUIOption>(elementIndex);
			}
		}
		GUIListBox* instance = GUIListBox::Create(tmpcontents, nativeArrayoptions);
		new (B3DAllocate<ScriptGUIListBox>())ScriptGUIListBox(managedInstance, instance);
	}

	void ScriptGUIListBox::InternalCreate1(MonoObject* managedInstance, MonoString* styleClass, MonoArray* options)
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
				nativeArrayoptions[elementIndex] = scriptArrayoptions.Get<GUIOption>(elementIndex);
			}
		}
		GUIListBox* instance = GUIListBox::Create(tmpstyleClass, nativeArrayoptions);
		new (B3DAllocate<ScriptGUIListBox>())ScriptGUIListBox(managedInstance, instance);
	}

	void ScriptGUIListBox::InternalCreate2(MonoObject* managedInstance, MonoArray* options)
	{
		TInlineArray<GUIOption, 4> nativeArrayoptions;
		if(options != nullptr)
		{
			ScriptArray scriptArrayoptions(options);
			nativeArrayoptions.resize(scriptArrayoptions.Size());
			for(int elementIndex = 0; elementIndex < (int)scriptArrayoptions.Size(); elementIndex++)
			{
				nativeArrayoptions[elementIndex] = scriptArrayoptions.Get<GUIOption>(elementIndex);
			}
		}
		GUIListBox* instance = GUIListBox::Create(nativeArrayoptions);
		new (B3DAllocate<ScriptGUIListBox>())ScriptGUIListBox(managedInstance, instance);
	}
}
