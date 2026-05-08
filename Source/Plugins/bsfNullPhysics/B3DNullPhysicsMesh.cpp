//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DNullPhysicsMesh.h"
#include "RTTI/B3DNullPhysicsMeshRTTI.h"
#include "Mesh/B3DMeshData.h"
#include "GpuBackend/B3DVertexDescription.h"

using namespace b3d;

NullPhysicsMeshImplementation::NullPhysicsMeshImplementation()
	: mMeshData(nullptr)
{}

NullPhysicsMeshImplementation::NullPhysicsMeshImplementation(const TShared<MeshData>& meshData, PhysicsMeshType type)
	: mMeshData(meshData)
{}

TShared<MeshData> NullPhysicsMeshImplementation::GetMeshData() const
{
	if (mMeshData)
		return mMeshData;

	TInlineArray<VertexElement, 8> vertexElements;
	vertexElements.Add(VertexElement(VET_FLOAT3, VES_POSITION));

	TShared<VertexDescription> vertexDesc = B3DMakeShared<VertexDescription>(vertexElements);
	return MeshData::Create(0, 0, vertexDesc);
}

RTTIType* NullPhysicsMeshImplementation::GetRttiStatic()
{
	return NullPhysicsMeshImplementationRTTI::Instance();
}

RTTIType* NullPhysicsMeshImplementation::GetRtti() const
{
	return GetRttiStatic();
}
