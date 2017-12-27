//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsRendererScene.h"
#include "Renderer/BsCamera.h"
#include "Renderer/BsLight.h"
#include "Renderer/BsReflectionProbe.h"
#include "Mesh/BsMesh.h"
#include "Renderer/BsRenderer.h"
#include "Material/BsPass.h"
#include "Material/BsGpuParamsSet.h"
#include "BsRenderBeastOptions.h"
#include "BsRenderBeast.h"
#include "Renderer/BsSkybox.h"

namespace bs {	namespace ct
{
	RendererScene::RendererScene(const SPtr<RenderBeastOptions>& options)
		:mOptions(options)
	{
	}

	RendererScene::~RendererScene()
	{
		for (auto& entry : mInfo.renderables)
			bs_delete(entry);

		for (auto& entry : mInfo.views)
			bs_delete(entry);

		assert(mSamplerOverrides.empty());
	}

	void RendererScene::registerCamera(Camera* camera)
	{
		RENDERER_VIEW_DESC viewDesc = createViewDesc(camera);

		RendererView* view = bs_new<RendererView>(viewDesc);
		view->setRenderSettings(camera->getRenderSettings());
		view->updatePerViewBuffer();

		UINT32 viewIdx = (UINT32)mInfo.views.size();
		mInfo.views.push_back(view);

		mInfo.cameraToView[camera] = viewIdx;
		camera->setRendererId(viewIdx);

		updateCameraRenderTargets(camera);
	}

	void RendererScene::updateCamera(Camera* camera, UINT32 updateFlag)
	{
		UINT32 cameraId = camera->getRendererId();
		RendererView* view = mInfo.views[cameraId];

		if((updateFlag & ((UINT32)ActorDirtyFlag::Everything | (UINT32)ActorDirtyFlag::Active)) != 0)
		{
			RENDERER_VIEW_DESC viewDesc = createViewDesc(camera);

			view->setView(viewDesc);
			view->setRenderSettings(camera->getRenderSettings());

			updateCameraRenderTargets(camera);
		}
		else if((updateFlag & (UINT32)CameraDirtyFlag::RenderSettings) != 0)
		{
			view->setRenderSettings(camera->getRenderSettings());
		}
		else // Transform
		{
			view = mInfo.views[cameraId];

			const Transform& tfrm = camera->getTransform();
			view->setTransform(
				tfrm.getPosition(),
				tfrm.getForward(),
				camera->getViewMatrix(),
				camera->getProjectionMatrixRS(),
				camera->getWorldFrustum());
		}

		view->updatePerViewBuffer();
	}

	void RendererScene::unregisterCamera(Camera* camera)
	{
		UINT32 cameraId = camera->getRendererId();

		Camera* lastCamera = mInfo.views.back()->getSceneCamera();
		UINT32 lastCameraId = lastCamera->getRendererId();
		
		if (cameraId != lastCameraId)
		{
			// Swap current last element with the one we want to erase
			std::swap(mInfo.views[cameraId], mInfo.views[lastCameraId]);
			lastCamera->setRendererId(cameraId);

			mInfo.cameraToView[lastCamera] = cameraId;
		}
		
		// Last element is the one we want to erase
		RendererView* view = mInfo.views[mInfo.views.size() - 1];
		bs_delete(view);

		mInfo.views.erase(mInfo.views.end() - 1);

		auto iterFind = mInfo.cameraToView.find(camera);
		if(iterFind != mInfo.cameraToView.end())
			mInfo.cameraToView.erase(iterFind);

		updateCameraRenderTargets(camera, true);
	}

	void RendererScene::registerLight(Light* light)
	{
		if (light->getType() == LightType::Directional)
		{
			UINT32 lightId = (UINT32)mInfo.directionalLights.size();
			light->setRendererId(lightId);

			mInfo.directionalLights.push_back(RendererLight(light));
		}
		else
		{
			if (light->getType() == LightType::Radial)
			{
				UINT32 lightId = (UINT32)mInfo.radialLights.size();
				light->setRendererId(lightId);

				mInfo.radialLights.push_back(RendererLight(light));
				mInfo.radialLightWorldBounds.push_back(light->getBounds());
			}
			else // Spot
			{
				UINT32 lightId = (UINT32)mInfo.spotLights.size();
				light->setRendererId(lightId);

				mInfo.spotLights.push_back(RendererLight(light));
				mInfo.spotLightWorldBounds.push_back(light->getBounds());
			}
		}
	}

