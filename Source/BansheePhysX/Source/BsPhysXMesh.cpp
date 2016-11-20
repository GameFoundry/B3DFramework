#include "BsPhysXMesh.h"
#include "BsPhysXMeshRTTI.h"
#include "BsMeshData.h"
#include "BsVertexDataDesc.h"
#include "BsPhysX.h"
#include "BsAABox.h"
#include "foundation\PxAllocatorCallback.h"
#include "geometry\PxTriangleMesh.h"
#include "geometry\PxConvexMesh.h"
#include "cooking\PxConvexMeshDesc.h"
#include "extensions\PxDefaultStreams.h"

using namespace physx;

namespace bs
{
	/**
	 * Attempts to cook a convex mesh from the provided mesh data. Assumes the mesh data is not null and contains vertex
	 * positions as well as face indices. If the method returns true the resulting convex mesh will be output in the @p
	 * data buffer, and its size in @p size. The data buffer will be allocated used the generic allocator and is up to the
	 * caller to free it.
	 */
	bool cookConvex(PxCooking* cooking, const SPtr<MeshData>& meshData, UINT8** data, UINT32& size)
	{
		SPtr<VertexDataDesc> vertexDesc = meshData->getVertexDesc();
		
		// Try to create hull from points
		PxConvexMeshDesc convexDesc;
		convexDesc.points.count = meshData->getNumVertices();
		convexDesc.points.stride = vertexDesc->getVertexStride();
		convexDesc.points.data = meshData->getElementData(VES_POSITION);
		convexDesc.flags |= PxConvexFlag::eCOMPUTE_CONVEX;

		PxDefaultMemoryOutputStream output;
		if (cooking->cookConvexMesh(convexDesc, output))
		{
			size = output.getSize();
			*data = (UINT8*)bs_alloc(size);

			memcpy(*data, output.getData(), size);
			return true;
		}

		// Try inflating the convex mesh
		convexDesc.flags |= PxConvexFlag::eINFLATE_CONVEX;
		if (cooking->cookConvexMesh(convexDesc, output))
		{
			size = output.getSize();
			*data = (UINT8*)bs_alloc(size);

			memcpy(*data, output.getData(), size);
			return true;
		}

		// Nothing works, just compute an AABB
		AABox box;

		auto vertIter = meshData->getVec3DataIter(VES_POSITION);
		do
		{
			box.merge(vertIter.getValue());
		}
		while (vertIter.moveNext());

		Vector3 aabbVerts[8];
		aabbVerts[0] = box.getCorner(AABox::FAR_LEFT_BOTTOM);
		aabbVerts[1] = box.getCorner(AABox::FAR_RIGHT_BOTTOM);
		aabbVerts[2] = box.getCorner(AABox::FAR_RIGHT_TOP);
		aabbVerts[3] = box.getCorner(AABox::FAR_LEFT_TOP);

		aabbVerts[4] = box.getCorner(AABox::NEAR_LEFT_BOTTOM);
		aabbVerts[5] = box.getCorner(AABox::NEAR_RIGHT_BOTTOM);
		aabbVerts[6] = box.getCorner(AABox::NEAR_RIGHT_TOP);
		aabbVerts[7] = box.getCorner(AABox::NEAR_LEFT_TOP);

		convexDesc.points.count = 8;
		convexDesc.points.stride = sizeof(Vector3);
		convexDesc.points.data = &aabbVerts[0];
		convexDesc.flags &= ~PxConvexFlag::eINFLATE_CONVEX;

		if (cooking->cookConvexMesh(convexDesc, output))
		{
			size = output.getSize();
			*data = (UINT8*)bs_alloc(size);

			memcpy(*data, output.getData(), size);
			return true;
		}

		return false;
	}

