//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsCorePrerequisites.h"
#include "Physics/BsBoxCollider.h"
#include "Components/BsCCollider.h"

namespace bs
{
	/** @addtogroup Components-Core
	 *  @{
	 */

	/**
	 * @copydoc	BoxCollider
	 *
	 * @note Wraps BoxCollider as a Component.
	 */
	class BS_CORE_EXPORT BS_SCRIPT_EXPORT(DocumentationGroup(Physics), ExportName(BoxCollider)) CBoxCollider : public CCollider
	{
	public:
		CBoxCollider(const HSceneObject& parent, const Vector3& extents = Vector3(0.5f, 0.5f, 0.5f));

		/** @copydoc BoxCollider::SetExtents */
		BS_SCRIPT_EXPORT(ExportName(Extents), Property(Setter))
		void SetExtents(const Vector3& extents);

		/** @copydoc BoxCollider::GetExtents */
		BS_SCRIPT_EXPORT(ExportName(Extents), Property(Getter))
		Vector3 GetExtents() const { return mExtents; }

		/** Determines the position of the box shape, relative to the component's scene object. */
		BS_SCRIPT_EXPORT(ExportName(Center), Property(Setter))
		void SetCenter(const Vector3& center);

		/** @copydoc SetCenter() */
		BS_SCRIPT_EXPORT(ExportName(Center), Property(Getter))
		Vector3 GetCenter() const { return mLocalPosition; }

		/** @name Internal
		 *  @{
		 */

		/**	Returns the box collider that this component wraps. */
		BoxCollider* GetInternalInternal() const { return static_cast<BoxCollider*>(mInternal.get()); }

		/** @} */

		/************************************************************************/
		/* 						COMPONENT OVERRIDES                      		*/
		/************************************************************************/
	protected:
		friend class SceneObject;

		SPtr<Collider> CreateInternal() override;

	protected:
		Vector3 mExtents = Vector3(0.5f, 0.5f, 0.5f);

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/
	public:
		friend class CBoxColliderRTTI;
		static RTTITypeBase* GetRttiStatic();
		RTTITypeBase* GetRtti() const override;

	protected:
		CBoxCollider(); // Serialization only
	};

	/** @} */
} // namespace bs
