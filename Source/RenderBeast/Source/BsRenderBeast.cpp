//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsRenderBeast.h"
#include "BsCCamera.h"
#include "BsCRenderable.h"
#include "BsMaterial.h"
#include "BsMesh.h"
#include "BsPass.h"
#include "BsSamplerState.h"
#include "BsCoreApplication.h"
#include "BsViewport.h"
#include "BsRenderTarget.h"
#include "BsRenderQueue.h"
#include "BsCoreThread.h"
#include "BsGpuParams.h"
#include "BsProfilerCPU.h"
#include "BsProfilerGPU.h"
#include "BsShader.h"
#include "BsGpuParamBlockBuffer.h"
#include "BsTime.h"
#include "BsRenderableElement.h"
#include "BsCoreObjectManager.h"
#include "BsRenderBeastOptions.h"
#include "BsSamplerOverrides.h"
#include "BsLight.h"
#include "BsRenderTexturePool.h"
#include "BsRenderTargets.h"
#include "BsRendererUtility.h"
#include "BsAnimationManager.h"
#include "BsSkeleton.h"
#include "BsGpuBuffer.h"
#include "BsGpuParamsSet.h"
#include "BsRendererExtension.h"
#include "BsReflectionCubemap.h"
#include "BsMeshData.h"

using namespace std::placeholders;

namespace bs { namespace ct
{
	RenderBeast::RenderBeast()
		: mDefaultMaterial(nullptr), mPointLightInMat(nullptr), mPointLightOutMat(nullptr), mDirLightMat(nullptr)
		, mSkyboxMat(nullptr), mObjectRenderer(nullptr), mOptions(bs_shared_ptr_new<RenderBeastOptions>())
		, mOptionsDirty(true)
	{ }

	const StringID& RenderBeast::getName() const
	{
		static StringID name = "RenderBeast";
		return name;
	}

	void RenderBeast::initialize()
	{
		CoreRenderer::initialize();

		gCoreThread().queueCommand(std::bind(&RenderBeast::initializeCore, this), CTQF_InternalQueue);
	}

	void RenderBeast::destroy()
	{
		CoreRenderer::destroy();

		gCoreThread().queueCommand(std::bind(&RenderBeast::destroyCore, this));
		gCoreThread().submit(true);
	}

	void RenderBeast::initializeCore()
	{
		RendererUtility::startUp();

		mCoreOptions = bs_shared_ptr_new<RenderBeastOptions>();
		mObjectRenderer = bs_new<ObjectRenderer>();

		mDefaultMaterial = bs_new<DefaultMaterial>();
		mPointLightInMat = bs_new<PointLightInMat>();
		mPointLightOutMat = bs_new<PointLightOutMat>();
		mDirLightMat = bs_new<DirectionalLightMat>();
		mSkyboxMat = bs_new<SkyboxMat>();

		RenderTexturePool::startUp();
		PostProcessing::startUp();
	}

	void RenderBeast::destroyCore()
	{
		if (mObjectRenderer != nullptr)
			bs_delete(mObjectRenderer);

		for (auto& entry : mRenderables)
			bs_delete(entry);

		for (auto& entry : mCameras)
			bs_delete(entry.second);

		mRenderTargets.clear();
		mCameras.clear();
		mRenderables.clear();
		mVisibility.clear();

		PostProcessing::shutDown();
		RenderTexturePool::shutDown();

		bs_delete(mDefaultMaterial);
		bs_delete(mPointLightInMat);
		bs_delete(mPointLightOutMat);
		bs_delete(mDirLightMat);
		bs_delete(mSkyboxMat);

		RendererUtility::shutDown();

		assert(mSamplerOverrides.empty());
	}

