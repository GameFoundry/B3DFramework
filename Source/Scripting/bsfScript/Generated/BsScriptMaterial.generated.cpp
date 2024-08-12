//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptMaterial.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "../../../Foundation/bsfCore/Material/BsMaterial.h"
#include "BsScriptResourceManager.h"
#include "Wrappers/BsScriptRRefBase.h"
#include "Wrappers/BsScriptColor.h"
#include "../../../Foundation/bsfCore/Material/BsMaterial.h"
#include "../../../Foundation/bsfCore/Material/BsShader.h"
#include "BsScriptShaderVariationParameters.generated.h"
#include "BsScriptColorGradientHDR.generated.h"
#include "BsScriptTAnimationCurve.generated.h"
#include "Wrappers/BsScriptVector.h"
#include "Wrappers/BsScriptVector.h"
#include "Wrappers/BsScriptVector.h"
#include "../../../Foundation/bsfCore/Image/BsTexture.h"
#include "../Extensions/BsMaterialEx.h"
#include "../../../Foundation/bsfCore/Image/BsSpriteImage.h"

namespace bs
{
	ScriptMaterial::ScriptMaterial(MonoObject* managedInstance, const TResourceHandle<Material>& value)
		:TScriptResource(managedInstance, value)
	{
	}

	void ScriptMaterial::InitRuntimeData()
	{
		metaData.ScriptClass->AddInternalCall("Internal_GetRef", (void*)&ScriptMaterial::InternalGetRef);
		metaData.ScriptClass->AddInternalCall("Internal_SetShader", (void*)&ScriptMaterial::InternalSetShader);
		metaData.ScriptClass->AddInternalCall("Internal_SetVariation", (void*)&ScriptMaterial::InternalSetVariation);
		metaData.ScriptClass->AddInternalCall("Internal_Clone", (void*)&ScriptMaterial::InternalClone);
		metaData.ScriptClass->AddInternalCall("Internal_GetShader", (void*)&ScriptMaterial::InternalGetShader);
		metaData.ScriptClass->AddInternalCall("Internal_GetVariation", (void*)&ScriptMaterial::InternalGetVariation);
		metaData.ScriptClass->AddInternalCall("Internal_SetFloat", (void*)&ScriptMaterial::InternalSetFloat);
		metaData.ScriptClass->AddInternalCall("Internal_SetFloatCurve", (void*)&ScriptMaterial::InternalSetFloatCurve);
		metaData.ScriptClass->AddInternalCall("Internal_SetColor", (void*)&ScriptMaterial::InternalSetColor);
		metaData.ScriptClass->AddInternalCall("Internal_SetColorGradient", (void*)&ScriptMaterial::InternalSetColorGradient);
		metaData.ScriptClass->AddInternalCall("Internal_SetVec2", (void*)&ScriptMaterial::InternalSetVec2);
		metaData.ScriptClass->AddInternalCall("Internal_SetVec3", (void*)&ScriptMaterial::InternalSetVec3);
		metaData.ScriptClass->AddInternalCall("Internal_SetVec4", (void*)&ScriptMaterial::InternalSetVec4);
		metaData.ScriptClass->AddInternalCall("Internal_SetMat3", (void*)&ScriptMaterial::InternalSetMat3);
		metaData.ScriptClass->AddInternalCall("Internal_SetMat4", (void*)&ScriptMaterial::InternalSetMat4);
		metaData.ScriptClass->AddInternalCall("Internal_GetFloat", (void*)&ScriptMaterial::InternalGetFloat);
		metaData.ScriptClass->AddInternalCall("Internal_GetFloatCurve", (void*)&ScriptMaterial::InternalGetFloatCurve);
		metaData.ScriptClass->AddInternalCall("Internal_GetColor", (void*)&ScriptMaterial::InternalGetColor);
		metaData.ScriptClass->AddInternalCall("Internal_GetColorGradient", (void*)&ScriptMaterial::InternalGetColorGradient);
		metaData.ScriptClass->AddInternalCall("Internal_GetVec2", (void*)&ScriptMaterial::InternalGetVec2);
		metaData.ScriptClass->AddInternalCall("Internal_GetVec3", (void*)&ScriptMaterial::InternalGetVec3);
		metaData.ScriptClass->AddInternalCall("Internal_GetVec4", (void*)&ScriptMaterial::InternalGetVec4);
		metaData.ScriptClass->AddInternalCall("Internal_GetMat3", (void*)&ScriptMaterial::InternalGetMat3);
		metaData.ScriptClass->AddInternalCall("Internal_GetMat4", (void*)&ScriptMaterial::InternalGetMat4);
		metaData.ScriptClass->AddInternalCall("Internal_IsAnimated", (void*)&ScriptMaterial::InternalIsAnimated);
		metaData.ScriptClass->AddInternalCall("Internal_Create", (void*)&ScriptMaterial::InternalCreate);
		metaData.ScriptClass->AddInternalCall("Internal_Create0", (void*)&ScriptMaterial::InternalCreate0);
		metaData.ScriptClass->AddInternalCall("Internal_SetTexture", (void*)&ScriptMaterial::InternalSetTexture);
		metaData.ScriptClass->AddInternalCall("Internal_GetTexture", (void*)&ScriptMaterial::InternalGetTexture);
		metaData.ScriptClass->AddInternalCall("Internal_SetSpriteImage", (void*)&ScriptMaterial::InternalSetSpriteImage);
		metaData.ScriptClass->AddInternalCall("Internal_GetSpriteImage", (void*)&ScriptMaterial::InternalGetSpriteImage);

	}

