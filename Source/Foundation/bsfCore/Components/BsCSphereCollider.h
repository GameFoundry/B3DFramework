//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsCorePrerequisites.h"
#include "Physics/BsSphereCollider.h"
#include "Components/BsCCollider.h"

namespace bs
{
	/** @addtogroup Components-Core
	 *  @{
	 */

	/**
	 * @copydoc	SphereCollider
	 *
	 * @note Wraps SphereCollider as a Component.
	 */
	class BS_CORE_EXPORT BS_SCRIPT_EXPORT(DocumentationGroup(Physics), ExportName(SphereCollider)) CSphereCollider : public CCollider
	{
	public:
		CSphereCollider(const HSceneObject& parent, float radius = 1.0f);

		/** @copydoc SphereCollider::SetRadius */
		BS_SCRIPT_EXPORT(ExportName(Radius), Property(Setter))
		void SetRadius(float radius);

		/** @copydoc SphereCollider::GetRadius */
		BS_SCRIPT_EXPORT(ExportName(Radius), Property(Getter))

		float GetRadius() const { return mRadius; }

		/** Determines position of the sphere shape, relative to the component's scene object. */
		BS_SCRIPT_EXPORT(ExportName(Center), Property(Setter))
		void SetCenter(const Vector3& center);

		/** @copydoc SetCenter() */
		BS_SCRIPT_EXPORT(ExportName(Center), Property(Getter))

		Vector3 GetCenter() const { return mLocalPosition; }

		/** @name Internal
		 *  @{
		 */

		/**	Returns the sphere collider that this component wraps. */
		SphereCollider* GetInternalInternal() const { return static_cast<SphereCollider*>(mInternal.get()); }

		/** @} */

		/************************************************************************/
		/* 						COMPONENT OVERRIDES                      		*/
		/************************************************************************/
	protected:
		friend class SceneObject;

		SPtr<Collider> CreateInternal() override;

	protected:
		float mRadius = 1.0f;

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/
	public:
		friend class CSphereColliderRTTI;
		static RTTITypeBase* GetRttiStatic();
		RTTITypeBase* GetRtti() const override;

	protected:
		CSphereCollider(); // Serialization only
	};

	/** @} */
} // namespace bs
