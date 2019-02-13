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
		:ScriptObject(managedInstance)
	{ }

	void ScriptCharRange::initRuntimeData()
	{ }

	MonoObject*ScriptCharRange::box(const CharRange& value)
	{
		return MonoUtil::box(metaData.scriptClass->_getInternalClass(), (void*)&value);
	}

	CharRange ScriptCharRange::unbox(MonoObject* value)
	{
		return *(CharRange*)MonoUtil::unbox(value);
	}

#endif
}
