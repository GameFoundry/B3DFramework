//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsSelectionRenderer.h"
#include "BsMesh.h"
#include "BsVertexDataDesc.h"
#include "BsCCamera.h"
#include "BsCoreThread.h"
#include "BsBuiltinEditorResources.h"
#include "BsMaterial.h"
#include "BsGpuParams.h"
#include "BsGpuParamsSet.h"
#include "BsRenderAPI.h"
#include "BsCoreRenderer.h"
#include "BsRendererManager.h"
#include "BsSelection.h"
#include "BsSceneObject.h"
#include "BsCRenderable.h"
#include "BsRenderable.h"
#include "BsSceneManager.h"
#include "BsRendererUtility.h"

using namespace std::placeholders;

namespace BansheeEngine
{
	SelectionRenderer::SelectionRenderer()
		:mCore(nullptr)
	{
		HMaterial selectionMat = BuiltinEditorResources::instance().createSelectionMat();
			
		mCore.store(bs_new<SelectionRendererCore>(SelectionRendererCore::PrivatelyConstuct()), std::memory_order_release);

		gCoreAccessor().queueCommand(std::bind(&SelectionRenderer::initializeCore, this, selectionMat->getCore()));
	}

	SelectionRenderer::~SelectionRenderer()
	{
		gCoreAccessor().queueCommand(std::bind(&SelectionRenderer::destroyCore, this, mCore.load(std::memory_order_relaxed)));
	}

	void SelectionRenderer::initializeCore(const SPtr<MaterialCore>& initData)
	{
		mCore.load(std::memory_order_acquire)->initialize(initData);
	}

	void SelectionRenderer::destroyCore(SelectionRendererCore* core)
	{
		bs_delete(core);
	}

	void SelectionRenderer::update(const SPtr<Camera>& camera)
	{
		Vector<ObjectData> objects;

		const Vector<HSceneObject>& sceneObjects = Selection::instance().getSceneObjects();
		const Map<Renderable*, SceneRenderableData>& renderables = SceneManager::instance().getAllRenderables();

		for (auto& renderable : renderables)
		{
			for (auto& so : sceneObjects)
			{
				if (!so->getActive())
					continue;

				if (renderable.second.sceneObject != so)
					continue;

				if (renderable.first->getMesh().isLoaded())
				{
					objects.push_back(ObjectData());

					ObjectData& newObjData = objects.back();
					newObjData.worldTfrm = so->getWorldTfrm();
					newObjData.mesh = renderable.first->getMesh()->getCore();
				}
			}
		}

		SelectionRendererCore* core = mCore.load(std::memory_order_relaxed);
		gCoreAccessor().queueCommand(std::bind(&SelectionRendererCore::updateData, core, camera->getCore(), objects));
	}

	const Color SelectionRendererCore::SELECTION_COLOR = Color(1.0f, 1.0f, 1.0f, 0.3f);

	SelectionRendererCore::SelectionRendererCore(const PrivatelyConstuct& dummy)
	{
	}

	SelectionRendererCore::~SelectionRendererCore()
	{
		SPtr<CoreRenderer> activeRenderer = RendererManager::instance().getActive();
		if (mCamera != nullptr)
			activeRenderer->unregisterRenderCallback(mCamera.get(), 10);
	}

	void SelectionRendererCore::initialize(const SPtr<MaterialCore>& mat)
	{
		THROW_IF_NOT_CORE_THREAD;

		mMaterial = mat;
		mParams = mat->createParamsSet();

		SPtr<GpuParamsCore> vertParams = mParams->getGpuParams(GPT_VERTEX_PROGRAM);
		vertParams->getParam("matWorldViewProj", mMatWorldViewProj);

		SPtr<GpuParamsCore> fragParams = mParams->getGpuParams(GPT_FRAGMENT_PROGRAM);
		fragParams->getParam("selColor", mColor);
	}

	void SelectionRendererCore::updateData(const SPtr<CameraCore>& camera, const Vector<SelectionRenderer::ObjectData>& objects)
	{
		if (mCamera != camera)
		{
			SPtr<CoreRenderer> activeRenderer = RendererManager::instance().getActive();
			if (mCamera != nullptr)
				activeRenderer->unregisterRenderCallback(mCamera.get(), 10);

			if (camera != nullptr)
				activeRenderer->registerRenderCallback(camera.get(), 10, std::bind(&SelectionRendererCore::render, this));
		}

		mCamera = camera;
		mObjects = objects;
	}

	void SelectionRendererCore::render()
	{
		THROW_IF_NOT_CORE_THREAD;

		if (mCamera == nullptr)
			return;

		Matrix4 viewProjMat = mCamera->getProjectionMatrixRS() * mCamera->getViewMatrix();

		for (auto& objData : mObjects)
		{
			Matrix4 worldViewProjMat = viewProjMat * objData.worldTfrm;

			mMatWorldViewProj.set(worldViewProjMat);
			mColor.set(SELECTION_COLOR);

			gRendererUtility().setPass(mMaterial, 0);
			gRendererUtility().setPassParams(mParams, 0);

			UINT32 numSubmeshes = objData.mesh->getProperties().getNumSubMeshes();

			for (UINT32 i = 0; i < numSubmeshes; i++)
				gRendererUtility().draw(objData.mesh, objData.mesh->getProperties().getSubMesh(i));
		}
	}
}