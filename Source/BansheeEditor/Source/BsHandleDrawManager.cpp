//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsHandleDrawManager.h"
#include "BsDrawHelper.h"
#include "BsMaterial.h"
#include "BsGpuParamsSet.h"
#include "BsBuiltinEditorResources.h"
#include "BsCoreThread.h"
#include "BsRendererManager.h"
#include "BsCoreRenderer.h"
#include "BsTransientMesh.h"
#include "BsCamera.h"
#include "BsRendererUtility.h"
#include "BsTexture.h"
#include "BsTime.h"
#include "BsRenderAPI.h"

using namespace std::placeholders;

namespace BansheeEngine
{
	const UINT32 HandleDrawManager::SPHERE_QUALITY = 1;
	const UINT32 HandleDrawManager::WIRE_SPHERE_QUALITY = 10;
	const UINT32 HandleDrawManager::ARC_QUALITY = 10;

	HandleDrawManager::HandleDrawManager()
		:mLastFrameIdx((UINT64)-1), mCore(nullptr)
	{
		mTransform = Matrix4::IDENTITY;
		mDrawHelper = bs_new<DrawHelper>();

		HMaterial solidMaterial = BuiltinEditorResources::instance().createSolidHandleMat();
		HMaterial lineMaterial = BuiltinEditorResources::instance().createLineHandleMat();
		HMaterial textMaterial = BuiltinEditorResources::instance().createTextGizmoMat();
		HMaterial clearMaterial = BuiltinEditorResources::instance().createHandleClearAlphaMat();

		SPtr<MaterialCore> solidMaterialProxy = solidMaterial->getCore();
		SPtr<MaterialCore> lineMaterialProxy = lineMaterial->getCore();
		SPtr<MaterialCore> textMaterialProxy = textMaterial->getCore();
		SPtr<MaterialCore> clearMaterialProxy = clearMaterial->getCore();

		mCore.store(bs_new<HandleDrawManagerCore>(HandleDrawManagerCore::PrivatelyConstruct()), std::memory_order_release);

		gCoreAccessor().queueCommand(std::bind(&HandleDrawManager::initializeCore, this, 
			lineMaterialProxy, solidMaterialProxy, textMaterialProxy, clearMaterialProxy));
	}

	HandleDrawManager::~HandleDrawManager()
	{
		clearMeshes();
		bs_delete(mDrawHelper);

		gCoreAccessor().queueCommand(std::bind(&HandleDrawManager::destroyCore, this, mCore.load(std::memory_order_relaxed)));
	}

	void HandleDrawManager::initializeCore(const SPtr<MaterialCore>& lineMat, const SPtr<MaterialCore>& solidMat, 
		const SPtr<MaterialCore>& textMat, const SPtr<MaterialCore>& clearMat)
	{
		THROW_IF_NOT_CORE_THREAD;

		mCore.load(std::memory_order_acquire)->initialize(lineMat, solidMat, textMat, clearMat);
	}

