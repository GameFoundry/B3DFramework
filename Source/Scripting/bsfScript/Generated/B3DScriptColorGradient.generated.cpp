//************************************ B3D Framework - Copyright 2025 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DScriptColorGradient.generated.h"
#include "B3DMonoMethod.h"
#include "B3DMonoClass.h"
#include "B3DMonoUtil.h"
#include "../../../Foundation/Utility/Image/B3DColorGradient.h"
#include "B3DScriptColor.generated.h"
#include "B3DScriptColorGradientKey.generated.h"
#include "../Extensions/B3DColorGradientEx.h"

namespace b3d
{
	ScriptColorGradient::ScriptColorGradient(const SPtr<ColorGradient>& nativeObject)
		:TScriptNonReflectableWrapper(nativeObject)
	{
		RegisterEvents();
	}

	ScriptColorGradient::~ScriptColorGradient()
	{
		UnregisterEvents();
	}

	void ScriptColorGradient::SetupScriptBindings()
	{
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_ColorGradient", (void*)&ScriptColorGradient::InternalColorGradient);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_ColorGradient0", (void*)&ScriptColorGradient::InternalColorGradient0);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_ColorGradient1", (void*)&ScriptColorGradient::InternalColorGradient1);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetKeys", (void*)&ScriptColorGradient::InternalSetKeys);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetKeys", (void*)&ScriptColorGradient::InternalGetKeys);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetNumKeys", (void*)&ScriptColorGradient::InternalGetNumKeys);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetKey", (void*)&ScriptColorGradient::InternalGetKey);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetConstant", (void*)&ScriptColorGradient::InternalSetConstant);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_Evaluate", (void*)&ScriptColorGradient::InternalEvaluate);

	}

	MonoObject* ScriptColorGradient::CreateScriptObject(bool construct)
	{
		bool dummy = false;
		void* ctorParams[1] = { &dummy };

		if(construct)
			return sInteropMetaData.ScriptClass->CreateInstance("bool", ctorParams);

		return sInteropMetaData.ScriptClass->CreateInstance(false);
	}
	void ScriptColorGradient::InternalColorGradient(MonoObject* scriptObject)
	{
		SPtr<ColorGradient> nativeObject = B3DMakeShared<ColorGradient>();
		ScriptObjectWrapper::Create<ScriptColorGradient>(nativeObject, scriptObject);
	}

	void ScriptColorGradient::InternalColorGradient0(MonoObject* scriptObject, Color* color)
	{
		SPtr<ColorGradient> nativeObject = B3DMakeShared<ColorGradient>(*color);
		ScriptObjectWrapper::Create<ScriptColorGradient>(nativeObject, scriptObject);
	}

	void ScriptColorGradient::InternalColorGradient1(MonoObject* scriptObject, MonoArray* keys)
	{
		Vector<ColorGradientKey> nativeArraykeys;
		if(keys != nullptr)
		{
			ScriptArray scriptArraykeys(keys);
			nativeArraykeys.resize(scriptArraykeys.Size());
			for(int elementIndex = 0; elementIndex < (int)scriptArraykeys.Size(); elementIndex++)
			{
				nativeArraykeys[elementIndex] = ScriptColorGradientKey::FromInterop(scriptArraykeys.Get<__ColorGradientKeyInterop>(elementIndex));
			}
		}
		SPtr<ColorGradient> nativeObject = B3DMakeShared<ColorGradient>(nativeArraykeys);
		ScriptObjectWrapper::Create<ScriptColorGradient>(nativeObject, scriptObject);
	}

	void ScriptColorGradient::InternalSetKeys(ScriptColorGradient* self, MonoArray* keys, float duration)
	{
		if(!self->IsNativeObjectValid())
			return;

		Vector<ColorGradientKey> nativeArraykeys;
		if(keys != nullptr)
		{
			ScriptArray scriptArraykeys(keys);
			nativeArraykeys.resize(scriptArraykeys.Size());
			for(int elementIndex = 0; elementIndex < (int)scriptArraykeys.Size(); elementIndex++)
			{
				nativeArraykeys[elementIndex] = ScriptColorGradientKey::FromInterop(scriptArraykeys.Get<__ColorGradientKeyInterop>(elementIndex));
			}

		}
		static_cast<ColorGradient*>(self->GetNativeObject())->SetKeys(nativeArraykeys, duration);
	}

	MonoArray* ScriptColorGradient::InternalGetKeys(ScriptColorGradient* self)
	{
		Vector<ColorGradientKey> nativeArray__output;
		if(!self->IsNativeObjectValid())
			return {};

		nativeArray__output = static_cast<ColorGradient*>(self->GetNativeObject())->GetKeys();

		MonoArray* __output;
		int elementCount__output = (int)nativeArray__output.size();
		ScriptArray scriptArray__output = ScriptArray::Create<ScriptColorGradientKey>(elementCount__output);
		for(int elementIndex = 0; elementIndex < elementCount__output; elementIndex++)
		{
			scriptArray__output.Set(elementIndex, ScriptColorGradientKey::ToInterop(nativeArray__output[elementIndex]));
		}
		__output = scriptArray__output.GetInternal();

		return __output;
	}

	uint32_t ScriptColorGradient::InternalGetNumKeys(ScriptColorGradient* self)
	{
		uint32_t tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<ColorGradient*>(self->GetNativeObject())->GetNumKeys();

		uint32_t __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptColorGradient::InternalGetKey(ScriptColorGradient* self, uint32_t index, __ColorGradientKeyInterop* __output)
	{
		if(!self->IsNativeObjectValid())
		{
			*__output = {};
			return;
		}

		ColorGradientKey tmp__output;
		tmp__output = static_cast<ColorGradient*>(self->GetNativeObject())->GetKey(index);

		__ColorGradientKeyInterop interop__output;
		interop__output = ScriptColorGradientKey::ToInterop(tmp__output);
		MonoUtil::ValueCopy(__output, &interop__output, ScriptColorGradientKey::GetMetaData()->ScriptClass->GetInternalClass());
	}

	void ScriptColorGradient::InternalSetConstant(ScriptColorGradient* self, Color* color)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<ColorGradient*>(self->GetNativeObject())->SetConstant(*color);
	}

	void ScriptColorGradient::InternalEvaluate(ScriptColorGradient* self, float t, Color* __output)
	{
		if(!self->IsNativeObjectValid())
		{
			*__output = {};
			return;
		}

		Color tmp__output;
		tmp__output = ColorGradientEx::Evaluate(std::static_pointer_cast<ColorGradient>(self->GetBaseNativeObjectAsShared()), t);

		*__output = tmp__output;
	}
}
