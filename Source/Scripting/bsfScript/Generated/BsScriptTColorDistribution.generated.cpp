//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptTColorDistribution.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "BsScriptColorGradient.generated.h"
#include "Wrappers/BsScriptColor.h"
#include "BsScriptColorGradientHDR.generated.h"

namespace bs
{
	ScriptTColorDistributionColorGradient::ScriptTColorDistributionColorGradient(MonoObject* managedInstance, const SPtr<TColorDistribution<ColorGradient>>& value)
		:ScriptObject(managedInstance), mInternal(value)
	{
	}

	void ScriptTColorDistributionColorGradient::InitRuntimeData()
	{
		metaData.ScriptClass->AddInternalCall("Internal_TColorDistribution", (void*)&ScriptTColorDistributionColorGradient::InternalTColorDistribution);
		metaData.ScriptClass->AddInternalCall("Internal_TColorDistribution0", (void*)&ScriptTColorDistributionColorGradient::InternalTColorDistribution0);
		metaData.ScriptClass->AddInternalCall("Internal_TColorDistribution1", (void*)&ScriptTColorDistributionColorGradient::InternalTColorDistribution1);
		metaData.ScriptClass->AddInternalCall("Internal_TColorDistribution2", (void*)&ScriptTColorDistributionColorGradient::InternalTColorDistribution2);
		metaData.ScriptClass->AddInternalCall("Internal_TColorDistribution3", (void*)&ScriptTColorDistributionColorGradient::InternalTColorDistribution3);
		metaData.ScriptClass->AddInternalCall("Internal_GetType", (void*)&ScriptTColorDistributionColorGradient::InternalGetType);
		metaData.ScriptClass->AddInternalCall("Internal_GetMinConstant", (void*)&ScriptTColorDistributionColorGradient::InternalGetMinConstant);
		metaData.ScriptClass->AddInternalCall("Internal_GetMaxConstant", (void*)&ScriptTColorDistributionColorGradient::InternalGetMaxConstant);
		metaData.ScriptClass->AddInternalCall("Internal_GetMinGradient", (void*)&ScriptTColorDistributionColorGradient::InternalGetMinGradient);
		metaData.ScriptClass->AddInternalCall("Internal_GetMaxGradient", (void*)&ScriptTColorDistributionColorGradient::InternalGetMaxGradient);

	}

	MonoObject* ScriptTColorDistributionColorGradient::Create(const SPtr<TColorDistribution<ColorGradient>>& value)
	{
		if(value == nullptr) return nullptr; 

		bool dummy = false;
		void* ctorParams[1] = { &dummy };

		MonoObject* managedInstance = metaData.ScriptClass->CreateInstance("bool", ctorParams);
		new (B3DAllocate<ScriptTColorDistributionColorGradient>()) ScriptTColorDistributionColorGradient(managedInstance, value);
		return managedInstance;
	}
	void ScriptTColorDistributionColorGradient::InternalTColorDistribution(MonoObject* managedInstance)
	{
		SPtr<TColorDistribution<ColorGradient>> instance = B3DMakeShared<TColorDistribution<ColorGradient>>();
		new (B3DAllocate<ScriptTColorDistributionColorGradient>())ScriptTColorDistributionColorGradient(managedInstance, instance);
	}

	void ScriptTColorDistributionColorGradient::InternalTColorDistribution0(MonoObject* managedInstance, Color* color)
	{
		SPtr<TColorDistribution<ColorGradient>> instance = B3DMakeShared<TColorDistribution<ColorGradient>>(*color);
		new (B3DAllocate<ScriptTColorDistributionColorGradient>())ScriptTColorDistributionColorGradient(managedInstance, instance);
	}

