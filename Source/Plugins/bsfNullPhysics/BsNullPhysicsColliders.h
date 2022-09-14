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
		Vector3 getPosition() const override { return mPosition; }

		/** @copydoc FCollider::getRotation */
		Quaternion getRotation() const override { return mRotation; }

		/** @copydoc FCollider::setTransform */
		void setTransform(const Vector3& pos, const Quaternion& rotation) override;

		/** @copydoc FCollider::setIsTrigger */
		void setIsTrigger(bool value) override { mIsTrigger = value; }

		/** @copydoc FCollider::getIsTrigger */
		bool getIsTrigger() const override { return mIsTrigger; }

		/** @copydoc FCollider::setIsStatic */
		void setIsStatic(bool value) override { mIsStatic = value; }

		/** @copydoc FCollider::getIsStatic */
		bool getIsStatic() const override { return mIsStatic; }

		/** @copydoc FCollider::setContactOffset */
		void setContactOffset(float value) override { mContactOffset = value; }

		/** @copydoc FCollider::getContactOffset */
		float getContactOffset() const override { return mContactOffset; }

		/** @copydoc FCollider::setRestOffset */
		void setRestOffset(float value) override { mRestOffset = value; }

		/** @copydoc FCollider::getRestOffset */
		float getRestOffset() const override { return mRestOffset; }

		/** @copydoc FCollider::getLayer */
		UINT64 getLayer() const override { return mLayer; }

		/** @copydoc FCollider::setLayer */
		void setLayer(UINT64 layer) override { mLayer = layer; }

		/** @copydoc FCollider::getCollisionReportMode */
		CollisionReportMode getCollisionReportMode() const override { return mCollisionReportMode; }

		/** @copydoc FCollider::setCollisionReportMode */
		void setCollisionReportMode(CollisionReportMode mode) override { mCollisionReportMode = mode; }

		/** @copydoc FCollider::_setCCD */
		void SetCCDInternal(bool enabled) override { }

	protected:
		Vector3 mPosition;
		Quaternion mRotation;
		bool mIsTrigger = false;
		bool mIsStatic = true;
		UINT64 mLayer = 1;
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
		void setExtents(const Vector3& extents) override { mExtents = extents; }

		/** @copydoc BoxCollider::getExtents */
		Vector3 getExtents() const override { return mExtents; }

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
		void setHalfHeight(float halfHeight) override { mHalfHeight = halfHeight; }

		/** @copydoc CapsuleCollider::getHalfHeight() */
		float getHalfHeight() const override { return mHalfHeight; }

		/** @copydoc CapsuleCollider::setRadius() */
		void setRadius(float radius) override { mRadius = radius; }

		/** @copydoc CapsuleCollider::getRadius() */
		float getRadius() const override { return mRadius; }

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
		void setRadius(float radius) override { mRadius = radius; }

		/** @copydoc SphereCollider::getRadius */
		float getRadius() const override { return mRadius; }

	private:
		float mRadius;
	};

	/** @} */
}
