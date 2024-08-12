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
	ScriptColorDistribution::ScriptColorDistribution(MonoObject* managedInstance, const SPtr<TColorDistribution<ColorGradient>>& value)
		:ScriptObject(managedInstance), mInternal(value)
	{
	}

	void ScriptColorDistribution::InitRuntimeData()
	{
		metaData.ScriptClass->AddInternalCall("Internal_TColorDistribution", (void*)&ScriptColorDistribution::InternalTColorDistribution);
		metaData.ScriptClass->AddInternalCall("Internal_TColorDistribution0", (void*)&ScriptColorDistribution::InternalTColorDistribution0);
		metaData.ScriptClass->AddInternalCall("Internal_TColorDistribution1", (void*)&ScriptColorDistribution::InternalTColorDistribution1);
		metaData.ScriptClass->AddInternalCall("Internal_TColorDistribution2", (void*)&ScriptColorDistribution::InternalTColorDistribution2);
		metaData.ScriptClass->AddInternalCall("Internal_TColorDistribution3", (void*)&ScriptColorDistribution::InternalTColorDistribution3);
		metaData.ScriptClass->AddInternalCall("Internal_GetType", (void*)&ScriptColorDistribution::InternalGetType);
		metaData.ScriptClass->AddInternalCall("Internal_GetMinConstant", (void*)&ScriptColorDistribution::InternalGetMinConstant);
		metaData.ScriptClass->AddInternalCall("Internal_GetMaxConstant", (void*)&ScriptColorDistribution::InternalGetMaxConstant);
		metaData.ScriptClass->AddInternalCall("Internal_GetMinGradient", (void*)&ScriptColorDistribution::InternalGetMinGradient);
		metaData.ScriptClass->AddInternalCall("Internal_GetMaxGradient", (void*)&ScriptColorDistribution::InternalGetMaxGradient);

	}

	MonoObject* ScriptColorDistribution::Create(const SPtr<TColorDistribution<ColorGradient>>& value)
	{
		if(value == nullptr) return nullptr; 

		bool dummy = false;
		void* ctorParams[1] = { &dummy };

		MonoObject* managedInstance = metaData.ScriptClass->CreateInstance("bool", ctorParams);
		new (B3DAllocate<ScriptColorDistribution>()) ScriptColorDistribution(managedInstance, value);
		return managedInstance;
	}
	void ScriptColorDistribution::InternalTColorDistribution(MonoObject* managedInstance)
	{
		SPtr<TColorDistribution<ColorGradient>> instance = B3DMakeShared<TColorDistribution<ColorGradient>>();
		new (B3DAllocate<ScriptColorDistribution>())ScriptColorDistribution(managedInstance, instance);
	}

	void ScriptColorDistribution::InternalTColorDistribution0(MonoObject* managedInstance, Color* color)
	{
		SPtr<TColorDistribution<ColorGradient>> instance = B3DMakeShared<TColorDistribution<ColorGradient>>(*color);
		new (B3DAllocate<ScriptColorDistribution>())ScriptColorDistribution(managedInstance, instance);
	}

	void ScriptColorDistribution::InternalTColorDistribution1(MonoObject* managedInstance, Color* minColor, Color* maxColor)
	{
		SPtr<TColorDistribution<ColorGradient>> instance = B3DMakeShared<TColorDistribution<ColorGradient>>(*minColor, *maxColor);
		new (B3DAllocate<ScriptColorDistribution>())ScriptColorDistribution(managedInstance, instance);
	}

	void ScriptColorDistribution::InternalTColorDistribution2(MonoObject* managedInstance, MonoObject* gradient)
	{
		SPtr<ColorGradient> tmpgradient;
		ScriptColorGradient* scriptObjectWrappergradient;
		scriptObjectWrappergradient = ScriptColorGradient::ToNative(gradient);
		if(scriptObjectWrappergradient != nullptr)
			tmpgradient = scriptObjectWrappergradient->GetInternal();
		SPtr<TColorDistribution<ColorGradient>> instance = B3DMakeShared<TColorDistribution<ColorGradient>>(*tmpgradient);
		new (B3DAllocate<ScriptColorDistribution>())ScriptColorDistribution(managedInstance, instance);
	}

	void ScriptColorDistribution::InternalTColorDistribution3(MonoObject* managedInstance, MonoObject* minGradient, MonoObject* maxGradient)
	{
		SPtr<ColorGradient> tmpminGradient;
		ScriptColorGradient* scriptObjectWrapperminGradient;
		scriptObjectWrapperminGradient = ScriptColorGradient::ToNative(minGradient);
		if(scriptObjectWrapperminGradient != nullptr)
			tmpminGradient = scriptObjectWrapperminGradient->GetInternal();
		SPtr<ColorGradient> tmpmaxGradient;
		ScriptColorGradient* scriptObjectWrappermaxGradient;
		scriptObjectWrappermaxGradient = ScriptColorGradient::ToNative(maxGradient);
		if(scriptObjectWrappermaxGradient != nullptr)
			tmpmaxGradient = scriptObjectWrappermaxGradient->GetInternal();
		SPtr<TColorDistribution<ColorGradient>> instance = B3DMakeShared<TColorDistribution<ColorGradient>>(*tmpminGradient, *tmpmaxGradient);
		new (B3DAllocate<ScriptColorDistribution>())ScriptColorDistribution(managedInstance, instance);
	}

	PropertyDistributionType ScriptColorDistribution::InternalGetType(ScriptColorDistribution* thisPtr)
	{
		PropertyDistributionType tmp__output;
		tmp__output = thisPtr->GetInternal()->GetType();

		PropertyDistributionType __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptColorDistribution::InternalGetMinConstant(ScriptColorDistribution* thisPtr, Color* __output)
	{
		Color tmp__output;
		tmp__output = thisPtr->GetInternal()->GetMinConstant();

		*__output = tmp__output;
	}

	void ScriptColorDistribution::InternalGetMaxConstant(ScriptColorDistribution* thisPtr, Color* __output)
	{
		Color tmp__output;
		tmp__output = thisPtr->GetInternal()->GetMaxConstant();

		*__output = tmp__output;
	}

	MonoObject* ScriptColorDistribution::InternalGetMinGradient(ScriptColorDistribution* thisPtr)
	{
		SPtr<ColorGradient> tmp__output = B3DMakeShared<ColorGradient>();
		*tmp__output = thisPtr->GetInternal()->GetMinGradient();

		MonoObject* __output;
		__output = ScriptColorGradient::Create(tmp__output);

		return __output;
	}

	MonoObject* ScriptColorDistribution::InternalGetMaxGradient(ScriptColorDistribution* thisPtr)
	{
		SPtr<ColorGradient> tmp__output = B3DMakeShared<ColorGradient>();
		*tmp__output = thisPtr->GetInternal()->GetMaxGradient();

		MonoObject* __output;
		__output = ScriptColorGradient::Create(tmp__output);

		return __output;
	}

	ScriptColorHDRDistribution::ScriptColorHDRDistribution(MonoObject* managedInstance, const SPtr<TColorDistribution<ColorGradientHDR>>& value)
		:ScriptObject(managedInstance), mInternal(value)
	{
	}

	void ScriptColorHDRDistribution::InitRuntimeData()
	{
		metaData.ScriptClass->AddInternalCall("Internal_TColorDistribution", (void*)&ScriptColorHDRDistribution::InternalTColorDistribution);
		metaData.ScriptClass->AddInternalCall("Internal_TColorDistribution0", (void*)&ScriptColorHDRDistribution::InternalTColorDistribution0);
		metaData.ScriptClass->AddInternalCall("Internal_TColorDistribution1", (void*)&ScriptColorHDRDistribution::InternalTColorDistribution1);
		metaData.ScriptClass->AddInternalCall("Internal_TColorDistribution2", (void*)&ScriptColorHDRDistribution::InternalTColorDistribution2);
		metaData.ScriptClass->AddInternalCall("Internal_TColorDistribution3", (void*)&ScriptColorHDRDistribution::InternalTColorDistribution3);
		metaData.ScriptClass->AddInternalCall("Internal_GetType", (void*)&ScriptColorHDRDistribution::InternalGetType);
		metaData.ScriptClass->AddInternalCall("Internal_GetMinConstant", (void*)&ScriptColorHDRDistribution::InternalGetMinConstant);
		metaData.ScriptClass->AddInternalCall("Internal_GetMaxConstant", (void*)&ScriptColorHDRDistribution::InternalGetMaxConstant);
		metaData.ScriptClass->AddInternalCall("Internal_GetMinGradient", (void*)&ScriptColorHDRDistribution::InternalGetMinGradient);
		metaData.ScriptClass->AddInternalCall("Internal_GetMaxGradient", (void*)&ScriptColorHDRDistribution::InternalGetMaxGradient);

	}

	MonoObject* ScriptColorHDRDistribution::Create(const SPtr<TColorDistribution<ColorGradientHDR>>& value)
	{
		if(value == nullptr) return nullptr; 

		bool dummy = false;
		void* ctorParams[1] = { &dummy };

		MonoObject* managedInstance = metaData.ScriptClass->CreateInstance("bool", ctorParams);
		new (B3DAllocate<ScriptColorHDRDistribution>()) ScriptColorHDRDistribution(managedInstance, value);
		return managedInstance;
	}
	void ScriptColorHDRDistribution::InternalTColorDistribution(MonoObject* managedInstance)
	{
		SPtr<TColorDistribution<ColorGradientHDR>> instance = B3DMakeShared<TColorDistribution<ColorGradientHDR>>();
		new (B3DAllocate<ScriptColorHDRDistribution>())ScriptColorHDRDistribution(managedInstance, instance);
	}

	void ScriptColorHDRDistribution::InternalTColorDistribution0(MonoObject* managedInstance, Color* color)
	{
		SPtr<TColorDistribution<ColorGradientHDR>> instance = B3DMakeShared<TColorDistribution<ColorGradientHDR>>(*color);
		new (B3DAllocate<ScriptColorHDRDistribution>())ScriptColorHDRDistribution(managedInstance, instance);
	}

	void ScriptColorHDRDistribution::InternalTColorDistribution1(MonoObject* managedInstance, Color* minColor, Color* maxColor)
	{
		SPtr<TColorDistribution<ColorGradientHDR>> instance = B3DMakeShared<TColorDistribution<ColorGradientHDR>>(*minColor, *maxColor);
		new (B3DAllocate<ScriptColorHDRDistribution>())ScriptColorHDRDistribution(managedInstance, instance);
	}

	void ScriptColorHDRDistribution::InternalTColorDistribution2(MonoObject* managedInstance, MonoObject* gradient)
	{
		SPtr<ColorGradientHDR> tmpgradient;
		ScriptColorGradientHDR* scriptObjectWrappergradient;
		scriptObjectWrappergradient = ScriptColorGradientHDR::ToNative(gradient);
		if(scriptObjectWrappergradient != nullptr)
			tmpgradient = scriptObjectWrappergradient->GetInternal();
		SPtr<TColorDistribution<ColorGradientHDR>> instance = B3DMakeShared<TColorDistribution<ColorGradientHDR>>(*tmpgradient);
		new (B3DAllocate<ScriptColorHDRDistribution>())ScriptColorHDRDistribution(managedInstance, instance);
	}

	void ScriptColorHDRDistribution::InternalTColorDistribution3(MonoObject* managedInstance, MonoObject* minGradient, MonoObject* maxGradient)
	{
		SPtr<ColorGradientHDR> tmpminGradient;
		ScriptColorGradientHDR* scriptObjectWrapperminGradient;
		scriptObjectWrapperminGradient = ScriptColorGradientHDR::ToNative(minGradient);
		if(scriptObjectWrapperminGradient != nullptr)
			tmpminGradient = scriptObjectWrapperminGradient->GetInternal();
		SPtr<ColorGradientHDR> tmpmaxGradient;
		ScriptColorGradientHDR* scriptObjectWrappermaxGradient;
		scriptObjectWrappermaxGradient = ScriptColorGradientHDR::ToNative(maxGradient);
		if(scriptObjectWrappermaxGradient != nullptr)
			tmpmaxGradient = scriptObjectWrappermaxGradient->GetInternal();
		SPtr<TColorDistribution<ColorGradientHDR>> instance = B3DMakeShared<TColorDistribution<ColorGradientHDR>>(*tmpminGradient, *tmpmaxGradient);
		new (B3DAllocate<ScriptColorHDRDistribution>())ScriptColorHDRDistribution(managedInstance, instance);
	}

	PropertyDistributionType ScriptColorHDRDistribution::InternalGetType(ScriptColorHDRDistribution* thisPtr)
	{
		PropertyDistributionType tmp__output;
		tmp__output = thisPtr->GetInternal()->GetType();

		PropertyDistributionType __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptColorHDRDistribution::InternalGetMinConstant(ScriptColorHDRDistribution* thisPtr, Color* __output)
	{
		Color tmp__output;
		tmp__output = thisPtr->GetInternal()->GetMinConstant();

		*__output = tmp__output;
	}

	void ScriptColorHDRDistribution::InternalGetMaxConstant(ScriptColorHDRDistribution* thisPtr, Color* __output)
	{
		Color tmp__output;
		tmp__output = thisPtr->GetInternal()->GetMaxConstant();

		*__output = tmp__output;
	}

	MonoObject* ScriptColorHDRDistribution::InternalGetMinGradient(ScriptColorHDRDistribution* thisPtr)
	{
		SPtr<ColorGradientHDR> tmp__output = B3DMakeShared<ColorGradientHDR>();
		*tmp__output = thisPtr->GetInternal()->GetMinGradient();

		MonoObject* __output;
		__output = ScriptColorGradientHDR::Create(tmp__output);

		return __output;
	}

	MonoObject* ScriptColorHDRDistribution::InternalGetMaxGradient(ScriptColorHDRDistribution* thisPtr)
	{
		SPtr<ColorGradientHDR> tmp__output = B3DMakeShared<ColorGradientHDR>();
		*tmp__output = thisPtr->GetInternal()->GetMaxGradient();

		MonoObject* __output;
		__output = ScriptColorGradientHDR::Create(tmp__output);

		return __output;
	}
}
