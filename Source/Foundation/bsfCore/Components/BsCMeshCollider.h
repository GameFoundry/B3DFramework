//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsCorePrerequisites.h"
#include "Physics/BsMeshCollider.h"
#include "Components/BsCCollider.h"

namespace bs
{
	/** @addtogroup Components-Core
	 *  @{
	 */

	/**
	 * @copydoc	MeshCollider
	 *
	 * @note Wraps MeshCollider as a Component.
	 */
	class B3D_CORE_EXPORT B3D_SCRIPT_EXPORT(DocumentationGroup(Physics), ExportName(MeshCollider)) CMeshCollider : public CCollider
	{
	public:
		CMeshCollider(const HSceneObject& parent);

		/** @copydoc MeshCollider::SetMesh */
		B3D_SCRIPT_EXPORT(ExportName(Mesh), Property(Setter))
		void SetMesh(const HPhysicsMesh& mesh);

		/** @copydoc MeshCollider::GetMesh */
		B3D_SCRIPT_EXPORT(ExportName(Mesh), Property(Getter))
		HPhysicsMesh GetMesh() const { return mMesh; }

		/************************************************************************/
		/* 						COMPONENT OVERRIDES                      		*/
		/************************************************************************/
	protected:
		friend class SceneObject;

		SPtr<Collider> CreateInternal() override;
		bool IsValidParent(const HRigidbody& parent) const override;

	protected:
		HPhysicsMesh mMesh;

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/
	public:
		friend class CMeshColliderRTTI;
		static RTTIType* GetRttiStatic();
		RTTIType* GetRtti() const;

	protected:
		CMeshCollider(); // Serialization only
	};

	/** @} */
} // namespace bs
