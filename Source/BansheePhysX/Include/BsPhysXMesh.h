//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsPhysXPrerequisites.h"
#include "BsPhysicsMesh.h"
#include "PxMaterial.h"

namespace bs
{
	/** @addtogroup PhysX
	 *  @{
	 */

	/** PhysX implementation of a PhysicsMesh. */
	class PhysXMesh : public PhysicsMesh
	{
	public:
		PhysXMesh(const SPtr<MeshData>& meshData, PhysicsMeshType type);

	private:
		/** @copydoc PhysicsMesh::initialize() */
		void initialize() override;

		/** @copydoc PhysicsMesh::initialize() */
		void destroy() override;

		// Note: Must not have its own RTTI type, it's important it shares the same type ID as PhysicsMesh so the
		// system knows to recognize it. Use FPhysicsMesh instead.
	};

	/** PhysX implementation of the PhysicsMesh foundation, FPhysicsMesh. */
	class FPhysXMesh : public FPhysicsMesh
	{
	public:
		FPhysXMesh(const SPtr<MeshData>& meshData, PhysicsMeshType type);
		~FPhysXMesh();

		/** @copydoc PhysicsMesh::getMeshData */
		SPtr<MeshData> getMeshData() const override;

		/** 
		 * Returns the internal PhysX representation of a triangle mesh. Caller must ensure the physics mesh type is 
		 * triangle. 
		 */
		physx::PxTriangleMesh* _getTriangle() const { assert(mType == PhysicsMeshType::Triangle); return mTriangleMesh; }

		/** 
		 * Returns the internal PhysX representation of a convex mesh. Caller must ensure the physics mesh type is 
		 * convex. 
		 */
		physx::PxConvexMesh* _getConvex() const { assert(mType == PhysicsMeshType::Convex); return mConvexMesh; }

	private:
		/** Creates the internal triangle/convex mesh */
		void initialize();

		physx::PxTriangleMesh* mTriangleMesh = nullptr;
		physx::PxConvexMesh* mConvexMesh = nullptr;

		UINT8* mCookedData = nullptr;
		UINT32 mCookedDataSize = 0;

		/************************************************************************/
		/* 								SERIALIZATION                      		*/
		/************************************************************************/
	public:
		FPhysXMesh(); // Serialization only

		friend class FPhysXMeshRTTI;
		static RTTITypeBase* getRTTIStatic();
		RTTITypeBase* getRTTI() const override;
	};

	/** @} */
}