//************************************ B3D Framework - Copyright 2025 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DScriptShaderMetaData.generated.h"
#include "B3DMonoMethod.h"
#include "B3DMonoClass.h"
#include "B3DMonoUtil.h"

namespace b3d
{
	ScriptShaderMetaData::ScriptShaderMetaData(const TShared<ShaderMetaData>& nativeObject)
		:TScriptReflectableWrapper(nativeObject)
	{
		RegisterEvents();
	}

	ScriptShaderMetaData::~ScriptShaderMetaData()
	{
		UnregisterEvents();
	}

	void ScriptShaderMetaData::SetupScriptBindings()
	{
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetIncludes", (void*)&ScriptShaderMetaData::InternalGetIncludes);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetIncludes", (void*)&ScriptShaderMetaData::InternalSetIncludes);

	}

	MonoObject* ScriptShaderMetaData::CreateScriptObject(bool construct)
	{
		bool dummy = false;
		void* ctorParams[1] = { &dummy };

		if(construct)
			return sInteropMetaData.ScriptClass->CreateInstance("bool", ctorParams);

		return sInteropMetaData.ScriptClass->CreateInstance(false);
	}
	MonoArray* ScriptShaderMetaData::InternalGetIncludes(ScriptShaderMetaData* self)
	{
		Vector<String> nativeArray__output;
		if(!self->IsNativeObjectValid())
			return {};

		nativeArray__output = static_cast<ShaderMetaData*>(self->GetNativeObject())->Includes;

		MonoArray* __output;
		int elementCount__output = (int)nativeArray__output.size();
		ScriptArray scriptArray__output = ScriptArray::Create<String>(elementCount__output);
		for(int elementIndex = 0; elementIndex < elementCount__output; elementIndex++)
		{
			scriptArray__output.Set(elementIndex, nativeArray__output[elementIndex]);
		}
		__output = scriptArray__output.GetInternal();

		return __output;
	}

	void ScriptShaderMetaData::InternalSetIncludes(ScriptShaderMetaData* self, MonoArray* value)
	{
		if(!self->IsNativeObjectValid())
			return;

		Vector<String> nativeArrayvalue;
		if(value != nullptr)
		{
			ScriptArray scriptArrayvalue(value);
			nativeArrayvalue.resize(scriptArrayvalue.Size());
			for(int elementIndex = 0; elementIndex < (int)scriptArrayvalue.Size(); elementIndex++)
			{
				nativeArrayvalue[elementIndex] = scriptArrayvalue.Get<String>(elementIndex);
			}

		}
		static_cast<ShaderMetaData*>(self->GetNativeObject())->Includes = nativeArrayvalue;
	}
}
