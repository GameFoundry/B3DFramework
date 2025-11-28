//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Debug/B3DDebugDraw.h"
#include "Mesh/B3DMesh.h"
#include "RenderAPI/B3DVertexDescription.h"
#include "Utility/B3DShapeMeshes3D.h"
#include "Image/B3DSpriteTexture.h"
#include "CoreObject/B3DRenderThread.h"
#include "Material/B3DMaterial.h"
#include "RenderAPI/B3DGpuParameterSet.h"
#include "Material/B3DMaterialParameterAdapter.h"
#include "Renderer/B3DRenderer.h"
#include "Renderer/B3DRendererUtility.h"
#include "Utility/B3DDrawHelper.h"
#include "Renderer/B3DRendererExtension.h"
#include "Resources/B3DBuiltinResources.h"
#include "Components/B3DCamera.h"
#include "Profiling/B3DProfilerGPU.h"


using namespace b3d;

DebugDraw::DebugDraw()
{
	mDrawHelper = B3DNew<DrawHelper>();
	mRenderer = RendererExtension::Create<render::DebugDrawRenderer>(nullptr);
}

DebugDraw::~DebugDraw()
{
	B3DDelete(mDrawHelper);
}

void DebugDraw::SetColor(const Color& color)
{
	mDrawHelper->SetColor(color);
}

void DebugDraw::SetTransform(const Matrix4& transform)
{
	mDrawHelper->SetTransform(transform);
}

void DebugDraw::DrawCube(const Vector3& position, const Vector3& extents)
{
	mDrawHelper->Cube(position, extents);
}

void DebugDraw::DrawSphere(const Vector3& position, float radius)
{
	mDrawHelper->Sphere(position, radius);
}

void DebugDraw::DrawCone(const Vector3& base, const Vector3& normal, float height, float radius, const Vector2& scale)
{
	mDrawHelper->Cone(base, normal, height, radius, scale);
}

void DebugDraw::DrawDisc(const Vector3& position, const Vector3& normal, float radius)
{
	mDrawHelper->Disc(position, normal, radius);
}

void DebugDraw::DrawWireCube(const Vector3& position, const Vector3& extents)
{
	mDrawHelper->WireCube(position, extents);
}

void DebugDraw::DrawWireSphere(const Vector3& position, float radius)
{
	mDrawHelper->WireSphere(position, radius);
}

void DebugDraw::DrawWireCone(const Vector3& base, const Vector3& normal, float height, float radius, const Vector2& scale)
{
	mDrawHelper->WireCone(base, normal, height, radius, scale);
}

void DebugDraw::DrawLine(const Vector3& start, const Vector3& end)
{
	mDrawHelper->Line(start, end);
}

void DebugDraw::DrawLineList(const Vector<Vector3>& linePoints)
{
	mDrawHelper->LineList(linePoints);
}

void DebugDraw::DrawWireDisc(const Vector3& position, const Vector3& normal, float radius)
{
	mDrawHelper->WireDisc(position, normal, radius);
}

void DebugDraw::DrawWireArc(const Vector3& position, const Vector3& normal, float radius, Degree startAngle, Degree amountAngle)
{
	mDrawHelper->WireArc(position, normal, radius, startAngle, amountAngle);
}

void DebugDraw::DrawWireMesh(const SPtr<MeshData>& meshData)
{
	mDrawHelper->WireMesh(meshData);
}

void DebugDraw::DrawFrustum(const Vector3& position, float aspect, Degree FOV, float near, float far)
{
	mDrawHelper->Frustum(position, aspect, FOV, near, far);
}

