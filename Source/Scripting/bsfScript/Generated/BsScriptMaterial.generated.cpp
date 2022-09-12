//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptMaterial.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "../../../Foundation/bsfCore/Material/BsMaterial.h"
#include "BsScriptResourceManager.h"
#include "Wrappers/BsScriptRRefBase.h"
#include "Wrappers/BsScriptVector.h"
#include "Wrappers/BsScriptColor.h"
#include "../../../Foundation/bsfCore/Material/BsMaterial.h"
#include "../../../Foundation/bsfCore/Material/BsShader.h"
#include "BsScriptShaderVariation.generated.h"
#include "Wrappers/BsScriptVector.h"
#include "BsScriptColorGradientHDR.generated.h"
#include "BsScriptTAnimationCurve.generated.h"
#include "Wrappers/BsScriptVector.h"
#include "../../../Foundation/bsfCore/Image/BsTexture.h"
#include "../Extensions/BsMaterialEx.h"
#include "../../../Foundation/bsfCore/Image/BsSpriteTexture.h"

namespace bs
{
	ScriptMaterial::ScriptMaterial(MonoObject* managedInstance, const ResourceHandle<Material>& value)
		:TScriptResource(managedInstance, value)
	{
	}

	void ScriptMaterial::InitRuntimeData()
	{
		metaData.scriptClass->AddInternalCall("Internal_GetRef", (void*)&ScriptMaterial::Internal_getRef);
		metaData.scriptClass->AddInternalCall("Internal_setShader", (void*)&ScriptMaterial::Internal_setShader);
		metaData.scriptClass->AddInternalCall("Internal_setVariation", (void*)&ScriptMaterial::Internal_setVariation);
		metaData.scriptClass->AddInternalCall("Internal_clone", (void*)&ScriptMaterial::Internal_clone);
		metaData.scriptClass->AddInternalCall("Internal_getShader", (void*)&ScriptMaterial::Internal_getShader);
		metaData.scriptClass->AddInternalCall("Internal_getVariation", (void*)&ScriptMaterial::Internal_getVariation);
		metaData.scriptClass->AddInternalCall("Internal_setFloat", (void*)&ScriptMaterial::Internal_setFloat);
		metaData.scriptClass->AddInternalCall("Internal_setFloatCurve", (void*)&ScriptMaterial::Internal_setFloatCurve);
		metaData.scriptClass->AddInternalCall("Internal_setColor", (void*)&ScriptMaterial::Internal_setColor);
		metaData.scriptClass->AddInternalCall("Internal_setColorGradient", (void*)&ScriptMaterial::Internal_setColorGradient);
		metaData.scriptClass->AddInternalCall("Internal_setVec2", (void*)&ScriptMaterial::Internal_setVec2);
		metaData.scriptClass->AddInternalCall("Internal_setVec3", (void*)&ScriptMaterial::Internal_setVec3);
		metaData.scriptClass->AddInternalCall("Internal_setVec4", (void*)&ScriptMaterial::Internal_setVec4);
		metaData.scriptClass->AddInternalCall("Internal_setMat3", (void*)&ScriptMaterial::Internal_setMat3);
		metaData.scriptClass->AddInternalCall("Internal_setMat4", (void*)&ScriptMaterial::Internal_setMat4);
		metaData.scriptClass->AddInternalCall("Internal_getFloat", (void*)&ScriptMaterial::Internal_getFloat);
		metaData.scriptClass->AddInternalCall("Internal_getFloatCurve", (void*)&ScriptMaterial::Internal_getFloatCurve);
		metaData.scriptClass->AddInternalCall("Internal_getColor", (void*)&ScriptMaterial::Internal_getColor);
		metaData.scriptClass->AddInternalCall("Internal_getColorGradient", (void*)&ScriptMaterial::Internal_getColorGradient);
		metaData.scriptClass->AddInternalCall("Internal_getVec2", (void*)&ScriptMaterial::Internal_getVec2);
		metaData.scriptClass->AddInternalCall("Internal_getVec3", (void*)&ScriptMaterial::Internal_getVec3);
		metaData.scriptClass->AddInternalCall("Internal_getVec4", (void*)&ScriptMaterial::Internal_getVec4);
		metaData.scriptClass->AddInternalCall("Internal_getMat3", (void*)&ScriptMaterial::Internal_getMat3);
		metaData.scriptClass->AddInternalCall("Internal_getMat4", (void*)&ScriptMaterial::Internal_getMat4);
		metaData.scriptClass->AddInternalCall("Internal_isAnimated", (void*)&ScriptMaterial::Internal_isAnimated);
		metaData.scriptClass->AddInternalCall("Internal_create", (void*)&ScriptMaterial::Internal_create);
		metaData.scriptClass->AddInternalCall("Internal_create0", (void*)&ScriptMaterial::Internal_create0);
		metaData.scriptClass->AddInternalCall("Internal_setTexture", (void*)&ScriptMaterial::Internal_setTexture);
		metaData.scriptClass->AddInternalCall("Internal_getTexture", (void*)&ScriptMaterial::Internal_getTexture);
		metaData.scriptClass->AddInternalCall("Internal_setSpriteTexture", (void*)&ScriptMaterial::Internal_setSpriteTexture);
		metaData.scriptClass->AddInternalCall("Internal_getSpriteTexture", (void*)&ScriptMaterial::Internal_getSpriteTexture);

	}

