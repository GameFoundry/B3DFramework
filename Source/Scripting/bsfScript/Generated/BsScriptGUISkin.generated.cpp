//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptGUISkin.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "../../../Foundation/bsfEngine/GUI/BsGUISkin.h"
#include "BsScriptResourceManager.h"
#include "Wrappers/BsScriptRRefBase.h"
#include "BsScriptGUIElementStyle.generated.h"
#include "../../../Foundation/bsfEngine/GUI/BsGUISkin.h"

namespace bs
{
	ScriptGUISkin::ScriptGUISkin(MonoObject* managedInstance, const ResourceHandle<GUISkin>& value)
		:TScriptResource(managedInstance, value)
	{
	}

	void ScriptGUISkin::InitRuntimeData()
	{
		metaData.scriptClass->AddInternalCall("Internal_GetRef", (void*)&ScriptGUISkin::Internal_getRef);
		metaData.scriptClass->AddInternalCall("Internal_hasStyle", (void*)&ScriptGUISkin::Internal_hasStyle);
		metaData.scriptClass->AddInternalCall("Internal_getStyle", (void*)&ScriptGUISkin::Internal_getStyle);
		metaData.scriptClass->AddInternalCall("Internal_setStyle", (void*)&ScriptGUISkin::Internal_setStyle);
		metaData.scriptClass->AddInternalCall("Internal_removeStyle", (void*)&ScriptGUISkin::Internal_removeStyle);
		metaData.scriptClass->AddInternalCall("Internal_getStyleNames", (void*)&ScriptGUISkin::Internal_getStyleNames);
		metaData.scriptClass->AddInternalCall("Internal_create", (void*)&ScriptGUISkin::Internal_create);

	}

	 MonoObject*ScriptGUISkin::createInstance()
	{
		bool dummy = false;
		void* ctorParams[1] = { &dummy };

		return metaData.scriptClass->CreateInstance("bool", ctorParams);
	}
	MonoObject* ScriptGUISkin::Internal_getRef(ScriptGUISkin* thisPtr)
	{
		return thisPtr->GetRRef();
	}

	bool ScriptGUISkin::Internal_hasStyle(ScriptGUISkin* thisPtr, MonoString* name)
	{
		bool tmp__output;
		String tmpname;
		tmpname = MonoUtil::monoToString(name);
		tmp__output = thisPtr->GetHandle()->hasStyle(tmpname);

		bool __output;
		__output = tmp__output;

		return __output;
	}

	MonoObject* ScriptGUISkin::Internal_getStyle(ScriptGUISkin* thisPtr, MonoString* guiElemType)
	{
		SPtr<GUIElementStyle> tmp__output = bs_shared_ptr_new<GUIElementStyle>();
		String tmpguiElemType;
		tmpguiElemType = MonoUtil::monoToString(guiElemType);
		*tmp__output = *thisPtr->GetHandle()->getStyle(tmpguiElemType);

		MonoObject* __output;
		__output = ScriptGUIElementStyle::create(tmp__output);

		return __output;
	}

	void ScriptGUISkin::Internal_setStyle(ScriptGUISkin* thisPtr, MonoString* guiElemType, MonoObject* style)
	{
		String tmpguiElemType;
		tmpguiElemType = MonoUtil::monoToString(guiElemType);
		SPtr<GUIElementStyle> tmpstyle;
		ScriptGUIElementStyle* scriptstyle;
		scriptstyle = ScriptGUIElementStyle::toNative(style);
		if(scriptstyle != nullptr)
			tmpstyle = scriptstyle->GetInternal();
		thisPtr->GetHandle()->setStyle(tmpguiElemType, *tmpstyle);
	}

	void ScriptGUISkin::Internal_removeStyle(ScriptGUISkin* thisPtr, MonoString* guiElemType)
	{
		String tmpguiElemType;
		tmpguiElemType = MonoUtil::monoToString(guiElemType);
		thisPtr->GetHandle()->removeStyle(tmpguiElemType);
	}

	MonoArray* ScriptGUISkin::Internal_getStyleNames(ScriptGUISkin* thisPtr)
	{
		Vector<String> vec__output;
		vec__output = thisPtr->GetHandle()->getStyleNames();

		MonoArray* __output;
		int arraySize__output = (int)vec__output.Size();
		ScriptArray array__output = ScriptArray::create<String>(arraySize__output);
		for(int i = 0; i < arraySize__output; i++)
		{
			array__output.Set(i, vec__output[i]);
		}
		__output = array__output.GetInternal();

		return __output;
	}

	void ScriptGUISkin::Internal_create(MonoObject* managedInstance)
	{
		ResourceHandle<GUISkin> instance = GUISkin::create();
		ScriptResourceManager::instance().CreateBuiltinScriptResource(instance, managedInstance);
	}
}
