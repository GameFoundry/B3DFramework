//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DRendererRenderable.h"

#include "Renderer/B3DRendererUtility.h"
#include "Mesh/B3DMesh.h"
#include "Utility/B3DBitwise.h"

namespace b3d {
namespace render {

PerObjectUniformDefinition gPerObjectUniformDefinition;

void RenderableElement::Draw(GpuCommandBuffer& commandBuffer) const
{
	if(MorphVertexDefinition == nullptr)
		GetRendererUtility().Draw(commandBuffer, Mesh, SubMesh);
	else
		GetRendererUtility().DrawMorph(commandBuffer, Mesh, SubMesh, MorphShapeBuffer, MorphVertexDefinition);
}

void RendererRenderable::UpdatePerObjectData(const RenderableProxy& proxy)
{
	WorldTransform = proxy.GetWorldTransformMatrix();
	WorldNoScale = proxy.GetWorldTransformMatrixWithoutScale();
	Layer = Bitwise::MostSignificantBit(proxy.GetLayer());
}

}} // namespace b3d::render