	void RenderBeast::notifyRenderableAdded(Renderable* renderable)
	{
		UINT32 renderableId = (UINT32)mRenderables.size();

		renderable->setRendererId(renderableId);

		mRenderables.push_back(bs_new<RendererObject>());
		mWorldBounds.push_back(renderable->getBounds());
		mVisibility.push_back(false);

		RendererObject* rendererObject = mRenderables.back();
		rendererObject->renderable = renderable;
		rendererObject->updatePerObjectBuffer();

		SPtr<Mesh> mesh = renderable->getMesh();
		if (mesh != nullptr)
		{
			const MeshProperties& meshProps = mesh->getProperties();
			SPtr<VertexDeclaration> vertexDecl = mesh->getVertexData()->vertexDeclaration;

			for (UINT32 i = 0; i < meshProps.getNumSubMeshes(); i++)
			{
				rendererObject->elements.push_back(BeastRenderableElement());
				BeastRenderableElement& renElement = rendererObject->elements.back();

				renElement.mesh = mesh;
				renElement.subMesh = meshProps.getSubMesh(i);
				renElement.renderableId = renderableId;
				renElement.animType = renderable->getAnimType();
				renElement.animationId = renderable->getAnimationId();
				renElement.morphShapeVersion = 0;
				renElement.morphShapeBuffer = renderable->getMorphShapeBuffer();
				renElement.boneMatrixBuffer = renderable->getBoneMatrixBuffer();
				renElement.morphVertexDeclaration = renderable->getMorphVertexDeclaration();

				renElement.material = renderable->getMaterial(i);
				if (renElement.material == nullptr)
					renElement.material = renderable->getMaterial(0);

				if (renElement.material != nullptr && renElement.material->getShader() == nullptr)
					renElement.material = nullptr;

				// If no material use the default material
				if (renElement.material == nullptr)
					renElement.material = mDefaultMaterial->getMaterial();

				// Determine which technique to use
				static StringID techniqueIDLookup[4] = { StringID::NONE, RTag_Skinned, RTag_Morph, RTag_SkinnedMorph };
				static_assert((UINT32)RenderableAnimType::Count == 4, "RenderableAnimType is expected to have four sequential entries.");
				
				UINT32 techniqueIdx = -1;
				RenderableAnimType animType = renderable->getAnimType();
				if(animType != RenderableAnimType::None)
					techniqueIdx = renElement.material->findTechnique(techniqueIDLookup[(int)animType]);

				if (techniqueIdx == (UINT32)-1)
					techniqueIdx = renElement.material->getDefaultTechnique();

				renElement.techniqueIdx = techniqueIdx;

				// Validate mesh <-> shader vertex bindings
				if (renElement.material != nullptr)
				{
					UINT32 numPasses = renElement.material->getNumPasses(techniqueIdx);
					for (UINT32 j = 0; j < numPasses; j++)
					{
						SPtr<Pass> pass = renElement.material->getPass(j, techniqueIdx);

						SPtr<VertexDeclaration> shaderDecl = pass->getVertexProgram()->getInputDeclaration();
						if (!vertexDecl->isCompatible(shaderDecl))
						{
							Vector<VertexElement> missingElements = vertexDecl->getMissingElements(shaderDecl);

							// If using morph shapes ignore POSITION1 and NORMAL1 missing since we assign them from within the renderer
							if(animType == RenderableAnimType::Morph || animType == RenderableAnimType::SkinnedMorph)
							{
								auto removeIter = std::remove_if(missingElements.begin(), missingElements.end(), [](const VertexElement& x)
								{
									return (x.getSemantic() == VES_POSITION && x.getSemanticIdx() == 1) ||
										(x.getSemantic() == VES_NORMAL && x.getSemanticIdx() == 1);
								});

								missingElements.erase(removeIter, missingElements.end());
							}

							if (!missingElements.empty())
							{
								StringStream wrnStream;
								wrnStream << "Provided mesh is missing required vertex attributes to render with the provided shader. Missing elements: " << std::endl;

								for (auto& entry : missingElements)
									wrnStream << "\t" << toString(entry.getSemantic()) << entry.getSemanticIdx() << std::endl;

								LOGWRN(wrnStream.str());
								break;
							}
						}
					}
				}

				// Generate or assigned renderer specific data for the material
				renElement.params = renElement.material->createParamsSet(techniqueIdx);
				renElement.material->updateParamsSet(renElement.params, true);

				// Generate or assign sampler state overrides
				SamplerOverrideKey samplerKey(renElement.material, techniqueIdx);
				auto iterFind = mSamplerOverrides.find(samplerKey);
				if (iterFind != mSamplerOverrides.end())
				{
					renElement.samplerOverrides = iterFind->second;
					iterFind->second->refCount++;
				}
				else
				{
					SPtr<Shader> shader = renElement.material->getShader();
					MaterialSamplerOverrides* samplerOverrides = SamplerOverrideUtility::generateSamplerOverrides(shader,
						renElement.material->_getInternalParams(), renElement.params, mCoreOptions);

					mSamplerOverrides[samplerKey] = samplerOverrides;

					renElement.samplerOverrides = samplerOverrides;
					samplerOverrides->refCount++;
				}

				mObjectRenderer->initElement(*rendererObject, renElement);
			}
		}
	}

	void RenderBeast::notifyRenderableRemoved(Renderable* renderable)
	{
		UINT32 renderableId = renderable->getRendererId();
		Renderable* lastRenerable = mRenderables.back()->renderable;
		UINT32 lastRenderableId = lastRenerable->getRendererId();

		RendererObject* rendererObject = mRenderables[renderableId];
		Vector<BeastRenderableElement>& elements = rendererObject->elements;
		for (auto& element : elements)
		{
			SamplerOverrideKey samplerKey(element.material, element.techniqueIdx);

			auto iterFind = mSamplerOverrides.find(samplerKey);
			assert(iterFind != mSamplerOverrides.end());

			MaterialSamplerOverrides* samplerOverrides = iterFind->second;
			samplerOverrides->refCount--;
			if (samplerOverrides->refCount == 0)
			{
				SamplerOverrideUtility::destroySamplerOverrides(samplerOverrides);
				mSamplerOverrides.erase(iterFind);
			}

			element.samplerOverrides = nullptr;
		}

		if (renderableId != lastRenderableId)
		{
			// Swap current last element with the one we want to erase
			std::swap(mRenderables[renderableId], mRenderables[lastRenderableId]);
			std::swap(mWorldBounds[renderableId], mWorldBounds[lastRenderableId]);

			lastRenerable->setRendererId(renderableId);

			for (auto& element : elements)
				element.renderableId = renderableId;
		}

		// Last element is the one we want to erase
		mRenderables.erase(mRenderables.end() - 1);
		mWorldBounds.erase(mWorldBounds.end() - 1);
		mVisibility.erase(mVisibility.end() - 1);

		bs_delete(rendererObject);
	}

