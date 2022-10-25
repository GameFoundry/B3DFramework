//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsNullPhysicsPrerequisites.h"
#include "Physics/BsPhysicsCommon.h"
#include "Physics/BsFCollider.h"
#include "Physics/BsBoxCollider.h"
#include "Physics/BsCapsuleCollider.h"
#include "Physics/BsMeshCollider.h"
#include "Physics/BsPlaneCollider.h"
#include "Physics/BsSphereCollider.h"

namespace bs
{
	/** @addtogroup NullPhysics
	 *  @{
	 */

	/** Null implementation of FCollider. */
	class FNullPhysicsCollider : public FCollider
	{
	public:
		explicit FNullPhysicsCollider(const Vector3& position, const Quaternion& rotation);
		~FNullPhysicsCollider() = default;

		/** @copydoc FCollider::getPosition */
		Vector3 GetPosition() const override { return mPosition; }

		/** @copydoc FCollider::getRotation */
		Quaternion GetRotation() const override { return mRotation; }

		/** @copydoc FCollider::setTransform */
		void SetTransform(const Vector3& pos, const Quaternion& rotation) override;

		/** @copydoc FCollider::setIsTrigger */
		void SetIsTrigger(bool value) override { mIsTrigger = value; }

		/** @copydoc FCollider::getIsTrigger */
		bool GetIsTrigger() const override { return mIsTrigger; }

		/** @copydoc FCollider::setIsStatic */
		void SetIsStatic(bool value) override { mIsStatic = value; }

		/** @copydoc FCollider::getIsStatic */
		bool GetIsStatic() const override { return mIsStatic; }

		/** @copydoc FCollider::setContactOffset */
		void SetContactOffset(float value) override { mContactOffset = value; }

		/** @copydoc FCollider::getContactOffset */
		float GetContactOffset() const override { return mContactOffset; }

		/** @copydoc FCollider::setRestOffset */
		void SetRestOffset(float value) override { mRestOffset = value; }

		/** @copydoc FCollider::getRestOffset */
		float GetRestOffset() const override { return mRestOffset; }

		/** @copydoc FCollider::getLayer */
		u64 GetLayer() const override { return mLayer; }

		/** @copydoc FCollider::setLayer */
		void SetLayer(u64 layer) override { mLayer = layer; }

		/** @copydoc FCollider::getCollisionReportMode */
		CollisionReportMode GetCollisionReportMode() const override { return mCollisionReportMode; }

		/** @copydoc FCollider::setCollisionReportMode */
		void SetCollisionReportMode(CollisionReportMode mode) override { mCollisionReportMode = mode; }

		/** @copydoc FCollider::_setCCD */
		void SetCCDInternal(bool enabled) override {}

	protected:
		Vector3 mPosition;
		Quaternion mRotation;
		bool mIsTrigger = false;
		bool mIsStatic = true;
		u64 mLayer = 1;
		bool mCCD = false;
		float mContactOffset = 0.005f;
		float mRestOffset = 0.005f;
		CollisionReportMode mCollisionReportMode = CollisionReportMode::None;
	};

	/** Null implementation of a BoxCollider. */
	class NullPhysicsBoxCollider : public BoxCollider
	{
	public:
		NullPhysicsBoxCollider(const Vector3& position, const Quaternion& rotation, const Vector3& extents);
		~NullPhysicsBoxCollider();

		/** @copydoc BoxCollider::setExtents */
		void SetExtents(const Vector3& extents) override { mExtents = extents; }

		/** @copydoc BoxCollider::getExtents */
		Vector3 GetExtents() const override { return mExtents; }

	private:
		Vector3 mExtents;
	};

	/** Null implementation of a CapsuleCollider. */
	class NullPhysicsCapsuleCollider : public CapsuleCollider
	{
	public:
		NullPhysicsCapsuleCollider(const Vector3& position, const Quaternion& rotation, float radius, float halfHeight);
		~NullPhysicsCapsuleCollider();

		/** @copydoc CapsuleCollider::setHalfHeight() */
		void SetHalfHeight(float halfHeight) override { mHalfHeight = halfHeight; }

		/** @copydoc CapsuleCollider::getHalfHeight() */
		float GetHalfHeight() const override { return mHalfHeight; }

		/** @copydoc CapsuleCollider::setRadius() */
		void SetRadius(float radius) override { mRadius = radius; }

		/** @copydoc CapsuleCollider::getRadius() */
		float GetRadius() const override { return mRadius; }

	private:
		float mRadius;
		float mHalfHeight;
	};

	/** Null implementation of a MeshCollider. */
	class NullPhysicsMeshCollider : public MeshCollider
	{
	public:
		NullPhysicsMeshCollider(const Vector3& position, const Quaternion& rotation);
		~NullPhysicsMeshCollider();
	};

	/** Null implementation of the PlaneCollider. */
	class NullPhysicsPlaneCollider : public PlaneCollider
	{
	public:
		NullPhysicsPlaneCollider(const Vector3& position, const Quaternion& rotation);
		~NullPhysicsPlaneCollider();
	};

	/** Null implementation of a SphereCollider. */
	class NullPhysicsSphereCollider : public SphereCollider
	{
	public:
		NullPhysicsSphereCollider(const Vector3& position, const Quaternion& rotation, float radius);
		~NullPhysicsSphereCollider();

		/** @copydoc SphereCollider::setRadius */
		void SetRadius(float radius) override { mRadius = radius; }

		/** @copydoc SphereCollider::getRadius */
		float GetRadius() const override { return mRadius; }

	private:
		float mRadius;
	};

	/** @} */
} // namespace bs
