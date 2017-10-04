//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "Handles/BsHandleDrawManager.h"
#include "Utility/BsDrawHelper.h"
#include "Material/BsMaterial.h"
#include "Material/BsGpuParamsSet.h"
#include "Utility/BsBuiltinEditorResources.h"
#include "CoreThread/BsCoreThread.h"
#include "Renderer/BsRendererManager.h"
#include "Renderer/BsRenderer.h"
#include "Mesh/BsTransientMesh.h"
#include "Renderer/BsCamera.h"
#include "Renderer/BsRendererUtility.h"
#include "Image/BsTexture.h"
#include "Utility/BsTime.h"
#include "RenderAPI/BsRenderAPI.h"

using namespace std::placeholders;

namespace bs
{
	const UINT32 HandleDrawManager::SPHERE_QUALITY = 1;
	const UINT32 HandleDrawManager::WIRE_SPHERE_QUALITY = 10;
	const UINT32 HandleDrawManager::ARC_QUALITY = 10;

	HandleDrawManager::HandleDrawManager()
		:mLastFrameIdx((UINT64)-1)
	{
		mTransform = Matrix4::IDENTITY;
		mDrawHelper = bs_new<DrawHelper>();

		HMaterial solidMaterial = BuiltinEditorResources::instance().createSolidHandleMat();
		HMaterial lineMaterial = BuiltinEditorResources::instance().createLineHandleMat();
		HMaterial textMaterial = BuiltinEditorResources::instance().createTextGizmoMat();
		HMaterial clearMaterial = BuiltinEditorResources::instance().createHandleClearAlphaMat();

		ct::HandleRenderer::InitData rendererInitData;
		rendererInitData.solidMat = solidMaterial->getCore();
		rendererInitData.lineMat = lineMaterial->getCore();
		rendererInitData.textMat = textMaterial->getCore();
		rendererInitData.clearMat = clearMaterial->getCore();

		mRenderer = RendererExtension::create<ct::HandleRenderer>(rendererInitData);
	}

	HandleDrawManager::~HandleDrawManager()
	{
		clearMeshes();
		bs_delete(mDrawHelper);
	}

	void HandleDrawManager::setColor(const Color& color)
	{
		mDrawHelper->setColor(color);
	}

	void HandleDrawManager::setTransform(const Matrix4& transform)
	{
		mTransform = transform;
	}

	void HandleDrawManager::setLayer(UINT64 layer)
	{
		mDrawHelper->setLayer(layer);
	}

	void HandleDrawManager::drawCube(const Vector3& position, const Vector3& extents, float size)
	{
		Matrix4 scale = Matrix4::scaling(size);
		mDrawHelper->setTransform(mTransform * scale);

		mDrawHelper->cube(position, extents);
	}

	void HandleDrawManager::drawSphere(const Vector3& position, float radius, float size)
	{
		Matrix4 scale = Matrix4::scaling(size);
		mDrawHelper->setTransform(mTransform * scale);

		mDrawHelper->sphere(position, radius);
	}

	void HandleDrawManager::drawWireCube(const Vector3& position, const Vector3& extents, float size)
	{
		Matrix4 scale = Matrix4::scaling(size);
		mDrawHelper->setTransform(mTransform * scale);

		mDrawHelper->wireCube(position, extents);
	}

	void HandleDrawManager::drawWireSphere(const Vector3& position, float radius, float size)
	{
		Matrix4 scale = Matrix4::scaling(size);
		mDrawHelper->setTransform(mTransform * scale);

		mDrawHelper->wireSphere(position, radius);
	}

	void HandleDrawManager::drawCone(const Vector3& base, const Vector3& normal, float height, float radius, float size)
	{
		Matrix4 scale = Matrix4::scaling(size);

		mDrawHelper->setTransform(mTransform * scale);
		mDrawHelper->cone(base, normal, height, radius);
	}

	void HandleDrawManager::drawLine(const Vector3& start, const Vector3& end, float size)
	{
		Matrix4 scale = Matrix4::scaling(size);
		mDrawHelper->setTransform(mTransform * scale);

		mDrawHelper->line(start, end);
	}