	void RenderBeast::notifyRenderableUpdated(Renderable* renderable)
	{
		UINT32 renderableId = renderable->getRendererId();

		mRenderables[renderableId]->updatePerObjectBuffer();
		mWorldBounds[renderableId] = renderable->getBounds();
	}

	void RenderBeast::notifyLightAdded(Light* light)
	{
		if (light->getType() == LightType::Directional)
		{
			UINT32 lightId = (UINT32)mDirectionalLights.size();
			light->setRendererId(lightId);

			mDirectionalLights.push_back(RendererLight());

			RendererLight& lightData = mDirectionalLights.back();
			lightData.internal = light;
		}
		else
		{
			UINT32 lightId = (UINT32)mPointLights.size();

			light->setRendererId(lightId);

			mPointLights.push_back(RendererLight());
			mLightWorldBounds.push_back(light->getBounds());

			RendererLight& lightData = mPointLights.back();
			lightData.internal = light;
		}
	}

	void RenderBeast::notifyLightUpdated(Light* light)
	{
		UINT32 lightId = light->getRendererId();

		if (light->getType() != LightType::Directional)
			mLightWorldBounds[lightId] = light->getBounds();
	}

	void RenderBeast::notifyLightRemoved(Light* light)
	{
		UINT32 lightId = light->getRendererId();
		if (light->getType() == LightType::Directional)
		{
			Light* lastLight = mDirectionalLights.back().internal;
			UINT32 lastLightId = lastLight->getRendererId();

			if (lightId != lastLightId)
			{
				// Swap current last element with the one we want to erase
				std::swap(mDirectionalLights[lightId], mDirectionalLights[lastLightId]);
				lastLight->setRendererId(lightId);
			}

			// Last element is the one we want to erase
			mDirectionalLights.erase(mDirectionalLights.end() - 1);
		}
		else
		{
			Light* lastLight = mPointLights.back().internal;
			UINT32 lastLightId = lastLight->getRendererId();

			if (lightId != lastLightId)
			{
				// Swap current last element with the one we want to erase
				std::swap(mPointLights[lightId], mPointLights[lastLightId]);
				std::swap(mLightWorldBounds[lightId], mLightWorldBounds[lastLightId]);

				lastLight->setRendererId(lightId);
			}

			// Last element is the one we want to erase
			mPointLights.erase(mPointLights.end() - 1);
			mLightWorldBounds.erase(mLightWorldBounds.end() - 1);
		}
	}

	void RenderBeast::notifyCameraAdded(const Camera* camera)
	{
		RendererCamera* renCamera = updateCameraData(camera);
		renCamera->updatePerViewBuffer();
	}

	void RenderBeast::notifyCameraUpdated(const Camera* camera, UINT32 updateFlag)
	{
		RendererCamera* rendererCam;
		if((updateFlag & (UINT32)CameraDirtyFlag::Everything) != 0)
		{
			rendererCam = updateCameraData(camera);
		}
		else if((updateFlag & (UINT32)CameraDirtyFlag::PostProcess) != 0)
		{
			rendererCam = mCameras[camera];

			rendererCam->setPostProcessSettings(camera->getPostProcessSettings());
		}
		else // Transform
		{
			rendererCam = mCameras[camera];

			rendererCam->setTransform(
				camera->getPosition(),
				camera->getForward(),
				camera->getViewMatrix(),
				camera->getProjectionMatrixRS());
		}

		rendererCam->updatePerViewBuffer();
	}

	void RenderBeast::notifyCameraRemoved(const Camera* camera)
	{
		updateCameraData(camera, true);
	}

	SPtr<PostProcessSettings> RenderBeast::createPostProcessSettings() const
	{
		return bs_shared_ptr_new<StandardPostProcessSettings>();
	}

