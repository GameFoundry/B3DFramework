//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsRendererRenderable.h"

#include "Renderer/BsRendererUtility.h"
#include "Mesh/BsMesh.h"
#include "Utility/BsBitwise.h"

namespace b3d {
namespace render {

PerObjectParamDef gPerObjectParamDef;
PerCallParamDef gPerCallParamDef;

void PerObjectBuffer::Update(SPtr<GpuBuffer>& buffer, const Matrix4& tfrm, const Matrix4& tfrmNoScale, const Matrix4& prevTfrm, u32 layer)
{
	gPerObjectParamDef.gMatWorld.Set(buffer, tfrm);
	gPerObjectParamDef.gMatInvWorld.Set(buffer, tfrm.InverseAffine());
	gPerObjectParamDef.gMatWorldNoScale.Set(buffer, tfrmNoScale);
	gPerObjectParamDef.gMatInvWorldNoScale.Set(buffer, tfrmNoScale.InverseAffine());
	gPerObjectParamDef.gMatPrevWorld.Set(buffer, prevTfrm);
	gPerObjectParamDef.gWorldDeterminantSign.Set(buffer, tfrm.Determinant3x3() >= 0.0f ? 1.0f : -1.0f);
	gPerObjectParamDef.gLayer.Set(buffer, (i32)layer);
}

void RenderableElement::Draw(GpuCommandBuffer& commandBuffer) const
{
	if(MorphVertexDefinition == nullptr)
		GetRendererUtility().Draw(commandBuffer, Mesh, SubMesh);
	else
		GetRendererUtility().DrawMorph(commandBuffer, Mesh, SubMesh, MorphShapeBuffer, MorphVertexDefinition);
}

RendererRenderable::RendererRenderable()
{
	PerObjectParamBuffer = gPerObjectParamDef.CreateBuffer();
	PerCallParamBuffer = gPerCallParamDef.CreateBuffer();
}

void RendererRenderable::UpdatePerObjectBuffer()
{
	const Matrix4 worldNoScaleTransform = Renderable->GetWorldTransformMatrixWithoutScale();
	const u32 layer = Bitwise::MostSignificantBit(Renderable->GetLayer());

	PerObjectBuffer::Update(PerObjectParamBuffer, WorldTfrm, worldNoScaleTransform, PrevWorldTfrm, layer);
}

void RendererRenderable::UpdatePerCallBuffer(const Matrix4& viewProj, bool flush)
{
	const Matrix4 worldViewProjMatrix = viewProj * Renderable->GetWorldTransformMatrix();

	gPerCallParamDef.gMatWorldViewProj.Set(PerCallParamBuffer, worldViewProjMatrix);

	if(flush)
		PerCallParamBuffer->FlushCache();
}
}} // namespace b3d::render
