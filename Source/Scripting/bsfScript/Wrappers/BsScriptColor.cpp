//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Wrappers/BsScriptColor.h"
#include "BsMonoManager.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"

namespace bs
{
	ScriptColor::ScriptColor(MonoObject* instance)
		: ScriptObject(instance)
	{}

	void ScriptColor::InitRuntimeData()
	{}

	MonoObject* ScriptColor::Box(const Color& value)
	{
		// We're casting away const but it's fine since structs are passed by value anyway
		return MonoUtil::Box(metaData.ScriptClass->GetInternalClassInternal(), (void*)&value);
	}

	Color ScriptColor::Unbox(MonoObject* obj)
	{
		return *(Color*)MonoUtil::Unbox(obj);
	}
} // namespace bs