	RendererCamera* RenderBeast::updateCameraData(const Camera* camera, bool forceRemove)
	{
		RendererCamera* output;

		SPtr<RenderTarget> renderTarget = camera->getViewport()->getTarget();

		auto iterFind = mCameras.find(camera);
		if(forceRemove)
		{
			if(iterFind != mCameras.end())
			{
				bs_delete(iterFind->second);
				mCameras.erase(iterFind);
			}

			renderTarget = nullptr;
			output = nullptr;
		}
		else
		{
			SPtr<Viewport> viewport = camera->getViewport();
			RENDERER_VIEW_DESC viewDesc;

			viewDesc.target.clearFlags = 0;
			if (viewport->getRequiresColorClear())
				viewDesc.target.clearFlags |= FBT_COLOR;

			if (viewport->getRequiresDepthClear())
				viewDesc.target.clearFlags |= FBT_DEPTH;

			if (viewport->getRequiresStencilClear())
				viewDesc.target.clearFlags |= FBT_STENCIL;

			viewDesc.target.clearColor = viewport->getClearColor();
			viewDesc.target.clearDepthValue = viewport->getClearDepthValue();
			viewDesc.target.clearStencilValue = viewport->getClearStencilValue();

			viewDesc.target.target = viewport->getTarget();
			viewDesc.target.nrmViewRect = viewport->getNormArea();
			viewDesc.target.viewRect = Rect2I(
				viewport->getX(),
				viewport->getY(),
				(UINT32)viewport->getWidth(),
				(UINT32)viewport->getHeight());

			if (viewDesc.target.target != nullptr)
			{
				viewDesc.target.targetWidth = viewDesc.target.target->getProperties().getWidth();
				viewDesc.target.targetHeight = viewDesc.target.target->getProperties().getHeight();
			}
			else
			{
				viewDesc.target.targetWidth = 0;
				viewDesc.target.targetHeight = 0;
			}

			viewDesc.target.numSamples = camera->getMSAACount();

			viewDesc.isOverlay = camera->getFlags().isSet(CameraFlag::Overlay);
			viewDesc.isHDR = camera->getFlags().isSet(CameraFlag::HDR);
			viewDesc.triggerCallbacks = true;
			viewDesc.runPostProcessing = true;

			viewDesc.cullFrustum = camera->getWorldFrustum();
			viewDesc.visibleLayers = camera->getLayers();
			viewDesc.nearPlane = camera->getNearClipDistance();
			viewDesc.flipView = false;

			viewDesc.viewOrigin = camera->getPosition();
			viewDesc.viewDirection = camera->getForward();
			viewDesc.projTransform = camera->getProjectionMatrixRS();
			viewDesc.viewTransform = camera->getViewMatrix();

			viewDesc.stateReduction = mCoreOptions->stateReductionMode;
			viewDesc.skyboxTexture = camera->getSkybox();
			viewDesc.sceneCamera = camera;

			if (iterFind != mCameras.end())
			{
				output = iterFind->second;
				output->setView(viewDesc);
			}
			else
			{
				output = bs_new<RendererCamera>(viewDesc);
				mCameras[camera] = output;
			}

			output->setPostProcessSettings(camera->getPostProcessSettings());
		}

		// Remove from render target list
		int rtChanged = 0; // 0 - No RT, 1 - RT found, 2 - RT changed
		for (auto iterTarget = mRenderTargets.begin(); iterTarget != mRenderTargets.end(); ++iterTarget)
		{
			RendererRenderTarget& target = *iterTarget;
			for (auto iterCam = target.cameras.begin(); iterCam != target.cameras.end(); ++iterCam)
			{
				if (camera == *iterCam)
				{
					if (renderTarget != target.target)
					{
						target.cameras.erase(iterCam);
						rtChanged = 2;

					}
					else
						rtChanged = 1;

					break;
				}
			}

			if (target.cameras.empty())
			{
				mRenderTargets.erase(iterTarget);
				break;
			}
		}

		// Register in render target list
		if (renderTarget != nullptr && (rtChanged == 0 || rtChanged == 2))
		{
			auto findIter = std::find_if(mRenderTargets.begin(), mRenderTargets.end(),
				[&](const RendererRenderTarget& x) { return x.target == renderTarget; });

			if (findIter != mRenderTargets.end())
			{
				findIter->cameras.push_back(camera);
			}
			else
			{
				mRenderTargets.push_back(RendererRenderTarget());
				RendererRenderTarget& renderTargetData = mRenderTargets.back();

				renderTargetData.target = renderTarget;
				renderTargetData.cameras.push_back(camera);
			}

			// Sort render targets based on priority
			auto cameraComparer = [&](const Camera* a, const Camera* b) { return a->getPriority() > b->getPriority(); };
			auto renderTargetInfoComparer = [&](const RendererRenderTarget& a, const RendererRenderTarget& b)
			{ return a.target->getProperties().getPriority() > b.target->getProperties().getPriority(); };
			std::sort(begin(mRenderTargets), end(mRenderTargets), renderTargetInfoComparer);

			for (auto& camerasPerTarget : mRenderTargets)
			{
				Vector<const Camera*>& cameras = camerasPerTarget.cameras;

				std::sort(begin(cameras), end(cameras), cameraComparer);
			}
		}

		return output;
	}

	void RenderBeast::setOptions(const SPtr<CoreRendererOptions>& options)
	{
		mOptions = std::static_pointer_cast<RenderBeastOptions>(options);
		mOptionsDirty = true;
	}

	SPtr<CoreRendererOptions> RenderBeast::getOptions() const
	{
		return mOptions;
	}

	void RenderBeast::syncOptions(const RenderBeastOptions& options)
	{
		bool filteringChanged = mCoreOptions->filtering != options.filtering;
		if (options.filtering == RenderBeastFiltering::Anisotropic)
			filteringChanged |= mCoreOptions->anisotropyMax != options.anisotropyMax;

		if (filteringChanged)
			refreshSamplerOverrides(true);

		*mCoreOptions = options;

		for (auto& entry : mCameras)
		{
			RendererCamera* rendererCam = entry.second;
			rendererCam->setStateReductionMode(mCoreOptions->stateReductionMode);
		}
	}