	 MonoObject*ScriptMaterial::CreateInstance()
	{
		bool dummy = false;
		void* ctorParams[1] = { &dummy };

		return metaData.ScriptClass->CreateInstance("bool", ctorParams);
	}
	MonoObject* ScriptMaterial::InternalGetRef(ScriptMaterial* thisPtr)
	{
		return thisPtr->GetRRef();
	}

	void ScriptMaterial::InternalSetShader(ScriptMaterial* thisPtr, MonoObject* shader)
	{
		TResourceHandle<Shader> tmpshader;
		ScriptRRefBase* scriptObjectWrappershader;
		scriptObjectWrappershader = ScriptRRefBase::ToNative(shader);
		if(scriptObjectWrappershader != nullptr)
			tmpshader = B3DStaticResourceCast<Shader>(scriptObjectWrappershader->GetHandle());
		thisPtr->GetHandle()->SetShader(tmpshader);
	}

	void ScriptMaterial::InternalSetVariation(ScriptMaterial* thisPtr, MonoObject* variation)
	{
		SPtr<ShaderVariationParameters> tmpvariation;
		ScriptShaderVariationParameters* scriptObjectWrappervariation;
		scriptObjectWrappervariation = ScriptShaderVariationParameters::ToNative(variation);
		if(scriptObjectWrappervariation != nullptr)
			tmpvariation = scriptObjectWrappervariation->GetInternal();
		thisPtr->GetHandle()->SetVariation(*tmpvariation);
	}

	MonoObject* ScriptMaterial::InternalClone(ScriptMaterial* thisPtr)
	{
		TResourceHandle<Material> tmp__output;
		tmp__output = thisPtr->GetHandle()->Clone();

		MonoObject* __output;
		ScriptRRefBase* script__output;
		script__output = ScriptResourceManager::Instance().GetScriptRRef(tmp__output);
		if(script__output != nullptr)
			__output = script__output->GetManagedInstance();
		else
			__output = nullptr;

		return __output;
	}

	MonoObject* ScriptMaterial::InternalGetShader(ScriptMaterial* thisPtr)
	{
		TResourceHandle<Shader> tmp__output;
		tmp__output = thisPtr->GetHandle()->GetShader();

		MonoObject* __output;
		ScriptRRefBase* script__output;
		script__output = ScriptResourceManager::Instance().GetScriptRRef(tmp__output);
		if(script__output != nullptr)
			__output = script__output->GetManagedInstance();
		else
			__output = nullptr;

		return __output;
	}