	 MonoObject*ScriptMaterial::createInstance()
	{
		bool dummy = false;
		void* ctorParams[1] = { &dummy };

		return metaData.scriptClass->CreateInstance("bool", ctorParams);
	}
	MonoObject* ScriptMaterial::Internal_getRef(ScriptMaterial* thisPtr)
	{
		return thisPtr->GetRRef();
	}

	void ScriptMaterial::Internal_setShader(ScriptMaterial* thisPtr, MonoObject* shader)
	{
		ResourceHandle<Shader> tmpshader;
		ScriptRRefBase* scriptshader;
		scriptshader = ScriptRRefBase::toNative(shader);
		if(scriptshader != nullptr)
			tmpshader = static_resource_cast<Shader>(scriptshader->GetHandle());
		thisPtr->GetHandle()->setShader(tmpshader);
	}

	void ScriptMaterial::Internal_setVariation(ScriptMaterial* thisPtr, MonoObject* variation)
	{
		SPtr<ShaderVariation> tmpvariation;
		ScriptShaderVariation* scriptvariation;
		scriptvariation = ScriptShaderVariation::toNative(variation);
		if(scriptvariation != nullptr)
			tmpvariation = scriptvariation->GetInternal();
		thisPtr->GetHandle()->setVariation(*tmpvariation);
	}

	MonoObject* ScriptMaterial::Internal_clone(ScriptMaterial* thisPtr)
	{
		ResourceHandle<Material> tmp__output;
		tmp__output = thisPtr->GetHandle()->clone();

		MonoObject* __output;
		ScriptRRefBase* script__output;
		script__output = ScriptResourceManager::instance().GetScriptRRef(tmp__output);
		if(script__output != nullptr)
			__output = script__output->GetManagedInstance();
		else
			__output = nullptr;

		return __output;
	}

	MonoObject* ScriptMaterial::Internal_getShader(ScriptMaterial* thisPtr)
	{
		ResourceHandle<Shader> tmp__output;
		tmp__output = thisPtr->GetHandle()->getShader();

		MonoObject* __output;
		ScriptRRefBase* script__output;
		script__output = ScriptResourceManager::instance().GetScriptRRef(tmp__output);
		if(script__output != nullptr)
			__output = script__output->GetManagedInstance();
		else
			__output = nullptr;

		return __output;
	}

