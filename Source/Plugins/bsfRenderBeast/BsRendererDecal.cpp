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

	void DecalRenderElement::Draw() const
	{
		gRendererUtility().Draw(mesh, subMesh);
	}

	RendererDecal::RendererDecal()
	{
		decalParamBuffer = gDecalParamDef.CreateBuffer();
		perObjectParamBuffer = gPerObjectParamDef.CreateBuffer();
		perCallParamBuffer = gPerCallParamDef.CreateBuffer();
	}

	void RendererDecal::UpdatePerObjectBuffer()
	{
		const Vector2 size = decal->GetWorldSize();
		const Vector2 extent = size * 0.5f;
		const float maxDistance = decal->GetWorldMaxDistance();

		const Vector3 scale(extent.x, extent.y, maxDistance * 0.5f);
		const Vector3 offset(0.0f, 0.0f, -maxDistance * 0.5f);

		const Matrix4 scaleAndOffset = Matrix4::TRS(offset, Quaternion::IDENTITY, scale);

		const Matrix4 worldTransform = decal->GetMatrix() * scaleAndOffset;
		const Matrix4 worldNoScaleTransform = decal->GetMatrixNoScale() * scaleAndOffset;

		// Note: Not providing the previous frame matrix here
		PerObjectBuffer::Update(perObjectParamBuffer, worldTransform, worldNoScaleTransform, worldTransform, 0);

		const Transform& tfrm = decal->GetTransform();

		const Matrix4 view = Matrix4::View(tfrm.GetPosition(), tfrm.GetRotation());
		const Matrix4 proj = Matrix4::ProjectionOrthographic(-extent.x, extent.x, -extent.y, extent.y, 0.0f,
			decal->GetWorldMaxDistance());

		const Matrix4 worldToDecal = proj * view;
		const Vector3 decalNormal = -decal->GetTransform().GetRotation().ZAxis();
		const float normalTolerance = -0.05f;

		float flipDerivatives = 1.0f;

		const Conventions& rapiConventions = gCaps().conventions;
		if(rapiConventions.uvYAxis == Conventions::Axis::Up)
			flipDerivatives = -1.0f;

		gDecalParamDef.gWorldToDecal.Set(decalParamBuffer, worldToDecal);
		gDecalParamDef.gDecalNormal.Set(decalParamBuffer, decalNormal);
		gDecalParamDef.gNormalTolerance.Set(decalParamBuffer, normalTolerance);
		gDecalParamDef.gFlipDerivatives.Set(decalParamBuffer, flipDerivatives);
		gDecalParamDef.gLayerMask.Set(decalParamBuffer, (INT32)decal->GetLayerMask());
	}

	void RendererDecal::UpdatePerCallBuffer(const Matrix4& viewProj, bool flush) const
	{
		const Matrix4 worldViewProjMatrix = viewProj * decal->GetMatrix();

		gPerCallParamDef.gMatWorldViewProj.Set(perCallParamBuffer, worldViewProjMatrix);

		if(flush)
			perCallParamBuffer->FlushToGpu();
	}
}}
