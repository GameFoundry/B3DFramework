//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "RenderState/B3DRenderableRenderState.h"

#include "Renderer/B3DRendererUtility.h"
#include "Mesh/B3DMesh.h"
#include "Utility/B3DBitwise.h"
#include "Animation/SkinCache/B3DSkinCache.h"

namespace b3d {
namespace render {

PerObjectUniformDefinition gPerObjectUniformDefinition;

void RenderableDrawCommand::Draw(GpuCommandBuffer& commandBuffer) const
{
	if(SkinCacheEntry != nullptr)
	{
		if(SkinCacheEntry->HasDeformedVertices())
			GetRendererUtility().Draw(commandBuffer, *SkinCacheEntry->GetVertexData(), Mesh->GetIndexBuffer(), SubMesh);
	}
	else if(MorphVertexDefinition == nullptr)
		GetRendererUtility().Draw(commandBuffer, Mesh, SubMesh);
	else
		GetRendererUtility().DrawMorph(commandBuffer, Mesh, SubMesh, MorphShapeBuffer, MorphVertexDefinition);
}

void RenderableRenderState::UpdatePerObjectData(const RenderableProxy& proxy)
{
	WorldTransform = proxy.GetWorldTransformMatrix();
	WorldNoScale = proxy.GetWorldTransformMatrixWithoutScale();
	Layer = Bitwise::MostSignificantBit(proxy.GetLayer());
}

}} // namespace b3d::render
