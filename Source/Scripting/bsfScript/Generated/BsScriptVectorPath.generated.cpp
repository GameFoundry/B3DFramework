//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptVectorPath.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "../../../Foundation/bsfCore/VectorGraphics/BsVectorGraphics.h"

namespace bs
{
	ScriptVectorPath::ScriptVectorPath(MonoObject* managedInstance, const TResourceHandle<VectorPath>& value)
		:TScriptResource(managedInstance, value)
	{
	}

	void ScriptVectorPath::InitRuntimeData()
	{
		metaData.ScriptClass->AddInternalCall("Internal_GetRef", (void*)&ScriptVectorPath::InternalGetRef);

	}

	 MonoObject*ScriptVectorPath::CreateInstance()
	{
		bool dummy = false;
		void* ctorParams[1] = { &dummy };

		return metaData.ScriptClass->CreateInstance("bool", ctorParams);
	}
	MonoObject* ScriptVectorPath::InternalGetRef(ScriptVectorPath* self)
	{
		return self->GetRRef();
	}

}
