//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptCharRange.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"

namespace bs
{
#if !BS_IS_BANSHEE3D
	ScriptCharRange::ScriptCharRange(MonoObject* managedInstance)
		: ScriptObject(managedInstance)
	{}

	void ScriptCharRange::InitRuntimeData()
	{}

	MonoObject* ScriptCharRange::Box(const CharRange& value)
	{
		return MonoUtil::Box(metaData.ScriptClass->GetInternalClassInternal(), (void*)&value);
	}

	CharRange ScriptCharRange::Unbox(MonoObject* value)
	{
		return *(CharRange*)MonoUtil::Unbox(value);
	}

#endif
} // namespace bs