	void RendererScene::updateLight(Light* light)
	{
		UINT32 lightId = light->getRendererId();

		if (light->getType() == LightType::Radial)
			mInfo.radialLightWorldBounds[lightId] = light->getBounds();
		else if(light->getType() == LightType::Spot)
			mInfo.spotLightWorldBounds[lightId] = light->getBounds();
	}

	void RendererScene::unregisterLight(Light* light)
	{
		UINT32 lightId = light->getRendererId();
		if (light->getType() == LightType::Directional)
		{
			Light* lastLight = mInfo.directionalLights.back().internal;
			UINT32 lastLightId = lastLight->getRendererId();

			if (lightId != lastLightId)
			{
				// Swap current last element with the one we want to erase
				std::swap(mInfo.directionalLights[lightId], mInfo.directionalLights[lastLightId]);
				lastLight->setRendererId(lightId);
			}

			// Last element is the one we want to erase
			mInfo.directionalLights.erase(mInfo.directionalLights.end() - 1);
		}
		else
		{
			if (light->getType() == LightType::Radial)
			{
				Light* lastLight = mInfo.radialLights.back().internal;
				UINT32 lastLightId = lastLight->getRendererId();

				if (lightId != lastLightId)
				{
					// Swap current last element with the one we want to erase
					std::swap(mInfo.radialLights[lightId], mInfo.radialLights[lastLightId]);
					std::swap(mInfo.radialLightWorldBounds[lightId], mInfo.radialLightWorldBounds[lastLightId]);

					lastLight->setRendererId(lightId);
				}

				// Last element is the one we want to erase
				mInfo.radialLights.erase(mInfo.radialLights.end() - 1);
				mInfo.radialLightWorldBounds.erase(mInfo.radialLightWorldBounds.end() - 1);
			}
			else // Spot
			{
				Light* lastLight = mInfo.spotLights.back().internal;
				UINT32 lastLightId = lastLight->getRendererId();

				if (lightId != lastLightId)
				{
					// Swap current last element with the one we want to erase
					std::swap(mInfo.spotLights[lightId], mInfo.spotLights[lastLightId]);
					std::swap(mInfo.spotLightWorldBounds[lightId], mInfo.spotLightWorldBounds[lastLightId]);

					lastLight->setRendererId(lightId);
				}

				// Last element is the one we want to erase
				mInfo.spotLights.erase(mInfo.spotLights.end() - 1);
				mInfo.spotLightWorldBounds.erase(mInfo.spotLightWorldBounds.end() - 1);
			}
		}
	}

