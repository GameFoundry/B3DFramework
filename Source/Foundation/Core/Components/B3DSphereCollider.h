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

	/** A collider with sphere geometry. */
	class B3D_EXPORT B3D_SCRIPT_EXPORT(DocumentationGroup(Physics)) SphereCollider : public Collider
	{
	public:
		SphereCollider(const HSceneObject& parent, float radius = 1.0f);

		/** Determines the radius of the sphere geometry. */
		B3D_SCRIPT_EXPORT(ExportName(Radius), Property(Setter))
		void SetRadius(float radius);

		/** @copydoc GetRadius */
		B3D_SCRIPT_EXPORT(ExportName(Radius), Property(Getter))
		float GetRadius() const { return mRadius; }

		/** Determines position of the sphere shape, relative to the component's scene object. */
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

		void OnCreated() override;

	protected:
		Vector3 mShapeLocalPosition = Vector3::kZero;
		float mRadius = 1.0f;

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/
	public:
		friend class CSphereColliderRTTI;
		static RTTIType* GetRttiStatic();
		RTTIType* GetRtti() const override;

	protected:
		SphereCollider(); // Serialization only
	};

	/** @} */
} // namespace b3d
