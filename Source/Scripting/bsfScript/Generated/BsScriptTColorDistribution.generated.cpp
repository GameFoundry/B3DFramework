//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptTColorDistribution.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "BsScriptColorGradient.generated.h"
#include "BsScriptColor.generated.h"
#include "BsScriptColorGradientHDR.generated.h"

namespace bs
{
	ScriptColorDistribution::ScriptColorDistribution(const SPtr<TColorDistribution<ColorGradient>>& nativeObject)
		:TScriptNonReflectableWrapper(nativeObject)
	{
		RegisterEvents();
	}

	void ScriptColorDistribution::SetupScriptBindings()
	{
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_TColorDistribution", (void*)&ScriptColorDistribution::InternalTColorDistribution);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_TColorDistribution0", (void*)&ScriptColorDistribution::InternalTColorDistribution0);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_TColorDistribution1", (void*)&ScriptColorDistribution::InternalTColorDistribution1);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_TColorDistribution2", (void*)&ScriptColorDistribution::InternalTColorDistribution2);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_TColorDistribution3", (void*)&ScriptColorDistribution::InternalTColorDistribution3);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetType", (void*)&ScriptColorDistribution::InternalGetType);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetMinConstant", (void*)&ScriptColorDistribution::InternalGetMinConstant);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetMaxConstant", (void*)&ScriptColorDistribution::InternalGetMaxConstant);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetMinGradient", (void*)&ScriptColorDistribution::InternalGetMinGradient);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetMaxGradient", (void*)&ScriptColorDistribution::InternalGetMaxGradient);

	}

	MonoObject* ScriptColorDistribution::CreateScriptObject(bool construct)
	{
		bool dummy = false;
		void* ctorParams[1] = { &dummy };

		if(construct)
			return sInteropMetaData.ScriptClass->CreateInstance("bool", ctorParams);

		return sInteropMetaData.ScriptClass->CreateInstance(false);
	}
	void ScriptColorDistribution::InternalTColorDistribution(MonoObject* scriptObject)
	{
		SPtr<TColorDistribution<ColorGradient>> nativeObject = B3DMakeShared<TColorDistribution<ColorGradient>>();
		ScriptObjectWrapper::Create<ScriptColorDistribution>(nativeObject, scriptObject);
	}

	void ScriptColorDistribution::InternalTColorDistribution0(MonoObject* scriptObject, Color* color)
	{
		SPtr<TColorDistribution<ColorGradient>> nativeObject = B3DMakeShared<TColorDistribution<ColorGradient>>(*color);
		ScriptObjectWrapper::Create<ScriptColorDistribution>(nativeObject, scriptObject);
	}

	void ScriptColorDistribution::InternalTColorDistribution1(MonoObject* scriptObject, Color* minColor, Color* maxColor)
	{
		SPtr<TColorDistribution<ColorGradient>> nativeObject = B3DMakeShared<TColorDistribution<ColorGradient>>(*minColor, *maxColor);
		ScriptObjectWrapper::Create<ScriptColorDistribution>(nativeObject, scriptObject);
	}

	void ScriptColorDistribution::InternalTColorDistribution2(MonoObject* scriptObject, MonoObject* gradient)
	{
		SPtr<ColorGradient> tmpgradient;
		ScriptColorGradient* scriptObjectWrappergradient;
		scriptObjectWrappergradient = ScriptColorGradient::GetScriptObjectWrapper(gradient);
		if(scriptObjectWrappergradient != nullptr)
			tmpgradient = std::static_pointer_cast<ColorGradient>(scriptObjectWrappergradient->GetBaseNativeObjectAsShared());
		SPtr<TColorDistribution<ColorGradient>> nativeObject = B3DMakeShared<TColorDistribution<ColorGradient>>(*tmpgradient);
		ScriptObjectWrapper::Create<ScriptColorDistribution>(nativeObject, scriptObject);
	}

	void ScriptColorDistribution::InternalTColorDistribution3(MonoObject* scriptObject, MonoObject* minGradient, MonoObject* maxGradient)
	{
		SPtr<ColorGradient> tmpminGradient;
		ScriptColorGradient* scriptObjectWrapperminGradient;
		scriptObjectWrapperminGradient = ScriptColorGradient::GetScriptObjectWrapper(minGradient);
		if(scriptObjectWrapperminGradient != nullptr)
			tmpminGradient = std::static_pointer_cast<ColorGradient>(scriptObjectWrapperminGradient->GetBaseNativeObjectAsShared());
		SPtr<ColorGradient> tmpmaxGradient;
		ScriptColorGradient* scriptObjectWrappermaxGradient;
		scriptObjectWrappermaxGradient = ScriptColorGradient::GetScriptObjectWrapper(maxGradient);
		if(scriptObjectWrappermaxGradient != nullptr)
			tmpmaxGradient = std::static_pointer_cast<ColorGradient>(scriptObjectWrappermaxGradient->GetBaseNativeObjectAsShared());
		SPtr<TColorDistribution<ColorGradient>> nativeObject = B3DMakeShared<TColorDistribution<ColorGradient>>(*tmpminGradient, *tmpmaxGradient);
		ScriptObjectWrapper::Create<ScriptColorDistribution>(nativeObject, scriptObject);
	}

	PropertyDistributionType ScriptColorDistribution::InternalGetType(ScriptColorDistribution* self)
	{
		PropertyDistributionType tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<TColorDistribution<ColorGradient>*>(self->GetNativeObject())->GetType();

		PropertyDistributionType __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptColorDistribution::InternalGetMinConstant(ScriptColorDistribution* self, Color* __output)
	{
		if(!self->IsNativeObjectValid())
			{
				__output = {};
				return;
			}

		Color tmp__output;
		tmp__output = static_cast<TColorDistribution<ColorGradient>*>(self->GetNativeObject())->GetMinConstant();

		*__output = tmp__output;
	}

	void ScriptColorDistribution::InternalGetMaxConstant(ScriptColorDistribution* self, Color* __output)
	{
		if(!self->IsNativeObjectValid())
			{
				__output = {};
				return;
			}

		Color tmp__output;
		tmp__output = static_cast<TColorDistribution<ColorGradient>*>(self->GetNativeObject())->GetMaxConstant();

		*__output = tmp__output;
	}

	MonoObject* ScriptColorDistribution::InternalGetMinGradient(ScriptColorDistribution* self)
	{
		SPtr<ColorGradient> tmp__output = B3DMakeShared<ColorGradient>();
		if(!self->IsNativeObjectValid())
			return {};

		*tmp__output = static_cast<TColorDistribution<ColorGradient>*>(self->GetNativeObject())->GetMinGradient();

		MonoObject* __output;
		__output = ScriptColorGradient::GetOrCreateScriptObject(tmp__output);

		return __output;
	}

	MonoObject* ScriptColorDistribution::InternalGetMaxGradient(ScriptColorDistribution* self)
	{
		SPtr<ColorGradient> tmp__output = B3DMakeShared<ColorGradient>();
		if(!self->IsNativeObjectValid())
			return {};

		*tmp__output = static_cast<TColorDistribution<ColorGradient>*>(self->GetNativeObject())->GetMaxGradient();

		MonoObject* __output;
		__output = ScriptColorGradient::GetOrCreateScriptObject(tmp__output);

		return __output;
	}

	ScriptColorHDRDistribution::ScriptColorHDRDistribution(const SPtr<TColorDistribution<ColorGradientHDR>>& nativeObject)
		:TScriptNonReflectableWrapper(nativeObject)
	{
		RegisterEvents();
	}

	void ScriptColorHDRDistribution::SetupScriptBindings()
	{
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_TColorDistribution", (void*)&ScriptColorHDRDistribution::InternalTColorDistribution);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_TColorDistribution0", (void*)&ScriptColorHDRDistribution::InternalTColorDistribution0);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_TColorDistribution1", (void*)&ScriptColorHDRDistribution::InternalTColorDistribution1);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_TColorDistribution2", (void*)&ScriptColorHDRDistribution::InternalTColorDistribution2);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_TColorDistribution3", (void*)&ScriptColorHDRDistribution::InternalTColorDistribution3);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetType", (void*)&ScriptColorHDRDistribution::InternalGetType);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetMinConstant", (void*)&ScriptColorHDRDistribution::InternalGetMinConstant);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetMaxConstant", (void*)&ScriptColorHDRDistribution::InternalGetMaxConstant);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetMinGradient", (void*)&ScriptColorHDRDistribution::InternalGetMinGradient);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetMaxGradient", (void*)&ScriptColorHDRDistribution::InternalGetMaxGradient);

	}

	MonoObject* ScriptColorHDRDistribution::CreateScriptObject(bool construct)
	{
		bool dummy = false;
		void* ctorParams[1] = { &dummy };

		if(construct)
			return sInteropMetaData.ScriptClass->CreateInstance("bool", ctorParams);

		return sInteropMetaData.ScriptClass->CreateInstance(false);
	}
	void ScriptColorHDRDistribution::InternalTColorDistribution(MonoObject* scriptObject)
	{
		SPtr<TColorDistribution<ColorGradientHDR>> nativeObject = B3DMakeShared<TColorDistribution<ColorGradientHDR>>();
		ScriptObjectWrapper::Create<ScriptColorHDRDistribution>(nativeObject, scriptObject);
	}

	void ScriptColorHDRDistribution::InternalTColorDistribution0(MonoObject* scriptObject, Color* color)
	{
		SPtr<TColorDistribution<ColorGradientHDR>> nativeObject = B3DMakeShared<TColorDistribution<ColorGradientHDR>>(*color);
		ScriptObjectWrapper::Create<ScriptColorHDRDistribution>(nativeObject, scriptObject);
	}

	void ScriptColorHDRDistribution::InternalTColorDistribution1(MonoObject* scriptObject, Color* minColor, Color* maxColor)
	{
		SPtr<TColorDistribution<ColorGradientHDR>> nativeObject = B3DMakeShared<TColorDistribution<ColorGradientHDR>>(*minColor, *maxColor);
		ScriptObjectWrapper::Create<ScriptColorHDRDistribution>(nativeObject, scriptObject);
	}

	void ScriptColorHDRDistribution::InternalTColorDistribution2(MonoObject* scriptObject, MonoObject* gradient)
	{
		SPtr<ColorGradientHDR> tmpgradient;
		ScriptColorGradientHDR* scriptObjectWrappergradient;
		scriptObjectWrappergradient = ScriptColorGradientHDR::GetScriptObjectWrapper(gradient);
		if(scriptObjectWrappergradient != nullptr)
			tmpgradient = std::static_pointer_cast<ColorGradientHDR>(scriptObjectWrappergradient->GetBaseNativeObjectAsShared());
		SPtr<TColorDistribution<ColorGradientHDR>> nativeObject = B3DMakeShared<TColorDistribution<ColorGradientHDR>>(*tmpgradient);
		ScriptObjectWrapper::Create<ScriptColorHDRDistribution>(nativeObject, scriptObject);
	}

	void ScriptColorHDRDistribution::InternalTColorDistribution3(MonoObject* scriptObject, MonoObject* minGradient, MonoObject* maxGradient)
	{
		SPtr<ColorGradientHDR> tmpminGradient;
		ScriptColorGradientHDR* scriptObjectWrapperminGradient;
		scriptObjectWrapperminGradient = ScriptColorGradientHDR::GetScriptObjectWrapper(minGradient);
		if(scriptObjectWrapperminGradient != nullptr)
			tmpminGradient = std::static_pointer_cast<ColorGradientHDR>(scriptObjectWrapperminGradient->GetBaseNativeObjectAsShared());
		SPtr<ColorGradientHDR> tmpmaxGradient;
		ScriptColorGradientHDR* scriptObjectWrappermaxGradient;
		scriptObjectWrappermaxGradient = ScriptColorGradientHDR::GetScriptObjectWrapper(maxGradient);
		if(scriptObjectWrappermaxGradient != nullptr)
			tmpmaxGradient = std::static_pointer_cast<ColorGradientHDR>(scriptObjectWrappermaxGradient->GetBaseNativeObjectAsShared());
		SPtr<TColorDistribution<ColorGradientHDR>> nativeObject = B3DMakeShared<TColorDistribution<ColorGradientHDR>>(*tmpminGradient, *tmpmaxGradient);
		ScriptObjectWrapper::Create<ScriptColorHDRDistribution>(nativeObject, scriptObject);
	}

	PropertyDistributionType ScriptColorHDRDistribution::InternalGetType(ScriptColorHDRDistribution* self)
	{
		PropertyDistributionType tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<TColorDistribution<ColorGradientHDR>*>(self->GetNativeObject())->GetType();

		PropertyDistributionType __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptColorHDRDistribution::InternalGetMinConstant(ScriptColorHDRDistribution* self, Color* __output)
	{
		if(!self->IsNativeObjectValid())
			{
				__output = {};
				return;
			}

		Color tmp__output;
		tmp__output = static_cast<TColorDistribution<ColorGradientHDR>*>(self->GetNativeObject())->GetMinConstant();

		*__output = tmp__output;
	}

	void ScriptColorHDRDistribution::InternalGetMaxConstant(ScriptColorHDRDistribution* self, Color* __output)
	{
		if(!self->IsNativeObjectValid())
			{
				__output = {};
				return;
			}

		Color tmp__output;
		tmp__output = static_cast<TColorDistribution<ColorGradientHDR>*>(self->GetNativeObject())->GetMaxConstant();

		*__output = tmp__output;
	}

	MonoObject* ScriptColorHDRDistribution::InternalGetMinGradient(ScriptColorHDRDistribution* self)
	{
		SPtr<ColorGradientHDR> tmp__output = B3DMakeShared<ColorGradientHDR>();
		if(!self->IsNativeObjectValid())
			return {};

		*tmp__output = static_cast<TColorDistribution<ColorGradientHDR>*>(self->GetNativeObject())->GetMinGradient();

		MonoObject* __output;
		__output = ScriptColorGradientHDR::GetOrCreateScriptObject(tmp__output);

		return __output;
	}

	MonoObject* ScriptColorHDRDistribution::InternalGetMaxGradient(ScriptColorHDRDistribution* self)
	{
		SPtr<ColorGradientHDR> tmp__output = B3DMakeShared<ColorGradientHDR>();
		if(!self->IsNativeObjectValid())
			return {};

		*tmp__output = static_cast<TColorDistribution<ColorGradientHDR>*>(self->GetNativeObject())->GetMaxGradient();

		MonoObject* __output;
		__output = ScriptColorGradientHDR::GetOrCreateScriptObject(tmp__output);

		return __output;
	}
}
