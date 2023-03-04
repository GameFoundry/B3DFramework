//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsRendererDecal.h"
#include "BsRendererRenderable.h"
#include "Renderer/BsDecal.h"
#include "Mesh/BsMesh.h"
#include "Renderer/BsRendererUtility.h"

namespace bs {
namespace ct {

DecalParamDef gDecalParamDef;

void DecalRenderElement::Draw() const
{
	GetRendererUtility().Draw(Mesh, SubMesh);
}

RendererDecal::RendererDecal()
{
	DecalParamBuffer = gDecalParamDef.CreateBuffer();
	PerObjectParamBuffer = gPerObjectParamDef.CreateBuffer();
	PerCallParamBuffer = gPerCallParamDef.CreateBuffer();
}

void RendererDecal::UpdatePerObjectBuffer()
{
	const Vector2 size = Decal->GetWorldSize();
	const Vector2 extent = size * 0.5f;
	const float maxDistance = Decal->GetWorldMaxDistance();

	const Vector3 scale(extent.X, extent.Y, maxDistance * 0.5f);
	const Vector3 offset(0.0f, 0.0f, -maxDistance * 0.5f);

	const Matrix4 scaleAndOffset = Matrix4::TRS(offset, Quaternion::kIdentity, scale);

	const Matrix4 worldTransform = Decal->GetMatrix() * scaleAndOffset;
	const Matrix4 worldNoScaleTransform = Decal->GetMatrixNoScale() * scaleAndOffset;

	// Note: Not providing the previous frame matrix here
	PerObjectBuffer::Update(PerObjectParamBuffer, worldTransform, worldNoScaleTransform, worldTransform, 0);

	const Transform& tfrm = Decal->GetTransform();

	const Matrix4 view = Matrix4::View(tfrm.GetPosition(), tfrm.GetRotation());
	const Matrix4 proj = Matrix4::ProjectionOrthographic(-extent.X, extent.X, -extent.Y, extent.Y, 0.0f, Decal->GetWorldMaxDistance());

	const Matrix4 worldToDecal = proj * view;
	const Vector3 decalNormal = -Decal->GetTransform().GetRotation().ZAxis();
	const float normalTolerance = -0.05f;

	float flipDerivatives = 1.0f;

	const GpuBackendConventions& rapiConventions = GetGpuDeviceCapabilities().Conventions;
	if(rapiConventions.UvYAxis == GpuBackendConventions::Axis::Up)
		flipDerivatives = -1.0f;

	gDecalParamDef.gWorldToDecal.Set(DecalParamBuffer, worldToDecal);
	gDecalParamDef.gDecalNormal.Set(DecalParamBuffer, decalNormal);
	gDecalParamDef.gNormalTolerance.Set(DecalParamBuffer, normalTolerance);
	gDecalParamDef.gFlipDerivatives.Set(DecalParamBuffer, flipDerivatives);
	gDecalParamDef.gLayerMask.Set(DecalParamBuffer, (i32)Decal->GetLayerMask());
}

void RendererDecal::UpdatePerCallBuffer(const Matrix4& viewProj, bool flush) const
{
	const Matrix4 worldViewProjMatrix = viewProj * Decal->GetMatrix();

	gPerCallParamDef.gMatWorldViewProj.Set(PerCallParamBuffer, worldViewProjMatrix);

	if(flush)
		PerCallParamBuffer->FlushToGpu();
}
}} // namespace bs::ct
