//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DNullPhysicsPrerequisites.h"
#include "Components/B3DCollider.h"
#include "Physics/B3DColliderShape.h"

namespace b3d
{
	/** @addtogroup NullPhysics
	 *  @{
	 */

	/** Null implementation of ColliderShape. */
	class NullPhysicsColliderShape : public ColliderShape
	{
	public:
		ColliderShapeType GetType() const override { return ColliderShapeType::Box; }
		void SetShape(const PlaneColliderShapeInformation& information) override {}
		void SetShape(const BoxColliderShapeInformation& information) override {}
		void SetShape(const SphereColliderShapeInformation& information) override {}
		void SetShape(const CapsuleColliderShapeInformation& information) override {}
		void SetShape(const MeshColliderShapeInformation& information) override {}

	protected:
		void UpdateTransform() override {}
	};

	/** Null implementation of IColliderImplementation. */
	class NullPhysicsCollider : public IColliderImplementation
	{
	public:
		NullPhysicsCollider() = default;
		~NullPhysicsCollider() override = default;

		void AddToScene(PhysicsScene& scene) override {}
		void RemoveFromScene() override {}
		void AttachShape(const SPtr<ColliderShape>& shape) override {}
		void DetachShape(const SPtr<ColliderShape>& shape) override {}
		void SetTransform(const Vector3& position, const Quaternion& rotation) override {}
	};

	/** @} */
} // namespace b3d
