//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptCReflectionProbe.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "../../../../../Foundation/bsfCore/Components/BsCReflectionProbe.h"
#include "BsScriptResourceManager.h"
#include "Wrappers/BsScriptRRefBase.h"
#include "Wrappers/BsScriptVector.h"
#include "../../../../../Foundation/bsfCore/Image/BsTexture.h"

namespace bs
{
	ScriptCReflectionProbe::ScriptCReflectionProbe(MonoObject* managedInstance, const GameObjectHandle<CReflectionProbe>& value)
		:TScriptComponent(managedInstance, value)
	{
	}

	void ScriptCReflectionProbe::initRuntimeData()
	{
		metaData.scriptClass->addInternalCall("Internal_getType", (void*)&ScriptCReflectionProbe::Internal_getType);
		metaData.scriptClass->addInternalCall("Internal_setType", (void*)&ScriptCReflectionProbe::Internal_setType);
		metaData.scriptClass->addInternalCall("Internal_getRadius", (void*)&ScriptCReflectionProbe::Internal_getRadius);
		metaData.scriptClass->addInternalCall("Internal_setRadius", (void*)&ScriptCReflectionProbe::Internal_setRadius);
		metaData.scriptClass->addInternalCall("Internal_getExtents", (void*)&ScriptCReflectionProbe::Internal_getExtents);
		metaData.scriptClass->addInternalCall("Internal_setExtents", (void*)&ScriptCReflectionProbe::Internal_setExtents);
		metaData.scriptClass->addInternalCall("Internal_getCustomTexture", (void*)&ScriptCReflectionProbe::Internal_getCustomTexture);
		metaData.scriptClass->addInternalCall("Internal_setCustomTexture", (void*)&ScriptCReflectionProbe::Internal_setCustomTexture);
		metaData.scriptClass->addInternalCall("Internal_capture", (void*)&ScriptCReflectionProbe::Internal_capture);

	}

	ReflectionProbeType ScriptCReflectionProbe::Internal_getType(ScriptCReflectionProbe* thisPtr)
	{
		ReflectionProbeType tmp__output;
		tmp__output = thisPtr->getHandle()->getType();

		ReflectionProbeType __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptCReflectionProbe::Internal_setType(ScriptCReflectionProbe* thisPtr, ReflectionProbeType type)
	{
		thisPtr->getHandle()->setType(type);
	}

	float ScriptCReflectionProbe::Internal_getRadius(ScriptCReflectionProbe* thisPtr)
	{
		float tmp__output;
		tmp__output = thisPtr->getHandle()->getRadius();

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptCReflectionProbe::Internal_setRadius(ScriptCReflectionProbe* thisPtr, float radius)
	{
		thisPtr->getHandle()->setRadius(radius);
	}

	void ScriptCReflectionProbe::Internal_getExtents(ScriptCReflectionProbe* thisPtr, Vector3* __output)
	{
		Vector3 tmp__output;
		tmp__output = thisPtr->getHandle()->getExtents();

		*__output = tmp__output;
	}

	void ScriptCReflectionProbe::Internal_setExtents(ScriptCReflectionProbe* thisPtr, Vector3* extents)
	{
		thisPtr->getHandle()->setExtents(*extents);
	}

	MonoObject* ScriptCReflectionProbe::Internal_getCustomTexture(ScriptCReflectionProbe* thisPtr)
	{
		ResourceHandle<Texture> tmp__output;
		tmp__output = thisPtr->getHandle()->getCustomTexture();

		MonoObject* __output;
		ScriptRRefBase* script__output;
		script__output = ScriptResourceManager::instance().getScriptRRef(tmp__output);
		if(script__output != nullptr)
			__output = script__output->getManagedInstance();
		else
			__output = nullptr;

		return __output;
	}

	void ScriptCReflectionProbe::Internal_setCustomTexture(ScriptCReflectionProbe* thisPtr, MonoObject* texture)
	{
		ResourceHandle<Texture> tmptexture;
		ScriptRRefBase* scripttexture;
		scripttexture = ScriptRRefBase::toNative(texture);
		if(scripttexture != nullptr)
			tmptexture = static_resource_cast<Texture>(scripttexture->getHandle());
		thisPtr->getHandle()->setCustomTexture(tmptexture);
	}

	void ScriptCReflectionProbe::Internal_capture(ScriptCReflectionProbe* thisPtr)
	{
		thisPtr->getHandle()->capture();
	}
}
