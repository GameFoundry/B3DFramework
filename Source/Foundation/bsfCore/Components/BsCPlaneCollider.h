//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsCorePrerequisites.h"
#include "Physics/BsPlaneCollider.h"
#include "Components/BsCCollider.h"

namespace bs
{
	/** @addtogroup Components-Core
	 *  @{
	 */

	/**
	 * @copydoc	PlaneCollider
	 *
	 * @note Wraps PlaneCollider as a Component.
	 */
	class BS_CORE_EXPORT BS_SCRIPT_EXPORT(DocumentationGroup(Physics), ExportName(PlaneCollider)) CPlaneCollider : public CCollider
	{
	public:
		CPlaneCollider(const HSceneObject& parent);

		/** Normal vector that determines the local orientation of the plane. */
		BS_SCRIPT_EXPORT(ExportName(Normal), Property(Setter))
		void SetNormal(const Vector3& normal);

		/** @copydoc SetNormal() */
		BS_SCRIPT_EXPORT(ExportName(Normal), Property(Getter))
		Vector3 GetNormal() const { return mNormal; }

		/** Determines the distance of the plane from the local origin, along its normal vector. */
		BS_SCRIPT_EXPORT(ExportName(Distance), Property(Setter))
		void SetDistance(float distance);

		/** @copydoc SetDistance() */
		BS_SCRIPT_EXPORT(ExportName(Distance), Property(Getter))
		float GetDistance() const { return mDistance; }

		/** @name Internal
		 *  @{
		 */

		/**	Returns the plane collider that this component wraps. */
		PlaneCollider* GetInternalInternal() const { return static_cast<PlaneCollider*>(mInternal.get()); }

		/** @} */

		/************************************************************************/
		/* 						COMPONENT OVERRIDES                      		*/
		/************************************************************************/
	protected:
		friend class SceneObject;

		SPtr<Collider> CreateInternal() override;
		bool IsValidParent(const HRigidbody& parent) const override;

	protected:
		Vector3 mNormal = Vector3::kUnitY;
		float mDistance = 0.0f;

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/
	public:
		friend class CPlaneColliderRTTI;
		static RTTITypeBase* GetRttiStatic();
		RTTITypeBase* GetRtti() const override;

	protected:
		CPlaneCollider(); // Serialization only
	};

	/** @} */
} // namespace bs
