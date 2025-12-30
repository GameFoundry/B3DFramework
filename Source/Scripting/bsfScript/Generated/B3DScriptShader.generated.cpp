//************************************ B3D Framework - Copyright 2025 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DScriptShader.generated.h"
#include "B3DMonoMethod.h"
#include "B3DMonoClass.h"
#include "B3DMonoUtil.h"
#include "../../../Foundation/Core/Material/B3DShader.h"
#include "B3DScriptShaderVariationParameterInformation.generated.h"
#include "B3DScriptShaderParameter.generated.h"
#include "../Extensions/B3DShaderEx.h"

namespace b3d
{
	ScriptShader::ScriptShader(const TResourceHandle<Shader>& nativeObject)
		:TScriptResourceWrapper(nativeObject)
	{
		RegisterEvents();
	}

	ScriptShader::~ScriptShader()
	{
		UnregisterEvents();
	}

	void ScriptShader::SetupScriptBindings()
	{
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetRef", (void*)&ScriptShader::InternalGetRef);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetVariationParams", (void*)&ScriptShader::InternalGetVariationParams);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetParameters", (void*)&ScriptShader::InternalGetParameters);

	}

	MonoObject* ScriptShader::CreateScriptObject(bool construct)
	{
		bool dummy = false;
		void* ctorParams[1] = { &dummy };

		if(construct)
			return sInteropMetaData.ScriptClass->CreateInstance("bool", ctorParams);

		return sInteropMetaData.ScriptClass->CreateInstance(false);
	}
	MonoObject* ScriptShader::InternalGetRef(ScriptShader* self)
	{
		return self->GetOrCreateResourceReference();
	}

	MonoArray* ScriptShader::InternalGetVariationParams(ScriptShader* self)
	{
		Vector<ShaderVariationParameterInformation> nativeArray__output;
		if(!self->IsNativeObjectValid())
			return {};

		nativeArray__output = static_cast<Shader*>(self->GetNativeObject())->GetVariationParameters();

		MonoArray* __output;
		int elementCount__output = (int)nativeArray__output.size();
		ScriptArray scriptArray__output = ScriptArray::Create<ScriptShaderVariationParameterInformation>(elementCount__output);
		for(int elementIndex = 0; elementIndex < elementCount__output; elementIndex++)
		{
			scriptArray__output.Set(elementIndex, ScriptShaderVariationParameterInformation::ToInterop(nativeArray__output[elementIndex]));
		}
		__output = scriptArray__output.GetInternal();

		return __output;
	}

	MonoArray* ScriptShader::InternalGetParameters(ScriptShader* self)
	{
		Vector<ShaderParameter> nativeArray__output;
		if(!self->IsNativeObjectValid())
			return {};

		nativeArray__output = ShaderEx::GetParameters(B3DStaticResourceCast<Shader>(self->GetBaseNativeObjectAsHandle()));

		MonoArray* __output;
		int elementCount__output = (int)nativeArray__output.size();
		ScriptArray scriptArray__output = ScriptArray::Create<ScriptShaderParameter>(elementCount__output);
		for(int elementIndex = 0; elementIndex < elementCount__output; elementIndex++)
		{
			scriptArray__output.Set(elementIndex, ScriptShaderParameter::ToInterop(nativeArray__output[elementIndex]));
		}
		__output = scriptArray__output.GetInternal();

		return __output;
	}
}