	/**
	 * Attempts to cook a triangle or convex mesh from the provided mesh data. Will log a warning and return false if it is
	 * unable to cook the mesh. If the method returns true the resulting convex mesh will be output in the @p data buffer,
	 * and its size in @p size. The data buffer will be allocated used the generic allocator and is up to the caller to 
	 * free it.
	 */
	bool cookMesh(const SPtr<MeshData>& meshData, PhysicsMeshType type, UINT8** data, UINT32& size)
	{
		if (meshData == nullptr)
			return false;

		PxCooking* cooking = gPhysX().getCooking();
		if (cooking == nullptr)
		{
			LOGWRN("Attempting to cook a physics mesh but cooking is not enabled globally.");
			return false;
		}

		SPtr<VertexDataDesc> vertexDesc = meshData->getVertexDesc();
		if (!vertexDesc->hasElement(VES_POSITION))
		{
			LOGWRN("Provided PhysicsMesh mesh data has no vertex positions.");
			return false;
		}

		if (type == PhysicsMeshType::Convex)
		{
			if(!cookConvex(cooking, meshData, data, size))
			{
				LOGWRN("Failed cooking a convex mesh. Perpahs it is too complex? Maximum number of convex vertices is 256.");
				return false;
			}
		}
		else
		{
			PxTriangleMeshDesc meshDesc;
			meshDesc.points.count = meshData->getNumVertices();
			meshDesc.points.stride = vertexDesc->getVertexStride();
			meshDesc.points.data = meshData->getElementData(VES_POSITION);

			meshDesc.triangles.count = meshData->getNumIndices() / 3;
			meshDesc.flags |= PxMeshFlag::eFLIPNORMALS;

			IndexType indexType = meshData->getIndexType();
			if (indexType == IT_32BIT)
			{
				meshDesc.triangles.stride = 3 * sizeof(PxU32);
				meshDesc.triangles.data = meshData->getIndices32();
			}
			else
			{
				meshDesc.triangles.stride = 3 * sizeof(PxU16);
				meshDesc.triangles.data = meshData->getIndices16();
				meshDesc.flags |= PxMeshFlag::e16_BIT_INDICES;
			}

			PxDefaultMemoryOutputStream output;
			if (!cooking->cookTriangleMesh(meshDesc, output))
				return false;

			size = output.getSize();
			*data = (UINT8*)bs_alloc(size);

			memcpy(*data, output.getData(), size);
		}

		return true;
	}

	PhysXMesh::PhysXMesh(const SPtr<MeshData>& meshData, PhysicsMeshType type)
		:PhysicsMesh(meshData, type)
	{ }

	void PhysXMesh::initialize()
	{
		if(mInternal == nullptr) // Could be not-null if we're deserializing
			mInternal = bs_shared_ptr_new<FPhysXMesh>(mInitMeshData, mType);

		PhysicsMesh::initialize();
	}

	void PhysXMesh::destroy()
	{
		mInternal = nullptr;

		PhysicsMesh::destroy();
	}

	FPhysXMesh::FPhysXMesh()
		:FPhysicsMesh(nullptr, PhysicsMeshType::Convex)
	{
		
	}

	FPhysXMesh::FPhysXMesh(const SPtr<MeshData>& meshData, PhysicsMeshType type)
		:FPhysicsMesh(meshData, type)
	{
		// Perform cooking if needed
		if (meshData != nullptr)
			cookMesh(meshData, mType, &mCookedData, mCookedDataSize);

		initialize();
	}

	FPhysXMesh::~FPhysXMesh()
	{
		if (mCookedData != nullptr)
		{
			bs_free(mCookedData);

			mCookedData = nullptr;
			mCookedDataSize = 0;
		}

		if (mTriangleMesh != nullptr)
		{
			mTriangleMesh->release();
			mTriangleMesh = nullptr;
		}

		if (mConvexMesh != nullptr)
		{
			mConvexMesh->release();
			mConvexMesh = nullptr;
		}
	}

	void FPhysXMesh::initialize()
	{
		if (mCookedData != nullptr && mCookedDataSize > 0)
		{
			PxPhysics* physx = gPhysX().getPhysX();

			PxDefaultMemoryInputData input(mCookedData, mCookedDataSize);
			if (mType == PhysicsMeshType::Convex)
				mConvexMesh = physx->createConvexMesh(input);
			else
				mTriangleMesh = physx->createTriangleMesh(input);
		}
	}

