//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsRendererDecal.h"
#include "BsRendererRenderable.h"
#include "Renderer/BsDecal.h"
#include "Mesh/BsMesh.h"
#include "Renderer/BsRendererUtility.h"

namespace bs { namespace ct
{
	DecalParamDef gDecalParamDef;

	void DecalRenderElement::draw() const
	{
		gRendererUtility().draw(mesh, subMesh);
	}

	RendererDecal::RendererDecal()
	{
		decalParamBuffer = gDecalParamDef.createBuffer();
		perObjectParamBuffer = gPerObjectParamDef.createBuffer();
		perCallParamBuffer = gPerCallParamDef.createBuffer();
	}

	void RendererDecal::updatePerObjectBuffer()
	{
		const Vector2 size = decal->getSize();
		const Vector3 scale(size.x, size.y, decal->getMaxDistance());
		const Vector3 offset(0.0f, 0.0f, -scale.z * 0.5f);

		const Matrix4 scaleAndOffset = Matrix4::TRS(offset, Quaternion::IDENTITY, scale);

		const Matrix4 worldTransform = decal->getMatrix() * scaleAndOffset;
		const Matrix4 worldNoScaleTransform = decal->getMatrixNoScale() * scaleAndOffset;

		PerObjectBuffer::update(perObjectParamBuffer, worldTransform, worldNoScaleTransform);

		const Transform& tfrm = decal->getTransform();
		const Vector2 extent = size * 0.5f;

		const Matrix4 view = Matrix4::view(tfrm.getPosition(), tfrm.getRotation());
		const Matrix4 proj = Matrix4::projectionOrthographic(-extent.x, extent.x, extent.y, -extent.y, 0.0f, 
			decal->getMaxDistance());

		const Matrix4 worldToDecal = proj * view;
		const Vector3 decalNormal = -decal->getTransform().getRotation().zAxis();
		const float normalTolerance = 0.01f;

		gDecalParamDef.gWorldToDecal.set(decalParamBuffer, worldToDecal);
		gDecalParamDef.gDecalNormal.set(decalParamBuffer, decalNormal);
		gDecalParamDef.gNormalTolerance.set(decalParamBuffer, normalTolerance);
	}

	void RendererDecal::updatePerCallBuffer(const Matrix4& viewProj, bool flush) const
	{
		const Matrix4 worldViewProjMatrix = viewProj * decal->getMatrix();

		gPerCallParamDef.gMatWorldViewProj.set(perCallParamBuffer, worldViewProjMatrix);

		if(flush)
			perCallParamBuffer->flushToGPU();
	}
}}
