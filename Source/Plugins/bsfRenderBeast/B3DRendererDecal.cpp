//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DRendererDecal.h"
#include "B3DRendererRenderable.h"
#include "B3DRenderBeast.h"
#include "Components/B3DDecal.h"
#include "Mesh/B3DMesh.h"
#include "Renderer/B3DRendererUtility.h"
#include "RenderAPI/B3DGpuCommandBuffer.h"

namespace b3d {
namespace render {

DecalUniformDefinition gDecalUniformDefinition;

void DecalRenderElement::Draw(GpuCommandBuffer& commandBuffer) const
{
	const u32 decalDynamicOffsetIndex = GetRenderBeast()->GetDecalParameterSetInfo().DecalDynamicOffsetIndex;
	commandBuffer.SetDynamicBufferOffset(GpuPipelineSet::kPerObject, decalDynamicOffsetIndex, DecalParamBufferOffset);

	GetRendererUtility().Draw(commandBuffer, Mesh, SubMesh);
}

void RendererDecal::UpdatePerObjectData()
{
	const Vector2 size = Decal->GetWorldSize();
	const Vector2 extent = size * 0.5f;
	const float maxDistance = Decal->GetWorldMaxDistance();

	const Vector3 scale(extent.X, extent.Y, maxDistance * 0.5f);
	const Vector3 offset(0.0f, 0.0f, -maxDistance * 0.5f);
	const Matrix4 scaleAndOffset = Matrix4::TRS(offset, Quaternion::kIdentity, scale);

	WorldTransform = Decal->GetWorldTransformMatrix() * scaleAndOffset;
	WorldNoScale = Decal->GetWorldTransformMatrixWithoutScale() * scaleAndOffset;
	PrevWorldTransform = WorldTransform; // Decals don't track previous frame
	Layer = 0;
}

}} // namespace b3d::render
