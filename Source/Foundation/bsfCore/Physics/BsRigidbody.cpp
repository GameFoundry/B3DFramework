//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Physics/BsRigidbody.h"
#include "Physics/BsPhysics.h"
#include "Scene/BsSceneObject.h"
#include "Scene/BsSceneInstance.h"

using namespace b3d;

Rigidbody::Rigidbody(const HSceneObject& linkedSO)
	: mLinkedSO(linkedSO)
{
}

void Rigidbody::SetTransformInternal(const Vector3& position, const Quaternion& rotation)
{
	mLinkedSO->SetWorldPosition(position);
	mLinkedSO->SetWorldRotation(rotation);
}

SPtr<Rigidbody> Rigidbody::Create(const HSceneObject& linkedSO)
{
	const SPtr<SceneInstance>& scene = linkedSO->GetScene();

	if(!scene)
	{
		B3D_LOG(Error, Physics, "Trying to create a Rigidbody with an uninstantiated scene object.");
		return nullptr;
	}

	return scene->GetPhysicsScene()->CreateRigidbody(linkedSO);
}