	MonoObject* ScriptMaterial::InternalGetVariation(ScriptMaterial* thisPtr)
	{
		SPtr<ShaderVariationParameters> tmp__output = B3DMakeShared<ShaderVariationParameters>();
		*tmp__output = thisPtr->GetHandle()->GetVariation();

		MonoObject* __output;
		__output = ScriptShaderVariationParameters::Create(tmp__output);

		return __output;
	}

	void ScriptMaterial::InternalSetFloat(ScriptMaterial* thisPtr, MonoString* name, float value, uint32_t arrayIdx)
	{
		String tmpname;
		tmpname = MonoUtil::MonoToString(name);
		thisPtr->GetHandle()->SetFloat(tmpname, value, arrayIdx);
	}

	void ScriptMaterial::InternalSetFloatCurve(ScriptMaterial* thisPtr, MonoString* name, MonoObject* value, uint32_t arrayIdx)
	{
		String tmpname;
		tmpname = MonoUtil::MonoToString(name);
		SPtr<TAnimationCurve<float>> tmpvalue;
		ScriptAnimationCurve* scriptObjectWrappervalue;
		scriptObjectWrappervalue = ScriptAnimationCurve::ToNative(value);
		if(scriptObjectWrappervalue != nullptr)
			tmpvalue = scriptObjectWrappervalue->GetInternal();
		thisPtr->GetHandle()->SetFloatCurve(tmpname, *tmpvalue, arrayIdx);
	}

	void ScriptMaterial::InternalSetColor(ScriptMaterial* thisPtr, MonoString* name, Color* value, uint32_t arrayIdx)
	{
		String tmpname;
		tmpname = MonoUtil::MonoToString(name);
		thisPtr->GetHandle()->SetColor(tmpname, *value, arrayIdx);
	}

	void ScriptMaterial::InternalSetColorGradient(ScriptMaterial* thisPtr, MonoString* name, MonoObject* value, uint32_t arrayIdx)
	{
		String tmpname;
		tmpname = MonoUtil::MonoToString(name);
		SPtr<ColorGradientHDR> tmpvalue;
		ScriptColorGradientHDR* scriptObjectWrappervalue;
		scriptObjectWrappervalue = ScriptColorGradientHDR::ToNative(value);
		if(scriptObjectWrappervalue != nullptr)
			tmpvalue = scriptObjectWrappervalue->GetInternal();
		thisPtr->GetHandle()->SetColorGradient(tmpname, *tmpvalue, arrayIdx);
	}

	void ScriptMaterial::InternalSetVec2(ScriptMaterial* thisPtr, MonoString* name, TVector2<float>* value, uint32_t arrayIdx)
	{
		String tmpname;
		tmpname = MonoUtil::MonoToString(name);
		thisPtr->GetHandle()->SetVec2(tmpname, *value, arrayIdx);
	}

	void ScriptMaterial::InternalSetVec3(ScriptMaterial* thisPtr, MonoString* name, TVector3<float>* value, uint32_t arrayIdx)
	{
		String tmpname;
		tmpname = MonoUtil::MonoToString(name);
		thisPtr->GetHandle()->SetVec3(tmpname, *value, arrayIdx);
	}

	void ScriptMaterial::InternalSetVec4(ScriptMaterial* thisPtr, MonoString* name, TVector4<float>* value, uint32_t arrayIdx)
	{
		String tmpname;
		tmpname = MonoUtil::MonoToString(name);
		thisPtr->GetHandle()->SetVec4(tmpname, *value, arrayIdx);
	}

	void ScriptMaterial::InternalSetMat3(ScriptMaterial* thisPtr, MonoString* name, Matrix3* value, uint32_t arrayIdx)
	{
		String tmpname;
		tmpname = MonoUtil::MonoToString(name);
		thisPtr->GetHandle()->SetMat3(tmpname, *value, arrayIdx);
	}

	void ScriptMaterial::InternalSetMat4(ScriptMaterial* thisPtr, MonoString* name, Matrix4* value, uint32_t arrayIdx)
	{
		String tmpname;
		tmpname = MonoUtil::MonoToString(name);
		thisPtr->GetHandle()->SetMat4(tmpname, *value, arrayIdx);
	}

