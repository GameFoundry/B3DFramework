//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsPhysXMesh.h"
#include "RTTI/BsPhysXMeshRTTI.h"
#include "Mesh/BsMeshData.h"
#include "RenderAPI/BsVertexDataDesc.h"
#include "BsPhysX.h"
#include "Math/BsAABox.h"
#include "foundation/PxAllocatorCallback.h"
#include "geometry/PxTriangleMesh.h"
#include "geometry/PxConvexMesh.h"
#include "cooking/PxConvexMeshDesc.h"
#include "extensions/PxDefaultStreams.h"

using namespace physx;

using namespace bs;

/**
 * Attempts to cook a convex mesh from the provided mesh data. Assumes the mesh data is not null and contains vertex
 * positions as well as face indices. If the method returns true the resulting convex mesh will be output in the @p
 * data buffer, and its size in @p size. The data buffer will be allocated used the generic allocator and is up to the
 * caller to free it.
 */
bool CookConvex(PxCooking* cooking, const SPtr<MeshData>& meshData, u8** data, u32& size)
{
	SPtr<VertexDataDesc> vertexDesc = meshData->GetVertexDescription();

	// Try to create hull from points
	PxConvexMeshDesc convexDesc;
	convexDesc.points.count = meshData->GetVertexCount();
	convexDesc.points.stride = vertexDesc->GetVertexStride();
	convexDesc.points.data = meshData->GetElementData(VES_POSITION);
	convexDesc.flags |= PxConvexFlag::eCOMPUTE_CONVEX;

	PxDefaultMemoryOutputStream output;
	if(cooking->cookConvexMesh(convexDesc, output))
	{
		size = output.getSize();
		*data = (u8*)B3DAllocate(size);

		memcpy(*data, output.getData(), size);
		return true;
	}

	// Try inflating the convex mesh
	convexDesc.flags |= PxConvexFlag::eINFLATE_CONVEX;
	if(cooking->cookConvexMesh(convexDesc, output))
	{
		size = output.getSize();
		*data = (u8*)B3DAllocate(size);

		memcpy(*data, output.getData(), size);
		return true;
	}

	// Nothing works, just compute an AABB
	AABox box;

	auto vertIter = meshData->GetVec3DataIter(VES_POSITION);
	do
	{
		box.Merge(vertIter.GetValue());
	}
	while(vertIter.MoveNext());

	Vector3 aabbVerts[8];
	aabbVerts[0] = box.GetCorner(AABox::FAR_LEFT_BOTTOM);
	aabbVerts[1] = box.GetCorner(AABox::FAR_RIGHT_BOTTOM);
	aabbVerts[2] = box.GetCorner(AABox::FAR_RIGHT_TOP);
	aabbVerts[3] = box.GetCorner(AABox::FAR_LEFT_TOP);

	aabbVerts[4] = box.GetCorner(AABox::NEAR_LEFT_BOTTOM);
	aabbVerts[5] = box.GetCorner(AABox::NEAR_RIGHT_BOTTOM);
	aabbVerts[6] = box.GetCorner(AABox::NEAR_RIGHT_TOP);
	aabbVerts[7] = box.GetCorner(AABox::NEAR_LEFT_TOP);

	convexDesc.points.count = 8;
	convexDesc.points.stride = sizeof(Vector3);
	convexDesc.points.data = &aabbVerts[0];
	convexDesc.flags &= ~PxConvexFlag::eINFLATE_CONVEX;

	if(cooking->cookConvexMesh(convexDesc, output))
	{
		size = output.getSize();
		*data = (u8*)B3DAllocate(size);

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
bool CookMesh(const SPtr<MeshData>& meshData, PhysicsMeshType type, u8** data, u32& size)
{
	if(meshData == nullptr)
		return false;

	PxCooking* cooking = GetPhysX().GetCooking();
	if(cooking == nullptr)
	{
		B3D_LOG(Warning, Physics, "Attempting to cook a physics mesh but cooking is not enabled globally.");
		return false;
	}

	SPtr<VertexDataDesc> vertexDesc = meshData->GetVertexDescription();
	if(!vertexDesc->HasElement(VES_POSITION))
	{
		B3D_LOG(Warning, Physics, "Provided PhysicsMesh mesh data has no vertex positions.");
		return false;
	}

	if(type == PhysicsMeshType::Convex)
	{
		if(!CookConvex(cooking, meshData, data, size))
		{
			B3D_LOG(Warning, Physics, "Failed cooking a convex mesh. Perpahs it is too complex? Maximum number of "
									 "convex vertices is 256.");
			return false;
		}
	}
	else
	{
		PxTriangleMeshDesc meshDesc;
		meshDesc.points.count = meshData->GetVertexCount();
		meshDesc.points.stride = vertexDesc->GetVertexStride();
		meshDesc.points.data = meshData->GetElementData(VES_POSITION);

		meshDesc.triangles.count = meshData->GetIndexCount() / 3;
		meshDesc.flags |= PxMeshFlag::eFLIPNORMALS;

		IndexType indexType = meshData->GetIndexType();
		if(indexType == IT_32BIT)
		{
			meshDesc.triangles.stride = 3 * sizeof(PxU32);
			meshDesc.triangles.data = meshData->GetIndices32();
		}
		else
		{
			meshDesc.triangles.stride = 3 * sizeof(PxU16);
			meshDesc.triangles.data = meshData->GetIndices16();
			meshDesc.flags |= PxMeshFlag::e16_BIT_INDICES;
		}

		PxDefaultMemoryOutputStream output;
		if(!cooking->cookTriangleMesh(meshDesc, output))
			return false;

		size = output.getSize();
		*data = (u8*)B3DAllocate(size);

		memcpy(*data, output.getData(), size);
	}

	return true;
}

PhysXMesh::PhysXMesh(const SPtr<MeshData>& meshData, PhysicsMeshType type)
	: PhysicsMesh(meshData, type)
{}

void PhysXMesh::Initialize()
{
	if(mInternal == nullptr) // Could be not-null if we're deserializing
		mInternal = B3DMakeShared<FPhysXMesh>(mInitMeshData, mType);

	PhysicsMesh::Initialize();
}

void PhysXMesh::Destroy()
{
	mInternal = nullptr;

	PhysicsMesh::Destroy();
}

FPhysXMesh::FPhysXMesh()
	: FPhysicsMesh(nullptr, PhysicsMeshType::Convex)
{
}

FPhysXMesh::FPhysXMesh(const SPtr<MeshData>& meshData, PhysicsMeshType type)
	: FPhysicsMesh(meshData, type)
{
	// Perform cooking if needed
	if(meshData != nullptr)
		CookMesh(meshData, mType, &mCookedData, mCookedDataSize);

	Initialize();
}

FPhysXMesh::~FPhysXMesh()
{
	if(mCookedData != nullptr)
	{
		B3DFree(mCookedData);

		mCookedData = nullptr;
		mCookedDataSize = 0;
	}

	if(mTriangleMesh != nullptr)
	{
		mTriangleMesh->release();
		mTriangleMesh = nullptr;
	}

	if(mConvexMesh != nullptr)
	{
		mConvexMesh->release();
		mConvexMesh = nullptr;
	}
}

void FPhysXMesh::Initialize()
{
	if(mCookedData != nullptr && mCookedDataSize > 0)
	{
		PxPhysics* physx = GetPhysX().GetPhysX();

		PxDefaultMemoryInputData input(mCookedData, mCookedDataSize);
		if(mType == PhysicsMeshType::Convex)
			mConvexMesh = physx->createConvexMesh(input);
		else
			mTriangleMesh = physx->createTriangleMesh(input);
	}
}

SPtr<MeshData> FPhysXMesh::GetMeshData() const
{
	SmallVector<VertexElement, 8> vertexElements;
	vertexElements.Add(VertexElement(VET_FLOAT3, VES_POSITION));

	SPtr<VertexDataDesc> vertexDescription = B3DMakeShared<VertexDataDesc>(vertexElements);

	if(mConvexMesh == nullptr && mTriangleMesh == nullptr)
		return MeshData::Create(0, 0, vertexDescription);

	u32 numVertices = 0;
	u32 numIndices = 0;

	if(mConvexMesh != nullptr)
	{
		numVertices = mConvexMesh->getNbVertices();

		u32 numPolygons = mConvexMesh->getNbPolygons();
		for(u32 i = 0; i < numPolygons; i++)
		{
			PxHullPolygon face;
			bool status = mConvexMesh->getPolygonData(i, face);
			B3D_ASSERT(status);

			numIndices += (face.mNbVerts - 2) * 3;
		}
	}
	else // Triangle
	{
		numVertices = mTriangleMesh->getNbVertices();
		numIndices = mTriangleMesh->getNbTriangles() * 3;
	}

	SPtr<MeshData> meshData = MeshData::Create(numVertices, numIndices, vertexDescription);

	auto posIter = meshData->GetVec3DataIter(VES_POSITION);
	u32* outIndices = meshData->GetIndices32();

	if(mConvexMesh != nullptr)
	{
		const PxVec3* convexVertices = mConvexMesh->getVertices();
		const u8* convexIndices = mConvexMesh->getIndexBuffer();

		for(u32 i = 0; i < numVertices; i++)
			posIter.AddValue(FromPxVector(convexVertices[i]));

		u32 numPolygons = mConvexMesh->getNbPolygons();
		for(u32 i = 0; i < numPolygons; i++)
		{
			PxHullPolygon face;
			bool status = mConvexMesh->getPolygonData(i, face);
			B3D_ASSERT(status);

			const PxU8* faceIndices = convexIndices + face.mIndexBase;
			for(u32 j = 2; j < face.mNbVerts; j++)
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
		for(u32 i = 0; i < numVertices; i++)
			posIter.AddValue(FromPxVector(vertices[i]));

		if(mTriangleMesh->getTriangleMeshFlags() & PxTriangleMeshFlag::e16_BIT_INDICES)
		{
			const u16* indices = (const u16*)mTriangleMesh->getTriangles();

			u32 numTriangles = numIndices / 3;
			for(u32 i = 0; i < numTriangles; i++)
			{
				// Flip triangles as PhysX keeps them opposite to what framework expects
				outIndices[i * 3 + 0] = (u32)indices[i * 3 + 0];
				outIndices[i * 3 + 1] = (u32)indices[i * 3 + 2];
				outIndices[i * 3 + 2] = (u32)indices[i * 3 + 1];
			}
		}
		else
		{
			const u32* indices = (const u32*)mTriangleMesh->getTriangles();

			u32 numTriangles = numIndices / 3;
			for(u32 i = 0; i < numTriangles; i++)
			{
				// Flip triangles as PhysX keeps them opposite to what framework expects
				outIndices[i * 3 + 0] = indices[i * 3 + 0];
				outIndices[i * 3 + 1] = indices[i * 3 + 2];
				outIndices[i * 3 + 2] = indices[i * 3 + 1];
			}
		}
	}

	return meshData;
}

RTTITypeBase* FPhysXMesh::GetRttiStatic()
{
	return FPhysXMeshRTTI::Instance();
}

RTTITypeBase* FPhysXMesh::GetRtti() const
{
	return GetRttiStatic();
}
