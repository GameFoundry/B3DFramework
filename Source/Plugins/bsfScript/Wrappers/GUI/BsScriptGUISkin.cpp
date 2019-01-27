//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Wrappers/GUI/BsScriptGUISkin.h"
#include "BsScriptMeta.h"
#include "BsMonoField.h"
#include "BsMonoClass.h"
#include "BsMonoManager.h"
#include "BsMonoUtil.h"
#include "GUI/BsGUISkin.h"
#include "BsScriptResourceManager.h"
#include "Wrappers/GUI/BsScriptGUIElementStyle.h"

namespace bs
{
	ScriptGUISkin::ScriptGUISkin(MonoObject* instance, const HGUISkin& skin)
		:TScriptResource(instance, skin)
	{ }

	void ScriptGUISkin::initRuntimeData()
	{
		metaData.scriptClass->addInternalCall("Internal_CreateInstance", (void*)&ScriptGUISkin::internal_CreateInstance);

		metaData.scriptClass->addInternalCall("Internal_GetStyle", (void*)&ScriptGUISkin::internal_GetStyle);
		metaData.scriptClass->addInternalCall("Internal_SetStyle", (void*)&ScriptGUISkin::internal_SetStyle);

		metaData.scriptClass->addInternalCall("Internal_HasStyle", (void*)&ScriptGUISkin::internal_HasStyle);
		metaData.scriptClass->addInternalCall("Internal_RemoveStyle", (void*)&ScriptGUISkin::internal_RemoveStyle);
		metaData.scriptClass->addInternalCall("Internal_GetStyleNames", (void*)&ScriptGUISkin::internal_GetStyleNames);
	}

	void ScriptGUISkin::internal_CreateInstance(MonoObject* instance)
	{
		HGUISkin skin = GUISkin::create();

		ScriptResourceManager::instance().createBuiltinScriptResource(skin, instance);
	}

	bool ScriptGUISkin::internal_HasStyle(ScriptGUISkin* thisPtr, MonoString* name)
	{
		String nativeName = MonoUtil::monoToString(name);

		return thisPtr->getHandle()->hasStyle(nativeName);
	}


	MonoObject* ScriptGUISkin::internal_GetStyle(ScriptGUISkin* thisPtr, MonoString* name)
	{
		HGUISkin skin = thisPtr->getHandle();
		String nativeName = MonoUtil::monoToString(name);

		const GUIElementStyle* style = skin->getStyle(nativeName);
		if (style == nullptr)
			return nullptr;
		
		return ScriptGUIElementStyle::create(*style);
	}

	void ScriptGUISkin::internal_SetStyle(ScriptGUISkin* thisPtr, MonoString* name, ScriptGUIElementStyle* style)
	{
		HGUISkin skin = thisPtr->getHandle();
		String nativeName = MonoUtil::monoToString(name);

		if (style != nullptr)
			skin->setStyle(nativeName, style->getInternalValue());
	}

	void ScriptGUISkin::internal_RemoveStyle(ScriptGUISkin* thisPtr, MonoString* name)
	{
		HGUISkin skin = thisPtr->getHandle();

		String nativeName = MonoUtil::monoToString(name);
		skin->removeStyle(nativeName);
	}

	MonoArray* ScriptGUISkin::internal_GetStyleNames(ScriptGUISkin* thisPtr)
	{
		HGUISkin skin = thisPtr->getHandle();

		Vector<String> styleNames = skin->getStyleNames();
		UINT32 numNames = (UINT32)styleNames.size();

		ScriptArray output = ScriptArray::create<String>(numNames);
		for (UINT32 i = 0; i < numNames; i++)
			output.set(i, styleNames[i]);

		return output.getInternal();
	}

	MonoObject* ScriptGUISkin::createInstance()
	{
		bool dummy = false;

		void* params[1];
		params[0] = &dummy;

		return metaData.scriptClass->createInstance("bool", params);
	}

	MonoObject* ScriptGUISkin::_createManagedInstance(bool construct)
	{
		if (construct)
			return createInstance();
		
		return metaData.scriptClass->createInstance(false);
	}
}