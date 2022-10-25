//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Wrappers/GUI/BsScriptGUIListBox.h"
#include "BsScriptMeta.h"
#include "BsMonoField.h"
#include "BsMonoClass.h"
#include "BsMonoMethod.h"
#include "BsMonoManager.h"
#include "Image/BsSpriteTexture.h"
#include "BsMonoUtil.h"
#include "GUI/BsGUILayout.h"
#include "GUI/BsGUIListBox.h"
#include "GUI/BsGUIOptions.h"
#include "Wrappers/GUI/BsScriptGUILayout.h"

#include "Generated/BsScriptHString.generated.h"
#include "Generated/BsScriptGUIElementStyle.generated.h"

using namespace std::placeholders;

namespace bs
{
	ScriptGUIListBox::OnSelectionChangedThunkDef ScriptGUIListBox::onSelectionChangedThunk;

	ScriptGUIListBox::ScriptGUIListBox(MonoObject* instance, GUIListBox* listBox)
		: TScriptGUIElement(instance, listBox)
	{
	}

	void ScriptGUIListBox::InitRuntimeData()
	{
		metaData.ScriptClass->AddInternalCall("Internal_CreateInstance", (void*)&ScriptGUIListBox::InternalCreateInstance);
		metaData.ScriptClass->AddInternalCall("Internal_SetElements", (void*)&ScriptGUIListBox::InternalSetElements);
		metaData.ScriptClass->AddInternalCall("Internal_SetTint", (void*)&ScriptGUIListBox::InternalSetTint);
		metaData.ScriptClass->AddInternalCall("Internal_SelectElement", (void*)&ScriptGUIListBox::InternalSelectElement);
		metaData.ScriptClass->AddInternalCall("Internal_DeselectElement", (void*)&ScriptGUIListBox::InternalDeselectElement);
		metaData.ScriptClass->AddInternalCall("Internal_GetElementStates", (void*)&ScriptGUIListBox::InternalGetElementStates);
		metaData.ScriptClass->AddInternalCall("Internal_SetElementStates", (void*)&ScriptGUIListBox::InternalSetElementStates);

		onSelectionChangedThunk = (OnSelectionChangedThunkDef)metaData.ScriptClass->GetMethod("DoOnSelectionChanged", 1)->GetThunk();
	}

	void ScriptGUIListBox::InternalCreateInstance(MonoObject* instance, MonoArray* elements, bool multiselect, MonoString* style, MonoArray* guiOptions)
	{
		GUIOptions options;

		ScriptArray scriptArray(guiOptions);
		u32 arrayLen = scriptArray.Size();
		for(u32 i = 0; i < arrayLen; i++)
			options.AddOption(scriptArray.Get<GUIOption>(i));

		ScriptArray elemsArray(elements);
		u32 elementsArrayLen = elemsArray.Size();
		Vector<HString> nativeElements;
		for(u32 i = 0; i < elementsArrayLen; i++)
		{
			MonoObject* stringManaged = elemsArray.Get<MonoObject*>(i);

			if(stringManaged == nullptr)
				nativeElements.push_back(HString::Dummy());
			else
			{
				ScriptHString* textScript = ScriptHString::ToNative(stringManaged);
				nativeElements.push_back(*textScript->GetInternal());
			}
		}

		GUIListBox* guiListBox = GUIListBox::Create(nativeElements, multiselect, options, MonoUtil::MonoToString(style));

		auto nativeInstance = new(bs_alloc<ScriptGUIListBox>()) ScriptGUIListBox(instance, guiListBox);

		guiListBox->OnSelectionToggled.Connect(std::bind(&::bs::ScriptGUIListBox::OnSelectionChanged, nativeInstance, _1, _2));
	}

	void ScriptGUIListBox::InternalSetElements(ScriptGUIListBox* nativeInstance, MonoArray* elements)
	{
		ScriptArray elemsArray(elements);
		u32 elementsArrayLen = elemsArray.Size();
		Vector<HString> nativeElements;
		for(u32 i = 0; i < elementsArrayLen; i++)
		{
			MonoObject* stringManaged = elemsArray.Get<MonoObject*>(i);

			if(stringManaged == nullptr)
				nativeElements.push_back(HString::Dummy());
			else
			{
				ScriptHString* textScript = ScriptHString::ToNative(stringManaged);
				nativeElements.push_back(*textScript->GetInternal());
			}
		}

		GUIListBox* listBox = (GUIListBox*)nativeInstance->GetGuiElement();
		listBox->SetElements(nativeElements);
	}

	void ScriptGUIListBox::InternalSetTint(ScriptGUIListBox* nativeInstance, Color* color)
	{
		GUIListBox* listBox = (GUIListBox*)nativeInstance->GetGuiElement();
		listBox->SetTint(*color);
	}

	void ScriptGUIListBox::InternalSelectElement(ScriptGUIListBox* nativeInstance, int idx)
	{
		GUIListBox* listBox = (GUIListBox*)nativeInstance->GetGuiElement();
		listBox->SelectElement(idx);
	}

	void ScriptGUIListBox::InternalDeselectElement(ScriptGUIListBox* nativeInstance, int idx)
	{
		GUIListBox* listBox = (GUIListBox*)nativeInstance->GetGuiElement();
		listBox->DeselectElement(idx);
	}

	MonoArray* ScriptGUIListBox::InternalGetElementStates(ScriptGUIListBox* nativeInstance)
	{
		GUIListBox* listBox = (GUIListBox*)nativeInstance->GetGuiElement();
		const Vector<bool>& states = listBox->GetElementStates();

		u32 numElements = (u32)states.size();
		ScriptArray outStates = ScriptArray::Create<bool>(numElements);

		for(u32 i = 0; i < numElements; i++)
			outStates.Set(i, (bool)states[i]);

		return outStates.GetInternal();
	}

	void ScriptGUIListBox::InternalSetElementStates(ScriptGUIListBox* nativeInstance, MonoArray* monoStates)
	{
		if(monoStates == nullptr)
			return;

		ScriptArray inStates(monoStates);
		u32 numElements = inStates.Size();

		Vector<bool> states(numElements);
		for(u32 i = 0; i < numElements; i++)
			states[i] = inStates.Get<bool>(i);

		GUIListBox* listBox = (GUIListBox*)nativeInstance->GetGuiElement();
		listBox->SetElementStates(states);
	}

	void ScriptGUIListBox::OnSelectionChanged(u32 index, bool enabled)
	{
		MonoUtil::InvokeThunk(onSelectionChangedThunk, GetManagedInstance(), index);
	}
} // namespace bs
