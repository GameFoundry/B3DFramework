//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptSubMesh.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"

namespace bs
{
	ScriptSubMesh::ScriptSubMesh(MonoObject* managedInstance)
		: ScriptObject(managedInstance)
	{}

	void ScriptSubMesh::InitRuntimeData()
	{}

	MonoObject* ScriptSubMesh::Box(const SubMesh& value)
	{
		return MonoUtil::Box(metaData.ScriptClass->GetInternalClassInternal(), (void*)&value);
	}

	SubMesh ScriptSubMesh::Unbox(MonoObject* value)
	{
		return *(SubMesh*)MonoUtil::Unbox(value);
	}

} // namespace bs