	void HandleDrawManager::drawDisc(const Vector3& position, const Vector3& normal, float radius, float size)
	{
		Matrix4 scale = Matrix4::scaling(size);
		mDrawHelper->setTransform(mTransform * scale);

		mDrawHelper->disc(position, normal, radius);
	}

	void HandleDrawManager::drawWireDisc(const Vector3& position, const Vector3& normal, float radius, float size)
	{
		Matrix4 scale = Matrix4::scaling(size);
		mDrawHelper->setTransform(mTransform * scale);

		mDrawHelper->wireDisc(position, normal, radius);
	}

	void HandleDrawManager::drawArc(const Vector3& position, const Vector3& normal, float radius, Degree startAngle, Degree amountAngle, float size)
	{
		Matrix4 scale = Matrix4::scaling(size);
		mDrawHelper->setTransform(mTransform * scale);

		mDrawHelper->arc(position, normal, radius, startAngle, amountAngle);
	}

	void HandleDrawManager::drawWireArc(const Vector3& position, const Vector3& normal, float radius, Degree startAngle, Degree amountAngle, float size)
	{
		Matrix4 scale = Matrix4::scaling(size);
		mDrawHelper->setTransform(mTransform * scale);

		mDrawHelper->wireArc(position, normal, radius, startAngle, amountAngle);
	}

	void HandleDrawManager::drawRect(const Rect3& area, float size)
	{
		Matrix4 scale = Matrix4::scaling(size);
		mDrawHelper->setTransform(mTransform * scale);

		mDrawHelper->rectangle(area);
	}

	void HandleDrawManager::drawText(const Vector3& position, const WString& text, const HFont& font, UINT32 fontSize, float size)
	{
		Matrix4 scale = Matrix4::scaling(size);
		mDrawHelper->setTransform(mTransform * scale);

		HFont myFont = font;
		if (myFont == nullptr)
			myFont = BuiltinEditorResources::instance().getDefaultAAFont();

		mDrawHelper->text(position, text, myFont, fontSize);
	}

	void HandleDrawManager::draw(const SPtr<Camera>& camera)
	{
		ct::HandleRenderer* renderer = mRenderer.get();

		// Clear meshes from previous frame
		UINT64 frameIdx = gTime().getFrameIdx();
		if(frameIdx != mLastFrameIdx)
		{
			gCoreThread().queueCommand(std::bind(&ct::HandleRenderer::clearQueued, renderer));

			clearMeshes();
			mLastFrameIdx = frameIdx;
		}

		mDrawHelper->buildMeshes(DrawHelper::SortType::BackToFront, camera->getPosition(), camera->getLayers());

		const Vector<DrawHelper::ShapeMeshData>& meshes = mDrawHelper->getMeshes();
		mActiveMeshes.push_back(meshes);

		Vector<ct::HandleRenderer::MeshData> proxyData;
		for (auto& meshData : meshes)
		{
			SPtr<ct::Texture> tex;
			if (meshData.texture.isLoaded())
				tex = meshData.texture->getCore();

			if (meshData.type == DrawHelper::MeshType::Solid)
			{
				proxyData.push_back(ct::HandleRenderer::MeshData(
					meshData.mesh->getCore(), tex, ct::HandleRenderer::MeshType::Solid));
			}
			else if (meshData.type == DrawHelper::MeshType::Line)
			{
				proxyData.push_back(ct::HandleRenderer::MeshData(
					meshData.mesh->getCore(), tex, ct::HandleRenderer::MeshType::Line));
			}
			else // Text
			{
				proxyData.push_back(ct::HandleRenderer::MeshData(
					meshData.mesh->getCore(), tex, ct::HandleRenderer::MeshType::Text));
			}
		}

		gCoreThread().queueCommand(std::bind(&ct::HandleRenderer::queueForDraw, renderer, camera->getCore(), proxyData));
	}

	void HandleDrawManager::clear()
	{
		mDrawHelper->clear();
	}

	void HandleDrawManager::clearMeshes()
	{
		for (auto entry : mActiveMeshes)
			mDrawHelper->clearMeshes(entry);

		mActiveMeshes.clear();
	}

	namespace ct
	{
	HandleParamBlockDef gHandleParamBlockDef;

