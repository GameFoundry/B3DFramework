//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsScriptMaterial.h"
#include "BsScriptResourceManager.h"
#include "BsScriptMeta.h"
#include "BsMonoField.h"
#include "BsMonoClass.h"
#include "BsMonoManager.h"
#include "BsMonoUtil.h"
#include "BsScriptShader.h"
#include "BsBuiltinResources.h"

#include "BsScriptTexture.generated.h"

namespace bs
{
	ScriptMaterial::ScriptMaterial(MonoObject* instance, const HMaterial& material)
		:TScriptResource(instance, material)
	{

	}

	void ScriptMaterial::initRuntimeData()
	{
		metaData.scriptClass->addInternalCall("Internal_CreateInstance", &ScriptMaterial::internal_CreateInstance);
		metaData.scriptClass->addInternalCall("Internal_Clone", &ScriptMaterial::internal_Clone);

		metaData.scriptClass->addInternalCall("Internal_GetShader", &ScriptMaterial::internal_GetShader);
		metaData.scriptClass->addInternalCall("Internal_SetShader", &ScriptMaterial::internal_SetShader);

		metaData.scriptClass->addInternalCall("Internal_SetFloat", &ScriptMaterial::internal_SetFloat);
		metaData.scriptClass->addInternalCall("Internal_SetVector2", &ScriptMaterial::internal_SetVector2);
		metaData.scriptClass->addInternalCall("Internal_SetVector3", &ScriptMaterial::internal_SetVector3);
		metaData.scriptClass->addInternalCall("Internal_SetVector4", &ScriptMaterial::internal_SetVector4);
		metaData.scriptClass->addInternalCall("Internal_SetMatrix3", &ScriptMaterial::internal_SetMatrix3);
		metaData.scriptClass->addInternalCall("Internal_SetMatrix4", &ScriptMaterial::internal_SetMatrix4);
		metaData.scriptClass->addInternalCall("Internal_SetColor", &ScriptMaterial::internal_SetColor);
		metaData.scriptClass->addInternalCall("Internal_SetTexture", &ScriptMaterial::internal_SetTexture);

		metaData.scriptClass->addInternalCall("Internal_GetFloat", &ScriptMaterial::internal_GetFloat);
		metaData.scriptClass->addInternalCall("Internal_GetVector2", &ScriptMaterial::internal_GetVector2);
		metaData.scriptClass->addInternalCall("Internal_GetVector3", &ScriptMaterial::internal_GetVector3);
		metaData.scriptClass->addInternalCall("Internal_GetVector4", &ScriptMaterial::internal_GetVector4);
		metaData.scriptClass->addInternalCall("Internal_GetMatrix3", &ScriptMaterial::internal_GetMatrix3);
		metaData.scriptClass->addInternalCall("Internal_GetMatrix4", &ScriptMaterial::internal_GetMatrix4);
		metaData.scriptClass->addInternalCall("Internal_GetColor", &ScriptMaterial::internal_GetColor);
		metaData.scriptClass->addInternalCall("Internal_GetTexture", &ScriptMaterial::internal_GetTexture);
	}

	void ScriptMaterial::internal_CreateInstance(MonoObject* instance, ScriptShader* shader)
	{
		HShader nativeShader;
		if (shader != nullptr)
			nativeShader = shader->getHandle();

		if (nativeShader == nullptr)
			nativeShader = BuiltinResources::instance().getBuiltinShader(BuiltinShader::Standard);

		HMaterial material = Material::create(nativeShader);
		ScriptResourceBase* scriptInstance = ScriptResourceManager::instance().createBuiltinScriptResource(material, instance);
	}

	MonoObject* ScriptMaterial::internal_Clone(ScriptMaterial* nativeInstance)
	{
		HMaterial clone = nativeInstance->getHandle()->clone();

		ScriptResourceBase* scriptClone = ScriptResourceManager::instance().createBuiltinScriptResource(clone);
		return scriptClone->getManagedInstance();
	}

	MonoObject* ScriptMaterial::internal_GetShader(ScriptMaterial* nativeInstance)
	{
		HShader shader = nativeInstance->getHandle()->getShader();

		if (!shader.isLoaded())
			return nullptr;

		ScriptResourceBase* scriptShader = ScriptResourceManager::instance().getScriptResource(shader, true);
		return scriptShader->getManagedInstance();
	}

	void ScriptMaterial::internal_SetShader(ScriptMaterial* nativeInstance, ScriptShader* shader)
	{
		HShader nativeShader;
		if (shader != nullptr)
			nativeShader = shader->getHandle();

		if (nativeShader == nullptr)
			nativeShader = BuiltinResources::instance().getBuiltinShader(BuiltinShader::Standard);

		nativeInstance->getHandle()->setShader(nativeShader);
	}

