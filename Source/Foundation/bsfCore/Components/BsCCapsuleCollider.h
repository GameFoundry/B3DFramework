//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsCorePrerequisites.h"
#include "Components/BsCCollider.h"

namespace b3d
{
	/** @addtogroup Components-Core
	 *  @{
	 */

	/**
	 * @copydoc	CapsuleCollider
	 *
	 * @note Wraps CapsuleCollider as a Component.
	 */
	class B3D_CORE_EXPORT B3D_SCRIPT_EXPORT(DocumentationGroup(Physics), ExportName(CapsuleCollider)) CCapsuleCollider : public CCollider
	{
	public:
		CCapsuleCollider(const HSceneObject& parent, float radius = 1.0f, float halfHeight = 0.5f);

		/** Normal vector that determines how is the capsule oriented. */
		B3D_SCRIPT_EXPORT(ExportName(Normal), Property(Setter))
		void SetNormal(const Vector3& normal);

		/** @copydoc SetNormal() */
		B3D_SCRIPT_EXPORT(ExportName(Normal), Property(Getter))
		Vector3 GetNormal() const { return mNormal; }

		/** Determines the position of the capsule shape, relative to the component's scene object. */
		B3D_SCRIPT_EXPORT(ExportName(Center), Property(Setter))
		void SetCenter(const Vector3& center);

		/** @copydoc SetCenter() */
		B3D_SCRIPT_EXPORT(ExportName(Center), Property(Getter))
		Vector3 GetCenter() const { return mShapeLocalPosition; }

		/** @copydoc CapsuleCollider::SetHalfHeight() */
		B3D_SCRIPT_EXPORT(ExportName(HalfHeight), Property(Setter))
		void SetHalfHeight(float halfHeight);

		/** @copydoc CapsuleCollider::GetHalfHeight() */
		B3D_SCRIPT_EXPORT(ExportName(HalfHeight), Property(Getter))
		float GetHalfHeight() const { return mHalfHeight; }

		/** @copydoc CapsuleCollider::SetRadius() */
		B3D_SCRIPT_EXPORT(ExportName(Radius), Property(Setter))
		void SetRadius(float radius);

		/** @copydoc CapsuleCollider::GetRadius() */
		B3D_SCRIPT_EXPORT(ExportName(Radius), Property(Getter))
		float GetRadius() const { return mRadius; }

		/************************************************************************/
		/* 						COMPONENT OVERRIDES                      		*/
		/************************************************************************/
	protected:
		friend class SceneObject;

		SPtr<Collider> CreateInternal() override;

	protected:
		Vector3 mNormal = Vector3::kUnitY;
		float mRadius = 1.0f;
		float mHalfHeight = 0.5f;

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/
	public:
		friend class CCapsuleColliderRTTI;
		static RTTIType* GetRttiStatic();
		RTTIType* GetRtti() const;

	protected:
		CCapsuleCollider(); // Serialization only
	};

	/** @} */
} // namespace b3d
