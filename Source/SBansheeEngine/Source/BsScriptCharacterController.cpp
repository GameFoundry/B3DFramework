//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsScriptCharacterController.h"
#include "BsMonoUtil.h"
#include "BsMonoClass.h"
#include "BsMonoMethod.h"

using namespace std::placeholders;

namespace bs
{
	ScriptCharacterController::OnHitThunkDef ScriptCharacterController::onColliderHitThunk = nullptr;
	ScriptCharacterController::OnHitThunkDef ScriptCharacterController::onControllerHitThunk = nullptr;

	ScriptCharacterController::ScriptCharacterController(MonoObject* instance, const SPtr<CharacterController>& charController)
		:ScriptObject(instance), mCharacterController(charController)
	{
		charController->onColliderHit.connect(std::bind(&ScriptCharacterController::onColliderHit, instance, _1));
		charController->onControllerHit.connect(std::bind(&ScriptCharacterController::onControllerHit, instance, _1));
	}

	void ScriptCharacterController::initRuntimeData()
	{
		metaData.scriptClass->addInternalCall("Internal_CreateInstance", &ScriptCharacterController::internal_CreateInstance);
		metaData.scriptClass->addInternalCall("Internal_Destroy", &ScriptCharacterController::internal_Destroy);
		metaData.scriptClass->addInternalCall("Internal_Move", &ScriptCharacterController::internal_Move);
		metaData.scriptClass->addInternalCall("Internal_GetPosition", &ScriptCharacterController::internal_GetPosition);
		metaData.scriptClass->addInternalCall("Internal_SetPosition", &ScriptCharacterController::internal_SetPosition);
		metaData.scriptClass->addInternalCall("Internal_GetFootPosition", &ScriptCharacterController::internal_GetFootPosition);
		metaData.scriptClass->addInternalCall("Internal_SetFootPosition", &ScriptCharacterController::internal_SetFootPosition);
		metaData.scriptClass->addInternalCall("Internal_SetRadius", &ScriptCharacterController::internal_SetRadius);
		metaData.scriptClass->addInternalCall("Internal_SetHeight", &ScriptCharacterController::internal_SetHeight);
		metaData.scriptClass->addInternalCall("Internal_SetUp", &ScriptCharacterController::internal_SetUp);
		metaData.scriptClass->addInternalCall("Internal_SetClimbingMode", &ScriptCharacterController::internal_SetClimbingMode);
		metaData.scriptClass->addInternalCall("Internal_SetNonWalkableMode", &ScriptCharacterController::internal_SetNonWalkableMode);
		metaData.scriptClass->addInternalCall("Internal_SetMinMoveDistance", &ScriptCharacterController::internal_SetMinMoveDistance);
		metaData.scriptClass->addInternalCall("Internal_SetContactOffset", &ScriptCharacterController::internal_SetContactOffset);
		metaData.scriptClass->addInternalCall("Internal_SetStepOffset", &ScriptCharacterController::internal_SetStepOffset);
		metaData.scriptClass->addInternalCall("Internal_SetSlopeLimit", &ScriptCharacterController::internal_SetSlopeLimit);
		metaData.scriptClass->addInternalCall("Internal_SetLayer", &ScriptCharacterController::internal_SetLayer);

		onColliderHitThunk = (OnHitThunkDef)metaData.scriptClass->getMethod("Internal_DoOnColliderHit", 1)->getThunk();
		onControllerHitThunk = (OnHitThunkDef)metaData.scriptClass->getMethod("Internal_DoOnControllerHit", 1)->getThunk();
	}

	void ScriptCharacterController::onColliderHit(MonoObject* instance, const ControllerColliderCollision& collisionData)
	{
		ScriptControllerCollision scriptCollisionData = ScriptControllerCollisionHelper::create(collisionData);
		MonoObject* managedData = ScriptControllerCollisionHelper::box(scriptCollisionData);

		MonoUtil::invokeThunk(onColliderHitThunk, instance, managedData);
	}

