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
#include "BsMeshData.h"

using namespace std::placeholders;

namespace bs
{
	RenderBeast::RendererFrame::RendererFrame(float delta, const RendererAnimationData& animData)
		:delta(delta), animData(animData)
	{ }

	RenderBeast::RenderBeast()
		: mDefaultMaterial(nullptr), mPointLightInMat(nullptr), mPointLightOutMat(nullptr), mDirLightMat(nullptr)
		, mObjectRenderer(nullptr), mOptions(bs_shared_ptr_new<RenderBeastOptions>()), mOptionsDirty(true)
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

		RendererUtility::shutDown();

		assert(mSamplerOverrides.empty());
	}

	void RenderBeast::notifyRenderableAdded(RenderableCore* renderable)
	{
		UINT32 renderableId = (UINT32)mRenderables.size();

		renderable->setRendererId(renderableId);

		mRenderables.push_back(bs_new<RendererObject>());
		mWorldBounds.push_back(renderable->getBounds());
		mVisibility.push_back(false);

		RendererObject* rendererObject = mRenderables.back();
		rendererObject->renderable = renderable;
		rendererObject->updatePerObjectBuffer();

		SPtr<MeshCore> mesh = renderable->getMesh();
		if (mesh != nullptr)
		{
			const MeshProperties& meshProps = mesh->getProperties();
			SPtr<VertexDeclarationCore> vertexDecl = mesh->getVertexData()->vertexDeclaration;

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
						SPtr<PassCore> pass = renElement.material->getPass(j, techniqueIdx);

						SPtr<VertexDeclarationCore> shaderDecl = pass->getVertexProgram()->getInputDeclaration();
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
				Any materialInfo = renElement.material->getRendererData();
				if(materialInfo.empty())
				{
					RendererMaterial matInfo;
					matInfo.params.resize(techniqueIdx + 1);
					matInfo.params[techniqueIdx] = renElement.material->createParamsSet(techniqueIdx);
					matInfo.matVersion = renElement.material->getVersion();

					renElement.material->updateParamsSet(matInfo.params[techniqueIdx], techniqueIdx, true);
					renElement.material->setRendererData(matInfo);
					renElement.params = matInfo.params[techniqueIdx];
				}
				else
				{
					RendererMaterial& matInfo = any_cast_ref<RendererMaterial>(materialInfo);
					if (matInfo.params.size() <= techniqueIdx)
						matInfo.params.resize(techniqueIdx + 1);

					if(matInfo.params[techniqueIdx] == nullptr || matInfo.matVersion != renElement.material->getVersion())
					{
						matInfo.params[techniqueIdx] = renElement.material->createParamsSet(techniqueIdx);
						matInfo.matVersion = renElement.material->getVersion();

						renElement.material->updateParamsSet(matInfo.params[techniqueIdx], techniqueIdx, true);
					}

					renElement.params = matInfo.params[techniqueIdx];
				}

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
					SPtr<ShaderCore> shader = renElement.material->getShader();
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

	void RenderBeast::notifyRenderableRemoved(RenderableCore* renderable)
	{
		UINT32 renderableId = renderable->getRendererId();
		RenderableCore* lastRenerable = mRenderables.back()->renderable;
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

	void RenderBeast::notifyRenderableUpdated(RenderableCore* renderable)
	{
		UINT32 renderableId = renderable->getRendererId();

		mRenderables[renderableId]->updatePerObjectBuffer();
		mWorldBounds[renderableId] = renderable->getBounds();
	}

	void RenderBeast::notifyLightAdded(LightCore* light)
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

	void RenderBeast::notifyLightUpdated(LightCore* light)
	{
		UINT32 lightId = light->getRendererId();

		if (light->getType() != LightType::Directional)
			mLightWorldBounds[lightId] = light->getBounds();
	}

	void RenderBeast::notifyLightRemoved(LightCore* light)
	{
		UINT32 lightId = light->getRendererId();
		if (light->getType() == LightType::Directional)
		{
			LightCore* lastLight = mDirectionalLights.back().internal;
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
			LightCore* lastLight = mPointLights.back().internal;
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

	void RenderBeast::notifyCameraAdded(const CameraCore* camera)
	{
		RendererCamera* renCamera = updateCameraData(camera);
		renCamera->updatePerCameraBuffer();
	}

	void RenderBeast::notifyCameraUpdated(const CameraCore* camera, UINT32 updateFlag)
	{
		RendererCamera* rendererCam;
		if((updateFlag & (UINT32)CameraDirtyFlag::Everything) != 0)
		{
			rendererCam = updateCameraData(camera);
		}
		else if((updateFlag & (UINT32)CameraDirtyFlag::PostProcess) != 0)
		{
			rendererCam = mCameras[camera];
			rendererCam->updatePP();
		}
		else
			rendererCam = mCameras[camera];

		rendererCam->updatePerCameraBuffer();
	}

	void RenderBeast::notifyCameraRemoved(const CameraCore* camera)
	{
		updateCameraData(camera, true);
	}

	SPtr<PostProcessSettings> RenderBeast::createPostProcessSettings() const
	{
		return bs_shared_ptr_new<StandardPostProcessSettings>();
	}

	RendererCamera* RenderBeast::updateCameraData(const CameraCore* camera, bool forceRemove)
	{
		RendererCamera* output;

		SPtr<RenderTargetCore> renderTarget = camera->getViewport()->getTarget();
		if(forceRemove)
		{
			auto iterFind = mCameras.find(camera);
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
			output = bs_new<RendererCamera>(camera, mCoreOptions->stateReductionMode);
			mCameras[camera] = output;
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
			auto cameraComparer = [&](const CameraCore* a, const CameraCore* b) { return a->getPriority() > b->getPriority(); };
			auto renderTargetInfoComparer = [&](const RendererRenderTarget& a, const RendererRenderTarget& b)
			{ return a.target->getProperties().getPriority() > b.target->getProperties().getPriority(); };
			std::sort(begin(mRenderTargets), end(mRenderTargets), renderTargetInfoComparer);

			for (auto& camerasPerTarget : mRenderTargets)
			{
				Vector<const CameraCore*>& cameras = camerasPerTarget.cameras;

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
			rendererCam->update(mCoreOptions->stateReductionMode);
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
		// issue consider handling this internally in MaterialCore which can only do it when sampler states
		// are actually modified after sync
		refreshSamplerOverrides();

		// Update global per-frame hardware buffers
		mObjectRenderer->setParamFrameParams(time);

		// Generate render queues per camera
		mVisibility.assign(mVisibility.size(), false);

		for (auto& entry : mCameras)
			entry.second->determineVisible(mRenderables, mWorldBounds, mVisibility);

		// Retrieve animation data
		AnimationManager::instance().waitUntilComplete();
		const RendererAnimationData& animData = AnimationManager::instance().getRendererData();
		RendererFrame frameInfo(delta, animData);

		// Update per-object, bone matrix and morph shape GPU buffers
		UINT32 numRenderables = (UINT32)mRenderables.size();
		for (UINT32 i = 0; i < numRenderables; i++)
		{
			if (!mVisibility[i])
				continue;

			// Note: Before uploading bone matrices perhaps check if they has actually been changed since last frame
			mRenderables[i]->renderable->updateAnimationBuffers(animData);

			// Note: Could this step be moved in notifyRenderableUpdated, so it only triggers when material actually gets
			// changed? Although it shouldn't matter much because if the internal dirty flags.
			for (auto& element : mRenderables[i]->elements)
				element.material->updateParamsSet(element.params, element.techniqueIdx);

			mRenderables[i]->perObjectParamBuffer->flushToGPU();
		}

		// Render everything, target by target
		for (auto& rtInfo : mRenderTargets)
		{
			SPtr<RenderTargetCore> target = rtInfo.target;
			Vector<const CameraCore*>& cameras = rtInfo.cameras;

			UINT32 numCameras = (UINT32)cameras.size();
			for (UINT32 i = 0; i < numCameras; i++)
			{
				bool isOverlayCamera = cameras[i]->getFlags().isSet(CameraFlag::Overlay);
				if (!isOverlayCamera)
					render(frameInfo, rtInfo, i);
				else
					renderOverlay(frameInfo, rtInfo, i);
			}
		}

		gProfilerGPU().endFrame();

		// Present render targets with back buffers
		for (auto& rtInfo : mRenderTargets)
			RenderAPICore::instance().swapBuffers(rtInfo.target);

		gProfilerCPU().endSample("renderAllCore");
	}

	void RenderBeast::render(const RendererFrame& frameInfo, RendererRenderTarget& rtInfo, UINT32 camIdx)
	{
		gProfilerCPU().beginSample("Render");

		const CameraCore* camera = rtInfo.cameras[camIdx];
		RendererCamera* rendererCam = mCameras[camera];
		SPtr<GpuParamBlockBufferCore> perCameraBuffer = rendererCam->getPerCameraBuffer();
		perCameraBuffer->flushToGPU();

		assert(!camera->getFlags().isSet(CameraFlag::Overlay));

		Matrix4 proj = camera->getProjectionMatrixRS();
		Matrix4 view = camera->getViewMatrix();
		Matrix4 viewProj = proj * view;

		// Assign camera and per-call data to all relevant renderables
		const Vector<bool>& visibility = rendererCam->getVisibilityMask();
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

		rendererCam->beginRendering(true);

		SPtr<RenderTargets> renderTargets = rendererCam->getRenderTargets();
		renderTargets->bindGBuffer();

		//// Trigger pre-scene callbacks
		auto iterCameraCallbacks = mRenderCallbacks.find(camera);
		if (iterCameraCallbacks != mRenderCallbacks.end())
		{
			for (auto& callbackPair : iterCameraCallbacks->second)
			{
				const RenderCallbackData& callbackData = callbackPair.second;

				if (callbackData.overlay)
					continue;

				if (callbackPair.first >= 0)
					break;

				callbackData.callback();
			}
		}

		//// Render base pass
		const Vector<RenderQueueElement>& opaqueElements = rendererCam->getOpaqueQueue()->getSortedElements();
		for (auto iter = opaqueElements.begin(); iter != opaqueElements.end(); ++iter)
		{
			BeastRenderableElement* renderElem = static_cast<BeastRenderableElement*>(iter->renderElem);
			renderElement(*renderElem, iter->passIdx, iter->applyPass, frameInfo, viewProj);
		}

		renderTargets->bindSceneColor(true);

		//// Render light pass
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

				float distToLight = (light.internal->getBounds().getCenter() - camera->getPosition()).squaredLength();
				float boundRadius = light.internal->getBounds().getRadius() * 1.05f + camera->getNearClipDistance() * 2.0f;

				bool cameraInLightGeometry = distToLight < boundRadius * boundRadius;
				if (!cameraInLightGeometry)
					continue;

				mPointLightInMat->setPerLightParams(light.internal);

				SPtr<MeshCore> mesh = light.internal->getMesh();
				gRendererUtility().draw(mesh, mesh->getProperties().getSubMesh(0));
			}

			// Draw other point lights
			mPointLightOutMat->bind(renderTargets, perCameraBuffer);

			for (auto& light : mPointLights)
			{
				if (!light.internal->getIsActive())
					continue;

				float distToLight = (light.internal->getBounds().getCenter() - camera->getPosition()).squaredLength();
				float boundRadius = light.internal->getBounds().getRadius() * 1.05f + camera->getNearClipDistance() * 2.0f;

				bool cameraInLightGeometry = distToLight < boundRadius * boundRadius;
				if (cameraInLightGeometry)
					continue;

				mPointLightOutMat->setPerLightParams(light.internal);

				SPtr<MeshCore> mesh = light.internal->getMesh();
				gRendererUtility().draw(mesh, mesh->getProperties().getSubMesh(0));
			}
		}

		renderTargets->bindSceneColor(false);
		
		// Render transparent objects (TODO - No lighting yet)
		const Vector<RenderQueueElement>& transparentElements = rendererCam->getTransparentQueue()->getSortedElements();
		for (auto iter = transparentElements.begin(); iter != transparentElements.end(); ++iter)
		{
			BeastRenderableElement* renderElem = static_cast<BeastRenderableElement*>(iter->renderElem);
			renderElement(*renderElem, iter->passIdx, iter->applyPass, frameInfo, viewProj);
		}

		// Render non-overlay post-scene callbacks
		if (iterCameraCallbacks != mRenderCallbacks.end())
		{
			for (auto& callbackPair : iterCameraCallbacks->second)
			{
				const RenderCallbackData& callbackData = callbackPair.second;

				if (callbackData.overlay || callbackPair.first < 0)
					continue;

				callbackData.callback();
			}
		}

		// TODO - If GBuffer has multiple samples, I should resolve them before post-processing
		PostProcessing::instance().postProcess(renderTargets->getSceneColorRT(),
			camera, rendererCam->getPPInfo(), frameInfo.delta);

		// Render overlay post-scene callbacks
		if (iterCameraCallbacks != mRenderCallbacks.end())
		{
			for (auto& callbackPair : iterCameraCallbacks->second)
			{
				const RenderCallbackData& callbackData = callbackPair.second;

				if (!callbackData.overlay)
					continue;

				callbackData.callback();
			}
		}

		rendererCam->endRendering();

		gProfilerCPU().endSample("Render");
	}

	void RenderBeast::renderOverlay(const RendererFrame& frameInfo, RendererRenderTarget& rtData, UINT32 camIdx)
	{
		gProfilerCPU().beginSample("RenderOverlay");

		const CameraCore* camera = rtData.cameras[camIdx];
		assert(camera->getFlags().isSet(CameraFlag::Overlay));

		SPtr<ViewportCore> viewport = camera->getViewport();
		RendererCamera* rendererCam = mCameras[camera];
		rendererCam->getPerCameraBuffer()->flushToGPU();

		rendererCam->beginRendering(false);

		SPtr<RenderTargetCore> target = rtData.target;

		RenderAPICore::instance().setRenderTarget(target);
		RenderAPICore::instance().setViewport(viewport->getNormArea());

		// If first camera in render target, prepare the render target
		if (camIdx == 0)
		{
			UINT32 clearBuffers = 0;
			if (viewport->getRequiresColorClear())
				clearBuffers |= FBT_COLOR;

			if (viewport->getRequiresDepthClear())
				clearBuffers |= FBT_DEPTH;

			if (viewport->getRequiresStencilClear())
				clearBuffers |= FBT_STENCIL;

			if (clearBuffers != 0)
			{
				RenderAPICore::instance().clearViewport(clearBuffers, viewport->getClearColor(),
					viewport->getClearDepthValue(), viewport->getClearStencilValue());
			}
		}

		// Render overlay post-scene callbacks
		auto iterCameraCallbacks = mRenderCallbacks.find(camera);
		if (iterCameraCallbacks != mRenderCallbacks.end())
		{
			for (auto& callbackPair : iterCameraCallbacks->second)
			{
				const RenderCallbackData& callbackData = callbackPair.second;

				if (!callbackData.overlay)
					continue;

				callbackData.callback();
			}
		}

		rendererCam->endRendering();

		gProfilerCPU().endSample("RenderOverlay");
	}
	
	void RenderBeast::renderElement(const BeastRenderableElement& element, UINT32 passIdx, bool bindPass,
		const RendererFrame& frameInfo, const Matrix4& viewProj)
	{
		SPtr<MaterialCore> material = element.material;

		if (bindPass)
			gRendererUtility().setPass(material, passIdx, element.techniqueIdx);

		gRendererUtility().setPassParams(element.params, passIdx);

		if(element.morphVertexDeclaration == nullptr)
			gRendererUtility().draw(element.mesh, element.subMesh);
		else
			gRendererUtility().drawMorph(element.mesh, element.subMesh, element.morphShapeBuffer, 
				element.morphVertexDeclaration);
	}

	void RenderBeast::refreshSamplerOverrides(bool force)
	{
		bool anyDirty = false;
		for (auto& entry : mSamplerOverrides)
		{
			SPtr<MaterialParamsCore> materialParams = entry.first.material->_getInternalParams();

			MaterialSamplerOverrides* materialOverrides = entry.second;
			for(UINT32 i = 0; i < materialOverrides->numOverrides; i++)
			{
				SamplerOverride& override = materialOverrides->overrides[i];
				const MaterialParamsBase::ParamData* materialParamData = materialParams->getParamData(override.paramIdx);

				SPtr<SamplerStateCore> samplerState;
				materialParams->getSamplerState(materialParamData->index, samplerState);

				UINT64 hash = 0;
				if (samplerState != nullptr)
					hash = samplerState->getProperties().getHash();

				if (hash != override.originalStateHash || force)
				{
					if (samplerState != nullptr)
						override.state = SamplerOverrideUtility::generateSamplerOverride(samplerState, mCoreOptions);
					else
						override.state = SamplerOverrideUtility::generateSamplerOverride(SamplerStateCore::getDefault(), mCoreOptions);

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
						SPtr<GpuParamsCore> params = element.params->getGpuParams(j);

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
}