	MonoObject* ScriptMaterial::Internal_getVariation(ScriptMaterial* thisPtr)
	{
		SPtr<ShaderVariation> tmp__output = bs_shared_ptr_new<ShaderVariation>();
		*tmp__output = thisPtr->GetHandle()->getVariation();

		MonoObject* __output;
		__output = ScriptShaderVariation::create(tmp__output);

		return __output;
	}

	void ScriptMaterial::Internal_setFloat(ScriptMaterial* thisPtr, MonoString* name, float value, uint32_t arrayIdx)
	{
		String tmpname;
		tmpname = MonoUtil::monoToString(name);
		thisPtr->GetHandle()->setFloat(tmpname, value, arrayIdx);
	}

	void ScriptMaterial::Internal_setFloatCurve(ScriptMaterial* thisPtr, MonoString* name, MonoObject* value, uint32_t arrayIdx)
	{
		String tmpname;
		tmpname = MonoUtil::monoToString(name);
		SPtr<TAnimationCurve<float>> tmpvalue;
		ScriptTAnimationCurvefloat* scriptvalue;
		scriptvalue = ScriptTAnimationCurvefloat::toNative(value);
		if(scriptvalue != nullptr)
			tmpvalue = scriptvalue->GetInternal();
		thisPtr->GetHandle()->setFloatCurve(tmpname, *tmpvalue, arrayIdx);
	}

	void ScriptMaterial::Internal_setColor(ScriptMaterial* thisPtr, MonoString* name, Color* value, uint32_t arrayIdx)
	{
		String tmpname;
		tmpname = MonoUtil::monoToString(name);
		thisPtr->GetHandle()->setColor(tmpname, *value, arrayIdx);
	}

	void ScriptMaterial::Internal_setColorGradient(ScriptMaterial* thisPtr, MonoString* name, MonoObject* value, uint32_t arrayIdx)
	{
		String tmpname;
		tmpname = MonoUtil::monoToString(name);
		SPtr<ColorGradientHDR> tmpvalue;
		ScriptColorGradientHDR* scriptvalue;
		scriptvalue = ScriptColorGradientHDR::toNative(value);
		if(scriptvalue != nullptr)
			tmpvalue = scriptvalue->GetInternal();
		thisPtr->GetHandle()->setColorGradient(tmpname, *tmpvalue, arrayIdx);
	}

	void ScriptMaterial::Internal_setVec2(ScriptMaterial* thisPtr, MonoString* name, Vector2* value, uint32_t arrayIdx)
	{
		String tmpname;
		tmpname = MonoUtil::monoToString(name);
		thisPtr->GetHandle()->setVec2(tmpname, *value, arrayIdx);
	}

	void ScriptMaterial::Internal_setVec3(ScriptMaterial* thisPtr, MonoString* name, Vector3* value, uint32_t arrayIdx)
	{
		String tmpname;
		tmpname = MonoUtil::monoToString(name);
		thisPtr->GetHandle()->setVec3(tmpname, *value, arrayIdx);
	}

	void ScriptMaterial::Internal_setVec4(ScriptMaterial* thisPtr, MonoString* name, Vector4* value, uint32_t arrayIdx)
	{
		String tmpname;
		tmpname = MonoUtil::monoToString(name);
		thisPtr->GetHandle()->setVec4(tmpname, *value, arrayIdx);
	}

	void ScriptMaterial::Internal_setMat3(ScriptMaterial* thisPtr, MonoString* name, Matrix3* value, uint32_t arrayIdx)
	{
		String tmpname;
		tmpname = MonoUtil::monoToString(name);
		thisPtr->GetHandle()->setMat3(tmpname, *value, arrayIdx);
	}

	void ScriptMaterial::Internal_setMat4(ScriptMaterial* thisPtr, MonoString* name, Matrix4* value, uint32_t arrayIdx)
	{
		String tmpname;
		tmpname = MonoUtil::monoToString(name);
		thisPtr->GetHandle()->setMat4(tmpname, *value, arrayIdx);
	}

