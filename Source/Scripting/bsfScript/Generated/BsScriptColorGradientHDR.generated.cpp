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
		:ScriptObject(managedInstance), mInternal(value)
	{
	}

	void ScriptColorGradientHDR::initRuntimeData()
	{
		metaData.scriptClass->addInternalCall("Internal_ColorGradientHDR", (void*)&ScriptColorGradientHDR::Internal_ColorGradientHDR);
		metaData.scriptClass->addInternalCall("Internal_ColorGradientHDR0", (void*)&ScriptColorGradientHDR::Internal_ColorGradientHDR0);
		metaData.scriptClass->addInternalCall("Internal_ColorGradientHDR1", (void*)&ScriptColorGradientHDR::Internal_ColorGradientHDR1);
		metaData.scriptClass->addInternalCall("Internal_setKeys", (void*)&ScriptColorGradientHDR::Internal_setKeys);
		metaData.scriptClass->addInternalCall("Internal_getKeys", (void*)&ScriptColorGradientHDR::Internal_getKeys);
		metaData.scriptClass->addInternalCall("Internal_getNumKeys", (void*)&ScriptColorGradientHDR::Internal_getNumKeys);
		metaData.scriptClass->addInternalCall("Internal_getKey", (void*)&ScriptColorGradientHDR::Internal_getKey);
		metaData.scriptClass->addInternalCall("Internal_setConstant", (void*)&ScriptColorGradientHDR::Internal_setConstant);
		metaData.scriptClass->addInternalCall("Internal_evaluate", (void*)&ScriptColorGradientHDR::Internal_evaluate);

	}

	MonoObject* ScriptColorGradientHDR::create(const SPtr<ColorGradientHDR>& value)
	{
		if(value == nullptr) return nullptr; 

		bool dummy = false;
		void* ctorParams[1] = { &dummy };

		MonoObject* managedInstance = metaData.scriptClass->createInstance("bool", ctorParams);
		new (bs_alloc<ScriptColorGradientHDR>()) ScriptColorGradientHDR(managedInstance, value);
		return managedInstance;
	}
	void ScriptColorGradientHDR::Internal_ColorGradientHDR(MonoObject* managedInstance)
	{
		SPtr<ColorGradientHDR> instance = bs_shared_ptr_new<ColorGradientHDR>();
		new (bs_alloc<ScriptColorGradientHDR>())ScriptColorGradientHDR(managedInstance, instance);
	}

	void ScriptColorGradientHDR::Internal_ColorGradientHDR0(MonoObject* managedInstance, Color* color)
	{
		SPtr<ColorGradientHDR> instance = bs_shared_ptr_new<ColorGradientHDR>(*color);
		new (bs_alloc<ScriptColorGradientHDR>())ScriptColorGradientHDR(managedInstance, instance);
	}

	void ScriptColorGradientHDR::Internal_ColorGradientHDR1(MonoObject* managedInstance, MonoArray* keys)
	{
		Vector<ColorGradientKey> veckeys;
		if(keys != nullptr)
		{
			ScriptArray arraykeys(keys);
			veckeys.resize(arraykeys.size());
			for(int i = 0; i < (int)arraykeys.size(); i++)
			{
				veckeys[i] = ScriptColorGradientKey::fromInterop(arraykeys.get<__ColorGradientKeyInterop>(i));
			}
		}
		SPtr<ColorGradientHDR> instance = bs_shared_ptr_new<ColorGradientHDR>(veckeys);
		new (bs_alloc<ScriptColorGradientHDR>())ScriptColorGradientHDR(managedInstance, instance);
	}

	void ScriptColorGradientHDR::Internal_setKeys(ScriptColorGradientHDR* thisPtr, MonoArray* keys, float duration)
	{
		Vector<ColorGradientKey> veckeys;
		if(keys != nullptr)
		{
			ScriptArray arraykeys(keys);
			veckeys.resize(arraykeys.size());
			for(int i = 0; i < (int)arraykeys.size(); i++)
			{
				veckeys[i] = ScriptColorGradientKey::fromInterop(arraykeys.get<__ColorGradientKeyInterop>(i));
			}

		}
		thisPtr->getInternal()->setKeys(veckeys, duration);
	}

	MonoArray* ScriptColorGradientHDR::Internal_getKeys(ScriptColorGradientHDR* thisPtr)
	{
		Vector<ColorGradientKey> vec__output;
		vec__output = thisPtr->getInternal()->getKeys();

		MonoArray* __output;
		int arraySize__output = (int)vec__output.size();
		ScriptArray array__output = ScriptArray::create<ScriptColorGradientKey>(arraySize__output);
		for(int i = 0; i < arraySize__output; i++)
		{
			array__output.set(i, ScriptColorGradientKey::toInterop(vec__output[i]));
		}
		__output = array__output.getInternal();

		return __output;
	}

	uint32_t ScriptColorGradientHDR::Internal_getNumKeys(ScriptColorGradientHDR* thisPtr)
	{
		uint32_t tmp__output;
		tmp__output = thisPtr->getInternal()->getNumKeys();

		uint32_t __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptColorGradientHDR::Internal_getKey(ScriptColorGradientHDR* thisPtr, uint32_t idx, __ColorGradientKeyInterop* __output)
	{
		ColorGradientKey tmp__output;
		tmp__output = thisPtr->getInternal()->getKey(idx);

		__ColorGradientKeyInterop interop__output;
		interop__output = ScriptColorGradientKey::toInterop(tmp__output);
		MonoUtil::valueCopy(__output, &interop__output, ScriptColorGradientKey::getMetaData()->scriptClass->_getInternalClass());
	}

	void ScriptColorGradientHDR::Internal_setConstant(ScriptColorGradientHDR* thisPtr, Color* color)
	{
		thisPtr->getInternal()->setConstant(*color);
	}

	void ScriptColorGradientHDR::Internal_evaluate(ScriptColorGradientHDR* thisPtr, float t, Color* __output)
	{
		Color tmp__output;
		tmp__output = ColorGradientHDREx::evaluate(thisPtr->getInternal(), t);

		*__output = tmp__output;
	}
}
