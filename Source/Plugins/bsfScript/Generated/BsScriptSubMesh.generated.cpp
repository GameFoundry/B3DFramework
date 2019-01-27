//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptSubMesh.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"

namespace bs
{
	ScriptSubMesh::ScriptSubMesh(MonoObject* managedInstance)
		:ScriptObject(managedInstance)
	{ }

	void ScriptSubMesh::initRuntimeData()
	{ }

	MonoObject*ScriptSubMesh::box(const SubMesh& value)
	{
		return MonoUtil::box(metaData.scriptClass->_getInternalClass(), (void*)&value);
	}

	SubMesh ScriptSubMesh::unbox(MonoObject* value)
	{
		return *(SubMesh*)MonoUtil::unbox(value);
	}

}
