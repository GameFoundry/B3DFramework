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
#include "BsAnimationManager.h"
#include "BsSkeleton.h"
#include "BsGpuBuffer.h"

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
		Vector<SPtr<RenderableCore>> objects;

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
					objects.push_back(renderable.first->getCore());
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

		mDefaultTechniqueIdx = mat->getDefaultTechnique();
		mAnimatedTechniqueIdx = mat->findTechnique(RTag_Animated);

		assert(mDefaultTechniqueIdx < 2 && mAnimatedTechniqueIdx < 2);

		mMaterial = mat;

		for(UINT32 i = 0; i < 2 ; i++)
		{
			mParams[i] = mat->createParamsSet(i);

			SPtr<GpuParamsCore> vertParams = mParams[i]->getGpuParams(GPT_VERTEX_PROGRAM);
			vertParams->getParam("matWorldViewProj", mMatWorldViewProj[i]);

			SPtr<GpuParamsCore> fragParams = mParams[i]->getGpuParams(GPT_FRAGMENT_PROGRAM);
			fragParams->getParam("selColor", mColor[i]);
		}

		SPtr<GpuParamsCore> vertParams = mParams[mAnimatedTechniqueIdx]->getGpuParams(GPT_VERTEX_PROGRAM);
		vertParams->getBufferParam("boneMatrices", mBoneMatrices);
	}

	void SelectionRendererCore::updateData(const SPtr<CameraCore>& camera, const Vector<SPtr<RenderableCore>>& objects)
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

		const RendererAnimationData& animData = AnimationManager::instance().getRendererData();
		Matrix4 viewProjMat = mCamera->getProjectionMatrixRS() * mCamera->getViewMatrix();

		for (auto& renderable : mObjects)
		{
			SPtr<MeshCore> mesh = renderable->getMesh();
			if (mesh == nullptr)
				continue;

			Matrix4 worldViewProjMat = viewProjMat * renderable->getTransform();
			UINT32 techniqueIdx = renderable->isAnimated() ? mAnimatedTechniqueIdx : mDefaultTechniqueIdx;

			mMatWorldViewProj[techniqueIdx].set(worldViewProjMat);
			mColor[techniqueIdx].set(SELECTION_COLOR);

			if(renderable->isAnimated())
			{
				SPtr<Skeleton> skeleton = mesh->getSkeleton();
				UINT32 numBones = skeleton != nullptr ? skeleton->getNumBones() : 0;

				if (numBones > 0)
				{
					// Note: If I had access to them I could just use the buffers from the Renderer instead of creating a
					// new one every frame
					SPtr<GpuBufferCore> buffer = GpuBufferCore::create(numBones * 3, 0, GBT_STANDARD, BF_32X4F, GBU_DYNAMIC);

					auto iterFind = animData.poseInfos.find(renderable->getAnimationId());
					if (iterFind != animData.poseInfos.end())
					{
						const RendererAnimationData::PoseInfo& poseInfo = iterFind->second;

						UINT8* dest = (UINT8*)buffer->lock(0, poseInfo.numBones * 3 * sizeof(Vector4), GBL_WRITE_ONLY_DISCARD);
						for (UINT32 j = 0; j < poseInfo.numBones; j++)
						{
							const Matrix4& transform = animData.transforms[poseInfo.startIdx + j];
							memcpy(dest, &transform, 12 * sizeof(float)); // Assuming row-major format

							dest += 12 * sizeof(float);
						}

						buffer->unlock();
					}

					mBoneMatrices.set(buffer);
				}
			}

			gRendererUtility().setPass(mMaterial, 0, techniqueIdx);
			gRendererUtility().setPassParams(mParams[techniqueIdx], 0);

			UINT32 numSubmeshes = mesh->getProperties().getNumSubMeshes();

			for (UINT32 i = 0; i < numSubmeshes; i++)
				gRendererUtility().draw(mesh, mesh->getProperties().getSubMesh(i));
		}
	}
}