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

	void PerObjectBuffer::Update(SPtr<GpuParamBlockBuffer>& buffer, const Matrix4& tfrm, const Matrix4& tfrmNoScale,
		const Matrix4& prevTfrm, UINT32 layer)
	{
		gPerObjectParamDef.gMatWorld.Set(buffer, tfrm);
		gPerObjectParamDef.gMatInvWorld.Set(buffer, tfrm.InverseAffine());
		gPerObjectParamDef.gMatWorldNoScale.Set(buffer, tfrmNoScale);
		gPerObjectParamDef.gMatInvWorldNoScale.Set(buffer, tfrmNoScale.InverseAffine());
		gPerObjectParamDef.gMatPrevWorld.Set(buffer, prevTfrm);
		gPerObjectParamDef.gWorldDeterminantSign.Set(buffer, tfrm.Determinant3x3() >= 0.0f ? 1.0f : -1.0f);
		gPerObjectParamDef.gLayer.Set(buffer, (INT32)layer);
	}

	void RenderableElement::Draw() const
	{
		if (morphVertexDeclaration == nullptr)
			gRendererUtility().Draw(mesh, subMesh);
		else
			gRendererUtility().DrawMorph(mesh, subMesh, morphShapeBuffer, morphVertexDeclaration);
	}

	RendererRenderable::RendererRenderable()
	{
		perObjectParamBuffer = gPerObjectParamDef.CreateBuffer();
		perCallParamBuffer = gPerCallParamDef.CreateBuffer();
	}

	void RendererRenderable::UpdatePerObjectBuffer()
	{
		const Matrix4 worldNoScaleTransform = renderable->GetMatrixNoScale();
		const UINT32 layer = Bitwise::MostSignificantBit(renderable->GetLayer());

		PerObjectBuffer::Update(perObjectParamBuffer, worldTfrm, worldNoScaleTransform, prevWorldTfrm, layer);
	}

	void RendererRenderable::UpdatePerCallBuffer(const Matrix4& viewProj, bool flush)
	{
		const Matrix4 worldViewProjMatrix = viewProj * renderable->GetMatrix();

		gPerCallParamDef.gMatWorldViewProj.Set(perCallParamBuffer, worldViewProjMatrix);

		if(flush)
			perCallParamBuffer->FlushToGpu();
	}
}}
