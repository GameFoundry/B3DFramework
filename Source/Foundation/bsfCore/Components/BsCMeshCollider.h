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
	class BS_CORE_EXPORT BS_SCRIPT_EXPORT(m:Physics,n:MeshCollider) CMeshCollider : public CCollider
	{
	public:
		CMeshCollider(const HSceneObject& parent);

		/** @copydoc MeshCollider::setMesh */
		BS_SCRIPT_EXPORT(n:Mesh,pr:setter)
		void SetMesh(const HPhysicsMesh& mesh);

		/** @copydoc MeshCollider::getMesh */
		BS_SCRIPT_EXPORT(n:Mesh,pr:getter)
		HPhysicsMesh GetMesh() const { return mMesh; }

		/** @name Internal
		 *  @{
		 */

		/**	Returns the mesh collider that this component wraps. */
		MeshCollider* _getInternal() const { return static_cast<MeshCollider*>(mInternal.Get()); }

		/** @} */

		/************************************************************************/
		/* 						COMPONENT OVERRIDES                      		*/
		/************************************************************************/
	protected:
		friend class SceneObject;

		/** @copydoc CCollider::createInternal */
		SPtr<Collider> CreateInternal() override;

		/** @copydoc CCollider::isValidParent */
		bool IsValidParent(const HRigidbody& parent) const override;

	protected:
		HPhysicsMesh mMesh;

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/
	public:
		friend class CMeshColliderRTTI;
		static RTTITypeBase* getRTTIStatic();
		RTTITypeBase* getRTTI() const override;

	protected:
		CMeshCollider(); // Serialization only
	};

	 /** @} */
}
