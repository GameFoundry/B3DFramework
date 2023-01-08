//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptVectorField.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "../../../Foundation/bsfCore/Particles/BsVectorField.h"
#include "BsScriptResourceManager.h"
#include "Wrappers/BsScriptRRefBase.h"
#include "../../../Foundation/bsfCore/Particles/BsVectorField.h"
#include "Wrappers/BsScriptVector.h"
#include "BsScriptVECTOR_FIELD_DESC.generated.h"

namespace bs
{
	ScriptVectorField::ScriptVectorField(MonoObject* managedInstance, const ResourceHandle<VectorField>& value)
		:TScriptResource(managedInstance, value)
	{
	}

	void ScriptVectorField::InitRuntimeData()
	{
		metaData.ScriptClass->AddInternalCall("Internal_GetRef", (void*)&ScriptVectorField::InternalGetRef);
		metaData.ScriptClass->AddInternalCall("Internal_Create", (void*)&ScriptVectorField::InternalCreate);

	}

	 MonoObject*ScriptVectorField::CreateInstance()
	{
		bool dummy = false;
		void* ctorParams[1] = { &dummy };

		return metaData.ScriptClass->CreateInstance("bool", ctorParams);
	}
	MonoObject* ScriptVectorField::InternalGetRef(ScriptVectorField* thisPtr)
	{
		return thisPtr->GetRRef();
	}

	void ScriptVectorField::InternalCreate(MonoObject* managedInstance, __VECTOR_FIELD_DESCInterop* desc, MonoArray* values)
	{
		VECTOR_FIELD_DESC tmpdesc;
		tmpdesc = ScriptVectorFieldOptions::FromInterop(*desc);
		Vector<Vector3> vecvalues;
		if(values != nullptr)
		{
			ScriptArray arrayvalues(values);
			vecvalues.resize(arrayvalues.Size());
			for(int i = 0; i < (int)arrayvalues.Size(); i++)
			{
				vecvalues[i] = arrayvalues.Get<Vector3>(i);
			}
		}
		ResourceHandle<VectorField> instance = VectorField::Create(tmpdesc, vecvalues);
		ScriptResourceManager::Instance().CreateBuiltinScriptResource(instance, managedInstance);
	}
}