	void HandleDrawManager::destroyCore(HandleDrawManagerCore* core)
	{
		THROW_IF_NOT_CORE_THREAD;

		bs_delete(core);
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
		HandleDrawManagerCore* core = mCore.load(std::memory_order_relaxed);

		// Clear meshes from previous frame
		UINT64 frameIdx = gTime().getFrameIdx();
		if(frameIdx != mLastFrameIdx)
		{
			gCoreAccessor().queueCommand(std::bind(&HandleDrawManagerCore::clearQueued, core));

			clearMeshes();
			mLastFrameIdx = frameIdx;
		}

		mDrawHelper->buildMeshes(DrawHelper::SortType::BackToFront, camera->getPosition(), camera->getLayers());

		const Vector<DrawHelper::ShapeMeshData>& meshes = mDrawHelper->getMeshes();
		mActiveMeshes.push_back(meshes);

		Vector<HandleDrawManagerCore::MeshData> proxyData;
		for (auto& meshData : meshes)
		{
			SPtr<TextureCore> tex;
			if (meshData.texture.isLoaded())
				tex = meshData.texture->getCore();

			if (meshData.type == DrawHelper::MeshType::Solid)
			{
				proxyData.push_back(HandleDrawManagerCore::MeshData(
					meshData.mesh->getCore(), tex, HandleDrawManagerCore::MeshType::Solid));
			}
			else if (meshData.type == DrawHelper::MeshType::Line)
			{
				proxyData.push_back(HandleDrawManagerCore::MeshData(
					meshData.mesh->getCore(), tex, HandleDrawManagerCore::MeshType::Line));
			}
			else // Text
			{
				proxyData.push_back(HandleDrawManagerCore::MeshData(
					meshData.mesh->getCore(), tex, HandleDrawManagerCore::MeshType::Text));
			}
		}

		gCoreAccessor().queueCommand(std::bind(&HandleDrawManagerCore::queueForDraw, core, camera->getCore(), proxyData));
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

	HandleDrawManagerCore::~HandleDrawManagerCore()
	{
		clearQueued();
	}

	void HandleDrawManagerCore::initialize(const SPtr<MaterialCore>& lineMat, const SPtr<MaterialCore>& solidMat, 
		const SPtr<MaterialCore>& textMat, const SPtr<MaterialCore>& clearMat)
	{
		{
			mLineMaterial.mat = lineMat;
			mLineMaterial.params = lineMat->createParamsSet();

			SPtr<GpuParamsCore> params = mLineMaterial.params->getGpuParams();
			params->getParam(GPT_VERTEX_PROGRAM, "matViewProj", mLineMaterial.viewProj);
		}

		{
			mSolidMaterial.mat = solidMat;
			mSolidMaterial.params = solidMat->createParamsSet();

			SPtr<GpuParamsCore> params = mSolidMaterial.params->getGpuParams();
			params->getParam(GPT_VERTEX_PROGRAM, "matViewProj", mSolidMaterial.viewProj);
			params->getParam(GPT_FRAGMENT_PROGRAM, "viewDir", mSolidMaterial.viewDir);
		}
		{
			mTextMaterial.mat = textMat;
			mTextMaterial.params = textMat->createParamsSet();

			SPtr<GpuParamsCore> params = mTextMaterial.params->getGpuParams();
			params->getParam(GPT_VERTEX_PROGRAM, "matViewProj", mTextMaterial.viewProj);
			params->getTextureParam(GPT_FRAGMENT_PROGRAM, "mainTexture", mTextMaterial.texture);
		}

		{
			mClearMaterial.mat = clearMat;
		}
	}

	void HandleDrawManagerCore::queueForDraw(const SPtr<CameraCore>& camera, const Vector<MeshData>& meshes)
	{
		SPtr<CoreRenderer> activeRenderer = RendererManager::instance().getActive();
		if (camera != nullptr)
		{
			UINT32 idx = (UINT32)mQueuedData.size();
			mQueuedData.push_back({ camera, meshes });

			activeRenderer->registerRenderCallback(camera.get(), 20, std::bind(&HandleDrawManagerCore::render, this, idx));
		}
	}

	void HandleDrawManagerCore::clearQueued()
	{
		SPtr<CoreRenderer> activeRenderer = RendererManager::instance().getActive();
		for (auto& entry : mQueuedData)
			activeRenderer->unregisterRenderCallback(entry.camera.get(), 20);

		mQueuedData.clear();
	}

	void HandleDrawManagerCore::render(UINT32 queuedDataIdx)
	{
		THROW_IF_NOT_CORE_THREAD;

		const QueuedData& queueData = mQueuedData[queuedDataIdx];
		SPtr<CameraCore> camera = queueData.camera;
		const Vector<MeshData>& meshes = queueData.meshes;

		SPtr<RenderTargetCore> renderTarget = camera->getViewport()->getTarget();

		float width = (float)renderTarget->getProperties().getWidth();
		float height = (float)renderTarget->getProperties().getHeight();

		Rect2 normArea = camera->getViewport()->getNormArea();

		Rect2I screenArea;
		screenArea.x = (int)(normArea.x * width);
		screenArea.y = (int)(normArea.y * height);
		screenArea.width = (int)(normArea.width * width);
		screenArea.height = (int)(normArea.height * height);

		Matrix4 viewProjMat = camera->getProjectionMatrixRS() * camera->getViewMatrix();
		mSolidMaterial.viewProj.set(viewProjMat);
		mSolidMaterial.viewDir.set((Vector4)camera->getForward());
		mLineMaterial.viewProj.set(viewProjMat);
		mTextMaterial.viewProj.set(viewProjMat);

		MeshType currentType = MeshType::Solid;
		if (meshes.size() > 0)
		{
			currentType = meshes[0].type;

			if (currentType == MeshType::Solid)
			{
				gRendererUtility().setPass(mSolidMaterial.mat);
				gRendererUtility().setPassParams(mSolidMaterial.params);
			}
			else if (currentType == MeshType::Line)
			{
				gRendererUtility().setPass(mLineMaterial.mat);
				gRendererUtility().setPassParams(mLineMaterial.params);
			}
			else
			{
				mTextMaterial.texture.set(meshes[0].texture);
				gRendererUtility().setPass(mTextMaterial.mat);
				gRendererUtility().setPassParams(mTextMaterial.params);
			}
		}

		for (auto& meshData : meshes)
		{
			if (currentType != meshData.type)
			{
				if (meshData.type == MeshType::Solid)
				{
					gRendererUtility().setPass(mSolidMaterial.mat);
					gRendererUtility().setPassParams(mSolidMaterial.params);
				}
				else if (meshData.type == MeshType::Line)
				{
					gRendererUtility().setPass(mLineMaterial.mat);
					gRendererUtility().setPassParams(mLineMaterial.params);
				}
				else
				{
					mTextMaterial.texture.set(meshData.texture);
					gRendererUtility().setPass(mTextMaterial.mat);
					gRendererUtility().setPassParams(mTextMaterial.params);
				}

				currentType = meshData.type;
			}

			gRendererUtility().draw(meshData.mesh, meshData.mesh->getProperties().getSubMesh(0));
		}

		// Set alpha of everything that was drawn to 1 so we can overlay this texture onto GUI using transparency
		gRendererUtility().setPass(mClearMaterial.mat, 0);
		gRendererUtility().drawScreenQuad();
	}
}