//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DNullPhysicsPrerequisites.h"
#include "Physics/B3DPhysicsCommon.h"
#include "Physics/B3DFCollider.h"
#include "Physics/B3DBoxCollider.h"
#include "Physics/B3DCapsuleCollider.h"
#include "Physics/B3DMeshCollider.h"
#include "Physics/B3DPlaneCollider.h"
#include "Physics/B3DSphereCollider.h"

namespace b3d
{
	/** @addtogroup NullPhysics
	 *  @{
	 */

	/** Null implementation of FCollider. */
	class FNullPhysicsCollider : public FCollider
	{
	public:
		explicit FNullPhysicsCollider(const Vector3& position, const Quaternion& rotation);
		~FNullPhysicsCollider() override = default;

		Vector3 GetPosition() const override { return mPosition; }
		Quaternion GetRotation() const override { return mRotation; }
		void SetTransform(const Vector3& pos, const Quaternion& rotation) override;
		void SetIsTrigger(bool value) override { mIsTrigger = value; }
		bool GetIsTrigger() const override { return mIsTrigger; }
		void SetIsStatic(bool value) override { mIsStatic = value; }
		bool GetIsStatic() const override { return mIsStatic; }
		void SetContactOffset(float value) override { mContactOffset = value; }
		float GetContactOffset() const override { return mContactOffset; }
		void SetRestOffset(float value) override { mRestOffset = value; }
		float GetRestOffset() const override { return mRestOffset; }
		u64 GetLayer() const override { return mLayer; }
		void SetLayer(u64 layer) override { mLayer = layer; }
		CollisionReportMode GetCollisionReportMode() const override { return mCollisionReportMode; }
		void SetCollisionReportMode(CollisionReportMode mode) override { mCollisionReportMode = mode; }
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
		~NullPhysicsBoxCollider() override;

		void SetExtents(const Vector3& extents) override { mExtents = extents; }
		Vector3 GetExtents() const override { return mExtents; }

	private:
		Vector3 mExtents;
	};

	/** Null implementation of a CapsuleCollider. */
	class NullPhysicsCapsuleCollider : public CapsuleCollider
	{
	public:
		NullPhysicsCapsuleCollider(const Vector3& position, const Quaternion& rotation, float radius, float halfHeight);
		~NullPhysicsCapsuleCollider() override;

		void SetHalfHeight(float halfHeight) override { mHalfHeight = halfHeight; }
		float GetHalfHeight() const override { return mHalfHeight; }
		void SetRadius(float radius) override { mRadius = radius; }
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
		~NullPhysicsMeshCollider() override;
	};

	/** Null implementation of the PlaneCollider. */
	class NullPhysicsPlaneCollider : public PlaneCollider
	{
	public:
		NullPhysicsPlaneCollider(const Vector3& position, const Quaternion& rotation);
		~NullPhysicsPlaneCollider() override;
	};

	/** Null implementation of a SphereCollider. */
	class NullPhysicsSphereCollider : public SphereCollider
	{
	public:
		NullPhysicsSphereCollider(const Vector3& position, const Quaternion& rotation, float radius);
		~NullPhysicsSphereCollider() override;

		void SetRadius(float radius) override { mRadius = radius; }
		float GetRadius() const override { return mRadius; }

	private:
		float mRadius;
	};

	/** @} */
} // namespace b3d
