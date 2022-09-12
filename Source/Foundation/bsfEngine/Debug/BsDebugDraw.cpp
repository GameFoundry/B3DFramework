//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Debug/BsDebugDraw.h"
#include "Mesh/BsMesh.h"
#include "RenderAPI/BsVertexDataDesc.h"
#include "Utility/BsShapeMeshes3D.h"
#include "Image/BsSpriteTexture.h"
#include "CoreThread/BsCoreThread.h"
#include "Material/BsMaterial.h"
#include "RenderAPI/BsGpuParams.h"
#include "Material/BsGpuParamsSet.h"
#include "RenderAPI/BsRenderAPI.h"
#include "Renderer/BsRenderer.h"
#include "Renderer/BsRendererUtility.h"
#include "Utility/BsDrawHelper.h"
#include "Renderer/BsRendererExtension.h"
#include "Resources/BsBuiltinResources.h"
#include "Renderer/BsCamera.h"
#include "Profiling/BsProfilerGPU.h"

using namespace std::placeholders;

namespace bs
{
	DebugDraw::DebugDraw()
	{
		mDrawHelper = bs_new<DrawHelper>();
		mRenderer = RendererExtension::create<ct::DebugDrawRenderer>(nullptr);
	}

	DebugDraw::~DebugDraw()
	{
		bs_delete(mDrawHelper);
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

	void DebugDraw::drawWireArc(const Vector3& position, const Vector3& normal, float radius,
		Degree startAngle, Degree amountAngle)
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
		for (auto& entry : meshData)
		{
			if (entry.type == DrawHelper::MeshType::Solid)
				proxyData.push_back(MeshRenderData(entry.mesh->GetCore(), entry.subMesh, DebugDrawMaterial::Solid));
			else if (entry.type == DrawHelper::MeshType::Wire)
				proxyData.push_back(MeshRenderData(entry.mesh->GetCore(), entry.subMesh, DebugDrawMaterial::Wire));
			else if (entry.type == DrawHelper::MeshType::Line)
				proxyData.push_back(MeshRenderData(entry.mesh->GetCore(), entry.subMesh, DebugDrawMaterial::Line));
		}

		return proxyData;
	}

	void DebugDraw::Clear()
	{
		mDrawHelper->Clear();
	}

	void DebugDraw::_update()
	{
		mActiveMeshes.Clear();
		mActiveMeshes = mDrawHelper->BuildMeshes(DrawHelper::SortType::None);

		Vector<MeshRenderData> proxyData = createMeshProxyData(mActiveMeshes);

		ct::DebugDrawRenderer* renderer = mRenderer.Get();
		gCoreThread().QueueCommand(std::bind(&ct::DebugDrawRenderer::updateData, renderer, proxyData));
	}

	namespace ct
	{

	DebugDrawParamsDef gDebugDrawParamsDef;

	DebugDrawMat::DebugDrawMat()
	{
		// Do nothing
	}

	void DebugDrawMat::Execute(const SPtr<GpuParamBlockBuffer>& params, const SPtr<Mesh>& mesh, const SubMesh& subMesh)
	{
		BS_RENMAT_PROFILE_BLOCK

		mParams->SetParamBlockBuffer("Params", params);

		bind();
		gRendererUtility().Draw(mesh, subMesh);
	}

	DebugDrawMat* DebugDrawMat::getVariation(DebugDrawMaterial mat)
	{
		if (mat == DebugDrawMaterial::Solid)
			return Get(getVariation<true, false, false>());
		
		if (mat == DebugDrawMaterial::Wire)
			return Get(getVariation<false, false, true>());

		return Get(getVariation<false, true, false>());
	}

	DebugDrawRenderer::DebugDrawRenderer()
		:RendererExtension(RenderLocation::PostLightPass, 0)
	{
	}

	void DebugDrawRenderer::Initialize(const Any& data)
	{
		THROW_IF_NOT_CORE_THREAD;

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
		if (renderTarget == nullptr)
			return;

		Matrix4 viewMatrix = camera.GetViewMatrix();
		Matrix4 projMatrix = camera.GetProjectionMatrixRS();
		Matrix4 viewProjMat = projMatrix * viewMatrix;

		gDebugDrawParamsDef.gMatViewProj.Set(mParamBuffer, viewProjMat);
		gDebugDrawParamsDef.gViewDir.Set(mParamBuffer, (Vector4)camera.getTransform().getForward());

		for (auto& entry : mMeshes)
		{
			DebugDrawMat* mat = DebugDrawMat::getVariation(entry.type);
			mat->Execute(mParamBuffer, entry.mesh, entry.subMesh);
		}
	}
	}
}
