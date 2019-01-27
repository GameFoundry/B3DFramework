//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptCSkybox.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "../../../../../Foundation/bsfCore/Components/BsCSkybox.h"
#include "BsScriptResourceManager.h"
#include "Wrappers/BsScriptRRefBase.h"
#include "../../../../../Foundation/bsfCore/Image/BsTexture.h"

namespace bs
{
	ScriptCSkybox::ScriptCSkybox(MonoObject* managedInstance, const GameObjectHandle<CSkybox>& value)
		:TScriptComponent(managedInstance, value)
	{
	}

	void ScriptCSkybox::initRuntimeData()
	{
		metaData.scriptClass->addInternalCall("Internal_getTexture", (void*)&ScriptCSkybox::Internal_getTexture);
		metaData.scriptClass->addInternalCall("Internal_setTexture", (void*)&ScriptCSkybox::Internal_setTexture);
		metaData.scriptClass->addInternalCall("Internal_setBrightness", (void*)&ScriptCSkybox::Internal_setBrightness);
		metaData.scriptClass->addInternalCall("Internal_getBrightness", (void*)&ScriptCSkybox::Internal_getBrightness);

	}

	MonoObject* ScriptCSkybox::Internal_getTexture(ScriptCSkybox* thisPtr)
	{
		ResourceHandle<Texture> tmp__output;
		tmp__output = thisPtr->getHandle()->getTexture();

		MonoObject* __output;
		ScriptRRefBase* script__output;
		script__output = ScriptResourceManager::instance().getScriptRRef(tmp__output);
		if(script__output != nullptr)
			__output = script__output->getManagedInstance();
		else
			__output = nullptr;

		return __output;
	}

	void ScriptCSkybox::Internal_setTexture(ScriptCSkybox* thisPtr, MonoObject* texture)
	{
		ResourceHandle<Texture> tmptexture;
		ScriptRRefBase* scripttexture;
		scripttexture = ScriptRRefBase::toNative(texture);
		if(scripttexture != nullptr)
			tmptexture = static_resource_cast<Texture>(scripttexture->getHandle());
		thisPtr->getHandle()->setTexture(tmptexture);
	}

	void ScriptCSkybox::Internal_setBrightness(ScriptCSkybox* thisPtr, float brightness)
	{
		thisPtr->getHandle()->setBrightness(brightness);
	}

	float ScriptCSkybox::Internal_getBrightness(ScriptCSkybox* thisPtr)
	{
		float tmp__output;
		tmp__output = thisPtr->getHandle()->getBrightness();

		float __output;
		__output = tmp__output;

		return __output;
	}
}
