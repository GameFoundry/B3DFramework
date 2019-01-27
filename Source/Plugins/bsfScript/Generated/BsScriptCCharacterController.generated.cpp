//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptCCharacterController.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "../../../../../Foundation/bsfCore/Components/BsCCharacterController.h"
#include "BsScriptControllerColliderCollision.generated.h"
#include "Wrappers/BsScriptVector.h"
#include "BsScriptControllerControllerCollision.generated.h"

namespace bs
{
	ScriptCCharacterController::onColliderHitThunkDef ScriptCCharacterController::onColliderHitThunk; 
	ScriptCCharacterController::onControllerHitThunkDef ScriptCCharacterController::onControllerHitThunk; 

	ScriptCCharacterController::ScriptCCharacterController(MonoObject* managedInstance, const GameObjectHandle<CCharacterController>& value)
		:TScriptComponent(managedInstance, value)
	{
		value->onColliderHit.connect(std::bind(&ScriptCCharacterController::onColliderHit, this, std::placeholders::_1));
		value->onControllerHit.connect(std::bind(&ScriptCCharacterController::onControllerHit, this, std::placeholders::_1));
	}

	void ScriptCCharacterController::initRuntimeData()
	{
		metaData.scriptClass->addInternalCall("Internal_move", (void*)&ScriptCCharacterController::Internal_move);
		metaData.scriptClass->addInternalCall("Internal_getFootPosition", (void*)&ScriptCCharacterController::Internal_getFootPosition);
		metaData.scriptClass->addInternalCall("Internal_setFootPosition", (void*)&ScriptCCharacterController::Internal_setFootPosition);
		metaData.scriptClass->addInternalCall("Internal_getRadius", (void*)&ScriptCCharacterController::Internal_getRadius);
		metaData.scriptClass->addInternalCall("Internal_setRadius", (void*)&ScriptCCharacterController::Internal_setRadius);
		metaData.scriptClass->addInternalCall("Internal_getHeight", (void*)&ScriptCCharacterController::Internal_getHeight);
		metaData.scriptClass->addInternalCall("Internal_setHeight", (void*)&ScriptCCharacterController::Internal_setHeight);
		metaData.scriptClass->addInternalCall("Internal_getUp", (void*)&ScriptCCharacterController::Internal_getUp);
		metaData.scriptClass->addInternalCall("Internal_setUp", (void*)&ScriptCCharacterController::Internal_setUp);
		metaData.scriptClass->addInternalCall("Internal_getClimbingMode", (void*)&ScriptCCharacterController::Internal_getClimbingMode);
		metaData.scriptClass->addInternalCall("Internal_setClimbingMode", (void*)&ScriptCCharacterController::Internal_setClimbingMode);
		metaData.scriptClass->addInternalCall("Internal_getNonWalkableMode", (void*)&ScriptCCharacterController::Internal_getNonWalkableMode);
		metaData.scriptClass->addInternalCall("Internal_setNonWalkableMode", (void*)&ScriptCCharacterController::Internal_setNonWalkableMode);
		metaData.scriptClass->addInternalCall("Internal_getMinMoveDistance", (void*)&ScriptCCharacterController::Internal_getMinMoveDistance);
		metaData.scriptClass->addInternalCall("Internal_setMinMoveDistance", (void*)&ScriptCCharacterController::Internal_setMinMoveDistance);
		metaData.scriptClass->addInternalCall("Internal_getContactOffset", (void*)&ScriptCCharacterController::Internal_getContactOffset);
		metaData.scriptClass->addInternalCall("Internal_setContactOffset", (void*)&ScriptCCharacterController::Internal_setContactOffset);
		metaData.scriptClass->addInternalCall("Internal_getStepOffset", (void*)&ScriptCCharacterController::Internal_getStepOffset);
		metaData.scriptClass->addInternalCall("Internal_setStepOffset", (void*)&ScriptCCharacterController::Internal_setStepOffset);
		metaData.scriptClass->addInternalCall("Internal_getSlopeLimit", (void*)&ScriptCCharacterController::Internal_getSlopeLimit);
		metaData.scriptClass->addInternalCall("Internal_setSlopeLimit", (void*)&ScriptCCharacterController::Internal_setSlopeLimit);
		metaData.scriptClass->addInternalCall("Internal_getLayer", (void*)&ScriptCCharacterController::Internal_getLayer);
		metaData.scriptClass->addInternalCall("Internal_setLayer", (void*)&ScriptCCharacterController::Internal_setLayer);

		onColliderHitThunk = (onColliderHitThunkDef)metaData.scriptClass->getMethodExact("Internal_onColliderHit", "ControllerColliderCollision&")->getThunk();
		onControllerHitThunk = (onControllerHitThunkDef)metaData.scriptClass->getMethodExact("Internal_onControllerHit", "ControllerControllerCollision&")->getThunk();
	}

	void ScriptCCharacterController::onColliderHit(const ControllerColliderCollision& p0)
	{
		MonoObject* tmpp0;
		__ControllerColliderCollisionInterop interopp0;
		interopp0 = ScriptControllerColliderCollision::toInterop(p0);
		tmpp0 = ScriptControllerColliderCollision::box(interopp0);
		MonoUtil::invokeThunk(onColliderHitThunk, getManagedInstance(), tmpp0);
	}

