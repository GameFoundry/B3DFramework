//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptKerningPair.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"

namespace bs
{
	ScriptKerningPair::ScriptKerningPair(MonoObject* managedInstance)
		:ScriptObject(managedInstance)
	{ }

	void ScriptKerningPair::initRuntimeData()
	{ }

	MonoObject*ScriptKerningPair::box(const KerningPair& value)
	{
		return MonoUtil::box(metaData.scriptClass->_getInternalClass(), (void*)&value);
	}

	KerningPair ScriptKerningPair::unbox(MonoObject* value)
	{
		return *(KerningPair*)MonoUtil::unbox(value);
	}

}
