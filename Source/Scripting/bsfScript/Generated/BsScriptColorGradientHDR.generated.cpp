//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptColorGradientHDR.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "../../../Foundation/bsfUtility/Image/BsColorGradient.h"
#include "Wrappers/BsScriptColor.h"
#include "../Extensions/BsColorGradientEx.h"
#include "BsScriptColorGradientKey.generated.h"

namespace bs
{
	ScriptColorGradientHDR::ScriptColorGradientHDR(MonoObject* managedInstance, const SPtr<ColorGradientHDR>& value)
		:ScriptObject(managedInstance), mInternal(value)
	{
	}

	void ScriptColorGradientHDR::InitRuntimeData()
	{
		metaData.ScriptClass->AddInternalCall("Internal_ColorGradientHDR", (void*)&ScriptColorGradientHDR::InternalColorGradientHDR);
		metaData.ScriptClass->AddInternalCall("Internal_ColorGradientHDR0", (void*)&ScriptColorGradientHDR::InternalColorGradientHDR0);
		metaData.ScriptClass->AddInternalCall("Internal_ColorGradientHDR1", (void*)&ScriptColorGradientHDR::InternalColorGradientHDR1);
		metaData.ScriptClass->AddInternalCall("Internal_SetKeys", (void*)&ScriptColorGradientHDR::InternalSetKeys);
		metaData.ScriptClass->AddInternalCall("Internal_GetKeys", (void*)&ScriptColorGradientHDR::InternalGetKeys);
		metaData.ScriptClass->AddInternalCall("Internal_GetNumKeys", (void*)&ScriptColorGradientHDR::InternalGetNumKeys);
		metaData.ScriptClass->AddInternalCall("Internal_GetKey", (void*)&ScriptColorGradientHDR::InternalGetKey);
		metaData.ScriptClass->AddInternalCall("Internal_SetConstant", (void*)&ScriptColorGradientHDR::InternalSetConstant);
		metaData.ScriptClass->AddInternalCall("Internal_Evaluate", (void*)&ScriptColorGradientHDR::InternalEvaluate);

	}

	MonoObject* ScriptColorGradientHDR::Create(const SPtr<ColorGradientHDR>& value)
	{
		if(value == nullptr) return nullptr; 

		bool dummy = false;
		void* ctorParams[1] = { &dummy };

		MonoObject* managedInstance = metaData.ScriptClass->CreateInstance("bool", ctorParams);
		new (B3DAllocate<ScriptColorGradientHDR>()) ScriptColorGradientHDR(managedInstance, value);
		return managedInstance;
	}
	void ScriptColorGradientHDR::InternalColorGradientHDR(MonoObject* managedInstance)
	{
		SPtr<ColorGradientHDR> nativeObject = B3DMakeShared<ColorGradientHDR>();
		new (B3DAllocate<ScriptColorGradientHDR>())ScriptColorGradientHDR(managedInstance, nativeObject);
	}

	void ScriptColorGradientHDR::InternalColorGradientHDR0(MonoObject* managedInstance, Color* color)
	{
		SPtr<ColorGradientHDR> nativeObject = B3DMakeShared<ColorGradientHDR>(*color);
		new (B3DAllocate<ScriptColorGradientHDR>())ScriptColorGradientHDR(managedInstance, nativeObject);
	}

	void ScriptColorGradientHDR::InternalColorGradientHDR1(MonoObject* managedInstance, MonoArray* keys)
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
		SPtr<ColorGradientHDR> nativeObject = B3DMakeShared<ColorGradientHDR>(nativeArraykeys);
		new (B3DAllocate<ScriptColorGradientHDR>())ScriptColorGradientHDR(managedInstance, nativeObject);
	}

	void ScriptColorGradientHDR::InternalSetKeys(ScriptColorGradientHDR* self, MonoArray* keys, float duration)
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
		self->GetInternal()->SetKeys(nativeArraykeys, duration);
	}

	MonoArray* ScriptColorGradientHDR::InternalGetKeys(ScriptColorGradientHDR* self)
	{
		Vector<ColorGradientKey> nativeArray__output;
		nativeArray__output = self->GetInternal()->GetKeys();

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
		tmp__output = self->GetInternal()->GetNumKeys();

		uint32_t __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptColorGradientHDR::InternalGetKey(ScriptColorGradientHDR* self, uint32_t idx, __ColorGradientKeyInterop* __output)
	{
		ColorGradientKey tmp__output;
		tmp__output = self->GetInternal()->GetKey(idx);

		__ColorGradientKeyInterop interop__output;
		interop__output = ScriptColorGradientKey::ToInterop(tmp__output);
		MonoUtil::ValueCopy(__output, &interop__output, ScriptColorGradientKey::GetMetaData()->ScriptClass->GetInternalClassInternal());
	}

	void ScriptColorGradientHDR::InternalSetConstant(ScriptColorGradientHDR* self, Color* color)
	{
		self->GetInternal()->SetConstant(*color);
	}

	void ScriptColorGradientHDR::InternalEvaluate(ScriptColorGradientHDR* self, float t, Color* __output)
	{
		Color tmp__output;
		tmp__output = ColorGradientHDREx::Evaluate(self->GetInternal(), t);

		*__output = tmp__output;
	}
}