	float ScriptMaterial::InternalGetFloat(ScriptMaterial* thisPtr, MonoString* name, uint32_t arrayIdx)
	{
		float tmp__output;
		String tmpname;
		tmpname = MonoUtil::MonoToString(name);
		tmp__output = thisPtr->GetHandle()->GetFloat(tmpname, arrayIdx);

		float __output;
		__output = tmp__output;

		return __output;
	}

	MonoObject* ScriptMaterial::InternalGetFloatCurve(ScriptMaterial* thisPtr, MonoString* name, uint32_t arrayIdx)
	{
		SPtr<TAnimationCurve<float>> tmp__output = B3DMakeShared<TAnimationCurve<float>>();
		String tmpname;
		tmpname = MonoUtil::MonoToString(name);
		*tmp__output = thisPtr->GetHandle()->GetFloatCurve(tmpname, arrayIdx);

		MonoObject* __output;
		__output = ScriptAnimationCurve::Create(tmp__output);

		return __output;
	}

	void ScriptMaterial::InternalGetColor(ScriptMaterial* thisPtr, MonoString* name, uint32_t arrayIdx, Color* __output)
	{
		String tmpname;
		tmpname = MonoUtil::MonoToString(name);
		Color tmp__output;
		tmp__output = thisPtr->GetHandle()->GetColor(tmpname, arrayIdx);

		*__output = tmp__output;
	}

	MonoObject* ScriptMaterial::InternalGetColorGradient(ScriptMaterial* thisPtr, MonoString* name, uint32_t arrayIdx)
	{
		SPtr<ColorGradientHDR> tmp__output = B3DMakeShared<ColorGradientHDR>();
		String tmpname;
		tmpname = MonoUtil::MonoToString(name);
		*tmp__output = thisPtr->GetHandle()->GetColorGradient(tmpname, arrayIdx);

		MonoObject* __output;
		__output = ScriptColorGradientHDR::Create(tmp__output);

		return __output;
	}

	void ScriptMaterial::InternalGetVec2(ScriptMaterial* thisPtr, MonoString* name, uint32_t arrayIdx, TVector2<float>* __output)
	{
		String tmpname;
		tmpname = MonoUtil::MonoToString(name);
		TVector2<float> tmp__output;
		tmp__output = thisPtr->GetHandle()->GetVec2(tmpname, arrayIdx);

		*__output = tmp__output;
	}

	void ScriptMaterial::InternalGetVec3(ScriptMaterial* thisPtr, MonoString* name, uint32_t arrayIdx, TVector3<float>* __output)
	{
		String tmpname;
		tmpname = MonoUtil::MonoToString(name);
		TVector3<float> tmp__output;
		tmp__output = thisPtr->GetHandle()->GetVec3(tmpname, arrayIdx);

		*__output = tmp__output;
	}

	void ScriptMaterial::InternalGetVec4(ScriptMaterial* thisPtr, MonoString* name, uint32_t arrayIdx, TVector4<float>* __output)
	{
		String tmpname;
		tmpname = MonoUtil::MonoToString(name);
		TVector4<float> tmp__output;
		tmp__output = thisPtr->GetHandle()->GetVec4(tmpname, arrayIdx);

		*__output = tmp__output;
	}

	void ScriptMaterial::InternalGetMat3(ScriptMaterial* thisPtr, MonoString* name, uint32_t arrayIdx, Matrix3* __output)
	{
		String tmpname;
		tmpname = MonoUtil::MonoToString(name);
		Matrix3 tmp__output;
		tmp__output = thisPtr->GetHandle()->GetMat3(tmpname, arrayIdx);

		*__output = tmp__output;
	}

	void ScriptMaterial::InternalGetMat4(ScriptMaterial* thisPtr, MonoString* name, uint32_t arrayIdx, Matrix4* __output)
	{
		String tmpname;
		tmpname = MonoUtil::MonoToString(name);
		Matrix4 tmp__output;
		tmp__output = thisPtr->GetHandle()->GetMat4(tmpname, arrayIdx);

		*__output = tmp__output;
	}