	void ScriptCCharacterController::onControllerHit(const ControllerControllerCollision& p0)
	{
		MonoObject* tmpp0;
		__ControllerControllerCollisionInterop interopp0;
		interopp0 = ScriptControllerControllerCollision::toInterop(p0);
		tmpp0 = ScriptControllerControllerCollision::box(interopp0);
		MonoUtil::invokeThunk(onControllerHitThunk, getManagedInstance(), tmpp0);
	}
	CharacterCollisionFlag ScriptCCharacterController::Internal_move(ScriptCCharacterController* thisPtr, Vector3* displacement)
	{
		Flags<CharacterCollisionFlag> tmp__output;
		tmp__output = thisPtr->getHandle()->move(*displacement);

		CharacterCollisionFlag __output;
		__output = (CharacterCollisionFlag)(uint32_t)tmp__output;

		return __output;
	}

	void ScriptCCharacterController::Internal_getFootPosition(ScriptCCharacterController* thisPtr, Vector3* __output)
	{
		Vector3 tmp__output;
		tmp__output = thisPtr->getHandle()->getFootPosition();

		*__output = tmp__output;
	}

	void ScriptCCharacterController::Internal_setFootPosition(ScriptCCharacterController* thisPtr, Vector3* position)
	{
		thisPtr->getHandle()->setFootPosition(*position);
	}

	float ScriptCCharacterController::Internal_getRadius(ScriptCCharacterController* thisPtr)
	{
		float tmp__output;
		tmp__output = thisPtr->getHandle()->getRadius();

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptCCharacterController::Internal_setRadius(ScriptCCharacterController* thisPtr, float radius)
	{
		thisPtr->getHandle()->setRadius(radius);
	}

	float ScriptCCharacterController::Internal_getHeight(ScriptCCharacterController* thisPtr)
	{
		float tmp__output;
		tmp__output = thisPtr->getHandle()->getHeight();

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptCCharacterController::Internal_setHeight(ScriptCCharacterController* thisPtr, float height)
	{
		thisPtr->getHandle()->setHeight(height);
	}

	void ScriptCCharacterController::Internal_getUp(ScriptCCharacterController* thisPtr, Vector3* __output)
	{
		Vector3 tmp__output;
		tmp__output = thisPtr->getHandle()->getUp();

		*__output = tmp__output;
	}

	void ScriptCCharacterController::Internal_setUp(ScriptCCharacterController* thisPtr, Vector3* up)
	{
		thisPtr->getHandle()->setUp(*up);
	}

	CharacterClimbingMode ScriptCCharacterController::Internal_getClimbingMode(ScriptCCharacterController* thisPtr)
	{
		CharacterClimbingMode tmp__output;
		tmp__output = thisPtr->getHandle()->getClimbingMode();

		CharacterClimbingMode __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptCCharacterController::Internal_setClimbingMode(ScriptCCharacterController* thisPtr, CharacterClimbingMode mode)
	{
		thisPtr->getHandle()->setClimbingMode(mode);
	}

	CharacterNonWalkableMode ScriptCCharacterController::Internal_getNonWalkableMode(ScriptCCharacterController* thisPtr)
	{
		CharacterNonWalkableMode tmp__output;
		tmp__output = thisPtr->getHandle()->getNonWalkableMode();

		CharacterNonWalkableMode __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptCCharacterController::Internal_setNonWalkableMode(ScriptCCharacterController* thisPtr, CharacterNonWalkableMode mode)
	{
		thisPtr->getHandle()->setNonWalkableMode(mode);
	}

	float ScriptCCharacterController::Internal_getMinMoveDistance(ScriptCCharacterController* thisPtr)
	{
		float tmp__output;
		tmp__output = thisPtr->getHandle()->getMinMoveDistance();

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptCCharacterController::Internal_setMinMoveDistance(ScriptCCharacterController* thisPtr, float value)
	{
		thisPtr->getHandle()->setMinMoveDistance(value);
	}

	float ScriptCCharacterController::Internal_getContactOffset(ScriptCCharacterController* thisPtr)
	{
		float tmp__output;
		tmp__output = thisPtr->getHandle()->getContactOffset();

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptCCharacterController::Internal_setContactOffset(ScriptCCharacterController* thisPtr, float value)
	{
		thisPtr->getHandle()->setContactOffset(value);
	}

	float ScriptCCharacterController::Internal_getStepOffset(ScriptCCharacterController* thisPtr)
	{
		float tmp__output;
		tmp__output = thisPtr->getHandle()->getStepOffset();

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptCCharacterController::Internal_setStepOffset(ScriptCCharacterController* thisPtr, float value)
	{
		thisPtr->getHandle()->setStepOffset(value);
	}

	void ScriptCCharacterController::Internal_getSlopeLimit(ScriptCCharacterController* thisPtr, Radian* __output)
	{
		Radian tmp__output;
		tmp__output = thisPtr->getHandle()->getSlopeLimit();

		*__output = tmp__output;
	}

	void ScriptCCharacterController::Internal_setSlopeLimit(ScriptCCharacterController* thisPtr, Radian* value)
	{
		thisPtr->getHandle()->setSlopeLimit(*value);
	}

	uint64_t ScriptCCharacterController::Internal_getLayer(ScriptCCharacterController* thisPtr)
	{
		uint64_t tmp__output;
		tmp__output = thisPtr->getHandle()->getLayer();

		uint64_t __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptCCharacterController::Internal_setLayer(ScriptCCharacterController* thisPtr, uint64_t layer)
	{
		thisPtr->getHandle()->setLayer(layer);
	}
}
