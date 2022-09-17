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

	void ScriptGUISkin::initRuntimeData()
	{
		metaData.scriptClass->AddInternalCall("Internal_GetRef", (void*)&ScriptGUISkin::InternalGetRef);
		metaData.scriptClass->AddInternalCall("Internal_hasStyle", (void*)&ScriptGUISkin::InternalHasStyle);
		metaData.scriptClass->addInternalCall("Internal_getStyle", (void*)&ScriptGUISkin::Internal_getStyle);
		metaData.scriptClass->addInternalCall("Internal_setStyle", (void*)&ScriptGUISkin::Internal_setStyle);
		metaData.scriptClass->addInternalCall("Internal_removeStyle", (void*)&ScriptGUISkin::Internal_removeStyle);
		metaData.scriptClass->addInternalCall("Internal_getStyleNames", (void*)&ScriptGUISkin::Internal_getStyleNames);
		metaData.scriptClass->addInternalCall("Internal_create", (void*)&ScriptGUISkin::Internal_create);

	}

	 MonoObject*ScriptGUISkin::CreateInstance()
	{
		bool dummy = false;
		void* ctorParams[1] = { &dummy };

		return metaData.scriptClass->createInstance("bool", ctorParams);
	}
	MonoObject* ScriptGUISkin::InternalGetRef(ScriptGUISkin* thisPtr)
	{
		return thisPtr->GetRRef();
	}

	bool ScriptGUISkin::InternalHasStyle(ScriptGUISkin* thisPtr, MonoString* name)
	{
		bool tmp__output;
		String tmpname;
		tmpname = MonoUtil::monoToString(name);
		tmp__output = thisPtr->GetHandle()->HasStyle(tmpname);

		bool __output;
		__output = tmp__output;

		return __output;
	}

	MonoObject* ScriptGUISkin::InternalGetStyle(ScriptGUISkin* thisPtr, MonoString* guiElemType)
	{
		SPtr<GUIElementStyle> tmp__output = bs_shared_ptr_new<GUIElementStyle>();
		String tmpguiElemType;
		tmpguiElemType = MonoUtil::monoToString(guiElemType);
		*tmp__output = *thisPtr->GetHandle()->GetStyle(tmpguiElemType);

		MonoObject* __output;
		__output = ScriptGUIElementStyle::Create(tmp__output);

		return __output;
	}

	void ScriptGUISkin::InternalSetStyle(ScriptGUISkin* thisPtr, MonoString* guiElemType, MonoObject* style)
	{
		String tmpguiElemType;
		tmpguiElemType = MonoUtil::monoToString(guiElemType);
		SPtr<GUIElementStyle> tmpstyle;
		ScriptGUIElementStyle* scriptstyle;
		scriptstyle = ScriptGUIElementStyle::toNative(style);
		if(scriptstyle != nullptr)
			tmpstyle = scriptstyle->GetInternal();
		thisPtr->GetHandle()->SetStyle(tmpguiElemType, *tmpstyle);
	}

	void ScriptGUISkin::InternalRemoveStyle(ScriptGUISkin* thisPtr, MonoString* guiElemType)
	{
		String tmpguiElemType;
		tmpguiElemType = MonoUtil::monoToString(guiElemType);
		thisPtr->GetHandle()->removeStyle(tmpguiElemType);
	}

	MonoArray* ScriptGUISkin::InternalGetStyleNames(ScriptGUISkin* thisPtr)
	{
		Vector<String> vec__output;
		vec__output = thisPtr->GetHandle()->GetStyleNames();

		MonoArray* __output;
		int arraySize__output = (int)vec__output.size();
		ScriptArray array__output = ScriptArray::create<String>(arraySize__output);
		for(int i = 0; i < arraySize__output; i++)
		{
			array__output.Set(i, vec__output[i]);
		}
		__output = array__output.getInternal();

		return __output;
	}

	void ScriptGUISkin::InternalCreate(MonoObject* managedInstance)
	{
		ResourceHandle<GUISkin> instance = GUISkin::Create();
		ScriptResourceManager::Instance().createBuiltinScriptResource(instance, managedInstance);
	}
}
