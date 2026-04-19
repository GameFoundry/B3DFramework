//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "Components/B3DCollider.h"

namespace b3d
{
	/** @addtogroup Physics
	 *  @{
	 */

	/** A collider with plane geometry. Plane colliders cannot be a part of non-kinematic rigidbodies. */
	class B3D_EXPORT B3D_SCRIPT_EXPORT(DocumentationGroup(Physics)) PlaneCollider : public Collider
	{
	public:
		PlaneCollider(const HSceneObject& parent);

		/** Normal vector that determines the local orientation of the plane. */
		B3D_SCRIPT_EXPORT(ExportName(Normal), Property(Setter))
		void SetNormal(const Vector3& normal);

		/** @copydoc SetNormal() */
		B3D_SCRIPT_EXPORT(ExportName(Normal), Property(Getter))
		Vector3 GetNormal() const { return mNormal; }

		/** Determines the distance of the plane from the local origin, along its normal vector. */
		B3D_SCRIPT_EXPORT(ExportName(Distance), Property(Setter))
		void SetDistance(float distance);

		/** @copydoc SetDistance() */
		B3D_SCRIPT_EXPORT(ExportName(Distance), Property(Getter))
		float GetDistance() const { return mDistance; }

		/************************************************************************/
		/* 						COMPONENT OVERRIDES                      		*/
		/************************************************************************/
	protected:
		friend class SceneObject;

		void OnCreated() override;
		bool IsValidParent(const HRigidbody& parent) const override;

	protected:
		Vector3 mShapeLocalPosition = Vector3::kZero;
		Quaternion mShapeLocalRotation = Quaternion::kIdentity;
		Vector3 mNormal = Vector3::kUnitY;
		float mDistance = 0.0f;

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/
	public:
		friend class PlaneColliderRTTI;
		static RTTIType* GetRttiStatic();
		RTTIType* GetRtti() const override;

	protected:
		PlaneCollider(); // Serialization only
	};

	/** @} */
} // namespace b3d
