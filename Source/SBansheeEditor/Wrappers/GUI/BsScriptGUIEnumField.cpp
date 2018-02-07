//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "Wrappers/GUI/BsScriptGUIEnumField.h"
#include "BsScriptMeta.h"
#include "BsMonoField.h"
#include "BsMonoClass.h"
#include "BsMonoManager.h"
#include "BsMonoMethod.h"
#include "BsMonoUtil.h"
#include "GUI/BsGUIListBoxField.h"
#include "GUI/BsGUIOptions.h"
#include "GUI/BsGUIContent.h"
#include "Wrappers/BsScriptHString.h"
#include "Wrappers/GUI/BsScriptGUIContent.h"

using namespace std::placeholders;

namespace bs
{
	ScriptGUIEnumField::OnSelectionChangedThunkDef ScriptGUIEnumField::onSelectionChangedThunk;

	ScriptGUIEnumField::ScriptGUIEnumField(MonoObject* instance, GUIListBoxField* listBoxField, const Vector<UINT64>& values)
		:TScriptGUIElement(instance, listBoxField), mValues(values)
	{

	}

	void ScriptGUIEnumField::initRuntimeData()
	{
		metaData.scriptClass->addInternalCall("Internal_CreateInstance", (void*)&ScriptGUIEnumField::internal_createInstance);
		metaData.scriptClass->addInternalCall("Internal_GetValue", (void*)&ScriptGUIEnumField::internal_getValue);
		metaData.scriptClass->addInternalCall("Internal_SetValue", (void*)&ScriptGUIEnumField::internal_setValue);
		metaData.scriptClass->addInternalCall("Internal_SetTint", (void*)&ScriptGUIEnumField::internal_setTint);
		metaData.scriptClass->addInternalCall("Internal_SelectElement", (void*)&ScriptGUIEnumField::internal_selectElement);
		metaData.scriptClass->addInternalCall("Internal_DeselectElement", (void*)&ScriptGUIEnumField::internal_deselectElement);
		metaData.scriptClass->addInternalCall("Internal_GetElementStates", (void*)&ScriptGUIEnumField::internal_getElementStates);
		metaData.scriptClass->addInternalCall("Internal_SetElementStates", (void*)&ScriptGUIEnumField::internal_setElementStates);

		onSelectionChangedThunk = (OnSelectionChangedThunkDef)metaData.scriptClass->getMethod("DoOnSelectionChanged", 1)->getThunk();
	}

	void ScriptGUIEnumField::internal_createInstance(MonoObject* instance, MonoArray* names, MonoArray* values, 
		bool multiselect, MonoObject* title, UINT32 titleWidth, MonoString* style, MonoArray* guiOptions, bool withTitle)
	{
		GUIOptions options;

		ScriptArray scriptArray(guiOptions);
		UINT32 arrayLen = scriptArray.size();
		for (UINT32 i = 0; i < arrayLen; i++)
			options.addOption(scriptArray.get<GUIOption>(i));

		String styleName = toString(MonoUtil::monoToWString(style));

		ScriptArray namesArr(names);
		Vector<HString> nativeNames;
		for (UINT32 i = 0; i < namesArr.size(); i++)
			nativeNames.push_back(HString(namesArr.get<WString>(i)));

		GUIListBoxField* guiField = nullptr;
		if (withTitle)
		{
			GUIContent nativeContent(ScriptGUIContent::getText(title), ScriptGUIContent::getImage(title), ScriptGUIContent::getTooltip(title));
			guiField = GUIListBoxField::create(nativeNames, multiselect, nativeContent, titleWidth, options, styleName);
		}
		else
		{
			guiField = GUIListBoxField::create(nativeNames, multiselect, options, styleName);
		}

		ScriptArray valuesArr(values);
		UINT32 elemSize = valuesArr.elementSize();

		Vector<UINT64> nativeValues;
		for (UINT32 i = 0; i < valuesArr.size(); i++)
		{
			UINT64 nativeValue = 0;

			switch (elemSize)
			{
			case sizeof(UINT8):
			{
				UINT8 value = 0;
				memcpy(&value, valuesArr.getRaw(i, elemSize), elemSize);
				nativeValue = (UINT64)value;
				break;
			}
			case sizeof(UINT16) :
			{
				UINT16 value = 0;
				memcpy(&value, valuesArr.getRaw(i, elemSize), elemSize);
				nativeValue = (UINT64)value;
				break;
			}
			case sizeof(UINT32):
			{
				UINT32 value = 0;
				memcpy(&value, valuesArr.getRaw(i, elemSize), elemSize);
				nativeValue = (UINT64)value;
				break;
			}
			case sizeof(UINT64) :
			{
				memcpy(&nativeValue, valuesArr.getRaw(i, elemSize), elemSize);
				break;
			}
			}

			nativeValues.push_back(nativeValue);
		}
			

		auto nativeInstance = new (bs_alloc<ScriptGUIEnumField>()) ScriptGUIEnumField(instance, guiField, nativeValues);

		guiField->onSelectionChanged.connect(std::bind(&ScriptGUIEnumField::onSelectionChanged, nativeInstance, _1, _2));

	}