Vector<DebugDraw::MeshRenderData> DebugDraw::CreateMeshProxyData(const Vector<DrawHelper::ShapeMeshData>& meshData)
{
	Vector<MeshRenderData> proxyData;
	for(auto& entry : meshData)
	{
		if(entry.Type == DrawHelper::MeshType::Solid)
			proxyData.push_back(MeshRenderData(B3DGetRenderProxy(entry.Mesh), entry.SubMesh, DebugDrawMaterialType::Solid));
		else if(entry.Type == DrawHelper::MeshType::Wire)
			proxyData.push_back(MeshRenderData(B3DGetRenderProxy(entry.Mesh), entry.SubMesh, DebugDrawMaterialType::Wire));
		else if(entry.Type == DrawHelper::MeshType::Line)
			proxyData.push_back(MeshRenderData(B3DGetRenderProxy(entry.Mesh), entry.SubMesh, DebugDrawMaterialType::Line));
	}

	return proxyData;
}

void DebugDraw::Clear()
{
	mDrawHelper->Clear();
}

void DebugDraw::UpdateInternal()
{
	mActiveMeshes.clear();
	mActiveMeshes = mDrawHelper->BuildMeshes(DrawHelper::SortType::None);

	Vector<MeshRenderData> proxyData = CreateMeshProxyData(mActiveMeshes);

	render::DebugDrawRenderer* renderer = mRenderer.get();
	GetRenderThread().PostCommand([renderer, proxyData]() { renderer->UpdateData(proxyData); }, "DebugDrawRenderer::UpdateData");
}

namespace b3d { namespace render
{

DebugDrawUniformDefinition gDebugDrawUniformDefinition;

void DebugDrawMaterial::Execute(GpuCommandBuffer& commandBuffer, const GpuBufferSuballocation& uniformBuffer, const SPtr<Mesh>& mesh, const SubMesh& subMesh)
{
	B3D_PROFILE_RENDERER_MATERIAL

	mGpuParameterSet->SetUniformBuffer("Params", uniformBuffer);

	Bind(commandBuffer);
	GetRendererUtility().Draw(commandBuffer, mesh, subMesh);
}

DebugDrawMaterial* DebugDrawMaterial::GetVariation(DebugDrawMaterialType material)
{
	if(material == DebugDrawMaterialType::Solid)
		return Get(GetVariation<true, false, false>());

	if(material == DebugDrawMaterialType::Wire)
		return Get(GetVariation<false, false, true>());

	return Get(GetVariation<false, true, false>());
}

DebugDrawRenderer::DebugDrawRenderer()
	: RendererExtension(RenderLocation::PostLightPass, 0)
{
}

void DebugDrawRenderer::Initialize(const Any& data)
{
	ASSERT_IF_NOT_RENDER_THREAD;
}

void DebugDrawRenderer::UpdateData(const Vector<DebugDraw::MeshRenderData>& meshes)
{
	mMeshes = meshes;
}

RendererExtensionRequest DebugDrawRenderer::Check(const Camera& camera)
{
	return mMeshes.empty() ? RendererExtensionRequest::RenderIfTargetDirty : RendererExtensionRequest::ForceRender;
}

void DebugDrawRenderer::Render(const Camera& camera, const RendererViewContext& viewContext)
{
	SPtr<RenderTarget> renderTarget = camera.GetViewport()->GetTarget();
	if(renderTarget == nullptr)
		return;

	Matrix4 viewMatrix = camera.GetViewMatrix();
	Matrix4 projMatrix = camera.GetProjectionMatrix();
	Matrix4 viewProjMat = projMatrix * viewMatrix;

	GpuBufferSuballocation uniformBuffer = gDebugDrawUniformDefinition.AllocateTransient();

	gDebugDrawUniformDefinition.gMatViewProj.Set(uniformBuffer, viewProjMat);
	gDebugDrawUniformDefinition.gViewDir.Set(uniformBuffer, (Vector4)camera.GetWorldTransform().GetForward());

	for(auto& entry : mMeshes)
	{
		DebugDrawMaterial* material = DebugDrawMaterial::GetVariation(entry.Type);
		material->Execute(*viewContext.CommandBuffer, uniformBuffer, entry.Mesh, entry.SubMesh);
	}
}
}}