	void ScriptTColorDistributionColorGradient::InternalTColorDistribution1(MonoObject* managedInstance, Color* minColor, Color* maxColor)
	{
		SPtr<TColorDistribution<ColorGradient>> instance = B3DMakeShared<TColorDistribution<ColorGradient>>(*minColor, *maxColor);
		new (B3DAllocate<ScriptTColorDistributionColorGradient>())ScriptTColorDistributionColorGradient(managedInstance, instance);
	}

	void ScriptTColorDistributionColorGradient::InternalTColorDistribution2(MonoObject* managedInstance, MonoObject* gradient)
	{
		SPtr<ColorGradient> tmpgradient;
		ScriptColorGradient* scriptgradient;
		scriptgradient = ScriptColorGradient::ToNative(gradient);
		if(scriptgradient != nullptr)
			tmpgradient = scriptgradient->GetInternal();
		SPtr<TColorDistribution<ColorGradient>> instance = B3DMakeShared<TColorDistribution<ColorGradient>>(*tmpgradient);
		new (B3DAllocate<ScriptTColorDistributionColorGradient>())ScriptTColorDistributionColorGradient(managedInstance, instance);
	}

	void ScriptTColorDistributionColorGradient::InternalTColorDistribution3(MonoObject* managedInstance, MonoObject* minGradient, MonoObject* maxGradient)
	{
		SPtr<ColorGradient> tmpminGradient;
		ScriptColorGradient* scriptminGradient;
		scriptminGradient = ScriptColorGradient::ToNative(minGradient);
		if(scriptminGradient != nullptr)
			tmpminGradient = scriptminGradient->GetInternal();
		SPtr<ColorGradient> tmpmaxGradient;
		ScriptColorGradient* scriptmaxGradient;
		scriptmaxGradient = ScriptColorGradient::ToNative(maxGradient);
		if(scriptmaxGradient != nullptr)
			tmpmaxGradient = scriptmaxGradient->GetInternal();
		SPtr<TColorDistribution<ColorGradient>> instance = B3DMakeShared<TColorDistribution<ColorGradient>>(*tmpminGradient, *tmpmaxGradient);
		new (B3DAllocate<ScriptTColorDistributionColorGradient>())ScriptTColorDistributionColorGradient(managedInstance, instance);
	}

