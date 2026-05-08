//************************************ B3D Framework - Copyright 2025 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DScriptMaterial.generated.h"
#include "B3DMonoMethod.h"
#include "B3DMonoClass.h"
#include "B3DMonoUtil.h"
#include "../../../Engine/Core/Material/B3DMaterial.h"
#include "B3DScriptResourceManager.h"
#include "Wrappers/B3DScriptRRefBase.h"
#include "B3DScriptColor.generated.h"
#include "../../../Engine/Core/Material/B3DMaterial.h"
#include "../../../Engine/Core/Material/B3DShader.h"
#include "B3DScriptShaderVariationParameters.generated.h"
#include "B3DScriptColorGradientHDR.generated.h"
#include "B3DScriptTAnimationCurve.generated.h"
#include "B3DScriptTVector2.generated.h"
#include "B3DScriptTVector3.generated.h"
#include "B3DScriptTVector4.generated.h"
#include "../../../Engine/Core/Image/B3DTexture.h"
#include "../Extensions/B3DMaterialEx.h"
#include "../../../Engine/Core/Image/B3DSpriteImage.h"

namespace b3d
{
	ScriptMaterial::ScriptMaterial(const TResourceHandle<Material>& nativeObject)
		:TScriptResourceWrapper(nativeObject)
	{
		RegisterEvents();
	}

	ScriptMaterial::~ScriptMaterial()
	{
		UnregisterEvents();
	}