	void RenderBeast::renderAll() 
	{
		// Sync all dirty sim thread CoreObject data to core thread
		CoreObjectManager::instance().syncToCore();

		if (mOptionsDirty)
		{
			gCoreThread().queueCommand(std::bind(&RenderBeast::syncOptions, this, *mOptions));
			mOptionsDirty = false;
		}

		gCoreThread().queueCommand(std::bind(&RenderBeast::renderAllCore, this, gTime().getTime(), gTime().getFrameDelta()));
	}

	void RenderBeast::renderAllCore(float time, float delta)
	{
		THROW_IF_NOT_CORE_THREAD;

		gProfilerGPU().beginFrame();
		gProfilerCPU().beginSample("renderAllCore");

		// Note: I'm iterating over all sampler states every frame. If this ends up being a performance
		// issue consider handling this internally in ct::Material which can only do it when sampler states
		// are actually modified after sync
		refreshSamplerOverrides();

		// Update global per-frame hardware buffers
		mObjectRenderer->setParamFrameParams(time);

		// Generate render queues per camera
		mVisibility.assign(mVisibility.size(), false);

		for (auto& entry : mCameras)
			entry.second->determineVisible(mRenderables, mWorldBounds, &mVisibility);

		// Retrieve animation data
		AnimationManager::instance().waitUntilComplete();
		const RendererAnimationData& animData = AnimationManager::instance().getRendererData();

		// Update per-object, bone matrix and morph shape GPU buffers
		UINT32 numRenderables = (UINT32)mRenderables.size();
		for (UINT32 i = 0; i < numRenderables; i++)
		{
			if (!mVisibility[i])
				continue;

			// Note: Before uploading bone matrices perhaps check if they has actually been changed since last frame
			mRenderables[i]->renderable->updateAnimationBuffers(animData);

			// Note: Could this step be moved in notifyRenderableUpdated, so it only triggers when material actually gets
			// changed? Although it shouldn't matter much because if the internal versions keeping track of dirty params.
			for (auto& element : mRenderables[i]->elements)
				element.material->updateParamsSet(element.params);

			mRenderables[i]->perObjectParamBuffer->flushToGPU();
		}

		// Render everything, target by target
		for (auto& rtInfo : mRenderTargets)
		{
			SPtr<RenderTarget> target = rtInfo.target;
			Vector<const Camera*>& cameras = rtInfo.cameras;

			UINT32 numCameras = (UINT32)cameras.size();
			for (UINT32 i = 0; i < numCameras; i++)
			{
				bool isOverlayCamera = cameras[i]->getFlags().isSet(CameraFlag::Overlay);
				if (!isOverlayCamera)
				{
					RendererCamera* viewInfo = mCameras[cameras[i]];

					render(viewInfo, delta);
				}
				else
				{
					bool clear = i == 0;

					renderOverlay(cameras[i], clear);
				}
			}
		}

		gProfilerGPU().endFrame();

		// Present render targets with back buffers
		for (auto& rtInfo : mRenderTargets)
		{
			if(rtInfo.target->getProperties().isWindow())
				RenderAPI::instance().swapBuffers(rtInfo.target);
		}

		gProfilerCPU().endSample("renderAllCore");
	}

