//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsCorePrerequisites.h"
#include "BsResource.h"

namespace bs
{
	/** @addtogroup Physics
	 *  @{
	 */

	class FPhysicsMesh;

	/** 
	 * Represents a physics mesh that can be used for physics MeshCollider%s. Physics mesh can be a generic triangle mesh
	 * or a convex mesh. Convex meshes are limited to 255 faces.
	 */
	class BS_CORE_EXPORT PhysicsMesh : public Resource
	{
	public:
		PhysicsMesh(const SPtr<MeshData>& meshData, PhysicsMeshType type);
		virtual ~PhysicsMesh() { }

		/** Returns the type of the physics mesh. */
		PhysicsMeshType getType() const;

		/** Returns the mesh's indices and vertices. */
		SPtr<MeshData> getMeshData() const;

		/** 
		 * Creates a new physics mesh. 
		 *
		 * @param[in]	meshData	Index and vertices of the mesh data.
		 * @param[in]	type		Type of the mesh. If convex the provided mesh geometry will be converted into a convex
		 *							mesh (that might not be the same as the provided mesh data).
		 */
		static HPhysicsMesh create(const SPtr<MeshData>& meshData, PhysicsMeshType type = PhysicsMeshType::Convex);

		/** @name Internal
		 *  @{
		 */

		/** Returns the internal implementation of the physics mesh. */
		virtual FPhysicsMesh* _getInternal() { return mInternal.get(); }

		/** 
		 * @copydoc create()
		 *
		 * For internal use. Requires manual initialization after creation.
		 */
		static SPtr<PhysicsMesh> _createPtr(const SPtr<MeshData>& meshData, PhysicsMeshType type);

		/** @} */

	protected:
		/** @copydoc Resource::initialize() */
		void initialize() override;

		SPtr<FPhysicsMesh> mInternal;
		SPtr<MeshData> mInitMeshData; // Transient, only used during initalization
		PhysicsMeshType mType; // Transient, only used during initalization

		/************************************************************************/
		/* 								SERIALIZATION                      		*/
		/************************************************************************/
	public:
		friend class PhysicsMeshRTTI;
		static RTTITypeBase* getRTTIStatic();
		RTTITypeBase* getRTTI() const override;
	};

	/** @} */
	/** @addtogroup Physics-Internal
	 *  @{
	 */

	/** Foundation that contains a specific implementation of a PhysicsMesh. */
	class BS_CORE_EXPORT FPhysicsMesh : public IReflectable
	{
	public:
		FPhysicsMesh(const SPtr<MeshData>& meshData, PhysicsMeshType type);
		virtual ~FPhysicsMesh();

		/** Returns the mesh's indices and vertices. */
		virtual SPtr<MeshData> getMeshData() const = 0;

	protected:
		friend class PhysicsMesh;

		PhysicsMeshType mType;
		
		/************************************************************************/
		/* 								SERIALIZATION                      		*/
		/************************************************************************/
	public:
		friend class FPhysicsMeshRTTI;
		static RTTITypeBase* getRTTIStatic();
		RTTITypeBase* getRTTI() const override;
	};

	/** @} */
}