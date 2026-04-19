//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DDecalRenderState.h"
#include "B3DRenderableRenderState.h"
#include "B3DRenderBeast.h"
#include "Components/B3DDecal.h"
#include "Mesh/B3DMesh.h"
#include "Renderer/B3DRendererUtility.h"
#include "RenderAPI/B3DGpuCommandBuffer.h"

namespace b3d {
namespace render {

DecalUniformDefinition gDecalUniformDefinition;

void DecalDrawCommand::Draw(GpuCommandBuffer& commandBuffer) const
{
	const u32 decalDynamicOffsetIndex = GetRenderBeast()->GetDecalParameterSetInfo().DecalDynamicOffsetIndex;
	commandBuffer.SetDynamicBufferOffset(GpuPipelineSet::kPerObject, decalDynamicOffsetIndex, DecalParamBufferOffset);

	GetRendererUtility().Draw(commandBuffer, Mesh, SubMesh);
}

void DecalRenderState::UpdatePerObjectData(const DecalProxy& proxy)
{
	const Vector2 worldSize = proxy.GetWorldSize();
	const float worldMaxDistance = proxy.GetWorldMaxDistance();

	const Vector2 extent = worldSize * 0.5f;
	const Vector3 scale(extent.X, extent.Y, worldMaxDistance * 0.5f);
	const Vector3 offset(0.0f, 0.0f, -worldMaxDistance * 0.5f);
	const Matrix4 scaleAndOffset = Matrix4::TRS(offset, Quaternion::kIdentity, scale);

	WorldTransform = proxy.GetWorldTransformMatrix() * scaleAndOffset;
	WorldNoScale = proxy.GetWorldTransformMatrixWithoutScale() * scaleAndOffset;
	PrevWorldTransform = WorldTransform; // Decals don't track previous frame
	Layer = 0;
}

}} // namespace b3d::render
