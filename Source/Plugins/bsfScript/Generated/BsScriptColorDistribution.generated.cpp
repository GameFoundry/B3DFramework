//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptColorDistribution.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "../../../../../Foundation/bsfCore/Particles/BsParticleDistribution.h"
#include "BsScriptColorGradient.generated.h"
#include "Wrappers/BsScriptColor.h"
#include "BsScriptRandom.generated.h"
#include "../../../Extensions/BsParticleDistributionEx.h"

namespace bs
{
	ScriptColorDistribution::ScriptColorDistribution(MonoObject* managedInstance, const SPtr<ColorDistribution>& value)
		:ScriptObject(managedInstance), mInternal(value)
	{
	}

	void ScriptColorDistribution::initRuntimeData()
	{
		metaData.scriptClass->addInternalCall("Internal_ColorDistribution", (void*)&ScriptColorDistribution::Internal_ColorDistribution);
		metaData.scriptClass->addInternalCall("Internal_ColorDistribution0", (void*)&ScriptColorDistribution::Internal_ColorDistribution0);
		metaData.scriptClass->addInternalCall("Internal_ColorDistribution1", (void*)&ScriptColorDistribution::Internal_ColorDistribution1);
		metaData.scriptClass->addInternalCall("Internal_ColorDistribution2", (void*)&ScriptColorDistribution::Internal_ColorDistribution2);
		metaData.scriptClass->addInternalCall("Internal_ColorDistribution3", (void*)&ScriptColorDistribution::Internal_ColorDistribution3);
		metaData.scriptClass->addInternalCall("Internal_getType", (void*)&ScriptColorDistribution::Internal_getType);
		metaData.scriptClass->addInternalCall("Internal_getMinConstant", (void*)&ScriptColorDistribution::Internal_getMinConstant);
		metaData.scriptClass->addInternalCall("Internal_getMaxConstant", (void*)&ScriptColorDistribution::Internal_getMaxConstant);
		metaData.scriptClass->addInternalCall("Internal_getMinGradient", (void*)&ScriptColorDistribution::Internal_getMinGradient);
		metaData.scriptClass->addInternalCall("Internal_getMaxGradient", (void*)&ScriptColorDistribution::Internal_getMaxGradient);
		metaData.scriptClass->addInternalCall("Internal_evaluate", (void*)&ScriptColorDistribution::Internal_evaluate);
		metaData.scriptClass->addInternalCall("Internal_evaluate0", (void*)&ScriptColorDistribution::Internal_evaluate0);

	}

	MonoObject* ScriptColorDistribution::create(const SPtr<ColorDistribution>& value)
	{
		if(value == nullptr) return nullptr; 

		bool dummy = false;
		void* ctorParams[1] = { &dummy };

		MonoObject* managedInstance = metaData.scriptClass->createInstance("bool", ctorParams);
		new (bs_alloc<ScriptColorDistribution>()) ScriptColorDistribution(managedInstance, value);
		return managedInstance;
	}
	void ScriptColorDistribution::Internal_ColorDistribution(MonoObject* managedInstance)
	{
		SPtr<ColorDistribution> instance = bs_shared_ptr_new<ColorDistribution>();
		new (bs_alloc<ScriptColorDistribution>())ScriptColorDistribution(managedInstance, instance);
	}

	void ScriptColorDistribution::Internal_ColorDistribution0(MonoObject* managedInstance, Color* color)
	{
		SPtr<ColorDistribution> instance = bs_shared_ptr_new<ColorDistribution>(*color);
		new (bs_alloc<ScriptColorDistribution>())ScriptColorDistribution(managedInstance, instance);
	}

	void ScriptColorDistribution::Internal_ColorDistribution1(MonoObject* managedInstance, Color* minColor, Color* maxColor)
	{
		SPtr<ColorDistribution> instance = bs_shared_ptr_new<ColorDistribution>(*minColor, *maxColor);
		new (bs_alloc<ScriptColorDistribution>())ScriptColorDistribution(managedInstance, instance);
	}

