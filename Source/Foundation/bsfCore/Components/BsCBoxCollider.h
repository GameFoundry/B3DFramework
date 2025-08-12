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
	 * @copydoc	BoxCollider
	 *
	 * @note Wraps BoxCollider as a Component.
	 */
	class B3D_CORE_EXPORT B3D_SCRIPT_EXPORT(DocumentationGroup(Physics), ExportName(BoxCollider)) CBoxCollider : public CCollider
	{
	public:
		CBoxCollider(const HSceneObject& parent, const Vector3& extents = Vector3(0.5f, 0.5f, 0.5f));

		/** @copydoc BoxCollider::SetExtents */
		B3D_SCRIPT_EXPORT(ExportName(Extents), Property(Setter))
		void SetExtents(const Vector3& extents);

		/** @copydoc BoxCollider::GetExtents */
		B3D_SCRIPT_EXPORT(ExportName(Extents), Property(Getter))
		Vector3 GetExtents() const { return mExtents; }

		/** Determines the position of the box shape, relative to the component's scene object. */
		B3D_SCRIPT_EXPORT(ExportName(Center), Property(Setter))
		void SetCenter(const Vector3& center);

		/** @copydoc SetCenter() */
		B3D_SCRIPT_EXPORT(ExportName(Center), Property(Getter))
		Vector3 GetCenter() const { return mShapeLocalPosition; }

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
		static RTTIType* GetRttiStatic();
		RTTIType* GetRtti() const override;

	protected:
		CBoxCollider(); // Serialization only
	};

	/** @} */
} // namespace b3d
