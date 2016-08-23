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

using namespace std::placeholders;

namespace BansheeEngine
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

		CoreThread::instance().queueCommand(std::bind(&RenderBeast::initializeCore, this));
	}

	void RenderBeast::destroy()
	{
		CoreRenderer::destroy();

		gCoreAccessor().queueCommand(std::bind(&RenderBeast::destroyCore, this));
		gCoreAccessor().submitToCoreThread(true);
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

		mRenderTargets.clear();
		mCameras.clear();
		mRenderables.clear();

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

		mRenderables.push_back(RendererObject());
		mRenderableShaderData.push_back(RenderableShaderData());
		mWorldBounds.push_back(renderable->getBounds());

		RendererObject& rendererObject = mRenderables.back();
		rendererObject.renderable = renderable;

		RenderableShaderData& shaderData = mRenderableShaderData.back();
		shaderData.worldTransform = renderable->getTransform();
		shaderData.invWorldTransform = shaderData.worldTransform.inverseAffine();
		shaderData.worldNoScaleTransform = renderable->getTransformNoScale();
		shaderData.invWorldNoScaleTransform = shaderData.worldNoScaleTransform.inverseAffine();
		shaderData.worldDeterminantSign = shaderData.worldTransform.determinant3x3() >= 0.0f ? 1.0f : -1.0f;

		SPtr<MeshCore> mesh = renderable->getMesh();
		if (mesh != nullptr)
		{
			const MeshProperties& meshProps = mesh->getProperties();
			SPtr<VertexDeclarationCore> vertexDecl = mesh->getVertexData()->vertexDeclaration;

			for (UINT32 i = 0; i < meshProps.getNumSubMeshes(); i++)
			{
				rendererObject.elements.push_back(BeastRenderableElement());
				BeastRenderableElement& renElement = rendererObject.elements.back();

				renElement.mesh = mesh;
				renElement.subMesh = meshProps.getSubMesh(i);
				renElement.renderableId = renderableId;
				renElement.animationId = renderable->getAnimationId();

				renElement.material = renderable->getMaterial(i);
				if (renElement.material == nullptr)
					renElement.material = renderable->getMaterial(0);

				if (renElement.material != nullptr && renElement.material->getShader() == nullptr)
					renElement.material = nullptr;

				// Validate mesh <-> shader vertex bindings
				if (renElement.material != nullptr)
				{
					UINT32 numPasses = renElement.material->getNumPasses();
					for (UINT32 j = 0; j < numPasses; j++)
					{
						SPtr<PassCore> pass = renElement.material->getPass(j);

						SPtr<VertexDeclarationCore> shaderDecl = pass->getVertexProgram()->getInputDeclaration();
						if (!vertexDecl->isCompatible(shaderDecl))
						{
							Vector<VertexElement> missingElements = vertexDecl->getMissingElements(shaderDecl);

							StringStream wrnStream;
							wrnStream << "Provided mesh is missing required vertex attributes to render with the provided shader. Missing elements: " << std::endl;

							for (auto& entry : missingElements)
								wrnStream << "\t" << toString(entry.getSemantic()) << entry.getSemanticIdx() << std::endl;

							LOGWRN(wrnStream.str());
							break;
						}
					}
				}

				// If no material use the default material
				if (renElement.material == nullptr)
					renElement.material = mDefaultMaterial->getMaterial();

				// Determine which technique to use
				UINT32 techniqueIdx = -1;
				if(renderable->isAnimated())
					techniqueIdx = renElement.material->findTechnique(RTag_Animated);

				if (techniqueIdx == (UINT32)-1)
					techniqueIdx = renElement.material->getDefaultTechnique();

				renElement.techniqueIdx = techniqueIdx;

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
					if(matInfo.matVersion != renElement.material->getVersion())
					{
						if (matInfo.params.size() <= techniqueIdx)
							matInfo.params.resize(techniqueIdx + 1);

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

				mObjectRenderer->initElement(renElement);
			}
		}
	}

	void RenderBeast::notifyRenderableRemoved(RenderableCore* renderable)
	{
		UINT32 renderableId = renderable->getRendererId();
		RenderableCore* lastRenerable = mRenderables.back().renderable;
		UINT32 lastRenderableId = lastRenerable->getRendererId();

		Vector<BeastRenderableElement>& elements = mRenderables[renderableId].elements;
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
			std::swap(mRenderableShaderData[renderableId], mRenderableShaderData[lastRenderableId]);

			lastRenerable->setRendererId(renderableId);

			for (auto& element : elements)
				element.renderableId = renderableId;
		}

		// Last element is the one we want to erase
		mRenderables.erase(mRenderables.end() - 1);
		mWorldBounds.erase(mWorldBounds.end() - 1);
		mRenderableShaderData.erase(mRenderableShaderData.end() - 1);
	}

	void RenderBeast::notifyRenderableUpdated(RenderableCore* renderable)
	{
		UINT32 renderableId = renderable->getRendererId();

		RenderableShaderData& shaderData = mRenderableShaderData[renderableId];
		shaderData.worldTransform = renderable->getTransform();
		shaderData.invWorldTransform = shaderData.worldTransform.inverseAffine();
		shaderData.worldNoScaleTransform = renderable->getTransformNoScale();
		shaderData.invWorldNoScaleTransform = shaderData.worldNoScaleTransform.inverseAffine();
		shaderData.worldDeterminantSign = shaderData.worldTransform.determinant3x3() >= 0.0f ? 1.0f : -1.0f;

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
		updateCameraData(camera);
	}

	void RenderBeast::notifyCameraUpdated(const CameraCore* camera, UINT32 updateFlag)
	{
		if((updateFlag & (UINT32)CameraDirtyFlag::Everything) != 0)
		{
			updateCameraData(camera);
		}
		else if((updateFlag & (UINT32)CameraDirtyFlag::PostProcess) != 0)
		{
			RendererCamera& rendererCam = mCameras[camera];
			rendererCam.updatePP();
		} 
	}

	void RenderBeast::notifyCameraRemoved(const CameraCore* camera)
	{
		updateCameraData(camera, true);
	}

	void RenderBeast::updateCameraData(const CameraCore* camera, bool forceRemove)
	{
		SPtr<RenderTargetCore> renderTarget = camera->getViewport()->getTarget();
		if(forceRemove)
		{
			mCameras.erase(camera);
			renderTarget = nullptr;
		}
		else
		{
			mCameras[camera] = RendererCamera(camera, mCoreOptions->stateReductionMode);
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
			RendererCamera& rendererCam = entry.second;
			rendererCam.update(mCoreOptions->stateReductionMode);
		}
	}

	void RenderBeast::renderAll() 
	{
		// Sync all dirty sim thread CoreObject data to core thread
		CoreObjectManager::instance().syncToCore(gCoreAccessor());

		if (mOptionsDirty)
		{
			gCoreAccessor().queueCommand(std::bind(&RenderBeast::syncOptions, this, *mOptions));
			mOptionsDirty = false;
		}

		gCoreAccessor().queueCommand(std::bind(&RenderBeast::renderAllCore, this, gTime().getTime(), gTime().getFrameDelta()));
	}

	void RenderBeast::renderAllCore(float time, float delta)
	{
		THROW_IF_NOT_CORE_THREAD;

		gProfilerCPU().beginSample("renderAllCore");

		// Note: I'm iterating over all sampler states every frame. If this ends up being a performance
		// issue consider handling this internally in MaterialCore which can only do it when sampler states
		// are actually modified after sync
		refreshSamplerOverrides();

		// Update global per-frame hardware buffers
		mObjectRenderer->setParamFrameParams(time);

		// Generate render queues per camera
		for (auto& entry : mCameras)
			entry.second.determineVisible(mRenderables, mWorldBounds);

		const RendererAnimationData& animData = AnimationManager::instance().getRendererData();
		RendererFrame frameInfo(delta, animData);

		// Render everything, target by target
		for (auto& rtInfo : mRenderTargets)
		{
			SPtr<RenderTargetCore> target = rtInfo.target;
			Vector<const CameraCore*>& cameras = rtInfo.cameras;

			RenderAPICore::instance().beginFrame();

			UINT32 numCameras = (UINT32)cameras.size();
			for (UINT32 i = 0; i < numCameras; i++)
			{
				bool isOverlayCamera = cameras[i]->getFlags().isSet(CameraFlag::Overlay);
				if (!isOverlayCamera)
					render(frameInfo, rtInfo, i);
				else
					renderOverlay(frameInfo, rtInfo, i);
			}

			RenderAPICore::instance().endFrame();
			RenderAPICore::instance().swapBuffers(target);
		}

		gProfilerCPU().endSample("renderAllCore");
	}

	void RenderBeast::render(const RendererFrame& frameInfo, RendererRenderTarget& rtInfo, UINT32 camIdx)
	{
		gProfilerCPU().beginSample("Render");

		const CameraCore* camera = rtInfo.cameras[camIdx];
		RendererCamera& rendererCam = mCameras[camera];
		CameraShaderData cameraShaderData = rendererCam.getShaderData();

		assert(!camera->getFlags().isSet(CameraFlag::Overlay));

		mObjectRenderer->setPerCameraParams(cameraShaderData);
		rendererCam.beginRendering(true);

		SPtr<RenderTargets> renderTargets = rendererCam.getRenderTargets();
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
		const Vector<RenderQueueElement>& opaqueElements = rendererCam.getOpaqueQueue()->getSortedElements();
		for (auto iter = opaqueElements.begin(); iter != opaqueElements.end(); ++iter)
		{
			BeastRenderableElement* renderElem = static_cast<BeastRenderableElement*>(iter->renderElem);
			renderElement(*renderElem, iter->passIdx, iter->applyPass, frameInfo, cameraShaderData.viewProj);
		}

		renderTargets->bindSceneColor(true);

		//// Render light pass
		{
			SPtr<GpuParamBlockBufferCore> perCameraBuffer = mObjectRenderer->getPerCameraParams().getBuffer();

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
		const Vector<RenderQueueElement>& transparentElements = rendererCam.getTransparentQueue()->getSortedElements();
		for (auto iter = transparentElements.begin(); iter != transparentElements.end(); ++iter)
		{
			BeastRenderableElement* renderElem = static_cast<BeastRenderableElement*>(iter->renderElem);
			renderElement(*renderElem, iter->passIdx, iter->applyPass, frameInfo, cameraShaderData.viewProj);
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
			camera, rendererCam.getPPInfo(), frameInfo.delta);

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

		rendererCam.endRendering();

		gProfilerCPU().endSample("Render");
	}

	void RenderBeast::renderOverlay(const RendererFrame& frameInfo, RendererRenderTarget& rtData, UINT32 camIdx)
	{
		gProfilerCPU().beginSample("RenderOverlay");

		const CameraCore* camera = rtData.cameras[camIdx];
		assert(camera->getFlags().isSet(CameraFlag::Overlay));

		SPtr<ViewportCore> viewport = camera->getViewport();
		RendererCamera& rendererCam = mCameras[camera];
		CameraShaderData cameraShaderData = rendererCam.getShaderData();

		mObjectRenderer->setPerCameraParams(cameraShaderData);
		rendererCam.beginRendering(false);

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

		rendererCam.endRendering();

		gProfilerCPU().endSample("RenderOverlay");
	}
	
	void RenderBeast::renderElement(const BeastRenderableElement& element, UINT32 passIdx, bool bindPass,
		const RendererFrame& frameInfo, const Matrix4& viewProj)
	{
		SPtr<MaterialCore> material = element.material;

		UINT32 rendererId = element.renderableId;
		Matrix4 worldViewProjMatrix = viewProj * mRenderableShaderData[rendererId].worldTransform;

		SPtr<GpuBufferCore> boneMatrices = element.boneMatrixBuffer;
		if(element.animationId != (UINT64)-1)
		{
			// Note: If multiple elements are using the same animation (not possible atm), this buffer should be shared by
			// all such elements

			const RendererAnimationData& animData = frameInfo.animData;

			auto iterFind = animData.poseInfos.find(element.animationId);
			if(iterFind != animData.poseInfos.end())
			{
				const RendererAnimationData::PoseInfo& poseInfo = iterFind->second;

				UINT8* dest = (UINT8*)boneMatrices->lock(0, poseInfo.numBones * 3 * sizeof(Vector4), GBL_WRITE_ONLY_DISCARD);
				for(UINT32 i = 0; i < poseInfo.numBones; i++)
				{
					const Matrix4& transform = animData.transforms[poseInfo.startIdx + i];
					memcpy(dest, &transform, 12 * sizeof(float)); // Assuming row-major format

					dest += 12 * sizeof(float);
				}

				boneMatrices->unlock();
			}
		}

		mObjectRenderer->setPerObjectParams(element, mRenderableShaderData[rendererId], worldViewProjMatrix, boneMatrices);
		material->updateParamsSet(element.params, element.techniqueIdx);

		if (bindPass)
			RendererUtility::instance().setPass(material, passIdx, element.techniqueIdx);

		if (element.samplerOverrides != nullptr)
			setPassParams(element.params, element.samplerOverrides, passIdx);
		else
			setPassParams(element.params, nullptr, passIdx);

		gRendererUtility().draw(element.mesh, element.subMesh);
	}

	void RenderBeast::refreshSamplerOverrides(bool force)
	{
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
						override.state = SamplerOverrideUtility::generateSamplerOverride(SamplerStateCore::getDefault(), mCoreOptions);;
				}
			}
		}
	}

	void RenderBeast::setPassParams(const SPtr<GpuParamsSetCore>& paramsSet, const MaterialSamplerOverrides* samplerOverrides,
		UINT32 passIdx)
	{
		THROW_IF_NOT_CORE_THREAD;

		RenderAPICore& rapi = RenderAPICore::instance();

		struct StageData
		{
			GpuProgramType type;
			SPtr<GpuParamsCore> params;
		};

		const UINT32 numStages = 6;
		GpuProgramType stages[numStages] =
		{
			{ GPT_VERTEX_PROGRAM },
			{ GPT_FRAGMENT_PROGRAM },
			{ GPT_GEOMETRY_PROGRAM },
			{ GPT_HULL_PROGRAM },
			{ GPT_DOMAIN_PROGRAM },
			{ GPT_COMPUTE_PROGRAM }
		};

		for (UINT32 i = 0; i < numStages; i++)
		{
			SPtr<GpuParamsCore> params = paramsSet->getGpuParams(stages[i], passIdx);
			if (params == nullptr)
				continue;

			const GpuParamDesc& paramDesc = params->getParamDesc();

			for (auto iter = paramDesc.samplers.begin(); iter != paramDesc.samplers.end(); ++iter)
			{
				SPtr<SamplerStateCore> samplerState;

				if (samplerOverrides != nullptr)
				{
					UINT32 overrideIndex = samplerOverrides->passes[passIdx].stages[i].stateOverrides[iter->second.slot];
					if(overrideIndex != (UINT32)-1)
						samplerState = samplerOverrides->overrides[overrideIndex].state;
				}
				
				if(samplerState == nullptr)
					samplerState = params->getSamplerState(iter->second.slot);

				if (samplerState == nullptr)
					rapi.setSamplerState(stages[i], iter->second.slot, SamplerStateCore::getDefault());
				else
					rapi.setSamplerState(stages[i], iter->second.slot, samplerState);
			}

			for (auto iter = paramDesc.textures.begin(); iter != paramDesc.textures.end(); ++iter)
			{
				SPtr<TextureCore> texture = params->getTexture(iter->second.slot);
				rapi.setTexture(stages[i], iter->second.slot, texture);
			}

			for (auto iter = paramDesc.loadStoreTextures.begin(); iter != paramDesc.loadStoreTextures.end(); ++iter)
			{
				SPtr<TextureCore> texture = params->getLoadStoreTexture(iter->second.slot);
				const TextureSurface& surface = params->getLoadStoreSurface(iter->second.slot);

				if (texture == nullptr)
					rapi.setLoadStoreTexture(stages[i], iter->second.slot, false, nullptr, surface);
				else
					rapi.setLoadStoreTexture(stages[i], iter->second.slot, true, texture, surface);
			}

			for (auto iter = paramDesc.buffers.begin(); iter != paramDesc.buffers.end(); ++iter)
			{
				SPtr<GpuBufferCore> buffer = params->getBuffer(iter->second.slot);

				bool isLoadStore = iter->second.type != GPOT_BYTE_BUFFER &&
					iter->second.type != GPOT_STRUCTURED_BUFFER;

				rapi.setBuffer(stages[i], iter->second.slot, buffer, isLoadStore);
			}

			for (auto iter = paramDesc.paramBlocks.begin(); iter != paramDesc.paramBlocks.end(); ++iter)
			{
				SPtr<GpuParamBlockBufferCore> blockBuffer = params->getParamBlockBuffer(iter->second.slot);
				blockBuffer->flushToGPU();

				rapi.setParamBuffer(stages[i], iter->second.slot, blockBuffer, paramDesc);
			}
		}
	}
}