	void RendererScene::registerRenderable(Renderable* renderable)
	{
		UINT32 renderableId = (UINT32)mInfo.renderables.size();

		renderable->setRendererId(renderableId);

		mInfo.renderables.push_back(bs_new<RendererObject>());
		mInfo.renderableCullInfos.push_back(CullInfo(renderable->getBounds(), renderable->getLayer()));

		RendererObject* rendererObject = mInfo.renderables.back();
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

				// If no mInfo.aterial use the default mInfo.aterial
				if (renElement.material == nullptr)
					renElement.material = DefaultMaterial::get()->getMaterial();

				// Determine which technique to use
				static StringID techniqueIDLookup[4] = { StringID::NONE, RTag_Skinned, RTag_Morph, RTag_SkinnedMorph };
				static_assert((UINT32)RenderableAnimType::Count == 4, "RenderableAnimType is expected to have four sequential entries.");

				UINT32 techniqueIdx = -1;
				RenderableAnimType animType = renderable->getAnimType();
				if (animType != RenderableAnimType::None)
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

							// If using mInfo.orph shapes ignore POSITION1 and NORMAL1 mInfo.issing since we assign them from within the renderer
							if (animType == RenderableAnimType::Morph || animType == RenderableAnimType::SkinnedMorph)
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
								wrnStream << "Provided mesh is mInfo.issing required vertex attributes to render with the provided shader. Missing elements: " << std::endl;

								for (auto& entry : missingElements)
									wrnStream << "\t" << toString(entry.getSemantic()) << entry.getSemanticIdx() << std::endl;

								LOGWRN(wrnStream.str());
								break;
							}
						}
					}
				}

				// Generate or assigned renderer specific data for the mInfo.aterial
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
						renElement.material->_getInternalParams(), renElement.params, mOptions);

					mSamplerOverrides[samplerKey] = samplerOverrides;

					renElement.samplerOverrides = samplerOverrides;
					samplerOverrides->refCount++;
				}
			}
		}
	}

	void RendererScene::updateRenderable(Renderable* renderable)
	{
		UINT32 renderableId = renderable->getRendererId();

		mInfo.renderables[renderableId]->updatePerObjectBuffer();
		mInfo.renderableCullInfos[renderableId].bounds = renderable->getBounds();
	}

	void RendererScene::unregisterRenderable(Renderable* renderable)
	{
		UINT32 renderableId = renderable->getRendererId();
		Renderable* lastRenerable = mInfo.renderables.back()->renderable;
		UINT32 lastRenderableId = lastRenerable->getRendererId();

		RendererObject* rendererObject = mInfo.renderables[renderableId];
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
			std::swap(mInfo.renderables[renderableId], mInfo.renderables[lastRenderableId]);
			std::swap(mInfo.renderableCullInfos[renderableId], mInfo.renderableCullInfos[lastRenderableId]);

			lastRenerable->setRendererId(renderableId);

			for (auto& element : elements)
				element.renderableId = renderableId;
		}

		// Last element is the one we want to erase
		mInfo.renderables.erase(mInfo.renderables.end() - 1);
		mInfo.renderableCullInfos.erase(mInfo.renderableCullInfos.end() - 1);

		bs_delete(rendererObject);
	}

	void RendererScene::registerReflectionProbe(ReflectionProbe* probe)
	{
		UINT32 probeId = (UINT32)mInfo.reflProbes.size();
		probe->setRendererId(probeId);

		mInfo.reflProbes.push_back(RendererReflectionProbe(probe));
		RendererReflectionProbe& probeInfo = mInfo.reflProbes.back();

		mInfo.reflProbeWorldBounds.push_back(probe->getBounds());

		// Find a spot in cubemap array
		UINT32 numArrayEntries = (UINT32)mInfo.reflProbeCubemapArrayUsedSlots.size();
		for(UINT32 i = 0; i < numArrayEntries; i++)
		{
			if(!mInfo.reflProbeCubemapArrayUsedSlots[i])
			{
				setReflectionProbeArrayIndex(probeId, i, false);
				mInfo.reflProbeCubemapArrayUsedSlots[i] = true;
				break;
			}
		}

		// No empty slot was found
		if (probeInfo.arrayIdx == (UINT32)-1)
		{
			setReflectionProbeArrayIndex(probeId, numArrayEntries, false);
			mInfo.reflProbeCubemapArrayUsedSlots.push_back(true);
		}

		if(probeInfo.arrayIdx > MaxReflectionCubemaps)
		{
			LOGERR("Reached the maximum number of allowed reflection probe cubemaps at once. "
				"Ignoring reflection probe data.");
		}
	}

	void RendererScene::updateReflectionProbe(ReflectionProbe* probe, bool texture)
	{
		// Should only get called if transform changes, any other mInfo.ajor changes and ReflProbeInfo entry gets rebuild
		UINT32 probeId = probe->getRendererId();
		mInfo.reflProbeWorldBounds[probeId] = probe->getBounds();

		if (texture)
		{
			RendererReflectionProbe& probeInfo = mInfo.reflProbes[probeId];
			probeInfo.arrayDirty = true;
		}
	}

	void RendererScene::unregisterReflectionProbe(ReflectionProbe* probe)
	{
		UINT32 probeId = probe->getRendererId();
		UINT32 arrayIdx = mInfo.reflProbes[probeId].arrayIdx;

		if (arrayIdx != (UINT32)-1)
			mInfo.reflProbeCubemapArrayUsedSlots[arrayIdx] = false;

		ReflectionProbe* lastProbe = mInfo.reflProbes.back().probe;
		UINT32 lastProbeId = lastProbe->getRendererId();

		if (probeId != lastProbeId)
		{
			// Swap current last element with the one we want to erase
			std::swap(mInfo.reflProbes[probeId], mInfo.reflProbes[lastProbeId]);
			std::swap(mInfo.reflProbeWorldBounds[probeId], mInfo.reflProbeWorldBounds[lastProbeId]);

			probe->setRendererId(probeId);
		}

		// Last element is the one we want to erase
		mInfo.reflProbes.erase(mInfo.reflProbes.end() - 1);
		mInfo.reflProbeWorldBounds.erase(mInfo.reflProbeWorldBounds.end() - 1);
	}

	void RendererScene::setReflectionProbeArrayIndex(UINT32 probeIdx, UINT32 arrayIdx, bool markAsClean)
	{
		RendererReflectionProbe* probe = &mInfo.reflProbes[probeIdx];
		probe->arrayIdx = arrayIdx;

		if (markAsClean)
			probe->arrayDirty = false;
	}

	void RendererScene::registerLightProbeVolume(LightProbeVolume* volume)
	{
		mInfo.lightProbes.notifyAdded(volume);
	}

	void RendererScene::updateLightProbeVolume(LightProbeVolume* volume)
	{
		mInfo.lightProbes.notifyDirty(volume);
	}

	void RendererScene::unregisterLightProbeVolume(LightProbeVolume* volume)
	{
		mInfo.lightProbes.notifyRemoved(volume);
	}

	void RendererScene::updateLightProbes()
	{
		mInfo.lightProbes.updateProbes();
	}

	void RendererScene::registerSkybox(Skybox* skybox)
	{
		mInfo.skybox = skybox;
	}

	void RendererScene::unregisterSkybox(Skybox* skybox)
	{
		if (mInfo.skybox == skybox)
			mInfo.skybox = nullptr;
	}

	void RendererScene::setOptions(const SPtr<RenderBeastOptions>& options)
	{
		mOptions = options;

		for (auto& entry : mInfo.views)
			entry->setStateReductionMode(mOptions->stateReductionMode);
	}

	RENDERER_VIEW_DESC RendererScene::createViewDesc(Camera* camera) const
	{
		SPtr<Viewport> viewport = camera->getViewport();
		ClearFlags clearFlags = viewport->getClearFlags();
		RENDERER_VIEW_DESC viewDesc;

		viewDesc.target.clearFlags = 0;
		if (clearFlags.isSet(ClearFlagBits::Color))
			viewDesc.target.clearFlags |= FBT_COLOR;

		if (clearFlags.isSet(ClearFlagBits::Depth))
			viewDesc.target.clearFlags |= FBT_DEPTH;

		if (clearFlags.isSet(ClearFlagBits::Stencil))
			viewDesc.target.clearFlags |= FBT_STENCIL;

		viewDesc.target.clearColor = viewport->getClearColorValue();
		viewDesc.target.clearDepthValue = viewport->getClearDepthValue();
		viewDesc.target.clearStencilValue = viewport->getClearStencilValue();

		viewDesc.target.target = viewport->getTarget();
		viewDesc.target.nrmViewRect = viewport->getArea();
		viewDesc.target.viewRect = viewport->getPixelArea();

		if (viewDesc.target.target != nullptr)
		{
			viewDesc.target.targetWidth = viewDesc.target.target->getProperties().width;
			viewDesc.target.targetHeight = viewDesc.target.target->getProperties().height;
		}
		else
		{
			viewDesc.target.targetWidth = 0;
			viewDesc.target.targetHeight = 0;
		}

		viewDesc.target.numSamples = camera->getMSAACount();

		viewDesc.triggerCallbacks = true;
		viewDesc.runPostProcessing = true;
		viewDesc.renderingReflections = false;

		viewDesc.cullFrustum = camera->getWorldFrustum();
		viewDesc.visibleLayers = camera->getLayers();
		viewDesc.nearPlane = camera->getNearClipDistance();
		viewDesc.farPlane = camera->getFarClipDistance();
		viewDesc.flipView = false;

		const Transform& tfrm = camera->getTransform();
		viewDesc.viewOrigin = tfrm.getPosition();
		viewDesc.viewDirection = tfrm.getForward();
		viewDesc.projTransform = camera->getProjectionMatrixRS();
		viewDesc.viewTransform = camera->getViewMatrix();
		viewDesc.projType = camera->getProjectionType();

		viewDesc.stateReduction = mOptions->stateReductionMode;
		viewDesc.sceneCamera = camera;

		return viewDesc;
	}

	void RendererScene::updateCameraRenderTargets(Camera* camera, bool remove)
	{
		SPtr<RenderTarget> renderTarget = camera->getViewport()->getTarget();

		// Remove from render target list
		int rtChanged = 0; // 0 - No RT, 1 - RT found, 2 - RT changed
		for (auto iterTarget = mInfo.renderTargets.begin(); iterTarget != mInfo.renderTargets.end(); ++iterTarget)
		{
			RendererRenderTarget& target = *iterTarget;
			for (auto iterCam = target.cameras.begin(); iterCam != target.cameras.end(); ++iterCam)
			{
				if (camera == *iterCam)
				{
					if(remove)
					{
						target.cameras.erase(iterCam);
						rtChanged = 1;
						
					}
					else
					{
						if (renderTarget != target.target)
						{
							target.cameras.erase(iterCam);
							rtChanged = 2;

						}
						else
							rtChanged = 1;
					}

					break;
				}
			}

			if (target.cameras.empty())
			{
				mInfo.renderTargets.erase(iterTarget);
				break;
			}
		}

		// Register in render target list
		if (renderTarget != nullptr && !remove && (rtChanged == 0 || rtChanged == 2))
		{
			auto findIter = std::find_if(mInfo.renderTargets.begin(), mInfo.renderTargets.end(),
				[&](const RendererRenderTarget& x) { return x.target == renderTarget; });

			if (findIter != mInfo.renderTargets.end())
			{
				findIter->cameras.push_back(camera);
			}
			else
			{
				mInfo.renderTargets.push_back(RendererRenderTarget());
				RendererRenderTarget& renderTargetData = mInfo.renderTargets.back();

				renderTargetData.target = renderTarget;
				renderTargetData.cameras.push_back(camera);
			}

			// Sort render targets based on priority
			auto cameraComparer = [&](const Camera* a, const Camera* b) { return a->getPriority() > b->getPriority(); };
			auto renderTargetInfoComparer = [&](const RendererRenderTarget& a, const RendererRenderTarget& b)
			{ return a.target->getProperties().priority > b.target->getProperties().priority; };
			std::sort(begin(mInfo.renderTargets), end(mInfo.renderTargets), renderTargetInfoComparer);

			for (auto& camerasPerTarget : mInfo.renderTargets)
			{
				Vector<Camera*>& cameras = camerasPerTarget.cameras;

				std::sort(begin(cameras), end(cameras), cameraComparer);
			}
		}
	}

	void RendererScene::refreshSamplerOverrides(bool force)
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
						override.state = SamplerOverrideUtility::generateSamplerOverride(samplerState, mOptions);
					else
						override.state = SamplerOverrideUtility::generateSamplerOverride(SamplerState::getDefault(), mOptions);

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

		UINT32 numRenderables = (UINT32)mInfo.renderables.size();
		for (UINT32 i = 0; i < numRenderables; i++)
		{
			for(auto& element : mInfo.renderables[i]->elements)
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

	void RendererScene::prepareRenderable(UINT32 idx, const FrameInfo& frameInfo)
	{
		if (mInfo.renderableReady[idx])
			return;
		
		// Note: Before uploading bone matrices perhaps check if they has actually been changed since last frame
		if(frameInfo.animData != nullptr)
			mInfo.renderables[idx]->renderable->updateAnimationBuffers(*frameInfo.animData);
		
		// Note: Could this step be moved in notifyRenderableUpdated, so it only triggers when material actually gets
		// changed? Although it shouldn't matter much because if the internal versions keeping track of dirty params.
		for (auto& element : mInfo.renderables[idx]->elements)
			element.material->updateParamsSet(element.params);
		
		mInfo.renderables[idx]->perObjectParamBuffer->flushToGPU();
		mInfo.renderableReady[idx] = true;
	}
}}
