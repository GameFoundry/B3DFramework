//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPhysXPrerequisites.h"
#include "Physics/B3DPhysicsMesh.h"
#include "PxMaterial.h"

namespace b3d
{
	/** @addtogroup PhysX
	 *  @{
	 */

	/** PhysX implementation of the physics mesh. */
	class PhysXMesh : public IPhysicsMeshImplementation
	{
	public:
		PhysXMesh(const SPtr<MeshData>& meshData, PhysicsMeshType type);
		~PhysXMesh() override;

		SPtr<MeshData> GetMeshData() const override;

		/**
		 * Returns the internal PhysX representation of a triangle mesh. Caller must ensure the physics mesh type is
		 * triangle.
		 */
		physx::PxTriangleMesh* GetPxTriangleMesh() const
		{
			B3D_ASSERT(mType == PhysicsMeshType::Triangle);
			return mTriangleMesh;
		}

		/**
		 * Returns the internal PhysX representation of a convex mesh. Caller must ensure the physics mesh type is
		 * convex.
		 */
		physx::PxConvexMesh* GetPxConvexMesh() const
		{
			B3D_ASSERT(mType == PhysicsMeshType::Convex);
			return mConvexMesh;
		}

	private:
		/** Creates the internal triangle/convex mesh */
		void Initialize();

		physx::PxTriangleMesh* mTriangleMesh = nullptr;
		physx::PxConvexMesh* mConvexMesh = nullptr;

		u8* mCookedData = nullptr;
		u32 mCookedDataSize = 0;
		PhysicsMeshType mType;

		/************************************************************************/
		/* 								SERIALIZATION                      		*/
		/************************************************************************/
	public:
		PhysXMesh() = default; // Serialization only

		friend class PhysXMeshRTTI;
		static RTTIType* GetRttiStatic();
		RTTIType* GetRtti() const override;
	};

	/** @} */
} // namespace b3d