	float ScriptMaterial::Internal_getFloat(ScriptMaterial* thisPtr, MonoString* name, uint32_t arrayIdx)
	{
		float tmp__output;
		String tmpname;
		tmpname = MonoUtil::monoToString(name);
		tmp__output = thisPtr->GetHandle()->getFloat(tmpname, arrayIdx);

		float __output;
		__output = tmp__output;

		return __output;
	}

	MonoObject* ScriptMaterial::Internal_getFloatCurve(ScriptMaterial* thisPtr, MonoString* name, uint32_t arrayIdx)
	{
		SPtr<TAnimationCurve<float>> tmp__output = bs_shared_ptr_new<TAnimationCurve<float>>();
		String tmpname;
		tmpname = MonoUtil::monoToString(name);
		*tmp__output = thisPtr->GetHandle()->getFloatCurve(tmpname, arrayIdx);

		MonoObject* __output;
		__output = ScriptTAnimationCurvefloat::create(tmp__output);

		return __output;
	}

	void ScriptMaterial::Internal_getColor(ScriptMaterial* thisPtr, MonoString* name, uint32_t arrayIdx, Color* __output)
	{
		String tmpname;
		tmpname = MonoUtil::monoToString(name);
		Color tmp__output;
		tmp__output = thisPtr->GetHandle()->getColor(tmpname, arrayIdx);

		*__output = tmp__output;
	}

	MonoObject* ScriptMaterial::Internal_getColorGradient(ScriptMaterial* thisPtr, MonoString* name, uint32_t arrayIdx)
	{
		SPtr<ColorGradientHDR> tmp__output = bs_shared_ptr_new<ColorGradientHDR>();
		String tmpname;
		tmpname = MonoUtil::monoToString(name);
		*tmp__output = thisPtr->GetHandle()->getColorGradient(tmpname, arrayIdx);

		MonoObject* __output;
		__output = ScriptColorGradientHDR::create(tmp__output);

		return __output;
	}

	void ScriptMaterial::Internal_getVec2(ScriptMaterial* thisPtr, MonoString* name, uint32_t arrayIdx, Vector2* __output)
	{
		String tmpname;
		tmpname = MonoUtil::monoToString(name);
		Vector2 tmp__output;
		tmp__output = thisPtr->GetHandle()->getVec2(tmpname, arrayIdx);

		*__output = tmp__output;
	}

	void ScriptMaterial::Internal_getVec3(ScriptMaterial* thisPtr, MonoString* name, uint32_t arrayIdx, Vector3* __output)
	{
		String tmpname;
		tmpname = MonoUtil::monoToString(name);
		Vector3 tmp__output;
		tmp__output = thisPtr->GetHandle()->getVec3(tmpname, arrayIdx);

		*__output = tmp__output;
	}

	void ScriptMaterial::Internal_getVec4(ScriptMaterial* thisPtr, MonoString* name, uint32_t arrayIdx, Vector4* __output)
	{
		String tmpname;
		tmpname = MonoUtil::monoToString(name);
		Vector4 tmp__output;
		tmp__output = thisPtr->GetHandle()->getVec4(tmpname, arrayIdx);

		*__output = tmp__output;
	}

	void ScriptMaterial::Internal_getMat3(ScriptMaterial* thisPtr, MonoString* name, uint32_t arrayIdx, Matrix3* __output)
	{
		String tmpname;
		tmpname = MonoUtil::monoToString(name);
		Matrix3 tmp__output;
		tmp__output = thisPtr->GetHandle()->getMat3(tmpname, arrayIdx);

		*__output = tmp__output;
	}

	void ScriptMaterial::Internal_getMat4(ScriptMaterial* thisPtr, MonoString* name, uint32_t arrayIdx, Matrix4* __output)
	{
		String tmpname;
		tmpname = MonoUtil::monoToString(name);
		Matrix4 tmp__output;
		tmp__output = thisPtr->GetHandle()->getMat4(tmpname, arrayIdx);

		*__output = tmp__output;
	}