	void ScriptCharacterController::onControllerHit(MonoObject* instance, const ControllerControllerCollision& collisionData)
	{
		ScriptControllerCollision scriptCollisionData = ScriptControllerCollisionHelper::create(collisionData);
		MonoObject* managedData = ScriptControllerCollisionHelper::box(scriptCollisionData);

		MonoUtil::invokeThunk(onControllerHitThunk, instance, managedData);
	}

	void ScriptCharacterController::internal_CreateInstance(MonoObject* instance, CHAR_CONTROLLER_DESC* initData)
	{
		SPtr<CharacterController> charController = CharacterController::create(*initData);
		charController->_setOwner(PhysicsOwnerType::Script, instance);

		new (bs_alloc<ScriptCharacterController>()) ScriptCharacterController(instance, charController);
	}

	void ScriptCharacterController::internal_Destroy(ScriptCharacterController* thisPtr)
	{
		thisPtr->mCharacterController = nullptr;
	}

	UINT32 ScriptCharacterController::internal_Move(ScriptCharacterController* thisPtr, Vector3* displacement)
	{
		return thisPtr->mCharacterController->move(*displacement);
	}

	void ScriptCharacterController::internal_GetPosition(ScriptCharacterController* thisPtr, Vector3* position)
	{
		*position = thisPtr->mCharacterController->getPosition();
	}

	void ScriptCharacterController::internal_SetPosition(ScriptCharacterController* thisPtr, Vector3* position)
	{
		thisPtr->mCharacterController->setPosition(*position);
	}

	void ScriptCharacterController::internal_GetFootPosition(ScriptCharacterController* thisPtr, Vector3* position)
	{
		*position = thisPtr->mCharacterController->getFootPosition();
	}

	void ScriptCharacterController::internal_SetFootPosition(ScriptCharacterController* thisPtr, Vector3* position)
	{
		thisPtr->mCharacterController->setFootPosition(*position);
	}

	void ScriptCharacterController::internal_SetRadius(ScriptCharacterController* thisPtr, float radius)
	{
		thisPtr->mCharacterController->setRadius(radius);
	}

	void ScriptCharacterController::internal_SetHeight(ScriptCharacterController* thisPtr, float height)
	{
		thisPtr->mCharacterController->setHeight(height);
	}

	void ScriptCharacterController::internal_SetUp(ScriptCharacterController* thisPtr, Vector3* up)
	{
		thisPtr->mCharacterController->setUp(*up);
	}

	void ScriptCharacterController::internal_SetClimbingMode(ScriptCharacterController* thisPtr, CharacterClimbingMode mode)
	{
		thisPtr->mCharacterController->setClimbingMode(mode);
	}

	void ScriptCharacterController::internal_SetNonWalkableMode(ScriptCharacterController* thisPtr, CharacterNonWalkableMode mode)
	{
		thisPtr->mCharacterController->setNonWalkableMode(mode);
	}

	void ScriptCharacterController::internal_SetMinMoveDistance(ScriptCharacterController* thisPtr, float value)
	{
		thisPtr->mCharacterController->setMinMoveDistance(value);
	}

	void ScriptCharacterController::internal_SetContactOffset(ScriptCharacterController* thisPtr, float value)
	{
		thisPtr->mCharacterController->setContactOffset(value);
	}

	void ScriptCharacterController::internal_SetStepOffset(ScriptCharacterController* thisPtr, float value)
	{
		thisPtr->mCharacterController->setStepOffset(value);
	}

	void ScriptCharacterController::internal_SetSlopeLimit(ScriptCharacterController* thisPtr, float value)
	{
		thisPtr->mCharacterController->setSlopeLimit(Radian(value));
	}

	void ScriptCharacterController::internal_SetLayer(ScriptCharacterController* thisPtr, UINT64 layer)
	{
		thisPtr->mCharacterController->setLayer(layer);
	}
}