	HandleRenderer::HandleRenderer()
		:RendererExtension(RenderLocation::PostLightPass, -20), mTypeCounters()
	{ }

	void HandleRenderer::initialize(const Any& data)
	{
		const InitData& initData = any_cast_ref<InitData>(data);

		mMaterials[(UINT32)MeshType::Line] = initData.lineMat;
		mMaterials[(UINT32)MeshType::Solid] = initData.solidMat;
		mMaterials[(UINT32)MeshType::Text] = initData.textMat;

		mClearMaterial = initData.clearMat;

		mParamBuffer = gHandleParamBlockDef.createBuffer();
	}

	void HandleRenderer::destroy()
	{
		clearQueued();
	}

	void HandleRenderer::queueForDraw(const SPtr<Camera>& camera, Vector<MeshData>& meshes)
	{
		SPtr<Renderer> activeRenderer = RendererManager::instance().getActive();
		if (camera != nullptr)
		{
			for(auto& entry : meshes)
			{
				UINT32 typeIdx = (UINT32)entry.type;
				UINT32 paramsIdx = mTypeCounters[typeIdx];

				entry.paramIdx = paramsIdx;

				SPtr<GpuParamsSet> paramsSet;
				if (paramsIdx >= mParamSets[typeIdx].size())
				{
					paramsSet = mMaterials[typeIdx]->createParamsSet();
					paramsSet->setParamBlockBuffer("Uniforms", mParamBuffer, true);

					mParamSets[typeIdx].push_back(paramsSet);
				}
				else
					paramsSet = mParamSets[typeIdx][entry.paramIdx];

				if(entry.type == MeshType::Text)
				{
					GpuParamTexture texture;

					paramsSet->getGpuParams()->getTextureParam(GPT_FRAGMENT_PROGRAM, "gMainTexture", texture);
					texture.set(entry.texture);
				}

				mTypeCounters[typeIdx]++;
			}

			mQueuedData.push_back({ camera, meshes });
		}
	}

	void HandleRenderer::clearQueued()
	{
		mQueuedData.clear();
		bs_zero_out(mTypeCounters);
	}

	bool HandleRenderer::check(const Camera& camera)
	{
		for(auto& entry : mQueuedData)
		{
			if (entry.camera.get() == &camera)
				return true;
		}

		return false;
	}

	void HandleRenderer::render(const Camera& camera)
	{
		THROW_IF_NOT_CORE_THREAD;

		for (auto& entry : mQueuedData)
		{
			if (entry.camera.get() != &camera)
				continue;

			const QueuedData& queueData = entry;
			const Vector<MeshData>& meshes = queueData.meshes;

			SPtr<RenderTarget> renderTarget = camera.getViewport()->getTarget();

			float width = (float)renderTarget->getProperties().width;
			float height = (float)renderTarget->getProperties().height;

			Rect2 normArea = camera.getViewport()->getArea();

			Rect2I screenArea;
			screenArea.x = (int)(normArea.x * width);
			screenArea.y = (int)(normArea.y * height);
			screenArea.width = (int)(normArea.width * width);
			screenArea.height = (int)(normArea.height * height);

			Matrix4 viewProjMat = camera.getProjectionMatrixRS() * camera.getViewMatrix();

			gHandleParamBlockDef.gMatViewProj.set(mParamBuffer, viewProjMat);
			gHandleParamBlockDef.gViewDir.set(mParamBuffer, (Vector4)camera.getForward());

			UINT32 currentType = -1;
			for (auto& meshData : meshes)
			{
				UINT32 typeIdx = (UINT32)meshData.type;
				if (currentType != typeIdx)
				{
					gRendererUtility().setPass(mMaterials[typeIdx]);
					currentType = typeIdx;
				}

				gRendererUtility().setPassParams(mParamSets[typeIdx][meshData.paramIdx]);
				gRendererUtility().draw(meshData.mesh, meshData.mesh->getProperties().getSubMesh(0));
			}

			// Set alpha of everything that was drawn to 1 so we can overlay this texture onto GUI using transparency
			gRendererUtility().setPass(mClearMaterial, 0);
			gRendererUtility().drawScreenQuad();
		}
	}
	}
}