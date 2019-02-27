//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Physics/BsRigidbody.h"
#include "Physics/BsPhysics.h"
#include "Scene/BsSceneObject.h"
#include "Scene/BsSceneManager.h"

namespace bs
{
	Rigidbody::Rigidbody(const HSceneObject& linkedSO)
		:mLinkedSO(linkedSO)
	{

	}

	void Rigidbody::_setTransform(const Vector3& position, const Quaternion& rotation)
	{
		mLinkedSO->setWorldPosition(position);
		mLinkedSO->setWorldRotation(rotation);
	}

	SPtr<Rigidbody> Rigidbody::create(const HSceneObject& linkedSO)
	{
		const SPtr<SceneInstance>& scene = linkedSO->getScene();

		if(!scene)
		{
			LOGERR("Trying to create a Rigidbody with an uninstantiated scene object.")
			return nullptr;
		}

		return scene->getPhysicsScene()->createRigidbody(linkedSO);
	}
}