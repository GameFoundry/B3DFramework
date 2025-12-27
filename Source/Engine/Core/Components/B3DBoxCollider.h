//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "Components/B3DCollider.h"

namespace b3d
{
	/** @addtogroup Components
	 *  @{
	 */

	/** Collider with box geometry. */
	class B3D_EXPORT B3D_SCRIPT_EXPORT(DocumentationGroup(Physics)) BoxCollider : public Collider
	{
	public:
		BoxCollider(const HSceneObject& parent, const Vector3& extents = Vector3(0.5f, 0.5f, 0.5f));

		/** Determines the extents (half size) of the geometry of the box. */
		B3D_SCRIPT_EXPORT(ExportName(Extents), Property(Setter))
		void SetExtents(const Vector3& extents);

		/** @copydoc SetExtents */
		B3D_SCRIPT_EXPORT(ExportName(Extents), Property(Getter))
		Vector3 GetExtents() const { return mExtents; }

		/** Determines the position of the box shape, relative to the component's scene object. */
		B3D_SCRIPT_EXPORT(ExportName(Center), Property(Setter))
		void SetCenter(const Vector3& center);

		/** @copydoc SetCenter */
		B3D_SCRIPT_EXPORT(ExportName(Center), Property(Getter))
		Vector3 GetCenter() const { return mShapeLocalPosition; }

		/************************************************************************/
		/* 						COMPONENT OVERRIDES                      		*/
		/************************************************************************/
	protected:
		friend class SceneObject;

		void OnCreated() override;

	protected:
		Vector3 mShapeLocalPosition = Vector3::kZero;
		Vector3 mExtents = Vector3(0.5f, 0.5f, 0.5f);

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/
	public:
		friend class BoxColliderRTTI;
		static RTTIType* GetRttiStatic();
		RTTIType* GetRtti() const override;

	protected:
		BoxCollider(); // Serialization only
	};

	/** @} */
} // namespace b3d
