//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsRendererRenderable.h"

namespace bs { namespace ct
{
	PerObjectParamDef gPerObjectParamDef;
	PerCallParamDef gPerCallParamDef;

	void PerObjectBuffer::update(SPtr<GpuParamBlockBuffer>& buffer, const Matrix4& tfrm, const Matrix4& tfrmNoScale)
	{
		gPerObjectParamDef.gMatWorld.set(buffer, tfrm);
		gPerObjectParamDef.gMatInvWorld.set(buffer, tfrm.inverseAffine());
		gPerObjectParamDef.gMatWorldNoScale.set(buffer, tfrmNoScale);
		gPerObjectParamDef.gMatInvWorldNoScale.set(buffer, tfrmNoScale.inverseAffine());
		gPerObjectParamDef.gWorldDeterminantSign.set(buffer, tfrm.determinant3x3() >= 0.0f ? 1.0f : -1.0f);
	}

	RendererRenderable::RendererRenderable()
	{
		perObjectParamBuffer = gPerObjectParamDef.createBuffer();
		perCallParamBuffer = gPerCallParamDef.createBuffer();
	}

	void RendererRenderable::updatePerObjectBuffer()
	{
		const Matrix4 worldTransform = renderable->getMatrix();
		const Matrix4 worldNoScaleTransform = renderable->getMatrixNoScale();

		PerObjectBuffer::update(perObjectParamBuffer, worldTransform, worldNoScaleTransform);
	}

	void RendererRenderable::updatePerCallBuffer(const Matrix4& viewProj, bool flush)
	{
		const Matrix4 worldViewProjMatrix = viewProj * renderable->getMatrix();

		gPerCallParamDef.gMatWorldViewProj.set(perCallParamBuffer, worldViewProjMatrix);

		if(flush)
			perCallParamBuffer->flushToGPU();
	}
}}