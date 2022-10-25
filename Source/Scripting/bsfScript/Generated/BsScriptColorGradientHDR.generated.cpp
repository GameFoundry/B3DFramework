//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
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
		: ScriptObject(managedInstance), mInternal(value)
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
		new(bs_alloc<ScriptColorGradientHDR>()) ScriptColorGradientHDR(managedInstance, value);
		return managedInstance;
	}

	void ScriptColorGradientHDR::InternalColorGradientHDR(MonoObject* managedInstance)
	{
		SPtr<ColorGradientHDR> instance = bs_shared_ptr_new<ColorGradientHDR>();
		new(bs_alloc<ScriptColorGradientHDR>()) ScriptColorGradientHDR(managedInstance, instance);
	}

	void ScriptColorGradientHDR::InternalColorGradientHDR0(MonoObject* managedInstance, Color* color)
	{
		SPtr<ColorGradientHDR> instance = bs_shared_ptr_new<ColorGradientHDR>(*color);
		new(bs_alloc<ScriptColorGradientHDR>()) ScriptColorGradientHDR(managedInstance, instance);
	}

	void ScriptColorGradientHDR::InternalColorGradientHDR1(MonoObject* managedInstance, MonoArray* keys)
	{
		Vector<ColorGradientKey> veckeys;
		if(keys != nullptr)
		{
			ScriptArray arraykeys(keys);
			veckeys.resize(arraykeys.Size());
			for(int i = 0; i < (int)arraykeys.Size(); i++)
			{
				veckeys[i] = ScriptColorGradientKey::FromInterop(arraykeys.Get<__ColorGradientKeyInterop>(i));
			}
		}
		SPtr<ColorGradientHDR> instance = bs_shared_ptr_new<ColorGradientHDR>(veckeys);
		new(bs_alloc<ScriptColorGradientHDR>()) ScriptColorGradientHDR(managedInstance, instance);
	}

	void ScriptColorGradientHDR::InternalSetKeys(ScriptColorGradientHDR* thisPtr, MonoArray* keys, float duration)
	{
		Vector<ColorGradientKey> veckeys;
		if(keys != nullptr)
		{
			ScriptArray arraykeys(keys);
			veckeys.resize(arraykeys.Size());
			for(int i = 0; i < (int)arraykeys.Size(); i++)
			{
				veckeys[i] = ScriptColorGradientKey::FromInterop(arraykeys.Get<__ColorGradientKeyInterop>(i));
			}
		}
		thisPtr->GetInternal()->SetKeys(veckeys, duration);
	}

	MonoArray* ScriptColorGradientHDR::InternalGetKeys(ScriptColorGradientHDR* thisPtr)
	{
		Vector<ColorGradientKey> vec__output;
		vec__output = thisPtr->GetInternal()->GetKeys();

		MonoArray* __output;
		int arraySize__output = (int)vec__output.size();
		ScriptArray array__output = ScriptArray::Create<ScriptColorGradientKey>(arraySize__output);
		for(int i = 0; i < arraySize__output; i++)
		{
			array__output.Set(i, ScriptColorGradientKey::ToInterop(vec__output[i]));
		}
		__output = array__output.GetInternal();

		return __output;
	}

	uint32_t ScriptColorGradientHDR::InternalGetNumKeys(ScriptColorGradientHDR* thisPtr)
	{
		uint32_t tmp__output;
		tmp__output = thisPtr->GetInternal()->GetNumKeys();

		uint32_t __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptColorGradientHDR::InternalGetKey(ScriptColorGradientHDR* thisPtr, uint32_t idx, __ColorGradientKeyInterop* __output)
	{
		ColorGradientKey tmp__output;
		tmp__output = thisPtr->GetInternal()->GetKey(idx);

		__ColorGradientKeyInterop interop__output;
		interop__output = ScriptColorGradientKey::ToInterop(tmp__output);
		MonoUtil::ValueCopy(__output, &interop__output, ScriptColorGradientKey::GetMetaData()->ScriptClass->GetInternalClassInternal());
	}

	void ScriptColorGradientHDR::InternalSetConstant(ScriptColorGradientHDR* thisPtr, Color* color)
	{
		thisPtr->GetInternal()->SetConstant(*color);
	}

	void ScriptColorGradientHDR::InternalEvaluate(ScriptColorGradientHDR* thisPtr, float t, Color* __output)
	{
		Color tmp__output;
		tmp__output = ColorGradientHDREx::Evaluate(thisPtr->GetInternal(), t);

		*__output = tmp__output;
	}
} // namespace bs
