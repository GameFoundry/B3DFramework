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
		:TScriptGUIElement(instance, listBox)
	{

	}

	void ScriptGUIListBox::InitRuntimeData()
	{
		metaData.scriptClass->AddInternalCall("Internal_CreateInstance", (void*)&ScriptGUIListBox::internal_createInstance);
		metaData.scriptClass->AddInternalCall("Internal_SetElements", (void*)&ScriptGUIListBox::internal_setElements);
		metaData.scriptClass->AddInternalCall("Internal_SetTint", (void*)&ScriptGUIListBox::internal_setTint);
		metaData.scriptClass->AddInternalCall("Internal_SelectElement", (void*)&ScriptGUIListBox::internal_selectElement);
		metaData.scriptClass->AddInternalCall("Internal_DeselectElement", (void*)&ScriptGUIListBox::internal_deselectElement);
		metaData.scriptClass->AddInternalCall("Internal_GetElementStates", (void*)&ScriptGUIListBox::internal_getElementStates);
		metaData.scriptClass->AddInternalCall("Internal_SetElementStates", (void*)&ScriptGUIListBox::internal_setElementStates);

		onSelectionChangedThunk = (OnSelectionChangedThunkDef)metaData.scriptClass->GetMethod("DoOnSelectionChanged", 1)->getThunk();
	}

	void ScriptGUIListBox::internal_createInstance(MonoObject* instance, MonoArray* elements, bool multiselect,
		MonoString* style, MonoArray* guiOptions)
	{
		GUIOptions options;

		ScriptArray ScriptArray(guiOptions);
		UINT32 arrayLen = scriptArray.size();
		for (UINT32 i = 0; i < arrayLen; i++)
			options.AddOption(scriptArray.get<GUIOption>(i));

		ScriptArray ElemsArray(elements);
		UINT32 elementsArrayLen = elemsArray.size();
		Vector<HString> nativeElements;
		for(UINT32 i = 0; i < elementsArrayLen; i++)
		{
			MonoObject* stringManaged = elemsArray.get<MonoObject*>(i);

			if(stringManaged == nullptr)
				nativeElements.push_back(HString::dummy());
			else
			{
				ScriptHString* textScript = ScriptHString::toNative(stringManaged);
				nativeElements.push_back(*textScript->GetInternal());
			}
		}

		GUIListBox* guiListBox = GUIListBox::create(nativeElements, multiselect, options, MonoUtil::monoToString(style));

		auto nativeInstance = new (bs_alloc<ScriptGUIListBox>()) ScriptGUIListBox(instance, guiListBox);

		guiListBox->onSelectionToggled.Connect(std::bind(&ScriptGUIListBox::onSelectionChanged, nativeInstance, _1, _2));
	}

	void ScriptGUIListBox::internal_setElements(ScriptGUIListBox* nativeInstance, MonoArray* elements)
	{
		ScriptArray ElemsArray(elements);
		UINT32 elementsArrayLen = elemsArray.size();
		Vector<HString> nativeElements;
		for(UINT32 i = 0; i < elementsArrayLen; i++)
		{
			MonoObject* stringManaged = elemsArray.get<MonoObject*>(i);

			if(stringManaged == nullptr)
				nativeElements.push_back(HString::dummy());
			else
			{
				ScriptHString* textScript = ScriptHString::toNative(stringManaged);
				nativeElements.push_back(*textScript->GetInternal());
			}
		}

		GUIListBox* listBox = (GUIListBox*)nativeInstance->GetGUIElement();
		listBox->SetElements(nativeElements);
	}

	void ScriptGUIListBox::internal_setTint(ScriptGUIListBox* nativeInstance, Color* color)
	{
		GUIListBox* listBox = (GUIListBox*)nativeInstance->GetGUIElement();
		listBox->SetTint(*color);
	}

	void ScriptGUIListBox::internal_selectElement(ScriptGUIListBox* nativeInstance, int idx)
	{
		GUIListBox* listBox = (GUIListBox*)nativeInstance->GetGUIElement();
		listBox->SelectElement(idx);
	}

	void ScriptGUIListBox::internal_deselectElement(ScriptGUIListBox* nativeInstance, int idx)
	{
		GUIListBox* listBox = (GUIListBox*)nativeInstance->GetGUIElement();
		listBox->DeselectElement(idx);
	}

	MonoArray* ScriptGUIListBox::internal_getElementStates(ScriptGUIListBox* nativeInstance)
	{
		GUIListBox* listBox = (GUIListBox*)nativeInstance->GetGUIElement();
		const Vector<bool>& states = listBox->GetElementStates();

		UINT32 numElements = (UINT32)states.size();
		ScriptArray outStates = ScriptArray::create<bool>(numElements);

		for (UINT32 i = 0; i < numElements; i++)
			outStates.Set(i, (bool)states[i]);

		return outStates.GetInternal();
	}

	void ScriptGUIListBox::internal_setElementStates(ScriptGUIListBox* nativeInstance, MonoArray* monoStates)
	{
		if (monoStates == nullptr)
			return;

		ScriptArray InStates(monoStates);
		UINT32 numElements = inStates.size();

		Vector<bool> States(numElements);
		for (UINT32 i = 0; i < numElements; i++)
			states[i] = inStates.get<bool>(i);

		GUIListBox* listBox = (GUIListBox*)nativeInstance->GetGUIElement();
		listBox->SetElementStates(states);
	}

	void ScriptGUIListBox::OnSelectionChanged(UINT32 index, bool enabled)
	{
		MonoUtil::invokeThunk(onSelectionChangedThunk, getManagedInstance(), index);
	}
}
