//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptColorGradient.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "../../../Foundation/bsfUtility/Image/BsColorGradient.h"
#include "Wrappers/BsScriptColor.h"
#include "BsScriptColorGradientKey.generated.h"
#include "../Extensions/BsColorGradientEx.h"

namespace bs
{
	ScriptColorGradient::ScriptColorGradient(MonoObject* managedInstance, const SPtr<ColorGradient>& value)
		:ScriptObject(managedInstance), mInternal(value)
	{
	}

	void ScriptColorGradient::InitRuntimeData()
	{
		metaData.ScriptClass->AddInternalCall("Internal_ColorGradient", (void*)&ScriptColorGradient::InternalColorGradient);
		metaData.ScriptClass->AddInternalCall("Internal_ColorGradient0", (void*)&ScriptColorGradient::InternalColorGradient0);
		metaData.ScriptClass->AddInternalCall("Internal_ColorGradient1", (void*)&ScriptColorGradient::InternalColorGradient1);
		metaData.ScriptClass->AddInternalCall("Internal_SetKeys", (void*)&ScriptColorGradient::InternalSetKeys);
		metaData.ScriptClass->AddInternalCall("Internal_GetKeys", (void*)&ScriptColorGradient::InternalGetKeys);
		metaData.ScriptClass->AddInternalCall("Internal_GetNumKeys", (void*)&ScriptColorGradient::InternalGetNumKeys);
		metaData.ScriptClass->AddInternalCall("Internal_GetKey", (void*)&ScriptColorGradient::InternalGetKey);
		metaData.ScriptClass->AddInternalCall("Internal_SetConstant", (void*)&ScriptColorGradient::InternalSetConstant);
		metaData.ScriptClass->AddInternalCall("Internal_Evaluate", (void*)&ScriptColorGradient::InternalEvaluate);

	}

	MonoObject* ScriptColorGradient::Create(const SPtr<ColorGradient>& value)
	{
		if(value == nullptr) return nullptr; 

		bool dummy = false;
		void* ctorParams[1] = { &dummy };

		MonoObject* managedInstance = metaData.ScriptClass->CreateInstance("bool", ctorParams);
		new (B3DAllocate<ScriptColorGradient>()) ScriptColorGradient(managedInstance, value);
		return managedInstance;
	}
	void ScriptColorGradient::InternalColorGradient(MonoObject* managedInstance)
	{
		SPtr<ColorGradient> instance = B3DMakeShared<ColorGradient>();
		new (B3DAllocate<ScriptColorGradient>())ScriptColorGradient(managedInstance, instance);
	}

	void ScriptColorGradient::InternalColorGradient0(MonoObject* managedInstance, Color* color)
	{
		SPtr<ColorGradient> instance = B3DMakeShared<ColorGradient>(*color);
		new (B3DAllocate<ScriptColorGradient>())ScriptColorGradient(managedInstance, instance);
	}

	void ScriptColorGradient::InternalColorGradient1(MonoObject* managedInstance, MonoArray* keys)
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
		SPtr<ColorGradient> instance = B3DMakeShared<ColorGradient>(nativeArraykeys);
		new (B3DAllocate<ScriptColorGradient>())ScriptColorGradient(managedInstance, instance);
	}

	void ScriptColorGradient::InternalSetKeys(ScriptColorGradient* thisPtr, MonoArray* keys, float duration)
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
		thisPtr->GetInternal()->SetKeys(nativeArraykeys, duration);
	}

	MonoArray* ScriptColorGradient::InternalGetKeys(ScriptColorGradient* thisPtr)
	{
		Vector<ColorGradientKey> nativeArray__output;
		nativeArray__output = thisPtr->GetInternal()->GetKeys();

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

	uint32_t ScriptColorGradient::InternalGetNumKeys(ScriptColorGradient* thisPtr)
	{
		uint32_t tmp__output;
		tmp__output = thisPtr->GetInternal()->GetNumKeys();

		uint32_t __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptColorGradient::InternalGetKey(ScriptColorGradient* thisPtr, uint32_t idx, __ColorGradientKeyInterop* __output)
	{
		ColorGradientKey tmp__output;
		tmp__output = thisPtr->GetInternal()->GetKey(idx);

		__ColorGradientKeyInterop interop__output;
		interop__output = ScriptColorGradientKey::ToInterop(tmp__output);
		MonoUtil::ValueCopy(__output, &interop__output, ScriptColorGradientKey::GetMetaData()->ScriptClass->GetInternalClassInternal());
	}

	void ScriptColorGradient::InternalSetConstant(ScriptColorGradient* thisPtr, Color* color)
	{
		thisPtr->GetInternal()->SetConstant(*color);
	}

	void ScriptColorGradient::InternalEvaluate(ScriptColorGradient* thisPtr, float t, Color* __output)
	{
		Color tmp__output;
		tmp__output = ColorGradientEx::Evaluate(thisPtr->GetInternal(), t);

		*__output = tmp__output;
	}
}