	bool ScriptMaterial::Internal_isAnimated(ScriptMaterial* thisPtr, MonoString* name, uint32_t arrayIdx)
	{
		bool tmp__output;
		String tmpname;
		tmpname = MonoUtil::monoToString(name);
		tmp__output = thisPtr->GetHandle()->isAnimated(tmpname, arrayIdx);

		bool __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptMaterial::Internal_create(MonoObject* managedInstance)
	{
		ResourceHandle<Material> instance = Material::create();
		ScriptResourceManager::instance().CreateBuiltinScriptResource(instance, managedInstance);
	}

	void ScriptMaterial::Internal_create0(MonoObject* managedInstance, MonoObject* shader)
	{
		ResourceHandle<Shader> tmpshader;
		ScriptRRefBase* scriptshader;
		scriptshader = ScriptRRefBase::toNative(shader);
		if(scriptshader != nullptr)
			tmpshader = static_resource_cast<Shader>(scriptshader->GetHandle());
		ResourceHandle<Material> instance = Material::create(tmpshader);
		ScriptResourceManager::instance().CreateBuiltinScriptResource(instance, managedInstance);
	}

	void ScriptMaterial::Internal_setTexture(ScriptMaterial* thisPtr, MonoString* name, MonoObject* value, uint32_t mipLevel, uint32_t numMipLevels, uint32_t arraySlice, uint32_t numArraySlices)
	{
		String tmpname;
		tmpname = MonoUtil::monoToString(name);
		ResourceHandle<Texture> tmpvalue;
		ScriptRRefBase* scriptvalue;
		scriptvalue = ScriptRRefBase::toNative(value);
		if(scriptvalue != nullptr)
			tmpvalue = static_resource_cast<Texture>(scriptvalue->GetHandle());
		MaterialEx::setTexture(thisPtr->GetHandle(), tmpname, tmpvalue, mipLevel, numMipLevels, arraySlice, numArraySlices);
	}

	MonoObject* ScriptMaterial::Internal_getTexture(ScriptMaterial* thisPtr, MonoString* name)
	{
		ResourceHandle<Texture> tmp__output;
		String tmpname;
		tmpname = MonoUtil::monoToString(name);
		tmp__output = MaterialEx::getTexture(thisPtr->GetHandle(), tmpname);

		MonoObject* __output;
		ScriptRRefBase* script__output;
		script__output = ScriptResourceManager::instance().GetScriptRRef(tmp__output);
		if(script__output != nullptr)
			__output = script__output->GetManagedInstance();
		else
			__output = nullptr;

		return __output;
	}

	void ScriptMaterial::Internal_setSpriteTexture(ScriptMaterial* thisPtr, MonoString* name, MonoObject* value)
	{
		String tmpname;
		tmpname = MonoUtil::monoToString(name);
		ResourceHandle<SpriteTexture> tmpvalue;
		ScriptRRefBase* scriptvalue;
		scriptvalue = ScriptRRefBase::toNative(value);
		if(scriptvalue != nullptr)
			tmpvalue = static_resource_cast<SpriteTexture>(scriptvalue->GetHandle());
		MaterialEx::setSpriteTexture(thisPtr->GetHandle(), tmpname, tmpvalue);
	}

	MonoObject* ScriptMaterial::Internal_getSpriteTexture(ScriptMaterial* thisPtr, MonoString* name)
	{
		ResourceHandle<SpriteTexture> tmp__output;
		String tmpname;
		tmpname = MonoUtil::monoToString(name);
		tmp__output = MaterialEx::getSpriteTexture(thisPtr->GetHandle(), tmpname);

		MonoObject* __output;
		ScriptRRefBase* script__output;
		script__output = ScriptResourceManager::instance().GetScriptRRef(tmp__output);
		if(script__output != nullptr)
			__output = script__output->GetManagedInstance();
		else
			__output = nullptr;

		return __output;
	}
}
