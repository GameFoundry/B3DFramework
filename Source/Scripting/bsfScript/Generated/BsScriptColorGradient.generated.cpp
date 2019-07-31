//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
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

	void ScriptColorGradient::initRuntimeData()
	{
		metaData.scriptClass->addInternalCall("Internal_ColorGradient", (void*)&ScriptColorGradient::Internal_ColorGradient);
		metaData.scriptClass->addInternalCall("Internal_ColorGradient0", (void*)&ScriptColorGradient::Internal_ColorGradient0);
		metaData.scriptClass->addInternalCall("Internal_ColorGradient1", (void*)&ScriptColorGradient::Internal_ColorGradient1);
		metaData.scriptClass->addInternalCall("Internal_setKeys", (void*)&ScriptColorGradient::Internal_setKeys);
		metaData.scriptClass->addInternalCall("Internal_getKeys", (void*)&ScriptColorGradient::Internal_getKeys);
		metaData.scriptClass->addInternalCall("Internal_getNumKeys", (void*)&ScriptColorGradient::Internal_getNumKeys);
		metaData.scriptClass->addInternalCall("Internal_getKey", (void*)&ScriptColorGradient::Internal_getKey);
		metaData.scriptClass->addInternalCall("Internal_setConstant", (void*)&ScriptColorGradient::Internal_setConstant);
		metaData.scriptClass->addInternalCall("Internal_evaluate", (void*)&ScriptColorGradient::Internal_evaluate);

	}

	MonoObject* ScriptColorGradient::create(const SPtr<ColorGradient>& value)
	{
		if(value == nullptr) return nullptr; 

		bool dummy = false;
		void* ctorParams[1] = { &dummy };

		MonoObject* managedInstance = metaData.scriptClass->createInstance("bool", ctorParams);
		new (bs_alloc<ScriptColorGradient>()) ScriptColorGradient(managedInstance, value);
		return managedInstance;
	}
	void ScriptColorGradient::Internal_ColorGradient(MonoObject* managedInstance)
	{
		SPtr<ColorGradient> instance = bs_shared_ptr_new<ColorGradient>();
		new (bs_alloc<ScriptColorGradient>())ScriptColorGradient(managedInstance, instance);
	}

	void ScriptColorGradient::Internal_ColorGradient0(MonoObject* managedInstance, Color* color)
	{
		SPtr<ColorGradient> instance = bs_shared_ptr_new<ColorGradient>(*color);
		new (bs_alloc<ScriptColorGradient>())ScriptColorGradient(managedInstance, instance);
	}

	void ScriptColorGradient::Internal_ColorGradient1(MonoObject* managedInstance, MonoArray* keys)
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
		SPtr<ColorGradient> instance = bs_shared_ptr_new<ColorGradient>(veckeys);
		new (bs_alloc<ScriptColorGradient>())ScriptColorGradient(managedInstance, instance);
	}

	void ScriptColorGradient::Internal_setKeys(ScriptColorGradient* thisPtr, MonoArray* keys, float duration)
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

	MonoArray* ScriptColorGradient::Internal_getKeys(ScriptColorGradient* thisPtr)
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

	uint32_t ScriptColorGradient::Internal_getNumKeys(ScriptColorGradient* thisPtr)
	{
		uint32_t tmp__output;
		tmp__output = thisPtr->getInternal()->getNumKeys();

		uint32_t __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptColorGradient::Internal_getKey(ScriptColorGradient* thisPtr, uint32_t idx, __ColorGradientKeyInterop* __output)
	{
		ColorGradientKey tmp__output;
		tmp__output = thisPtr->getInternal()->getKey(idx);

		__ColorGradientKeyInterop interop__output;
		interop__output = ScriptColorGradientKey::toInterop(tmp__output);
		MonoUtil::valueCopy(__output, &interop__output, ScriptColorGradientKey::getMetaData()->scriptClass->_getInternalClass());
	}

	void ScriptColorGradient::Internal_setConstant(ScriptColorGradient* thisPtr, Color* color)
	{
		thisPtr->getInternal()->setConstant(*color);
	}

	void ScriptColorGradient::Internal_evaluate(ScriptColorGradient* thisPtr, float t, Color* __output)
	{
		Color tmp__output;
		tmp__output = ColorGradientEx::evaluate(thisPtr->getInternal(), t);

		*__output = tmp__output;
	}
}