	void ScriptColorDistribution::Internal_ColorDistribution2(MonoObject* managedInstance, MonoObject* gradient)
	{
		SPtr<ColorGradient> tmpgradient;
		ScriptColorGradient* scriptgradient;
		scriptgradient = ScriptColorGradient::toNative(gradient);
		if(scriptgradient != nullptr)
			tmpgradient = scriptgradient->getInternal();
		SPtr<ColorDistribution> instance = bs_shared_ptr_new<ColorDistribution>(*tmpgradient);
		new (bs_alloc<ScriptColorDistribution>())ScriptColorDistribution(managedInstance, instance);
	}

	void ScriptColorDistribution::Internal_ColorDistribution3(MonoObject* managedInstance, MonoObject* minGradient, MonoObject* maxGradient)
	{
		SPtr<ColorGradient> tmpminGradient;
		ScriptColorGradient* scriptminGradient;
		scriptminGradient = ScriptColorGradient::toNative(minGradient);
		if(scriptminGradient != nullptr)
			tmpminGradient = scriptminGradient->getInternal();
		SPtr<ColorGradient> tmpmaxGradient;
		ScriptColorGradient* scriptmaxGradient;
		scriptmaxGradient = ScriptColorGradient::toNative(maxGradient);
		if(scriptmaxGradient != nullptr)
			tmpmaxGradient = scriptmaxGradient->getInternal();
		SPtr<ColorDistribution> instance = bs_shared_ptr_new<ColorDistribution>(*tmpminGradient, *tmpmaxGradient);
		new (bs_alloc<ScriptColorDistribution>())ScriptColorDistribution(managedInstance, instance);
	}

	PropertyDistributionType ScriptColorDistribution::Internal_getType(ScriptColorDistribution* thisPtr)
	{
		PropertyDistributionType tmp__output;
		tmp__output = thisPtr->getInternal()->getType();

		PropertyDistributionType __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptColorDistribution::Internal_getMinConstant(ScriptColorDistribution* thisPtr, Color* __output)
	{
		Color tmp__output;
		tmp__output = thisPtr->getInternal()->getMinConstant();

		*__output = tmp__output;
	}

	void ScriptColorDistribution::Internal_getMaxConstant(ScriptColorDistribution* thisPtr, Color* __output)
	{
		Color tmp__output;
		tmp__output = thisPtr->getInternal()->getMaxConstant();

		*__output = tmp__output;
	}

	MonoObject* ScriptColorDistribution::Internal_getMinGradient(ScriptColorDistribution* thisPtr)
	{
		SPtr<ColorGradient> tmp__output = bs_shared_ptr_new<ColorGradient>();
		*tmp__output = thisPtr->getInternal()->getMinGradient();

		MonoObject* __output;
		__output = ScriptColorGradient::create(tmp__output);

		return __output;
	}

	MonoObject* ScriptColorDistribution::Internal_getMaxGradient(ScriptColorDistribution* thisPtr)
	{
		SPtr<ColorGradient> tmp__output = bs_shared_ptr_new<ColorGradient>();
		*tmp__output = thisPtr->getInternal()->getMaxGradient();

		MonoObject* __output;
		__output = ScriptColorGradient::create(tmp__output);

		return __output;
	}

	void ScriptColorDistribution::Internal_evaluate(ScriptColorDistribution* thisPtr, float t, float factor, Color* __output)
	{
		Color tmp__output;
		tmp__output = ColorDistributionEx::evaluate(thisPtr->getInternal(), t, factor);

		*__output = tmp__output;
	}

	void ScriptColorDistribution::Internal_evaluate0(ScriptColorDistribution* thisPtr, float t, MonoObject** factor, Color* __output)
	{
		SPtr<Random> tmpfactor = bs_shared_ptr_new<Random>();
		Color tmp__output;
		tmp__output = ColorDistributionEx::evaluate(thisPtr->getInternal(), t, *tmpfactor);

		MonoUtil::referenceCopy(factor, ScriptRandom::create(tmpfactor));
		*__output = tmp__output;
	}
}
