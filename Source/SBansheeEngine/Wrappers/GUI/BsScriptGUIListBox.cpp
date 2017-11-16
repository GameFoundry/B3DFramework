//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "Wrappers/GUI/BsScriptGUIListBox.h"
#include "BsScriptMeta.h"
#include "BsMonoField.h"
#include "BsMonoClass.h"
#include "BsMonoMethod.h"
#include "BsMonoManager.h"
#include "2D/BsSpriteTexture.h"
#include "BsMonoUtil.h"
#include "GUI/BsGUILayout.h"
#include "GUI/BsGUIListBox.h"
#include "GUI/BsGUIOptions.h"
#include "Wrappers/GUI/BsScriptGUIElementStyle.h"
#include "Wrappers/GUI/BsScriptGUILayout.h"
#include "Wrappers/BsScriptHString.h"

using namespace std::placeholders;

namespace bs
{
	ScriptGUIListBox::OnSelectionChangedThunkDef ScriptGUIListBox::onSelectionChangedThunk;

	ScriptGUIListBox::ScriptGUIListBox(MonoObject* instance, GUIListBox* listBox)
		:TScriptGUIElement(instance, listBox)
	{

	}

	void ScriptGUIListBox::initRuntimeData()
	{
		metaData.scriptClass->addInternalCall("Internal_CreateInstance", (void*)&ScriptGUIListBox::internal_createInstance);
		metaData.scriptClass->addInternalCall("Internal_SetElements", (void*)&ScriptGUIListBox::internal_setElements);
		metaData.scriptClass->addInternalCall("Internal_SetTint", (void*)&ScriptGUIListBox::internal_setTint);
		metaData.scriptClass->addInternalCall("Internal_SelectElement", (void*)&ScriptGUIListBox::internal_selectElement);
		metaData.scriptClass->addInternalCall("Internal_DeselectElement", (void*)&ScriptGUIListBox::internal_deselectElement);
		metaData.scriptClass->addInternalCall("Internal_GetElementStates", (void*)&ScriptGUIListBox::internal_getElementStates);
		metaData.scriptClass->addInternalCall("Internal_SetElementStates", (void*)&ScriptGUIListBox::internal_setElementStates);

		onSelectionChangedThunk = (OnSelectionChangedThunkDef)metaData.scriptClass->getMethod("DoOnSelectionChanged", 1)->getThunk();
	}

	void ScriptGUIListBox::internal_createInstance(MonoObject* instance, MonoArray* elements, bool multiselect,
		MonoString* style, MonoArray* guiOptions)
	{
		GUIOptions options;

		ScriptArray scriptArray(guiOptions);
		UINT32 arrayLen = scriptArray.size();
		for (UINT32 i = 0; i < arrayLen; i++)
			options.addOption(scriptArray.get<GUIOption>(i));

		ScriptArray elemsArray(elements);
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
				nativeElements.push_back(textScript->getInternalValue());
			}
		}

		GUIListBox* guiListBox = GUIListBox::create(nativeElements, multiselect, options, toString(MonoUtil::monoToWString(style)));

		auto nativeInstance = new (bs_alloc<ScriptGUIListBox>()) ScriptGUIListBox(instance, guiListBox);

		guiListBox->onSelectionToggled.connect(std::bind(&ScriptGUIListBox::onSelectionChanged, nativeInstance, _1, _2));
	}

	void ScriptGUIListBox::internal_setElements(ScriptGUIListBox* nativeInstance, MonoArray* elements)
	{
		ScriptArray elemsArray(elements);
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
				nativeElements.push_back(textScript->getInternalValue());
			}
		}

		GUIListBox* listBox = (GUIListBox*)nativeInstance->getGUIElement();
		listBox->setElements(nativeElements);
	}

	void ScriptGUIListBox::internal_setTint(ScriptGUIListBox* nativeInstance, Color* color)
	{
		GUIListBox* listBox = (GUIListBox*)nativeInstance->getGUIElement();
		listBox->setTint(*color);
	}

	void ScriptGUIListBox::internal_selectElement(ScriptGUIListBox* nativeInstance, int idx)
	{
		GUIListBox* listBox = (GUIListBox*)nativeInstance->getGUIElement();
		listBox->selectElement(idx);
	}

	void ScriptGUIListBox::internal_deselectElement(ScriptGUIListBox* nativeInstance, int idx)
	{
		GUIListBox* listBox = (GUIListBox*)nativeInstance->getGUIElement();
		listBox->deselectElement(idx);
	}

	MonoArray* ScriptGUIListBox::internal_getElementStates(ScriptGUIListBox* nativeInstance)
	{
		GUIListBox* listBox = (GUIListBox*)nativeInstance->getGUIElement();
		const Vector<bool>& states = listBox->getElementStates();

		UINT32 numElements = (UINT32)states.size();
		ScriptArray outStates = ScriptArray::create<bool>(numElements);

		for (UINT32 i = 0; i < numElements; i++)
			outStates.set(i, states[i]);

		return outStates.getInternal();
	}

	void ScriptGUIListBox::internal_setElementStates(ScriptGUIListBox* nativeInstance, MonoArray* monoStates)
	{
		if (monoStates == nullptr)
			return;

		ScriptArray inStates(monoStates);
		UINT32 numElements = inStates.size();

		Vector<bool> states(numElements);
		for (UINT32 i = 0; i < numElements; i++)
			states[i] = inStates.get<bool>(i);

		GUIListBox* listBox = (GUIListBox*)nativeInstance->getGUIElement();
		listBox->setElementStates(states);
	}

	void ScriptGUIListBox::onSelectionChanged(UINT32 index, bool enabled)
	{
		MonoUtil::invokeThunk(onSelectionChangedThunk, getManagedInstance(), index);
	}
}