	bool ScriptMaterial::InternalIsAnimated(ScriptMaterial* thisPtr, MonoString* name, uint32_t arrayIdx)
	{
		bool tmp__output;
		String tmpname;
		tmpname = MonoUtil::MonoToString(name);
		tmp__output = thisPtr->GetHandle()->IsAnimated(tmpname, arrayIdx);

		bool __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptMaterial::InternalCreate(MonoObject* managedInstance)
	{
		TResourceHandle<Material> instance = Material::Create();
		ScriptResourceManager::Instance().CreateBuiltinScriptResource(instance, managedInstance);
	}

	void ScriptMaterial::InternalCreate0(MonoObject* managedInstance, MonoObject* shader)
	{
		TResourceHandle<Shader> tmpshader;
		ScriptRRefBase* scriptObjectWrappershader;
		scriptObjectWrappershader = ScriptRRefBase::ToNative(shader);
		if(scriptObjectWrappershader != nullptr)
			tmpshader = B3DStaticResourceCast<Shader>(scriptObjectWrappershader->GetHandle());
		TResourceHandle<Material> instance = Material::Create(tmpshader);
		ScriptResourceManager::Instance().CreateBuiltinScriptResource(instance, managedInstance);
	}

	void ScriptMaterial::InternalSetTexture(ScriptMaterial* thisPtr, MonoString* name, MonoObject* value, uint32_t mipLevel, uint32_t numMipLevels, uint32_t arraySlice, uint32_t numArraySlices)
	{
		String tmpname;
		tmpname = MonoUtil::MonoToString(name);
		TResourceHandle<Texture> tmpvalue;
		ScriptRRefBase* scriptObjectWrappervalue;
		scriptObjectWrappervalue = ScriptRRefBase::ToNative(value);
		if(scriptObjectWrappervalue != nullptr)
			tmpvalue = B3DStaticResourceCast<Texture>(scriptObjectWrappervalue->GetHandle());
		MaterialEx::SetTexture(thisPtr->GetHandle(), tmpname, tmpvalue, mipLevel, numMipLevels, arraySlice, numArraySlices);
	}

	MonoObject* ScriptMaterial::InternalGetTexture(ScriptMaterial* thisPtr, MonoString* name)
	{
		TResourceHandle<Texture> tmp__output;
		String tmpname;
		tmpname = MonoUtil::MonoToString(name);
		tmp__output = MaterialEx::GetTexture(thisPtr->GetHandle(), tmpname);

		MonoObject* __output;
		ScriptRRefBase* script__output;
		script__output = ScriptResourceManager::Instance().GetScriptRRef(tmp__output);
		if(script__output != nullptr)
			__output = script__output->GetManagedInstance();
		else
			__output = nullptr;

		return __output;
	}

	void ScriptMaterial::InternalSetSpriteImage(ScriptMaterial* thisPtr, MonoString* name, MonoObject* value)
	{
		String tmpname;
		tmpname = MonoUtil::MonoToString(name);
		TResourceHandle<SpriteImage> tmpvalue;
		ScriptRRefBase* scriptObjectWrappervalue;
		scriptObjectWrappervalue = ScriptRRefBase::ToNative(value);
		if(scriptObjectWrappervalue != nullptr)
			tmpvalue = B3DStaticResourceCast<SpriteImage>(scriptObjectWrappervalue->GetHandle());
		MaterialEx::SetSpriteImage(thisPtr->GetHandle(), tmpname, tmpvalue);
	}

	MonoObject* ScriptMaterial::InternalGetSpriteImage(ScriptMaterial* thisPtr, MonoString* name)
	{
		TResourceHandle<SpriteImage> tmp__output;
		String tmpname;
		tmpname = MonoUtil::MonoToString(name);
		tmp__output = MaterialEx::GetSpriteImage(thisPtr->GetHandle(), tmpname);

		MonoObject* __output;
		ScriptRRefBase* script__output;
		script__output = ScriptResourceManager::Instance().GetScriptRRef(tmp__output);
		if(script__output != nullptr)
			__output = script__output->GetManagedInstance();
		else
			__output = nullptr;

		return __output;
	}
}
