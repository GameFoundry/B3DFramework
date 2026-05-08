//************************************ B3D Framework - Copyright 2025 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DScriptColorGradientHDR.generated.h"
#include "B3DMonoMethod.h"
#include "B3DMonoClass.h"
#include "B3DMonoUtil.h"
#include "../../../Engine/Utility/Image/B3DColorGradient.h"
#include "B3DScriptColor.generated.h"
#include "../Extensions/B3DColorGradientEx.h"
#include "B3DScriptColorGradientKey.generated.h"

namespace b3d
{
	ScriptColorGradientHDR::ScriptColorGradientHDR(const TShared<ColorGradientHDR>& nativeObject)
		:TScriptNonReflectableWrapper(nativeObject)
	{
		RegisterEvents();
	}

	ScriptColorGradientHDR::~ScriptColorGradientHDR()
	{
		UnregisterEvents();
	}

	void ScriptColorGradientHDR::SetupScriptBindings()
	{
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_ColorGradientHDR", (void*)&ScriptColorGradientHDR::InternalColorGradientHDR);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_ColorGradientHDR0", (void*)&ScriptColorGradientHDR::InternalColorGradientHDR0);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_ColorGradientHDR1", (void*)&ScriptColorGradientHDR::InternalColorGradientHDR1);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetKeys", (void*)&ScriptColorGradientHDR::InternalSetKeys);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetKeys", (void*)&ScriptColorGradientHDR::InternalGetKeys);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetNumKeys", (void*)&ScriptColorGradientHDR::InternalGetNumKeys);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetKey", (void*)&ScriptColorGradientHDR::InternalGetKey);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetConstant", (void*)&ScriptColorGradientHDR::InternalSetConstant);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_Evaluate", (void*)&ScriptColorGradientHDR::InternalEvaluate);

	}

	MonoObject* ScriptColorGradientHDR::CreateScriptObject(bool construct)
	{
		bool dummy = false;
		void* ctorParams[1] = { &dummy };

		if(construct)
			return sInteropMetaData.ScriptClass->CreateInstance("bool", ctorParams);

		return sInteropMetaData.ScriptClass->CreateInstance(false);
	}
	void ScriptColorGradientHDR::InternalColorGradientHDR(MonoObject* scriptObject)
	{
		TShared<ColorGradientHDR> nativeObject = B3DMakeShared<ColorGradientHDR>();
		ScriptObjectWrapper::Create<ScriptColorGradientHDR>(nativeObject, scriptObject);
	}

	void ScriptColorGradientHDR::InternalColorGradientHDR0(MonoObject* scriptObject, Color* color)
	{
		TShared<ColorGradientHDR> nativeObject = B3DMakeShared<ColorGradientHDR>(*color);
		ScriptObjectWrapper::Create<ScriptColorGradientHDR>(nativeObject, scriptObject);
	}

	void ScriptColorGradientHDR::InternalColorGradientHDR1(MonoObject* scriptObject, MonoArray* keys)
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
		TShared<ColorGradientHDR> nativeObject = B3DMakeShared<ColorGradientHDR>(nativeArraykeys);
		ScriptObjectWrapper::Create<ScriptColorGradientHDR>(nativeObject, scriptObject);
	}

	void ScriptColorGradientHDR::InternalSetKeys(ScriptColorGradientHDR* self, MonoArray* keys, float duration)
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
		static_cast<ColorGradientHDR*>(self->GetNativeObject())->SetKeys(nativeArraykeys, duration);
	}

	MonoArray* ScriptColorGradientHDR::InternalGetKeys(ScriptColorGradientHDR* self)
	{
		Vector<ColorGradientKey> nativeArray__output;
		if(!self->IsNativeObjectValid())
			return {};

		nativeArray__output = static_cast<ColorGradientHDR*>(self->GetNativeObject())->GetKeys();

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

	uint32_t ScriptColorGradientHDR::InternalGetNumKeys(ScriptColorGradientHDR* self)
	{
		uint32_t tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<ColorGradientHDR*>(self->GetNativeObject())->GetNumKeys();

		uint32_t __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptColorGradientHDR::InternalGetKey(ScriptColorGradientHDR* self, uint32_t index, __ColorGradientKeyInterop* __output)
	{
		if(!self->IsNativeObjectValid())
		{
			*__output = {};
			return;
		}

		ColorGradientKey tmp__output;
		tmp__output = static_cast<ColorGradientHDR*>(self->GetNativeObject())->GetKey(index);

		__ColorGradientKeyInterop interop__output;
		interop__output = ScriptColorGradientKey::ToInterop(tmp__output);
		MonoUtil::ValueCopy(__output, &interop__output, ScriptColorGradientKey::GetMetaData()->ScriptClass->GetInternalClass());
	}

	void ScriptColorGradientHDR::InternalSetConstant(ScriptColorGradientHDR* self, Color* color)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<ColorGradientHDR*>(self->GetNativeObject())->SetConstant(*color);
	}

	void ScriptColorGradientHDR::InternalEvaluate(ScriptColorGradientHDR* self, float t, Color* __output)
	{
		if(!self->IsNativeObjectValid())
		{
			*__output = {};
			return;
		}

		Color tmp__output;
		tmp__output = ColorGradientHDREx::Evaluate(std::static_pointer_cast<ColorGradientHDR>(self->GetBaseNativeObjectAsShared()), t);

		*__output = tmp__output;
	}
}