	void RenderBeast::render(RendererCamera* viewInfo, float frameDelta)
	{
		gProfilerCPU().beginSample("Render");

		const Camera* sceneCamera = viewInfo->getSceneCamera();

		SPtr<GpuParamBlockBuffer> perCameraBuffer = viewInfo->getPerViewBuffer();
		perCameraBuffer->flushToGPU();

		Matrix4 viewProj = viewInfo->getViewProjMatrix();

		// Assign camera and per-call data to all relevant renderables
		const Vector<bool>& visibility = viewInfo->getVisibilityMask();
		UINT32 numRenderables = (UINT32)mRenderables.size();
		for (UINT32 i = 0; i < numRenderables; i++)
		{
			if (!visibility[i])
				continue;

			RendererObject* rendererObject = mRenderables[i];
			rendererObject->updatePerCallBuffer(viewProj);

			for (auto& element : mRenderables[i]->elements)
			{
				if (element.perCameraBindingIdx != -1)
					element.params->setParamBlockBuffer(element.perCameraBindingIdx, perCameraBuffer, true);
			}
		}

		viewInfo->beginRendering(true);

		SPtr<RenderTargets> renderTargets = viewInfo->getRenderTargets();
		renderTargets->bindGBuffer();

		// Trigger pre-base-pass callbacks
		auto iterRenderCallback = mCallbacks.begin();

		if (viewInfo->checkTriggerCallbacks())
		{
			while (iterRenderCallback != mCallbacks.end())
			{
				RendererExtension* extension = *iterRenderCallback;
				if (extension->getLocation() != RenderLocation::PreBasePass)
					break;

				if (extension->check(*sceneCamera))
					extension->render(*sceneCamera);

				++iterRenderCallback;
			}
		}

		// Render base pass
		const Vector<RenderQueueElement>& opaqueElements = viewInfo->getOpaqueQueue()->getSortedElements();
		for (auto iter = opaqueElements.begin(); iter != opaqueElements.end(); ++iter)
		{
			BeastRenderableElement* renderElem = static_cast<BeastRenderableElement*>(iter->renderElem);
			renderElement(*renderElem, iter->passIdx, iter->applyPass, viewProj);
		}

		// Trigger post-base-pass callbacks
		if (viewInfo->checkTriggerCallbacks())
		{
			while (iterRenderCallback != mCallbacks.end())
			{
				RendererExtension* extension = *iterRenderCallback;
				if (extension->getLocation() != RenderLocation::PostBasePass)
					break;

				if (extension->check(*sceneCamera))
					extension->render(*sceneCamera);

				++iterRenderCallback;
			}
		}

		renderTargets->bindSceneColor(true);

		// Render light pass
		{
			mDirLightMat->bind(renderTargets, perCameraBuffer);
			for (auto& light : mDirectionalLights)
			{
				if (!light.internal->getIsActive())
					continue;

				mDirLightMat->setPerLightParams(light.internal);
				gRendererUtility().drawScreenQuad();
			}

			// Draw point lights which our camera is within
			// TODO - Possibly use instanced drawing here as only two meshes are drawn with various properties
			mPointLightInMat->bind(renderTargets, perCameraBuffer);

			// TODO - Cull lights based on visibility, right now I just iterate over all of them. 
			for (auto& light : mPointLights)
			{
				if (!light.internal->getIsActive())
					continue;

				float distToLight = (light.internal->getBounds().getCenter() - viewInfo->getViewOrigin()).squaredLength();
				float boundRadius = light.internal->getBounds().getRadius() * 1.05f + viewInfo->getNearPlane() * 2.0f;

				bool cameraInLightGeometry = distToLight < boundRadius * boundRadius;
				if (!cameraInLightGeometry)
					continue;

				mPointLightInMat->setPerLightParams(light.internal);

				SPtr<Mesh> mesh = light.internal->getMesh();
				gRendererUtility().draw(mesh, mesh->getProperties().getSubMesh(0));
			}

			// Draw other point lights
			mPointLightOutMat->bind(renderTargets, perCameraBuffer);

			for (auto& light : mPointLights)
			{
				if (!light.internal->getIsActive())
					continue;

				float distToLight = (light.internal->getBounds().getCenter() - viewInfo->getViewOrigin()).squaredLength();
				float boundRadius = light.internal->getBounds().getRadius() * 1.05f + viewInfo->getNearPlane() * 2.0f;

				bool cameraInLightGeometry = distToLight < boundRadius * boundRadius;
				if (cameraInLightGeometry)
					continue;

				mPointLightOutMat->setPerLightParams(light.internal);

				SPtr<Mesh> mesh = light.internal->getMesh();
				gRendererUtility().draw(mesh, mesh->getProperties().getSubMesh(0));
			}
		}

		// Render skybox (if any)
		SPtr<Texture> skyTexture = viewInfo->getSkybox();
		if (skyTexture != nullptr && skyTexture->getProperties().getTextureType() == TEX_TYPE_CUBE_MAP)
		{
			mSkyboxMat->bind(perCameraBuffer);
			mSkyboxMat->setParams(skyTexture);

			SPtr<Mesh> mesh = gRendererUtility().getSkyBoxMesh();
			gRendererUtility().draw(mesh, mesh->getProperties().getSubMesh(0));
		}

		renderTargets->bindSceneColor(false);

		// Render transparent objects (TODO - No lighting yet)
		const Vector<RenderQueueElement>& transparentElements = viewInfo->getTransparentQueue()->getSortedElements();
		for (auto iter = transparentElements.begin(); iter != transparentElements.end(); ++iter)
		{
			BeastRenderableElement* renderElem = static_cast<BeastRenderableElement*>(iter->renderElem);
			renderElement(*renderElem, iter->passIdx, iter->applyPass, viewProj);
		}

		// Trigger post-light-pass callbacks
		if (viewInfo->checkTriggerCallbacks())
		{
			while (iterRenderCallback != mCallbacks.end())
			{
				RendererExtension* extension = *iterRenderCallback;
				if (extension->getLocation() != RenderLocation::PostLightPass)
					break;

				if (extension->check(*sceneCamera))
					extension->render(*sceneCamera);

				++iterRenderCallback;
			}
		}

		// Post-processing and final resolve
		if (viewInfo->checkRunPostProcessing())
		{
			// TODO - If GBuffer has multiple samples, I should resolve them before post-processing
			PostProcessing::instance().postProcess(viewInfo, renderTargets->getSceneColorRT(), frameDelta);
		}
		else
		{
			// Just copy from scene color to output if no post-processing
			RenderAPI& rapi = RenderAPI::instance();
			SPtr<RenderTarget> target = viewInfo->getFinalTarget();
			Rect2 viewportArea = viewInfo->getViewportRect();

			rapi.setRenderTarget(target);
			rapi.setViewport(viewportArea);

			SPtr<Texture> sceneColor = renderTargets->getSceneColorRT()->getColorTexture(0);
			gRendererUtility().blit(sceneColor, Rect2I::EMPTY, viewInfo->getFlipView());
		}

		// Trigger overlay callbacks
		if (viewInfo->checkTriggerCallbacks())
		{
			while (iterRenderCallback != mCallbacks.end())
			{
				RendererExtension* extension = *iterRenderCallback;
				if (extension->getLocation() != RenderLocation::Overlay)
					break;

				if (extension->check(*sceneCamera))
					extension->render(*sceneCamera);

				++iterRenderCallback;
			}
		}

		viewInfo->endRendering();

		gProfilerCPU().endSample("Render");
	}

