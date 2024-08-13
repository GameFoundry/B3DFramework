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
#include "BsScriptVECTOR_FIELD_DESC.generated.h"
#include "Wrappers/BsScriptVector.h"

namespace bs
{
	ScriptVectorField::ScriptVectorField(MonoObject* managedInstance, const TResourceHandle<VectorField>& value)
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
	MonoObject* ScriptVectorField::InternalGetRef(ScriptVectorField* self)
	{
		return self->GetRRef();
	}

	void ScriptVectorField::InternalCreate(MonoObject* managedInstance, __VECTOR_FIELD_DESCInterop* desc, MonoArray* values)
	{
		VECTOR_FIELD_DESC tmpdesc;
		tmpdesc = ScriptVectorFieldOptions::FromInterop(*desc);
		Vector<TVector3<float>> nativeArrayvalues;
		if(values != nullptr)
		{
			ScriptArray scriptArrayvalues(values);
			nativeArrayvalues.resize(scriptArrayvalues.Size());
			for(int elementIndex = 0; elementIndex < (int)scriptArrayvalues.Size(); elementIndex++)
			{
				nativeArrayvalues[elementIndex] = scriptArrayvalues.Get<TVector3<float>>(elementIndex);
			}
		}
		TResourceHandle<VectorField> nativeObject = VectorField::Create(tmpdesc, nativeArrayvalues);
		ScriptResourceManager::Instance().CreateBuiltinScriptResource(nativeObject, managedInstance);
	}
}