	void ScriptMaterial::internal_SetFloat(ScriptMaterial* nativeInstance, MonoString* name, float value)
	{
		String paramName = MonoUtil::monoToString(name);

		nativeInstance->getHandle()->setFloat(paramName, value);
	}

	void ScriptMaterial::internal_SetVector2(ScriptMaterial* nativeInstance, MonoString* name, Vector2* value)
	{
		String paramName = MonoUtil::monoToString(name);

		nativeInstance->getHandle()->setVec2(paramName, *value);
	}

	void ScriptMaterial::internal_SetVector3(ScriptMaterial* nativeInstance, MonoString* name, Vector3* value)
	{
		String paramName = MonoUtil::monoToString(name);

		nativeInstance->getHandle()->setVec3(paramName, *value);
	}

	void ScriptMaterial::internal_SetVector4(ScriptMaterial* nativeInstance, MonoString* name, Vector4* value)
	{
		String paramName = MonoUtil::monoToString(name);

		nativeInstance->getHandle()->setVec4(paramName, *value);
	}

	void ScriptMaterial::internal_SetMatrix3(ScriptMaterial* nativeInstance, MonoString* name, Matrix3* value)
	{
		String paramName = MonoUtil::monoToString(name);

		nativeInstance->getHandle()->setMat3(paramName, *value);
	}

	void ScriptMaterial::internal_SetMatrix4(ScriptMaterial* nativeInstance, MonoString* name, Matrix4* value)
	{
		String paramName = MonoUtil::monoToString(name);

		nativeInstance->getHandle()->setMat4(paramName, *value);
	}

	void ScriptMaterial::internal_SetColor(ScriptMaterial* nativeInstance, MonoString* name, Color* value)
	{
		String paramName = MonoUtil::monoToString(name);

		nativeInstance->getHandle()->setColor(paramName, *value);
	}

	void ScriptMaterial::internal_SetTexture(ScriptMaterial* nativeInstance, MonoString* name, ScriptTexture* value)
	{
		String paramName = MonoUtil::monoToString(name);

		HTexture texture;

		if (value != nullptr)
			texture = value->getHandle();

		nativeInstance->getHandle()->setTexture(paramName, texture);
	}

	float ScriptMaterial::internal_GetFloat(ScriptMaterial* nativeInstance, MonoString* name)
	{
		String paramName = MonoUtil::monoToString(name);

		return nativeInstance->getHandle()->getFloat(paramName);
	}

	void ScriptMaterial::internal_GetVector2(ScriptMaterial* nativeInstance, MonoString* name, Vector2* value)
	{
		String paramName = MonoUtil::monoToString(name);

		*value = nativeInstance->getHandle()->getVec2(paramName);
	}

	void ScriptMaterial::internal_GetVector3(ScriptMaterial* nativeInstance, MonoString* name, Vector3* value)
	{
		String paramName = MonoUtil::monoToString(name);

		*value = nativeInstance->getHandle()->getVec3(paramName);
	}

	void ScriptMaterial::internal_GetVector4(ScriptMaterial* nativeInstance, MonoString* name, Vector4* value)
	{
		String paramName = MonoUtil::monoToString(name);

		*value = nativeInstance->getHandle()->getVec4(paramName);
	}

	void ScriptMaterial::internal_GetMatrix3(ScriptMaterial* nativeInstance, MonoString* name, Matrix3* value)
	{
		String paramName = MonoUtil::monoToString(name);

		*value = nativeInstance->getHandle()->getMat3(paramName);
	}

	void ScriptMaterial::internal_GetMatrix4(ScriptMaterial* nativeInstance, MonoString* name, Matrix4* value)
	{
		String paramName = MonoUtil::monoToString(name);

		*value = nativeInstance->getHandle()->getMat4(paramName);
	}

	void ScriptMaterial::internal_GetColor(ScriptMaterial* nativeInstance, MonoString* name, Color* value)
	{
		String paramName = MonoUtil::monoToString(name);

		*value = nativeInstance->getHandle()->getColor(paramName);
	}

	MonoObject* ScriptMaterial::internal_GetTexture(ScriptMaterial* nativeInstance, MonoString* name)
	{
		String paramName = MonoUtil::monoToString(name);

		HTexture texture = nativeInstance->getHandle()->getTexture(paramName);
		if (texture == nullptr)
			return nullptr;

		ScriptResourceBase* scriptTexture = ScriptResourceManager::instance().getScriptResource(texture, true);
		return scriptTexture->getManagedInstance();
	}

	MonoObject* ScriptMaterial::createInstance()
	{
		bool dummy = false;

		void* params[1];
		params[0] = &dummy;

		return metaData.scriptClass->createInstance("bool", params);
	}

	MonoObject* ScriptMaterial::_createManagedInstance(bool construct)
	{
		if (construct)
			return createInstance();

		return metaData.scriptClass->createInstance(false);
	}
}