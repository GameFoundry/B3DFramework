//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DRendererRenderable.h"

#include "Renderer/B3DRendererUtility.h"
#include "Mesh/B3DMesh.h"
#include "Utility/B3DBitwise.h"

namespace b3d {
namespace render {

PerObjectUniformDefinition gPerObjectUniformDefinition;

void PerObjectBuffer::Update(const GpuBufferSuballocation& suballocation, const Matrix4& tfrm, const Matrix4& tfrmNoScale, const Matrix4& prevTfrm, u32 layer)
{
	gPerObjectUniformDefinition.gMatWorld.Set(suballocation, tfrm);
	gPerObjectUniformDefinition.gMatInvWorld.Set(suballocation, tfrm.InverseAffine());
	gPerObjectUniformDefinition.gMatWorldNoScale.Set(suballocation, tfrmNoScale);
	gPerObjectUniformDefinition.gMatInvWorldNoScale.Set(suballocation, tfrmNoScale.InverseAffine());
	gPerObjectUniformDefinition.gMatPrevWorld.Set(suballocation, prevTfrm);
	gPerObjectUniformDefinition.gWorldDeterminantSign.Set(suballocation, tfrm.Determinant3x3() >= 0.0f ? 1.0f : -1.0f);
	gPerObjectUniformDefinition.gLayer.Set(suballocation, (i32)layer);
}

void RenderableElement::Draw(GpuCommandBuffer& commandBuffer) const
{
	if(MorphVertexDefinition == nullptr)
		GetRendererUtility().Draw(commandBuffer, Mesh, SubMesh);
	else
		GetRendererUtility().DrawMorph(commandBuffer, Mesh, SubMesh, MorphShapeBuffer, MorphVertexDefinition);
}

void RendererRenderable::UpdatePerObjectBuffer()
{
	const Matrix4 worldNoScaleTransform = Renderable->GetWorldTransformMatrixWithoutScale();
	const u32 layer = Bitwise::MostSignificantBit(Renderable->GetLayer());

	PerObjectBuffer::Update(BufferAllocation.PerObjectSuballocation, WorldTfrm, worldNoScaleTransform, PrevWorldTfrm, layer);
}
}} // namespace b3d::render