	UINT64 ScriptGUIEnumField::internal_getValue(ScriptGUIEnumField* nativeInstance)
	{
		GUIListBoxField* field = static_cast<GUIListBoxField*>(nativeInstance->getGUIElement());

		UINT32 outValue = 0;

		const Vector<UINT64>& values = nativeInstance->mValues;
		Vector<bool> states = field->getElementStates();
		for (UINT32 i = 0; i < (UINT32)values.size(); i++)
			outValue |= states[i] ? values[i] : 0;

		return outValue;
	}

	void ScriptGUIEnumField::internal_setValue(ScriptGUIEnumField* nativeInstance, UINT64 value)
	{
		GUIListBoxField* field = static_cast<GUIListBoxField*>(nativeInstance->getGUIElement());

		const Vector<UINT64>& values = nativeInstance->mValues;
		Vector<bool> states = field->getElementStates();

		if (field->isMultiselect())
		{
			for (UINT32 i = 0; i < (UINT32)values.size(); i++)
				states[i] = (values[i] & value) == values[i];
		}
		else
		{
			for (UINT32 i = 0; i < (UINT32)values.size(); i++)
				states[i] = values[i] == value;
		}

		field->setElementStates(states);
	}

	void ScriptGUIEnumField::internal_selectElement(ScriptGUIEnumField* nativeInstance, int idx)
	{
		GUIListBoxField* listBox = (GUIListBoxField*)nativeInstance->getGUIElement();
		listBox->selectElement(idx);
	}

	void ScriptGUIEnumField::internal_deselectElement(ScriptGUIEnumField* nativeInstance, int idx)
	{
		GUIListBoxField* listBox = (GUIListBoxField*)nativeInstance->getGUIElement();
		listBox->deselectElement(idx);
	}

	MonoArray* ScriptGUIEnumField::internal_getElementStates(ScriptGUIEnumField* nativeInstance)
	{
		GUIListBoxField* listBox = (GUIListBoxField*)nativeInstance->getGUIElement();
		const Vector<bool>& states = listBox->getElementStates();

		UINT32 numElements = (UINT32)states.size();
		ScriptArray outStates = ScriptArray::create<bool>(numElements);

		for (UINT32 i = 0; i < numElements; i++)
			outStates.set(i, states[i]);

		return outStates.getInternal();
	}

	void ScriptGUIEnumField::internal_setElementStates(ScriptGUIEnumField* nativeInstance, MonoArray* monoStates)
	{
		if (monoStates == nullptr)
			return;

		ScriptArray inStates(monoStates);
		UINT32 numElements = inStates.size();

		Vector<bool> states(numElements);
		for (UINT32 i = 0; i < numElements; i++)
			states[i] = inStates.get<bool>(i);

		GUIListBoxField* listBox = (GUIListBoxField*)nativeInstance->getGUIElement();
		listBox->setElementStates(states);
	}

	void ScriptGUIEnumField::internal_setTint(ScriptGUIEnumField* nativeInstance, Color* color)
	{
		GUIListBoxField* field = (GUIListBoxField*)nativeInstance->getGUIElement();
		field->setTint(*color);
	}

	void ScriptGUIEnumField::onSelectionChanged(UINT64 newIndex, bool enabled)
	{
		GUIListBoxField* field = static_cast<GUIListBoxField*>(getGUIElement());

		UINT32 outValue = 0;

		Vector<bool> states = field->getElementStates();
		for (UINT32 i = 0; i < (UINT32)mValues.size(); i++)
			outValue |= states[i] ? mValues[i] : 0;

		MonoUtil::invokeThunk(onSelectionChangedThunk, getManagedInstance(), outValue);
	}
}