	void RenderBeast::renderOverlay(const Camera* camera, bool clear)
	{
		gProfilerCPU().beginSample("RenderOverlay");

		SPtr<Viewport> viewport = camera->getViewport();
		RendererCamera* rendererCam = mCameras[camera];
		rendererCam->getPerViewBuffer()->flushToGPU();

		rendererCam->beginRendering(false);

		SPtr<RenderTarget> target = camera->getViewport()->getTarget();

		// If first camera in render target, prepare the render target
		if (clear)
		{
			RenderAPI::instance().setRenderTarget(target);

			UINT32 clearBuffers = 0;
			if (viewport->getRequiresColorClear())
				clearBuffers |= FBT_COLOR;

			if (viewport->getRequiresDepthClear())
				clearBuffers |= FBT_DEPTH;

			if (viewport->getRequiresStencilClear())
				clearBuffers |= FBT_STENCIL;

			if (clearBuffers != 0)
			{
				RenderAPI::instance().clearViewport(clearBuffers, viewport->getClearColor(),
					viewport->getClearDepthValue(), viewport->getClearStencilValue());
			}
		}
		else
			RenderAPI::instance().setRenderTarget(target, false, RT_COLOR0);

		RenderAPI::instance().setViewport(viewport->getNormArea());

		// Trigger overlay callbacks
		auto iterRenderCallback = mCallbacks.begin();
		while (iterRenderCallback != mCallbacks.end())
		{
			RendererExtension* extension = *iterRenderCallback;
			if (extension->getLocation() != RenderLocation::Overlay)
			{
				++iterRenderCallback;
				continue;
			}

			if (extension->check(*camera))
				extension->render(*camera);

			++iterRenderCallback;
		}

		rendererCam->endRendering();

		gProfilerCPU().endSample("RenderOverlay");
	}
	
	void RenderBeast::renderElement(const BeastRenderableElement& element, UINT32 passIdx, bool bindPass, 
									const Matrix4& viewProj)
	{
		SPtr<Material> material = element.material;

		if (bindPass)
			gRendererUtility().setPass(material, passIdx, element.techniqueIdx);

		gRendererUtility().setPassParams(element.params, passIdx);

		if(element.morphVertexDeclaration == nullptr)
			gRendererUtility().draw(element.mesh, element.subMesh);
		else
			gRendererUtility().drawMorph(element.mesh, element.subMesh, element.morphShapeBuffer, 
				element.morphVertexDeclaration);
	}

	SPtr<Texture> RenderBeast::captureSceneCubeMap(const Vector3& position, bool hdr, UINT32 size)
	{
		TEXTURE_DESC cubeMapDesc;
		cubeMapDesc.type = TEX_TYPE_CUBE_MAP;
		cubeMapDesc.format = hdr ? PF_FLOAT16_RGBA : PF_R8G8B8A8;
		cubeMapDesc.width = size;
		cubeMapDesc.height = size;
		cubeMapDesc.numMips = PixelUtil::getMaxMipmaps(size, size, 1, cubeMapDesc.format);
		cubeMapDesc.usage = TU_RENDERTARGET;

		SPtr<Texture> cubemap = Texture::create(cubeMapDesc);

		Matrix4 projTransform = Matrix4::projectionPerspective(Degree(90.0f), 1.0f, 0.05f, 1000.0f);
		ConvexVolume localFrustum(projTransform);
		RenderAPI::instance().convertProjectionMatrix(projTransform, projTransform);

		RENDERER_VIEW_DESC viewDesc;
		viewDesc.target.clearFlags = FBT_COLOR | FBT_DEPTH;
		viewDesc.target.clearColor = Color::Black;
		viewDesc.target.clearDepthValue = 1.0f;
		viewDesc.target.clearStencilValue = 0;

		viewDesc.target.nrmViewRect = Rect2(0, 0, 1.0f, 1.0f);
		viewDesc.target.viewRect = Rect2I(0, 0, size, size);
		viewDesc.target.targetWidth = size;
		viewDesc.target.targetHeight = size;
		viewDesc.target.numSamples = 1;

		viewDesc.isOverlay = false;
		viewDesc.isHDR = hdr;
		viewDesc.triggerCallbacks = false;
		viewDesc.runPostProcessing = false;

		viewDesc.visibleLayers = 0xFFFFFFFFFFFFFFFF;
		viewDesc.nearPlane = 0.5f;
		viewDesc.flipView = RenderAPI::instance().getAPIInfo().getUVYAxisUp();

		viewDesc.viewOrigin = position;
		viewDesc.projTransform = projTransform;

		viewDesc.stateReduction = mCoreOptions->stateReductionMode;
		viewDesc.sceneCamera = nullptr;

		// Note: Find a camera to receive skybox from. Skybox should probably be a global property instead of a per-camera
		// one.
		for(auto& entry : mRenderTargets)
		{
			for(auto& camera : entry.cameras)
			{
				if (camera->getSkybox() != nullptr)
				{
					viewDesc.skyboxTexture = camera->getSkybox();
					break;
				}
			}
		}

		Matrix4 viewOffsetMat = Matrix4::translation(-position);

		RendererCamera view(viewDesc);
		for(UINT32 i = 0; i < 6; i++)
		{
			// Calculate view matrix
			Matrix3 viewRotationMat;
			Vector3 forward;

			Vector3 up = Vector3::UNIT_Y;

			switch (i)
			{
			case CF_PositiveX:
				forward = Vector3::UNIT_X;
				break;
			case CF_NegativeX:
				forward = -Vector3::UNIT_X;
				break;
			case CF_PositiveY:
				forward = Vector3::UNIT_Y;
				up = -Vector3::UNIT_Z;
				break;
			case CF_NegativeY:
				forward = Vector3::UNIT_X;
				up = Vector3::UNIT_Z;
				break;
			case CF_PositiveZ:
				forward = Vector3::UNIT_Z;
				break;
			case CF_NegativeZ:
				forward = -Vector3::UNIT_Z;
				break;
			}

			Vector3 right = Vector3::cross(up, forward);
			viewRotationMat = Matrix3(right, up, forward);

			viewDesc.viewDirection = forward;
			viewDesc.viewTransform = Matrix4(viewRotationMat) * viewOffsetMat;

			// Calculate world frustum for culling
			const Vector<Plane>& frustumPlanes = localFrustum.getPlanes();
			Matrix4 worldMatrix = viewDesc.viewTransform.transpose();

			Vector<Plane> worldPlanes(frustumPlanes.size());
			UINT32 j = 0;
			for (auto& plane : frustumPlanes)
			{
				worldPlanes[j] = worldMatrix.multiplyAffine(plane);
				j++;
			}

			viewDesc.cullFrustum = ConvexVolume(worldPlanes);

			// Set up face render target
			RENDER_TEXTURE_DESC cubeFaceRTDesc;
			cubeFaceRTDesc.colorSurfaces[0].texture = cubemap;
			cubeFaceRTDesc.colorSurfaces[0].face = i;
			cubeFaceRTDesc.colorSurfaces[0].numFaces = 1;
			
			viewDesc.target.target = RenderTexture::create(cubeFaceRTDesc);

			view.setView(viewDesc);
			view.updatePerViewBuffer();
			view.determineVisible(mRenderables, mWorldBounds);

			render(&view, 0.0f);
		}

		ReflectionCubemap::filterCubemapForSpecular(cubemap);

		return cubemap;
	}