	SPtr<MeshData> FPhysXMesh::getMeshData() const
	{
		SPtr<VertexDataDesc> vertexDesc = VertexDataDesc::create();
		vertexDesc->addVertElem(VET_FLOAT3, VES_POSITION);

		if (mConvexMesh == nullptr && mTriangleMesh == nullptr)
			return MeshData::create(0, 0, vertexDesc);

		UINT32 numVertices = 0;
		UINT32 numIndices = 0;

		if(mConvexMesh != nullptr)
		{
			numVertices = mConvexMesh->getNbVertices();

			UINT32 numPolygons = mConvexMesh->getNbPolygons();
			for (UINT32 i = 0; i < numPolygons; i++)
			{
				PxHullPolygon face;
				bool status = mConvexMesh->getPolygonData(i, face);
				assert(status);

				numIndices += (face.mNbVerts - 2) * 3;
			}
		}
		else // Triangle
		{
			numVertices = mTriangleMesh->getNbVertices();
			numIndices = mTriangleMesh->getNbTriangles() * 3;
		}

		SPtr<MeshData> meshData = MeshData::create(numVertices, numIndices, vertexDesc);

		auto posIter = meshData->getVec3DataIter(VES_POSITION);
		UINT32* outIndices = meshData->getIndices32();

		if (mConvexMesh != nullptr)
		{
			const PxVec3* convexVertices = mConvexMesh->getVertices();
			const UINT8* convexIndices = mConvexMesh->getIndexBuffer();

			for (UINT32 i = 0; i < numVertices; i++)
				posIter.addValue(fromPxVector(convexVertices[i]));

			UINT32 numPolygons = mConvexMesh->getNbPolygons();
			for (UINT32 i = 0; i < numPolygons; i++)
			{
				PxHullPolygon face;
				bool status = mConvexMesh->getPolygonData(i, face);
				assert(status);

				const PxU8* faceIndices = convexIndices + face.mIndexBase;
				for (UINT32 j = 2; j < face.mNbVerts; j++)
				{
					*outIndices++ = faceIndices[0];
					*outIndices++ = faceIndices[j];
					*outIndices++ = faceIndices[j - 1];
				}
			}
		}
		else
		{
			const PxVec3* vertices = mTriangleMesh->getVertices();
			for (UINT32 i = 0; i < numVertices; i++)
				posIter.addValue(fromPxVector(vertices[i]));

			if(mTriangleMesh->getTriangleMeshFlags() & PxTriangleMeshFlag::e16_BIT_INDICES)
			{
				const UINT16* indices = (const UINT16*)mTriangleMesh->getTriangles();

				UINT32 numTriangles = numIndices / 3;
				for (UINT32 i = 0; i < numTriangles; i++)
				{
					// Flip triangles as PhysX keeps them opposite to what Banshee expects
					outIndices[i * 3 + 0] = (UINT32)indices[i * 3 + 0];
					outIndices[i * 3 + 1] = (UINT32)indices[i * 3 + 2];
					outIndices[i * 3 + 2] = (UINT32)indices[i * 3 + 1];
				}
			}
			else
			{
				const UINT32* indices = (const UINT32*)mTriangleMesh->getTriangles();

				UINT32 numTriangles = numIndices / 3;
				for (UINT32 i = 0; i < numTriangles; i++)
				{
					// Flip triangles as PhysX keeps them opposite to what Banshee expects
					outIndices[i * 3 + 0] = indices[i * 3 + 0];
					outIndices[i * 3 + 1] = indices[i * 3 + 2];
					outIndices[i * 3 + 2] = indices[i * 3 + 1];
				}
			}
		}

		return meshData;
	}

	RTTITypeBase* FPhysXMesh::getRTTIStatic()
	{
		return FPhysXMeshRTTI::instance();
	}

	RTTITypeBase* FPhysXMesh::getRTTI() const
	{
		return getRTTIStatic();
	}
}