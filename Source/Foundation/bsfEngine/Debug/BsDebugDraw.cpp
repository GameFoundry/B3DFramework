//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Debug/BsDebugDraw.h"
#include "Mesh/BsMesh.h"
#include "RenderAPI/BsVertexDescription.h"
#include "Utility/BsShapeMeshes3D.h"
#include "Image/BsSpriteTexture.h"
#include "CoreObject/BsRenderThread.h"
#include "Material/BsMaterial.h"
#include "RenderAPI/BsGpuParameters.h"
#include "Material/BsGpuParamsSet.h"
#include "Renderer/BsRenderer.h"
#include "Renderer/BsRendererUtility.h"
#include "Utility/BsDrawHelper.h"
#include "Renderer/BsRendererExtension.h"
#include "Resources/BsBuiltinResources.h"
#include "Renderer/BsCamera.h"
#include "Profiling/BsProfilerGPU.h"

using namespace std::placeholders;

using namespace bs;

DebugDraw::DebugDraw()
{
	mDrawHelper = B3DNew<DrawHelper>();
	mRenderer = RendererExtension::Create<ct::DebugDrawRenderer>(nullptr);
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
			proxyData.push_back(MeshRenderData(B3DGetRenderProxy(entry.Mesh), entry.SubMesh, DebugDrawMaterial::Solid));
		else if(entry.Type == DrawHelper::MeshType::Wire)
			proxyData.push_back(MeshRenderData(B3DGetRenderProxy(entry.Mesh), entry.SubMesh, DebugDrawMaterial::Wire));
		else if(entry.Type == DrawHelper::MeshType::Line)
			proxyData.push_back(MeshRenderData(B3DGetRenderProxy(entry.Mesh), entry.SubMesh, DebugDrawMaterial::Line));
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

	ct::DebugDrawRenderer* renderer = mRenderer.get();
	GetRenderThread().PostCommand(std::bind(&ct::DebugDrawRenderer::UpdateData, renderer, proxyData), "DebugDrawRenderer::UpdateData");
}

namespace bs { namespace ct
{

DebugDrawParamsDef gDebugDrawParamsDef;

void DebugDrawMat::Execute(GpuCommandBuffer& commandBuffer, const SPtr<GpuBuffer>& params, const SPtr<Mesh>& mesh, const SubMesh& subMesh)
{
	BS_RENMAT_PROFILE_BLOCK

	mGPUParameters->SetUniformBuffer("Params", params);

	Bind(commandBuffer);
	GetRendererUtility().Draw(commandBuffer, mesh, subMesh);
}

DebugDrawMat* DebugDrawMat::GetVariation(DebugDrawMaterial mat)
{
	if(mat == DebugDrawMaterial::Solid)
		return Get(GetVariation<true, false, false>());

	if(mat == DebugDrawMaterial::Wire)
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

	mParamBuffer = gDebugDrawParamsDef.CreateBuffer();
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
	Matrix4 projMatrix = camera.GetProjectionMatrixRs();
	Matrix4 viewProjMat = projMatrix * viewMatrix;

	gDebugDrawParamsDef.gMatViewProj.Set(mParamBuffer, viewProjMat);
	gDebugDrawParamsDef.gViewDir.Set(mParamBuffer, (Vector4)camera.GetTransform().GetForward());

	for(auto& entry : mMeshes)
	{
		DebugDrawMat* mat = DebugDrawMat::GetVariation(entry.Type);
		mat->Execute(*viewContext.CommandBuffer, mParamBuffer, entry.Mesh, entry.SubMesh);
	}
}
}}