	void ScriptMaterial::SetupScriptBindings()
	{
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetRef", (void*)&ScriptMaterial::InternalGetRef);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetShader", (void*)&ScriptMaterial::InternalSetShader);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetVariation", (void*)&ScriptMaterial::InternalSetVariation);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_Clone", (void*)&ScriptMaterial::InternalClone);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetShader", (void*)&ScriptMaterial::InternalGetShader);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetVariationParameters", (void*)&ScriptMaterial::InternalGetVariationParameters);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetFloat", (void*)&ScriptMaterial::InternalSetFloat);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetFloatCurve", (void*)&ScriptMaterial::InternalSetFloatCurve);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetColor", (void*)&ScriptMaterial::InternalSetColor);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetColorGradient", (void*)&ScriptMaterial::InternalSetColorGradient);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetVec2", (void*)&ScriptMaterial::InternalSetVec2);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetVec3", (void*)&ScriptMaterial::InternalSetVec3);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetVec4", (void*)&ScriptMaterial::InternalSetVec4);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetMat3", (void*)&ScriptMaterial::InternalSetMat3);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetMat4", (void*)&ScriptMaterial::InternalSetMat4);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetFloat", (void*)&ScriptMaterial::InternalGetFloat);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetFloatCurve", (void*)&ScriptMaterial::InternalGetFloatCurve);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetColor", (void*)&ScriptMaterial::InternalGetColor);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetColorGradient", (void*)&ScriptMaterial::InternalGetColorGradient);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetVec2", (void*)&ScriptMaterial::InternalGetVec2);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetVec3", (void*)&ScriptMaterial::InternalGetVec3);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetVec4", (void*)&ScriptMaterial::InternalGetVec4);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetMat3", (void*)&ScriptMaterial::InternalGetMat3);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetMat4", (void*)&ScriptMaterial::InternalGetMat4);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_IsAnimated", (void*)&ScriptMaterial::InternalIsAnimated);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_Create", (void*)&ScriptMaterial::InternalCreate);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_Create0", (void*)&ScriptMaterial::InternalCreate0);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetTexture", (void*)&ScriptMaterial::InternalSetTexture);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetTexture", (void*)&ScriptMaterial::InternalGetTexture);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetSpriteImage", (void*)&ScriptMaterial::InternalSetSpriteImage);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetSpriteImage", (void*)&ScriptMaterial::InternalGetSpriteImage);

	}

	MonoObject* ScriptMaterial::CreateScriptObject(bool construct)
	{
		bool dummy = false;
		void* ctorParams[1] = { &dummy };

		if(construct)
			return sInteropMetaData.ScriptClass->CreateInstance("bool", ctorParams);

		return sInteropMetaData.ScriptClass->CreateInstance(false);
	}
	MonoObject* ScriptMaterial::InternalGetRef(ScriptMaterial* self)
	{
		return self->GetOrCreateResourceReference();
	}

	void ScriptMaterial::InternalSetShader(ScriptMaterial* self, MonoObject* shader)
	{
		if(!self->IsNativeObjectValid())
			return;

		TResourceHandle<Shader> tmpshader;
		ScriptRRefBase* scriptObjectWrappershader;
		scriptObjectWrappershader = ScriptRRefBase::GetScriptObjectWrapper(shader);
		if(scriptObjectWrappershader != nullptr)
			tmpshader = B3DStaticResourceCast<Shader>(scriptObjectWrappershader->GetNativeObject());
		static_cast<Material*>(self->GetNativeObject())->SetShader(tmpshader);
	}

	void ScriptMaterial::InternalSetVariation(ScriptMaterial* self, MonoObject* variation)
	{
		if(!self->IsNativeObjectValid())
			return;

		TShared<ShaderVariationParameters> tmpvariation;
		ScriptShaderVariationParameters* scriptObjectWrappervariation;
		scriptObjectWrappervariation = ScriptShaderVariationParameters::GetScriptObjectWrapper(variation);
		if(scriptObjectWrappervariation != nullptr)
			tmpvariation = std::static_pointer_cast<ShaderVariationParameters>(scriptObjectWrappervariation->GetBaseNativeObjectAsShared());
		static_cast<Material*>(self->GetNativeObject())->SetVariation(*tmpvariation);
	}

	MonoObject* ScriptMaterial::InternalClone(ScriptMaterial* self)
	{
		TResourceHandle<Material> tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<Material*>(self->GetNativeObject())->Clone();

		MonoObject* __output;
		ScriptRRefBase* script__output;
		script__output = ScriptResourceManager::Instance().GetScriptRRef(tmp__output);
		if(script__output != nullptr)
			__output = script__output->GetScriptObject();
		else
			__output = nullptr;

		return __output;
	}

	MonoObject* ScriptMaterial::InternalGetShader(ScriptMaterial* self)
	{
		TResourceHandle<Shader> tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<Material*>(self->GetNativeObject())->GetShader();

		MonoObject* __output;
		ScriptRRefBase* script__output;
		script__output = ScriptResourceManager::Instance().GetScriptRRef(tmp__output);
		if(script__output != nullptr)
			__output = script__output->GetScriptObject();
		else
			__output = nullptr;

		return __output;
	}

	MonoObject* ScriptMaterial::InternalGetVariationParameters(ScriptMaterial* self)
	{
		TShared<ShaderVariationParameters> tmp__output = B3DMakeShared<ShaderVariationParameters>();
		if(!self->IsNativeObjectValid())
			return {};

		*tmp__output = static_cast<Material*>(self->GetNativeObject())->GetVariationParameters();

		MonoObject* __output;
		__output = ScriptShaderVariationParameters::GetOrCreateScriptObject(tmp__output);

		return __output;
	}

	void ScriptMaterial::InternalSetFloat(ScriptMaterial* self, MonoString* name, float value, uint32_t arrayIdx)
	{
		if(!self->IsNativeObjectValid())
			return;

		String tmpname;
		tmpname = MonoUtil::MonoToString(name);
		static_cast<Material*>(self->GetNativeObject())->SetFloat(tmpname, value, arrayIdx);
	}

	void ScriptMaterial::InternalSetFloatCurve(ScriptMaterial* self, MonoString* name, MonoObject* value, uint32_t arrayIdx)
	{
		if(!self->IsNativeObjectValid())
			return;

		String tmpname;
		tmpname = MonoUtil::MonoToString(name);
		TShared<TAnimationCurve<float>> tmpvalue;
		ScriptAnimationCurve* scriptObjectWrappervalue;
		scriptObjectWrappervalue = ScriptAnimationCurve::GetScriptObjectWrapper(value);
		if(scriptObjectWrappervalue != nullptr)
			tmpvalue = std::static_pointer_cast<TAnimationCurve<float>>(scriptObjectWrappervalue->GetBaseNativeObjectAsShared());
		static_cast<Material*>(self->GetNativeObject())->SetFloatCurve(tmpname, *tmpvalue, arrayIdx);
	}

	void ScriptMaterial::InternalSetColor(ScriptMaterial* self, MonoString* name, Color* value, uint32_t arrayIdx)
	{
		if(!self->IsNativeObjectValid())
			return;

		String tmpname;
		tmpname = MonoUtil::MonoToString(name);
		static_cast<Material*>(self->GetNativeObject())->SetColor(tmpname, *value, arrayIdx);
	}

	void ScriptMaterial::InternalSetColorGradient(ScriptMaterial* self, MonoString* name, MonoObject* value, uint32_t arrayIdx)
	{
		if(!self->IsNativeObjectValid())
			return;

		String tmpname;
		tmpname = MonoUtil::MonoToString(name);
		TShared<ColorGradientHDR> tmpvalue;
		ScriptColorGradientHDR* scriptObjectWrappervalue;
		scriptObjectWrappervalue = ScriptColorGradientHDR::GetScriptObjectWrapper(value);
		if(scriptObjectWrappervalue != nullptr)
			tmpvalue = std::static_pointer_cast<ColorGradientHDR>(scriptObjectWrappervalue->GetBaseNativeObjectAsShared());
		static_cast<Material*>(self->GetNativeObject())->SetColorGradient(tmpname, *tmpvalue, arrayIdx);
	}

	void ScriptMaterial::InternalSetVec2(ScriptMaterial* self, MonoString* name, TVector2<float>* value, uint32_t arrayIdx)
	{
		if(!self->IsNativeObjectValid())
			return;

		String tmpname;
		tmpname = MonoUtil::MonoToString(name);
		static_cast<Material*>(self->GetNativeObject())->SetVec2(tmpname, *value, arrayIdx);
	}

	void ScriptMaterial::InternalSetVec3(ScriptMaterial* self, MonoString* name, TVector3<float>* value, uint32_t arrayIdx)
	{
		if(!self->IsNativeObjectValid())
			return;

		String tmpname;
		tmpname = MonoUtil::MonoToString(name);
		static_cast<Material*>(self->GetNativeObject())->SetVec3(tmpname, *value, arrayIdx);
	}

	void ScriptMaterial::InternalSetVec4(ScriptMaterial* self, MonoString* name, TVector4<float>* value, uint32_t arrayIdx)
	{
		if(!self->IsNativeObjectValid())
			return;

		String tmpname;
		tmpname = MonoUtil::MonoToString(name);
		static_cast<Material*>(self->GetNativeObject())->SetVec4(tmpname, *value, arrayIdx);
	}

	void ScriptMaterial::InternalSetMat3(ScriptMaterial* self, MonoString* name, TMatrix3<float>* value, uint32_t arrayIdx)
	{
		if(!self->IsNativeObjectValid())
			return;

		String tmpname;
		tmpname = MonoUtil::MonoToString(name);
		static_cast<Material*>(self->GetNativeObject())->SetMat3(tmpname, *value, arrayIdx);
	}

	void ScriptMaterial::InternalSetMat4(ScriptMaterial* self, MonoString* name, TMatrix4<float>* value, uint32_t arrayIdx)
	{
		if(!self->IsNativeObjectValid())
			return;

		String tmpname;
		tmpname = MonoUtil::MonoToString(name);
		static_cast<Material*>(self->GetNativeObject())->SetMat4(tmpname, *value, arrayIdx);
	}

	float ScriptMaterial::InternalGetFloat(ScriptMaterial* self, MonoString* name, uint32_t arrayIdx)
	{
		float tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		String tmpname;
		tmpname = MonoUtil::MonoToString(name);
		tmp__output = static_cast<Material*>(self->GetNativeObject())->GetFloat(tmpname, arrayIdx);

		float __output;
		__output = tmp__output;

		return __output;
	}

	MonoObject* ScriptMaterial::InternalGetFloatCurve(ScriptMaterial* self, MonoString* name, uint32_t arrayIdx)
	{
		TShared<TAnimationCurve<float>> tmp__output = B3DMakeShared<TAnimationCurve<float>>();
		if(!self->IsNativeObjectValid())
			return {};

		String tmpname;
		tmpname = MonoUtil::MonoToString(name);
		*tmp__output = static_cast<Material*>(self->GetNativeObject())->GetFloatCurve(tmpname, arrayIdx);

		MonoObject* __output;
		__output = ScriptAnimationCurve::GetOrCreateScriptObject(tmp__output);

		return __output;
	}

	void ScriptMaterial::InternalGetColor(ScriptMaterial* self, MonoString* name, uint32_t arrayIdx, Color* __output)
	{
		if(!self->IsNativeObjectValid())
		{
			*__output = {};
			return;
		}

		String tmpname;
		tmpname = MonoUtil::MonoToString(name);
		Color tmp__output;
		tmp__output = static_cast<Material*>(self->GetNativeObject())->GetColor(tmpname, arrayIdx);

		*__output = tmp__output;
	}

	MonoObject* ScriptMaterial::InternalGetColorGradient(ScriptMaterial* self, MonoString* name, uint32_t arrayIdx)
	{
		TShared<ColorGradientHDR> tmp__output = B3DMakeShared<ColorGradientHDR>();
		if(!self->IsNativeObjectValid())
			return {};

		String tmpname;
		tmpname = MonoUtil::MonoToString(name);
		*tmp__output = static_cast<Material*>(self->GetNativeObject())->GetColorGradient(tmpname, arrayIdx);

		MonoObject* __output;
		__output = ScriptColorGradientHDR::GetOrCreateScriptObject(tmp__output);

		return __output;
	}

	void ScriptMaterial::InternalGetVec2(ScriptMaterial* self, MonoString* name, uint32_t arrayIdx, TVector2<float>* __output)
	{
		if(!self->IsNativeObjectValid())
		{
			*__output = {};
			return;
		}

		String tmpname;
		tmpname = MonoUtil::MonoToString(name);
		TVector2<float> tmp__output;
		tmp__output = static_cast<Material*>(self->GetNativeObject())->GetVec2(tmpname, arrayIdx);

		*__output = tmp__output;
	}

	void ScriptMaterial::InternalGetVec3(ScriptMaterial* self, MonoString* name, uint32_t arrayIdx, TVector3<float>* __output)
	{
		if(!self->IsNativeObjectValid())
		{
			*__output = {};
			return;
		}

		String tmpname;
		tmpname = MonoUtil::MonoToString(name);
		TVector3<float> tmp__output;
		tmp__output = static_cast<Material*>(self->GetNativeObject())->GetVec3(tmpname, arrayIdx);

		*__output = tmp__output;
	}

	void ScriptMaterial::InternalGetVec4(ScriptMaterial* self, MonoString* name, uint32_t arrayIdx, TVector4<float>* __output)
	{
		if(!self->IsNativeObjectValid())
		{
			*__output = {};
			return;
		}

		String tmpname;
		tmpname = MonoUtil::MonoToString(name);
		TVector4<float> tmp__output;
		tmp__output = static_cast<Material*>(self->GetNativeObject())->GetVec4(tmpname, arrayIdx);

		*__output = tmp__output;
	}

	void ScriptMaterial::InternalGetMat3(ScriptMaterial* self, MonoString* name, uint32_t arrayIdx, TMatrix3<float>* __output)
	{
		if(!self->IsNativeObjectValid())
		{
			*__output = {};
			return;
		}

		String tmpname;
		tmpname = MonoUtil::MonoToString(name);
		TMatrix3<float> tmp__output;
		tmp__output = static_cast<Material*>(self->GetNativeObject())->GetMat3(tmpname, arrayIdx);

		*__output = tmp__output;
	}

	void ScriptMaterial::InternalGetMat4(ScriptMaterial* self, MonoString* name, uint32_t arrayIdx, TMatrix4<float>* __output)
	{
		if(!self->IsNativeObjectValid())
		{
			*__output = {};
			return;
		}

		String tmpname;
		tmpname = MonoUtil::MonoToString(name);
		TMatrix4<float> tmp__output;
		tmp__output = static_cast<Material*>(self->GetNativeObject())->GetMat4(tmpname, arrayIdx);

		*__output = tmp__output;
	}

	bool ScriptMaterial::InternalIsAnimated(ScriptMaterial* self, MonoString* name, uint32_t arrayIdx)
	{
		bool tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		String tmpname;
		tmpname = MonoUtil::MonoToString(name);
		tmp__output = static_cast<Material*>(self->GetNativeObject())->IsAnimated(tmpname, arrayIdx);

		bool __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptMaterial::InternalCreate(MonoObject* scriptObject)
	{
		TResourceHandle<Material> nativeObject = Material::Create();
		ScriptObjectWrapper::Create<ScriptMaterial>(nativeObject, scriptObject);
	}

	void ScriptMaterial::InternalCreate0(MonoObject* scriptObject, MonoObject* shader)
	{
		TResourceHandle<Shader> tmpshader;
		ScriptRRefBase* scriptObjectWrappershader;
		scriptObjectWrappershader = ScriptRRefBase::GetScriptObjectWrapper(shader);
		if(scriptObjectWrappershader != nullptr)
			tmpshader = B3DStaticResourceCast<Shader>(scriptObjectWrappershader->GetNativeObject());
		TResourceHandle<Material> nativeObject = Material::Create(tmpshader);
		ScriptObjectWrapper::Create<ScriptMaterial>(nativeObject, scriptObject);
	}

	void ScriptMaterial::InternalSetTexture(ScriptMaterial* self, MonoString* name, MonoObject* value, uint32_t mipLevel, uint32_t numMipLevels, uint32_t arraySlice, uint32_t numArraySlices)
	{
		if(!self->IsNativeObjectValid())
			return;

		String tmpname;
		tmpname = MonoUtil::MonoToString(name);
		TResourceHandle<Texture> tmpvalue;
		ScriptRRefBase* scriptObjectWrappervalue;
		scriptObjectWrappervalue = ScriptRRefBase::GetScriptObjectWrapper(value);
		if(scriptObjectWrappervalue != nullptr)
			tmpvalue = B3DStaticResourceCast<Texture>(scriptObjectWrappervalue->GetNativeObject());
		MaterialEx::SetTexture(B3DStaticResourceCast<Material>(self->GetBaseNativeObjectAsHandle()), tmpname, tmpvalue, mipLevel, numMipLevels, arraySlice, numArraySlices);
	}

	MonoObject* ScriptMaterial::InternalGetTexture(ScriptMaterial* self, MonoString* name)
	{
		TResourceHandle<Texture> tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		String tmpname;
		tmpname = MonoUtil::MonoToString(name);
		tmp__output = MaterialEx::GetTexture(B3DStaticResourceCast<Material>(self->GetBaseNativeObjectAsHandle()), tmpname);

		MonoObject* __output;
		ScriptRRefBase* script__output;
		script__output = ScriptResourceManager::Instance().GetScriptRRef(tmp__output);
		if(script__output != nullptr)
			__output = script__output->GetScriptObject();
		else
			__output = nullptr;

		return __output;
	}

	void ScriptMaterial::InternalSetSpriteImage(ScriptMaterial* self, MonoString* name, MonoObject* value)
	{
		if(!self->IsNativeObjectValid())
			return;

		String tmpname;
		tmpname = MonoUtil::MonoToString(name);
		TResourceHandle<SpriteImage> tmpvalue;
		ScriptRRefBase* scriptObjectWrappervalue;
		scriptObjectWrappervalue = ScriptRRefBase::GetScriptObjectWrapper(value);
		if(scriptObjectWrappervalue != nullptr)
			tmpvalue = B3DStaticResourceCast<SpriteImage>(scriptObjectWrappervalue->GetNativeObject());
		MaterialEx::SetSpriteImage(B3DStaticResourceCast<Material>(self->GetBaseNativeObjectAsHandle()), tmpname, tmpvalue);
	}

	MonoObject* ScriptMaterial::InternalGetSpriteImage(ScriptMaterial* self, MonoString* name)
	{
		TResourceHandle<SpriteImage> tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		String tmpname;
		tmpname = MonoUtil::MonoToString(name);
		tmp__output = MaterialEx::GetSpriteImage(B3DStaticResourceCast<Material>(self->GetBaseNativeObjectAsHandle()), tmpname);

		MonoObject* __output;
		ScriptRRefBase* script__output;
		script__output = ScriptResourceManager::Instance().GetScriptRRef(tmp__output);
		if(script__output != nullptr)
			__output = script__output->GetScriptObject();
		else
			__output = nullptr;

		return __output;
	}
}
