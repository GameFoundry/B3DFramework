//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsScriptLight.h"
#include "BsScriptSceneObject.h"
#include "BsSceneObject.h"
#include "BsSceneManager.h"

namespace bs
{
	ScriptLight::ScriptLight(MonoObject* managedInstance, const HSceneObject& parentSO)
		:ScriptObject(managedInstance), mLight(nullptr), mLastUpdateHash(0)
	{
		mLight = Light::create();
		gSceneManager()._registerLight(mLight, parentSO);
	}

	ScriptLight::~ScriptLight()
	{ }

	void ScriptLight::initRuntimeData()
	{
		metaData.scriptClass->addInternalCall("Internal_Create", &ScriptLight::internal_create);

		metaData.scriptClass->addInternalCall("Internal_GetType", &ScriptLight::internal_getType);
		metaData.scriptClass->addInternalCall("Internal_SetType", &ScriptLight::internal_setType);
		metaData.scriptClass->addInternalCall("Internal_GetCastsShadow", &ScriptLight::internal_getCastsShadow);
		metaData.scriptClass->addInternalCall("Internal_SetCastsShadow", &ScriptLight::internal_setCastsShadow);
		metaData.scriptClass->addInternalCall("Internal_GetPhysicallyBasedAttenuation", 
			&ScriptLight::internal_getPhysicallyBasedAttenuation);
		metaData.scriptClass->addInternalCall("Internal_SetPhysicallyBasedAttenuation", 
			&ScriptLight::internal_setPhysicallyBasedAttenuation);
		metaData.scriptClass->addInternalCall("Internal_GetColor", &ScriptLight::internal_getColor);
		metaData.scriptClass->addInternalCall("Internal_SetColor", &ScriptLight::internal_setColor);
		metaData.scriptClass->addInternalCall("Internal_GetRange", &ScriptLight::internal_getRange);
		metaData.scriptClass->addInternalCall("Internal_SetRange", &ScriptLight::internal_setRange);
		metaData.scriptClass->addInternalCall("Internal_GetIntensity", &ScriptLight::internal_getIntensity);
		metaData.scriptClass->addInternalCall("Internal_SetIntensity", &ScriptLight::internal_setIntensity);
		metaData.scriptClass->addInternalCall("Internal_GetSpotAngle", &ScriptLight::internal_getSpotAngle);
		metaData.scriptClass->addInternalCall("Internal_SetSpotAngle", &ScriptLight::internal_setSpotAngle);
		metaData.scriptClass->addInternalCall("Internal_GetSpotFalloffAngle", &ScriptLight::internal_getSpotFalloffAngle);
		metaData.scriptClass->addInternalCall("Internal_SetSpotFalloffAngle", &ScriptLight::internal_setSpotFalloffAngle);
		metaData.scriptClass->addInternalCall("Internal_GetBounds", &ScriptLight::internal_getBounds);
		metaData.scriptClass->addInternalCall("Internal_UpdateTransform", &ScriptLight::internal_updateTransform);
		metaData.scriptClass->addInternalCall("Internal_OnDestroy", &ScriptLight::internal_onDestroy);
	}

	void ScriptLight::internal_create(MonoObject* managedInstance, ScriptSceneObject* parentSO)
	{
		HSceneObject so;
		if (parentSO != nullptr)
			so = parentSO->getNativeHandle();

		new (bs_alloc<ScriptLight>()) ScriptLight(managedInstance, so);
	}

	LightType ScriptLight::internal_getType(ScriptLight* thisPtr)
	{
		return thisPtr->getInternal()->getType();
	}

	void ScriptLight::internal_setType(ScriptLight* thisPtr, LightType type)
	{
		thisPtr->getInternal()->setType(type);
	}

	bool ScriptLight::internal_getCastsShadow(ScriptLight* thisPtr)
	{
		return thisPtr->getInternal()->getCastsShadow();
	}

	void ScriptLight::internal_setCastsShadow(ScriptLight* thisPtr, bool castsShadow)
	{
		thisPtr->getInternal()->setCastsShadow(castsShadow);
	}

	bool ScriptLight::internal_getPhysicallyBasedAttenuation(ScriptLight* thisPtr)
	{
		return thisPtr->getInternal()->getPhysicallyBasedAttenuation();
	}

	void ScriptLight::internal_setPhysicallyBasedAttenuation(ScriptLight* thisPtr, bool value)
	{
		thisPtr->getInternal()->setPhysicallyBasedAttenuation(value);
	}

	void ScriptLight::internal_getColor(ScriptLight* thisPtr, Color* color)
	{
		*color = thisPtr->getInternal()->getColor();
	}

	void ScriptLight::internal_setColor(ScriptLight* thisPtr, Color color)
	{
		thisPtr->getInternal()->setColor(color);
	}

	float ScriptLight::internal_getRange(ScriptLight* thisPtr)
	{
		return thisPtr->getInternal()->getRange();
	}

	void ScriptLight::internal_setRange(ScriptLight* thisPtr, float range)
	{
		thisPtr->getInternal()->setRange(range);
	}

	float ScriptLight::internal_getIntensity(ScriptLight* thisPtr)
	{
		return thisPtr->getInternal()->getIntensity();
	}

	void ScriptLight::internal_setIntensity(ScriptLight* thisPtr, float intensity)
	{
		thisPtr->getInternal()->setIntensity(intensity);
	}

	float ScriptLight::internal_getSpotAngle(ScriptLight* thisPtr)
	{
		return thisPtr->getInternal()->getSpotAngle().valueDegrees();
	}

	void ScriptLight::internal_setSpotAngle(ScriptLight* thisPtr, float spotAngle)
	{
		thisPtr->getInternal()->setSpotAngle(Degree(spotAngle));
	}

	float ScriptLight::internal_getSpotFalloffAngle(ScriptLight* thisPtr)
	{
		return thisPtr->getInternal()->getSpotFalloffAngle().valueDegrees();
	}

	void ScriptLight::internal_setSpotFalloffAngle(ScriptLight* thisPtr, float spotFalloffAngle)
	{
		thisPtr->getInternal()->setSpotFalloffAngle(Degree(spotFalloffAngle));
	}

	void ScriptLight::internal_getBounds(ScriptLight* thisPtr, Sphere* bounds)
	{
		*bounds = thisPtr->getInternal()->getBounds();
	}

	void ScriptLight::internal_updateTransform(ScriptLight* thisPtr, ScriptSceneObject* parent)
	{
		HSceneObject parentSO = parent->getNativeSceneObject();

		if (!parentSO.isDestroyed())
		{
			thisPtr->getInternal()->_updateTransform(parentSO);

			if (parentSO->getActive() != thisPtr->getInternal()->getIsActive())
			{
				thisPtr->getInternal()->setIsActive(parentSO->getActive());
			}
		}
	}

	void ScriptLight::internal_onDestroy(ScriptLight* instance)
	{
		instance->destroy();
	}

	void ScriptLight::destroy()
	{
		if (mLight->isDestroyed())
			return;

		gSceneManager()._unregisterLight(mLight);
		mLight->destroy();
	}

	void ScriptLight::_onManagedInstanceDeleted()
	{
		destroy();

		ScriptObject::_onManagedInstanceDeleted();
	}
}