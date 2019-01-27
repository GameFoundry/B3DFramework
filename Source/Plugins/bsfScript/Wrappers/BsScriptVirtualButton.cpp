//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Wrappers/BsScriptVirtualButton.h"
#include "BsMonoManager.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"

namespace bs
{
	ScriptVirtualButton::ScriptVirtualButton(MonoObject* instance)
		:ScriptObject(instance)
	{ }

	void ScriptVirtualButton::initRuntimeData()
	{
		metaData.scriptClass->addInternalCall("Internal_InitVirtualButton", (void*)&ScriptVirtualButton::internal_InitVirtualButton);
	}

	UINT32 ScriptVirtualButton::internal_InitVirtualButton(MonoString* name)
	{
		String nameStr = MonoUtil::monoToString(name);

		VirtualButton vb(nameStr);
		return vb.buttonIdentifier;
	}

	MonoObject* ScriptVirtualButton::box(const VirtualButton& value)
	{
		// We're casting away const but it's fine since structs are passed by value anyway
		return MonoUtil::box(metaData.scriptClass->_getInternalClass(), (void*)&value);
	}

	VirtualButton ScriptVirtualButton::unbox(MonoObject* obj)
	{
		return *(VirtualButton*)MonoUtil::unbox(obj);
	}
}