	PropertyDistributionType ScriptTColorDistributionColorGradient::InternalGetType(ScriptTColorDistributionColorGradient* thisPtr)
	{
		PropertyDistributionType tmp__output;
		tmp__output = thisPtr->GetInternal()->GetType();

		PropertyDistributionType __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptTColorDistributionColorGradient::InternalGetMinConstant(ScriptTColorDistributionColorGradient* thisPtr, Color* __output)
	{
		Color tmp__output;
		tmp__output = thisPtr->GetInternal()->GetMinConstant();

		*__output = tmp__output;
	}

	void ScriptTColorDistributionColorGradient::InternalGetMaxConstant(ScriptTColorDistributionColorGradient* thisPtr, Color* __output)
	{
		Color tmp__output;
		tmp__output = thisPtr->GetInternal()->GetMaxConstant();

		*__output = tmp__output;
	}

	MonoObject* ScriptTColorDistributionColorGradient::InternalGetMinGradient(ScriptTColorDistributionColorGradient* thisPtr)
	{
		SPtr<ColorGradient> tmp__output = B3DMakeShared<ColorGradient>();
		*tmp__output = thisPtr->GetInternal()->GetMinGradient();

		MonoObject* __output;
		__output = ScriptColorGradient::Create(tmp__output);

		return __output;
	}

	MonoObject* ScriptTColorDistributionColorGradient::InternalGetMaxGradient(ScriptTColorDistributionColorGradient* thisPtr)
	{
		SPtr<ColorGradient> tmp__output = B3DMakeShared<ColorGradient>();
		*tmp__output = thisPtr->GetInternal()->GetMaxGradient();

		MonoObject* __output;
		__output = ScriptColorGradient::Create(tmp__output);

		return __output;
	}

	ScriptTColorDistributionColorGradientHDR::ScriptTColorDistributionColorGradientHDR(MonoObject* managedInstance, const SPtr<TColorDistribution<ColorGradientHDR>>& value)
		:ScriptObject(managedInstance), mInternal(value)
	{
	}

	void ScriptTColorDistributionColorGradientHDR::InitRuntimeData()
	{
		metaData.ScriptClass->AddInternalCall("Internal_TColorDistribution", (void*)&ScriptTColorDistributionColorGradientHDR::InternalTColorDistribution);
		metaData.ScriptClass->AddInternalCall("Internal_TColorDistribution0", (void*)&ScriptTColorDistributionColorGradientHDR::InternalTColorDistribution0);
		metaData.ScriptClass->AddInternalCall("Internal_TColorDistribution1", (void*)&ScriptTColorDistributionColorGradientHDR::InternalTColorDistribution1);
		metaData.ScriptClass->AddInternalCall("Internal_TColorDistribution2", (void*)&ScriptTColorDistributionColorGradientHDR::InternalTColorDistribution2);
		metaData.ScriptClass->AddInternalCall("Internal_TColorDistribution3", (void*)&ScriptTColorDistributionColorGradientHDR::InternalTColorDistribution3);
		metaData.ScriptClass->AddInternalCall("Internal_GetType", (void*)&ScriptTColorDistributionColorGradientHDR::InternalGetType);
		metaData.ScriptClass->AddInternalCall("Internal_GetMinConstant", (void*)&ScriptTColorDistributionColorGradientHDR::InternalGetMinConstant);
		metaData.ScriptClass->AddInternalCall("Internal_GetMaxConstant", (void*)&ScriptTColorDistributionColorGradientHDR::InternalGetMaxConstant);
		metaData.ScriptClass->AddInternalCall("Internal_GetMinGradient", (void*)&ScriptTColorDistributionColorGradientHDR::InternalGetMinGradient);
		metaData.ScriptClass->AddInternalCall("Internal_GetMaxGradient", (void*)&ScriptTColorDistributionColorGradientHDR::InternalGetMaxGradient);

	}

	MonoObject* ScriptTColorDistributionColorGradientHDR::Create(const SPtr<TColorDistribution<ColorGradientHDR>>& value)
	{
		if(value == nullptr) return nullptr; 

		bool dummy = false;
		void* ctorParams[1] = { &dummy };

		MonoObject* managedInstance = metaData.ScriptClass->CreateInstance("bool", ctorParams);
		new (B3DAllocate<ScriptTColorDistributionColorGradientHDR>()) ScriptTColorDistributionColorGradientHDR(managedInstance, value);
		return managedInstance;
	}
	void ScriptTColorDistributionColorGradientHDR::InternalTColorDistribution(MonoObject* managedInstance)
	{
		SPtr<TColorDistribution<ColorGradientHDR>> instance = B3DMakeShared<TColorDistribution<ColorGradientHDR>>();
		new (B3DAllocate<ScriptTColorDistributionColorGradientHDR>())ScriptTColorDistributionColorGradientHDR(managedInstance, instance);
	}

	void ScriptTColorDistributionColorGradientHDR::InternalTColorDistribution0(MonoObject* managedInstance, Color* color)
	{
		SPtr<TColorDistribution<ColorGradientHDR>> instance = B3DMakeShared<TColorDistribution<ColorGradientHDR>>(*color);
		new (B3DAllocate<ScriptTColorDistributionColorGradientHDR>())ScriptTColorDistributionColorGradientHDR(managedInstance, instance);
	}

	void ScriptTColorDistributionColorGradientHDR::InternalTColorDistribution1(MonoObject* managedInstance, Color* minColor, Color* maxColor)
	{
		SPtr<TColorDistribution<ColorGradientHDR>> instance = B3DMakeShared<TColorDistribution<ColorGradientHDR>>(*minColor, *maxColor);
		new (B3DAllocate<ScriptTColorDistributionColorGradientHDR>())ScriptTColorDistributionColorGradientHDR(managedInstance, instance);
	}

	void ScriptTColorDistributionColorGradientHDR::InternalTColorDistribution2(MonoObject* managedInstance, MonoObject* gradient)
	{
		SPtr<ColorGradientHDR> tmpgradient;
		ScriptColorGradientHDR* scriptgradient;
		scriptgradient = ScriptColorGradientHDR::ToNative(gradient);
		if(scriptgradient != nullptr)
			tmpgradient = scriptgradient->GetInternal();
		SPtr<TColorDistribution<ColorGradientHDR>> instance = B3DMakeShared<TColorDistribution<ColorGradientHDR>>(*tmpgradient);
		new (B3DAllocate<ScriptTColorDistributionColorGradientHDR>())ScriptTColorDistributionColorGradientHDR(managedInstance, instance);
	}

	void ScriptTColorDistributionColorGradientHDR::InternalTColorDistribution3(MonoObject* managedInstance, MonoObject* minGradient, MonoObject* maxGradient)
	{
		SPtr<ColorGradientHDR> tmpminGradient;
		ScriptColorGradientHDR* scriptminGradient;
		scriptminGradient = ScriptColorGradientHDR::ToNative(minGradient);
		if(scriptminGradient != nullptr)
			tmpminGradient = scriptminGradient->GetInternal();
		SPtr<ColorGradientHDR> tmpmaxGradient;
		ScriptColorGradientHDR* scriptmaxGradient;
		scriptmaxGradient = ScriptColorGradientHDR::ToNative(maxGradient);
		if(scriptmaxGradient != nullptr)
			tmpmaxGradient = scriptmaxGradient->GetInternal();
		SPtr<TColorDistribution<ColorGradientHDR>> instance = B3DMakeShared<TColorDistribution<ColorGradientHDR>>(*tmpminGradient, *tmpmaxGradient);
		new (B3DAllocate<ScriptTColorDistributionColorGradientHDR>())ScriptTColorDistributionColorGradientHDR(managedInstance, instance);
	}

	PropertyDistributionType ScriptTColorDistributionColorGradientHDR::InternalGetType(ScriptTColorDistributionColorGradientHDR* thisPtr)
	{
		PropertyDistributionType tmp__output;
		tmp__output = thisPtr->GetInternal()->GetType();

		PropertyDistributionType __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptTColorDistributionColorGradientHDR::InternalGetMinConstant(ScriptTColorDistributionColorGradientHDR* thisPtr, Color* __output)
	{
		Color tmp__output;
		tmp__output = thisPtr->GetInternal()->GetMinConstant();

		*__output = tmp__output;
	}

	void ScriptTColorDistributionColorGradientHDR::InternalGetMaxConstant(ScriptTColorDistributionColorGradientHDR* thisPtr, Color* __output)
	{
		Color tmp__output;
		tmp__output = thisPtr->GetInternal()->GetMaxConstant();

		*__output = tmp__output;
	}

	MonoObject* ScriptTColorDistributionColorGradientHDR::InternalGetMinGradient(ScriptTColorDistributionColorGradientHDR* thisPtr)
	{
		SPtr<ColorGradientHDR> tmp__output = B3DMakeShared<ColorGradientHDR>();
		*tmp__output = thisPtr->GetInternal()->GetMinGradient();

		MonoObject* __output;
		__output = ScriptColorGradientHDR::Create(tmp__output);

		return __output;
	}

	MonoObject* ScriptTColorDistributionColorGradientHDR::InternalGetMaxGradient(ScriptTColorDistributionColorGradientHDR* thisPtr)
	{
		SPtr<ColorGradientHDR> tmp__output = B3DMakeShared<ColorGradientHDR>();
		*tmp__output = thisPtr->GetInternal()->GetMaxGradient();

		MonoObject* __output;
		__output = ScriptColorGradientHDR::Create(tmp__output);

		return __output;
	}
}