	void RenderBeast::refreshSamplerOverrides(bool force)
	{
		bool anyDirty = false;
		for (auto& entry : mSamplerOverrides)
		{
			SPtr<MaterialParams> materialParams = entry.first.material->_getInternalParams();

			MaterialSamplerOverrides* materialOverrides = entry.second;
			for(UINT32 i = 0; i < materialOverrides->numOverrides; i++)
			{
				SamplerOverride& override = materialOverrides->overrides[i];
				const MaterialParamsBase::ParamData* materialParamData = materialParams->getParamData(override.paramIdx);

				SPtr<SamplerState> samplerState;
				materialParams->getSamplerState(*materialParamData, samplerState);

				UINT64 hash = 0;
				if (samplerState != nullptr)
					hash = samplerState->getProperties().getHash();

				if (hash != override.originalStateHash || force)
				{
					if (samplerState != nullptr)
						override.state = SamplerOverrideUtility::generateSamplerOverride(samplerState, mCoreOptions);
					else
						override.state = SamplerOverrideUtility::generateSamplerOverride(SamplerState::getDefault(), mCoreOptions);

					override.originalStateHash = override.state->getProperties().getHash();
					materialOverrides->isDirty = true;
				}

				// Dirty flag can also be set externally, so check here even though we assign it above
				if (materialOverrides->isDirty)
					anyDirty = true;
			}
		}

		// Early exit if possible
		if (!anyDirty)
			return;

		UINT32 numRenderables = (UINT32)mRenderables.size();
		for (UINT32 i = 0; i < numRenderables; i++)
		{
			for(auto& element : mRenderables[i]->elements)
			{
				MaterialSamplerOverrides* overrides = element.samplerOverrides;
				if(overrides != nullptr && overrides->isDirty)
				{
					UINT32 numPasses = element.material->getNumPasses();
					for(UINT32 j = 0; j < numPasses; j++)
					{
						SPtr<GpuParams> params = element.params->getGpuParams(j);

						const UINT32 numStages = 6;
						for (UINT32 k = 0; k < numStages; k++)
						{
							GpuProgramType type = (GpuProgramType)k;

							SPtr<GpuParamDesc> paramDesc = params->getParamDesc(type);
							if (paramDesc == nullptr)
								continue;

							for (auto& samplerDesc : paramDesc->samplers)
							{
								UINT32 set = samplerDesc.second.set;
								UINT32 slot = samplerDesc.second.slot;

								UINT32 overrideIndex = overrides->passes[j].stateOverrides[set][slot];
								if (overrideIndex == (UINT32)-1)
									continue;

								params->setSamplerState(set, slot, overrides->overrides[overrideIndex].state);
							}
						}
					}
				}
			}
		}

		for (auto& entry : mSamplerOverrides)
			entry.second->isDirty = false;
	}
}}