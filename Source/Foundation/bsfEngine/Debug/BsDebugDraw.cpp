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

	void DebugDraw::setColor(const Color& color)
	{
		mDrawHelper->setColor(color);
	}

	void DebugDraw::setTransform(const Matrix4& transform)
	{
		mDrawHelper->setTransform(transform);
	}

	void DebugDraw::drawCube(const Vector3& position, const Vector3& extents)
	{
		mDrawHelper->cube(position, extents);
	}

	void DebugDraw::drawSphere(const Vector3& position, float radius)
	{
		mDrawHelper->sphere(position, radius);
	}

	void DebugDraw::drawCone(const Vector3& base, const Vector3& normal, float height, float radius, const Vector2& scale)
	{
		mDrawHelper->cone(base, normal, height, radius, scale);
	}

	void DebugDraw::drawDisc(const Vector3& position, const Vector3& normal, float radius)
	{
		mDrawHelper->disc(position, normal, radius);
	}

	void DebugDraw::drawWireCube(const Vector3& position, const Vector3& extents)
	{
		mDrawHelper->wireCube(position, extents);
	}

	void DebugDraw::drawWireSphere(const Vector3& position, float radius)
	{
		mDrawHelper->wireSphere(position, radius);
	}

	void DebugDraw::drawWireCone(const Vector3& base, const Vector3& normal, float height, float radius, const Vector2& scale)
	{
		mDrawHelper->wireCone(base, normal, height, radius, scale);
	}

	void DebugDraw::drawLine(const Vector3& start, const Vector3& end)
	{
		mDrawHelper->line(start, end);
	}

	void DebugDraw::drawLineList(const Vector<Vector3>& linePoints)
	{
		mDrawHelper->lineList(linePoints);
	}

	void DebugDraw::drawWireDisc(const Vector3& position, const Vector3& normal, float radius)
	{
		mDrawHelper->wireDisc(position, normal, radius);
	}

	void DebugDraw::drawWireArc(const Vector3& position, const Vector3& normal, float radius,
		Degree startAngle, Degree amountAngle)
	{
		mDrawHelper->wireArc(position, normal, radius, startAngle, amountAngle);
	}

	void DebugDraw::drawWireMesh(const SPtr<MeshData>& meshData)
	{
		mDrawHelper->wireMesh(meshData);
	}

	void DebugDraw::drawFrustum(const Vector3& position, float aspect, Degree FOV, float near, float far)
	{
		mDrawHelper->frustum(position, aspect, FOV, near, far);
	}

	Vector<DebugDraw::MeshRenderData> DebugDraw::createMeshProxyData(const Vector<DrawHelper::ShapeMeshData>& meshData)
	{
		Vector<MeshRenderData> proxyData;
		for (auto& entry : meshData)
		{
			if (entry.type == DrawHelper::MeshType::Solid)
				proxyData.push_back(MeshRenderData(entry.mesh->getCore(), entry.subMesh, DebugDrawMaterial::Solid));
			else if (entry.type == DrawHelper::MeshType::Wire)
				proxyData.push_back(MeshRenderData(entry.mesh->getCore(), entry.subMesh, DebugDrawMaterial::Wire));
			else if (entry.type == DrawHelper::MeshType::Line)
				proxyData.push_back(MeshRenderData(entry.mesh->getCore(), entry.subMesh, DebugDrawMaterial::Line));
		}

		return proxyData;
	}

	void DebugDraw::clear()
	{
		mDrawHelper->clear();
	}

	void DebugDraw::_update()
	{
		mActiveMeshes.clear();
		mActiveMeshes = mDrawHelper->buildMeshes(DrawHelper::SortType::None);

		Vector<MeshRenderData> proxyData = createMeshProxyData(mActiveMeshes);

		ct::DebugDrawRenderer* renderer = mRenderer.get();
		gCoreThread().queueCommand(std::bind(&ct::DebugDrawRenderer::updateData, renderer, proxyData));
	}

	namespace ct
	{

	DebugDrawParamsDef gDebugDrawParamsDef;

	DebugDrawMat::DebugDrawMat()
	{
		// Do nothing
	}

	void DebugDrawMat::execute(const SPtr<GpuParamBlockBuffer>& params, const SPtr<Mesh>& mesh, const SubMesh& subMesh)
	{
		BS_RENMAT_PROFILE_BLOCK

		mParams->setParamBlockBuffer("Params", params);

		bind();
		gRendererUtility().draw(mesh, subMesh);
	}

	DebugDrawMat* DebugDrawMat::getVariation(DebugDrawMaterial mat)
	{
		if (mat == DebugDrawMaterial::Solid)
			return get(getVariation<true, false, false>());
		
		if (mat == DebugDrawMaterial::Wire)
			return get(getVariation<false, false, true>());

		return get(getVariation<false, true, false>());
	}

	DebugDrawRenderer::DebugDrawRenderer()
		:RendererExtension(RenderLocation::PostLightPass, 0)
	{
	}

	void DebugDrawRenderer::initialize(const Any& data)
	{
		THROW_IF_NOT_CORE_THREAD;

		mParamBuffer = gDebugDrawParamsDef.createBuffer();
	}

	void DebugDrawRenderer::updateData(const Vector<DebugDraw::MeshRenderData>& meshes)
	{
		mMeshes = meshes;
	}

	RendererExtensionRequest DebugDrawRenderer::check(const Camera& camera)
	{
		return mMeshes.empty() ? RendererExtensionRequest::RenderIfTargetDirty : RendererExtensionRequest::ForceRender;
	}

	void DebugDrawRenderer::render(const Camera& camera, const RendererViewContext& viewContext)
	{
		SPtr<RenderTarget> renderTarget = camera.getViewport()->getTarget();
		if (renderTarget == nullptr)
			return;

		Matrix4 viewMatrix = camera.getViewMatrix();
		Matrix4 projMatrix = camera.getProjectionMatrixRS();
		Matrix4 viewProjMat = projMatrix * viewMatrix;

		gDebugDrawParamsDef.gMatViewProj.set(mParamBuffer, viewProjMat);
		gDebugDrawParamsDef.gViewDir.set(mParamBuffer, (Vector4)camera.getTransform().getForward());

		for (auto& entry : mMeshes)
		{
			DebugDrawMat* mat = DebugDrawMat::getVariation(entry.type);
			mat->execute(mParamBuffer, entry.mesh, entry.subMesh);
		}
	}
	}
}
