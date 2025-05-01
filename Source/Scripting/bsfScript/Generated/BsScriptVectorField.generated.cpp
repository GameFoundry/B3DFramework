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
#include "BsScriptTVector3.generated.h"

namespace bs
{
	ScriptVectorField::ScriptVectorField(const TResourceHandle<VectorField>& nativeObject)
		:TScriptResourceWrapper(nativeObject)
	{
		RegisterEvents();
	}

	ScriptVectorField::~ScriptVectorField()
	{
		UnregisterEvents();
	}

	void ScriptVectorField::SetupScriptBindings()
	{
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetRef", (void*)&ScriptVectorField::InternalGetRef);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_Create", (void*)&ScriptVectorField::InternalCreate);

	}

	MonoObject* ScriptVectorField::CreateScriptObject(bool construct)
	{
		bool dummy = false;
		void* ctorParams[1] = { &dummy };

		if(construct)
			return sInteropMetaData.ScriptClass->CreateInstance("bool", ctorParams);

		return sInteropMetaData.ScriptClass->CreateInstance(false);
	}
	MonoObject* ScriptVectorField::InternalGetRef(ScriptVectorField* self)
	{
		return self->GetOrCreateResourceReference();
	}

	void ScriptVectorField::InternalCreate(MonoObject* scriptObject, __VECTOR_FIELD_DESCInterop* desc, MonoArray* values)
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
		ScriptObjectWrapper::Create<ScriptVectorField>(nativeObject, scriptObject);
	}
}
