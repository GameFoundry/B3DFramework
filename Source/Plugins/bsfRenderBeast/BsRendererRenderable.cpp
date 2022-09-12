//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsRendererRenderable.h"
#include "Renderer/BsRendererUtility.h"
#include "Mesh/BsMesh.h"
#include "Utility/BsBitwise.h"

namespace bs { namespace ct
{
	PerObjectParamDef gPerObjectParamDef;
	PerCallParamDef gPerCallParamDef;

	void PerObjectBuffer::update(SPtr<GpuParamBlockBuffer>& buffer, const Matrix4& tfrm, const Matrix4& tfrmNoScale,
		const Matrix4& prevTfrm, UINT32 layer)
	{
		gPerObjectParamDef.gMatWorld.set(buffer, tfrm);
		gPerObjectParamDef.gMatInvWorld.set(buffer, tfrm.inverseAffine());
		gPerObjectParamDef.gMatWorldNoScale.set(buffer, tfrmNoScale);
		gPerObjectParamDef.gMatInvWorldNoScale.set(buffer, tfrmNoScale.inverseAffine());
		gPerObjectParamDef.gMatPrevWorld.set(buffer, prevTfrm);
		gPerObjectParamDef.gWorldDeterminantSign.set(buffer, tfrm.determinant3x3() >= 0.0f ? 1.0f : -1.0f);
		gPerObjectParamDef.gLayer.set(buffer, (INT32)layer);
	}

	void RenderableElement::Draw() const
	{
		if (morphVertexDeclaration == nullptr)
			gRendererUtility().draw(mesh, subMesh);
		else
			gRendererUtility().drawMorph(mesh, subMesh, morphShapeBuffer, morphVertexDeclaration);
	}

	RendererRenderable::RendererRenderable()
	{
		perObjectParamBuffer = gPerObjectParamDef.createBuffer();
		perCallParamBuffer = gPerCallParamDef.createBuffer();
	}

	void RendererRenderable::UpdatePerObjectBuffer()
	{
		const Matrix4 worldNoScaleTransform = renderable->getMatrixNoScale();
		const UINT32 layer = Bitwise::mostSignificantBit(renderable->getLayer());

		PerObjectBuffer::update(perObjectParamBuffer, worldTfrm, worldNoScaleTransform, prevWorldTfrm, layer);
	}

	void RendererRenderable::UpdatePerCallBuffer(const Matrix4& viewProj, bool flush)
	{
		const Matrix4 worldViewProjMatrix = viewProj * renderable->getMatrix();

		gPerCallParamDef.gMatWorldViewProj.set(perCallParamBuffer, worldViewProjMatrix);

		if(flush)
			perCallParamBuffer->flushToGPU();
	}
}}
