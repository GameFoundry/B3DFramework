//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsPhysXPrerequisites.h"
#include "Physics/BsPhysicsMesh.h"
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
		void Initialize() override;
		void Destroy() override;

		// Note: Must not have its own RTTI type, it's important it shares the same type ID as PhysicsMesh so the
		// system knows to recognize it. Use FPhysicsMesh instead.
	};

	/** PhysX implementation of the PhysicsMesh foundation, FPhysicsMesh. */
	class FPhysXMesh : public FPhysicsMesh
	{
	public:
		FPhysXMesh(const SPtr<MeshData>& meshData, PhysicsMeshType type);
		~FPhysXMesh();

		SPtr<MeshData> GetMeshData() const override;

		/**
		 * Returns the internal PhysX representation of a triangle mesh. Caller must ensure the physics mesh type is
		 * triangle.
		 */
		physx::PxTriangleMesh* GetTriangleInternal() const
		{
			assert(mType == PhysicsMeshType::Triangle);
			return mTriangleMesh;
		}

		/**
		 * Returns the internal PhysX representation of a convex mesh. Caller must ensure the physics mesh type is
		 * convex.
		 */
		physx::PxConvexMesh* GetConvexInternal() const
		{
			assert(mType == PhysicsMeshType::Convex);
			return mConvexMesh;
		}

	private:
		/** Creates the internal triangle/convex mesh */
		void Initialize();

		physx::PxTriangleMesh* mTriangleMesh = nullptr;
		physx::PxConvexMesh* mConvexMesh = nullptr;

		u8* mCookedData = nullptr;
		u32 mCookedDataSize = 0;

		/************************************************************************/
		/* 								SERIALIZATION                      		*/
		/************************************************************************/
	public:
		FPhysXMesh(); // Serialization only

		friend class FPhysXMeshRTTI;
		static RTTITypeBase* GetRttiStatic();
		RTTITypeBase* GetRtti() const override;
	};

	/** @} */
} // namespace bs
