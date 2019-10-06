//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsRendererScene.h"
#include "Renderer/BsCamera.h"
#include "Renderer/BsLight.h"
#include "Renderer/BsSkybox.h"
#include "Renderer/BsReflectionProbe.h"
#include "Renderer/BsRenderer.h"
#include "Particles/BsParticleManager.h"
#include "Mesh/BsMesh.h"
#include "Material/BsPass.h"
#include "Material/BsGpuParamsSet.h"
#include "Utility/BsSamplerOverrides.h"
#include "BsRenderBeastOptions.h"
#include "BsRenderBeast.h"
#include "BsRendererDecal.h"
#include "Image/BsSpriteTexture.h"
#include "Shading/BsGpuParticleSimulation.h"
#include "Renderer/BsDecal.h"
#include "Renderer/BsRendererUtility.h"

namespace bs {	namespace ct
{
	PerFrameParamDef gPerFrameParamDef;

	static const ShaderVariation* DECAL_VAR_LOOKUP[2][3] =
	{
		{
			&getDecalShaderVariation<false, MSAAMode::None>(),
			&getDecalShaderVariation<false, MSAAMode::Single>(),
			&getDecalShaderVariation<false, MSAAMode::Full>()
		},
		{
			&getDecalShaderVariation<true, MSAAMode::None>(),
			&getDecalShaderVariation<true, MSAAMode::Single>(),
			&getDecalShaderVariation<true, MSAAMode::Full>()
		}
	};

	/** Returns a specific forward rendering shader variation. */
	template<bool SKINNED, bool MORPH, bool CLUSTERED, bool WRITE_VELOCITY>
	static const ShaderVariation& getForwardRenderingVariation(bool supportsVelocityWrites)
	{
		if (!supportsVelocityWrites)
		{
			static ShaderVariation variation = ShaderVariation(
				{
					ShaderVariation::Param("SKINNED", SKINNED),
					ShaderVariation::Param("MORPH", MORPH),
					ShaderVariation::Param("CLUSTERED", CLUSTERED),
				});

			return variation;
		}
		else
		{
			static ShaderVariation variation = ShaderVariation(
				{
					ShaderVariation::Param("SKINNED", SKINNED),
					ShaderVariation::Param("MORPH", MORPH),
					ShaderVariation::Param("CLUSTERED", CLUSTERED),
					ShaderVariation::Param("WRITE_VELOCITY", WRITE_VELOCITY),
				});

			return variation;
		}
	}


	/** Returns a specific forward rendering shader variation. */
	template<bool CLUSTERED, bool WRITE_VELOCITY>
	static const ShaderVariation* getClusteredForwardRenderingVariation(RenderableAnimType animType, bool shaderCanWriteVelocity)
	{
		const ShaderVariation* VAR_LOOKUP[4];
		VAR_LOOKUP[0] = &getForwardRenderingVariation<false, false, CLUSTERED, WRITE_VELOCITY>(shaderCanWriteVelocity);
		VAR_LOOKUP[1] = &getForwardRenderingVariation<true, false, CLUSTERED, WRITE_VELOCITY>(shaderCanWriteVelocity);
		VAR_LOOKUP[2] = &getForwardRenderingVariation<false, true, CLUSTERED, WRITE_VELOCITY>(shaderCanWriteVelocity);
		VAR_LOOKUP[3] = &getForwardRenderingVariation<true, true, CLUSTERED, WRITE_VELOCITY>(shaderCanWriteVelocity);

		return VAR_LOOKUP[(int)animType];
	}

	/** Returns a specific base pass shader variation. */
	template<bool WRITE_VELOCITY>
	static const ShaderVariation* getBasePassVariation(bool useForwardRendering, bool supportsClusteredForward,
		bool shaderCanWriteVelocity, RenderableAnimType animType)
	{
		if (useForwardRendering)
		{
			if (supportsClusteredForward)
				return getClusteredForwardRenderingVariation<true, WRITE_VELOCITY>(animType, shaderCanWriteVelocity);
			else
				return getClusteredForwardRenderingVariation<false, WRITE_VELOCITY>(animType, shaderCanWriteVelocity);
		}
		else
		{
			const ShaderVariation* VAR_LOOKUP[4];
			VAR_LOOKUP[0] = &getVertexInputVariation<false, false, WRITE_VELOCITY>(shaderCanWriteVelocity);
			VAR_LOOKUP[1] = &getVertexInputVariation<true, false, WRITE_VELOCITY>(shaderCanWriteVelocity);
			VAR_LOOKUP[2] = &getVertexInputVariation<false, true, WRITE_VELOCITY>(shaderCanWriteVelocity);
			VAR_LOOKUP[3] = &getVertexInputVariation<true, true, WRITE_VELOCITY>(shaderCanWriteVelocity);

			return VAR_LOOKUP[(int)animType];
		}
	}

	/** Initializes a specific base pass technique on the provided material and returns the technique index. */
	static UINT32 initAndRetrieveBasePassTechnique(Material& material, bool useForwardRendering, bool supportsClusteredForward,
		bool shaderCanWriteVelocity, bool writeVelocity, RenderableAnimType animType)
	{
		const ShaderVariation* variation = writeVelocity ?
			getBasePassVariation<true>(useForwardRendering, supportsClusteredForward, shaderCanWriteVelocity, animType) :
			getBasePassVariation<false>(useForwardRendering, supportsClusteredForward, shaderCanWriteVelocity, animType);
		
		FIND_TECHNIQUE_DESC findDesc;
		findDesc.variation = variation;
		findDesc.override = true;

		UINT32 techniqueIdx = material.findTechnique(findDesc);

		if (techniqueIdx == (UINT32)-1)
			techniqueIdx = material.getDefaultTechnique();

		// Make sure the technique shaders are compiled
		const SPtr<Technique>& technique = material.getTechnique(techniqueIdx);
		if (technique)
			technique->compile();

		return techniqueIdx;
	}

	static void validateBasePassMaterial(Material& material, RenderableAnimType animType, UINT32 techniqueIdx, VertexDeclaration& vertexDecl)
	{
		// Validate mesh <-> shader vertex bindings
		UINT32 numPasses = material.getNumPasses(techniqueIdx);
		for (UINT32 j = 0; j < numPasses; j++)
		{
			SPtr<Pass> pass = material.getPass(j, techniqueIdx);
			SPtr<GraphicsPipelineState> graphicsPipeline = pass->getGraphicsPipelineState();

			SPtr<VertexDeclaration> shaderDecl = graphicsPipeline->getVertexProgram()->getInputDeclaration();
			if (shaderDecl && !vertexDecl.isCompatible(shaderDecl))
			{
				Vector<VertexElement> missingElements = vertexDecl.getMissingElements(shaderDecl);

				// If using morph shapes ignore POSITION1 and NORMAL1 missing since we assign them from within the renderer
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
					wrnStream << "Provided mesh is missing required vertex attributes to render with the \
									provided shader. Missing elements: " << std::endl;

					for (auto& entry : missingElements)
						wrnStream << "\t" << toString(entry.getSemantic()) << entry.getSemanticIdx() << std::endl;

					BS_LOG(Warning, Renderer, wrnStream.str());
					break;
				}
			}
		}
	}

	RendererScene::RendererScene(const SPtr<RenderBeastOptions>& options)
		:mOptions(options)
	{
		mPerFrameParamBuffer = gPerFrameParamDef.createBuffer();
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

		if ((updateFlag & (UINT32)CameraDirtyFlag::Redraw) != 0)
			view->_notifyNeedsRedraw();

		UINT32 updateEverythingFlag = (UINT32)ActorDirtyFlag::Everything
			| (UINT32)ActorDirtyFlag::Active
			| (UINT32)CameraDirtyFlag::Viewport;

		if((updateFlag & updateEverythingFlag) != 0)
		{
			RENDERER_VIEW_DESC viewDesc = createViewDesc(camera);

			view->setView(viewDesc);
			view->setRenderSettings(camera->getRenderSettings());

			updateCameraRenderTargets(camera);
			return;
		}

		if ((updateFlag & (UINT32)CameraDirtyFlag::RenderSettings) != 0)
			view->setRenderSettings(camera->getRenderSettings());

		const Transform& tfrm = camera->getTransform();
		view->setTransform(
			tfrm.getPosition(),
			tfrm.getForward(),
			camera->getViewMatrix(),
			camera->getProjectionMatrixRS(),
			camera->getWorldFrustum());

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

		mInfo.renderables.push_back(bs_new<RendererRenderable>());
		mInfo.renderableCullInfos.push_back(CullInfo(renderable->getBounds(), renderable->getLayer(), renderable->getCullDistanceFactor()));

		RendererRenderable* rendererRenderable = mInfo.renderables.back();
		rendererRenderable->renderable = renderable;
		rendererRenderable->worldTfrm = renderable->getMatrix();
		rendererRenderable->prevWorldTfrm = rendererRenderable->worldTfrm;
		rendererRenderable->prevFrameDirtyState = PrevFrameDirtyState::Clean;
		rendererRenderable->updatePerObjectBuffer();

		SPtr<Mesh> mesh = renderable->getMesh();
		if (mesh != nullptr)
		{
			const MeshProperties& meshProps = mesh->getProperties();
			SPtr<VertexDeclaration> vertexDecl = mesh->getVertexData()->vertexDeclaration;

			for (UINT32 i = 0; i < meshProps.getNumSubMeshes(); i++)
			{
				rendererRenderable->elements.push_back(RenderableElement());
				RenderableElement& renElement = rendererRenderable->elements.back();

				renElement.type = (UINT32)RenderElementType::Renderable;
				renElement.mesh = mesh;
				renElement.subMesh = meshProps.getSubMesh(i);
				renElement.animType = renderable->getAnimType();
				renElement.animationId = renderable->getAnimationId();
				renElement.morphShapeVersion = 0;
				renElement.morphShapeBuffer = renderable->getMorphShapeBuffer();
				renElement.boneMatrixBuffer = renderable->getBoneMatrixBuffer();
				renElement.bonePrevMatrixBuffer = renderable->getBonePrevMatrixBuffer();
				renElement.morphVertexDeclaration = renderable->getMorphVertexDeclaration();

				renElement.material = renderable->getMaterial(i);
				if (renElement.material == nullptr)
					renElement.material = renderable->getMaterial(0);

				if (renElement.material != nullptr && renElement.material->getShader() == nullptr)
					renElement.material = nullptr;

				// If no material use the default material
				if (renElement.material == nullptr)
					renElement.material = Material::create(DefaultMaterial::get()->getShader());

				// Determine which technique to use
				static_assert((UINT32)RenderableAnimType::Count == 4, "RenderableAnimType is expected to have four sequential entries.");

				const SPtr<Shader>& shader = renElement.material->getShader();
				ShaderFlags shaderFlags = shader->getFlags();
				const bool useForwardRendering = shaderFlags.isSet(ShaderFlag::Forward) || shaderFlags.isSet(ShaderFlag::Transparent);
				bool supportsClusteredForward = gRenderBeast()->getFeatureSet() == RenderBeastFeatureSet::Desktop;

				const Vector<ShaderVariationParamInfo>& variationParams = shader->getVariationParams();
				const bool shaderCanWriteVelocity = std::find_if(variationParams.begin(), variationParams.end(),
					[](const ShaderVariationParamInfo& x) { return x.identifier == "WRITE_VELOCITY"; }) != variationParams.end();
				
				const bool writeVelocity = shaderCanWriteVelocity && renderable->getWriteVelocity();
				
				RenderableAnimType animType = renderable->getAnimType();

				renElement.defaultTechniqueIdx = initAndRetrieveBasePassTechnique(*renElement.material, useForwardRendering,
					supportsClusteredForward, shaderCanWriteVelocity, false, animType);

#if BS_DEBUG_MODE
				validateBasePassMaterial(*renElement.material, animType, renElement.defaultTechniqueIdx, *vertexDecl);
#endif

				// Generate or assigned renderer specific data for the material
				renElement.params = renElement.material->createParamsSet(renElement.defaultTechniqueIdx);
				renElement.material->updateParamsSet(renElement.params, 0.0f, true);

				if (writeVelocity)
				{
					renElement.writeVelocityTechniqueIdx = initAndRetrieveBasePassTechnique(*renElement.material, useForwardRendering,
						supportsClusteredForward, shaderCanWriteVelocity, true, animType);

#if BS_DEBUG_MODE
					validateBasePassMaterial(*renElement.material, animType, renElement.writeVelocityTechniqueIdx, *vertexDecl);
#endif

					// Note: Using the same params as the non-velocity technique. There are assumed to be no differences
				}
				else
					renElement.writeVelocityTechniqueIdx = (UINT32)-1;
				
				// Generate or assign sampler state overrides
				renElement.samplerOverrides = allocSamplerStateOverrides(renElement);
			}
		}

		// Prepare all parameter bindings
		for(auto& element : rendererRenderable->elements)
		{
			SPtr<Shader> shader = element.material->getShader();
			if (shader == nullptr)
			{
				BS_LOG(Warning, Renderer, "Missing shader on material.");
				continue;
			}

			SPtr<GpuParams> gpuParams = element.params->getGpuParams();

			// Note: Perhaps perform buffer validation to ensure expected buffer has the same size and layout as the
			// provided buffer, and show a warning otherwise. But this is perhaps better handled on a higher level.
			gpuParams->setParamBlockBuffer("PerFrame", mPerFrameParamBuffer);
			gpuParams->setParamBlockBuffer("PerObject", rendererRenderable->perObjectParamBuffer);
			gpuParams->setParamBlockBuffer("PerCall", rendererRenderable->perCallParamBuffer);

			gpuParams->getParamInfo()->getBindings(
				GpuPipelineParamInfoBase::ParamType::ParamBlock,
				"PerCamera",
				element.perCameraBindings
			);

			if (gpuParams->hasBuffer(GPT_VERTEX_PROGRAM, "boneMatrices"))
				gpuParams->setBuffer(GPT_VERTEX_PROGRAM, "boneMatrices", element.boneMatrixBuffer);

			if (gpuParams->hasBuffer(GPT_VERTEX_PROGRAM, "prevBoneMatrices"))
				gpuParams->setBuffer(GPT_VERTEX_PROGRAM, "prevBoneMatrices", element.bonePrevMatrixBuffer);

			ShaderFlags shaderFlags = shader->getFlags();
			const bool useForwardRendering = shaderFlags.isSet(ShaderFlag::Forward) || shaderFlags.isSet(ShaderFlag::Transparent);

			if (useForwardRendering)
			{
				const bool supportsClusteredForward = gRenderBeast()->getFeatureSet() == RenderBeastFeatureSet::Desktop;

				element.forwardLightingParams.populate(gpuParams, supportsClusteredForward);
				element.imageBasedParams.populate(gpuParams, GPT_FRAGMENT_PROGRAM, true, supportsClusteredForward,
					supportsClusteredForward);
			}
		}
	}

	void RendererScene::updateRenderable(Renderable* renderable)
	{
		UINT32 renderableId = renderable->getRendererId();

		RendererRenderable* rendererRenderable = mInfo.renderables[renderableId];

		if(rendererRenderable->prevFrameDirtyState != PrevFrameDirtyState::Updated)
			rendererRenderable->prevWorldTfrm = rendererRenderable->worldTfrm;

		rendererRenderable->worldTfrm = renderable->getMatrix();
		rendererRenderable->prevFrameDirtyState = PrevFrameDirtyState::Updated;

		mInfo.renderables[renderableId]->updatePerObjectBuffer();
		mInfo.renderableCullInfos[renderableId].bounds = renderable->getBounds();
		mInfo.renderableCullInfos[renderableId].cullDistanceFactor = renderable->getCullDistanceFactor();
	}

	void RendererScene::unregisterRenderable(Renderable* renderable)
	{
		UINT32 renderableId = renderable->getRendererId();
		Renderable* lastRenerable = mInfo.renderables.back()->renderable;
		UINT32 lastRenderableId = lastRenerable->getRendererId();

		RendererRenderable* rendererRenderable = mInfo.renderables[renderableId];
		Vector<RenderableElement>& elements = rendererRenderable->elements;
		for (auto& element : elements)
		{
			freeSamplerStateOverrides(element);
			element.samplerOverrides = nullptr;
		}

		if (renderableId != lastRenderableId)
		{
			// Swap current last element with the one we want to erase
			std::swap(mInfo.renderables[renderableId], mInfo.renderables[lastRenderableId]);
			std::swap(mInfo.renderableCullInfos[renderableId], mInfo.renderableCullInfos[lastRenderableId]);

			lastRenerable->setRendererId(renderableId);
		}

		// Last element is the one we want to erase
		mInfo.renderables.erase(mInfo.renderables.end() - 1);
		mInfo.renderableCullInfos.erase(mInfo.renderableCullInfos.end() - 1);

		bs_delete(rendererRenderable);
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
			BS_LOG(Error, Renderer, "Reached the maximum number of allowed reflection probe cubemaps at once. "
				"Ignoring reflection probe data.");
		}
	}

	void RendererScene::updateReflectionProbe(ReflectionProbe* probe, bool texture)
	{
		// Should only get called if transform changes, any other major changes and ReflProbeInfo entry gets rebuild
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

			lastProbe->setRendererId(probeId);
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

	void RendererScene::registerParticleSystem(ParticleSystem* particleSystem)
	{
		const auto rendererId = (UINT32)mInfo.particleSystems.size();
		particleSystem->setRendererId(rendererId);

		mInfo.particleSystems.push_back(RendererParticles());
		mInfo.particleSystemCullInfos.push_back(CullInfo(Bounds(), particleSystem->getLayer()));

		RendererParticles& rendererParticles = mInfo.particleSystems.back();
		rendererParticles.particleSystem = particleSystem;

		updateParticleSystem(particleSystem, false);

		rendererParticles.prevLocalToWorld = rendererParticles.localToWorld;
		rendererParticles.prevFrameDirtyState = PrevFrameDirtyState::Clean;
	}

	void RendererScene::updateParticleSystem(ParticleSystem* particleSystem, bool tfrmOnly)
	{
		const UINT32 rendererId = particleSystem->getRendererId();
		RendererParticles& rendererParticles = mInfo.particleSystems[rendererId];

		rendererParticles.prevLocalToWorld = rendererParticles.localToWorld;
		rendererParticles.prevFrameDirtyState = PrevFrameDirtyState::Updated;

		const ParticleSystemSettings& settings = particleSystem->getSettings();
		if (settings.simulationSpace == ParticleSimulationSpace::Local)
		{
			const Transform& tfrm = particleSystem->getTransform();
			rendererParticles.localToWorld = tfrm.getMatrix();
		}
		else
			rendererParticles.localToWorld = Matrix4::IDENTITY;

		if(tfrmOnly)
		{
			rendererParticles.updatePerObjectBuffer();
			return;
		}

		rendererParticles.perObjectParamBuffer = gPerObjectParamDef.createBuffer();
		rendererParticles.updatePerObjectBuffer();

		SPtr<GpuParamBlockBuffer> particlesParamBuffer = gParticlesParamDef.createBuffer();
		rendererParticles.particlesParamBuffer = particlesParamBuffer;

		Vector3 axisForward = settings.orientationPlaneNormal;

		Vector3 axisUp = Vector3::UNIT_Y;
		if (axisForward.dot(axisUp) > 0.9998f)
			axisUp = Vector3::UNIT_Z;

		Vector3 axisRight = axisUp.cross(axisForward);
		Vector3::orthonormalize(axisRight, axisUp, axisForward);

		gParticlesParamDef.gAxisUp.set(particlesParamBuffer, axisUp);
		gParticlesParamDef.gAxisRight.set(particlesParamBuffer, axisRight);

		// Initialize the variant of the particle system for GPU simulation, if needed
		if (settings.gpuSimulation)
		{
			if (!rendererParticles.gpuParticleSystem)
				rendererParticles.gpuParticleSystem = bs_pool_new<GpuParticleSystem>(particleSystem);
		}
		else
		{
			if (rendererParticles.gpuParticleSystem)
			{
				bs_pool_delete(rendererParticles.gpuParticleSystem);
				rendererParticles.gpuParticleSystem = nullptr;
			}
		}

		ParticlesRenderElement& renElement = rendererParticles.renderElement;
		renElement.type = (UINT32)RenderElementType::Particle;

		renElement.material = settings.material;

		if (renElement.material != nullptr && renElement.material->getShader() == nullptr)
			renElement.material = nullptr;

		// If no material use the default material
		if (renElement.material == nullptr)
			renElement.material = Material::create(DefaultParticlesMat::get()->getShader());

		const SPtr<Shader> shader = renElement.material->getShader();

		SpriteTexture* spriteTexture = nullptr;
		if (shader->hasTextureParam("gTexture"))
			spriteTexture = renElement.material->getSpriteTexture("gTexture").get();

		if(!spriteTexture && shader->hasTextureParam("gAlbedoTex"))
			spriteTexture = renElement.material->getSpriteTexture("gAlbedoTex").get();

		if (spriteTexture)
		{
			gParticlesParamDef.gUVOffset.set(particlesParamBuffer, spriteTexture->getOffset());
			gParticlesParamDef.gUVScale.set(particlesParamBuffer, spriteTexture->getScale());

			const SpriteSheetGridAnimation& anim = spriteTexture->getAnimation();
			gParticlesParamDef.gSubImageSize.set(particlesParamBuffer,
				Vector4((float)anim.numColumns, (float)anim.numRows, 1.0f / anim.numColumns, 1.0f / anim.numRows));
		}
		else
		{
			gParticlesParamDef.gUVOffset.set(particlesParamBuffer, Vector2::ZERO);
			gParticlesParamDef.gUVScale.set(particlesParamBuffer, Vector2::ONE);
			gParticlesParamDef.gSubImageSize.set(particlesParamBuffer, Vector4(1.0f, 1.0f, 1.0f, 1.0f));
		}

		const ParticleOrientation orientation = settings.orientation;
		const bool lockY = settings.orientationLockY;
		const bool gpu = settings.gpuSimulation;
		const bool is3d = settings.renderMode == ParticleRenderMode::Mesh;

		ShaderFlags shaderFlags = shader->getFlags();
		const bool requiresForwardLighting = shaderFlags.isSet(ShaderFlag::Forward);
		const bool supportsClusteredForward = gRenderBeast()->getFeatureSet() == RenderBeastFeatureSet::Desktop;

		ParticleForwardLightingType forwardLightingType;
		if(requiresForwardLighting)
		{
			forwardLightingType = supportsClusteredForward
				? ParticleForwardLightingType::Clustered
				: ParticleForwardLightingType::Standard;
		}
		else
			forwardLightingType = ParticleForwardLightingType::None;

		const ShaderVariation* variation = &getParticleShaderVariation(orientation, lockY, gpu, is3d, forwardLightingType);

		FIND_TECHNIQUE_DESC findDesc;
		findDesc.variation = variation;
		findDesc.override = true;

		UINT32 techniqueIdx = renElement.material->findTechnique(findDesc);

		if (techniqueIdx == (UINT32)-1)
			techniqueIdx = renElement.material->getDefaultTechnique();

		renElement.defaultTechniqueIdx = techniqueIdx;

		// Make sure the technique shaders are compiled
		const SPtr<Technique>& technique = renElement.material->getTechnique(techniqueIdx);
		if (technique)
			technique->compile();

		// Generate or assigned renderer specific data for the material
		renElement.params = renElement.material->createParamsSet(techniqueIdx);
		renElement.material->updateParamsSet(renElement.params, 0.0f, true);

		SPtr<GpuParams> gpuParams = renElement.params->getGpuParams();

		if (gpu)
		{
			gpuParams->getTextureParam(GPT_VERTEX_PROGRAM, "gPositionTimeTex",
				renElement.paramsGPU.positionTimeTexture);
			gpuParams->getTextureParam(GPT_VERTEX_PROGRAM, "gSizeRotationTex",
				renElement.paramsGPU.sizeRotationTexture);
			gpuParams->getTextureParam(GPT_VERTEX_PROGRAM, "gCurvesTex",
				renElement.paramsGPU.curvesTexture);

			rendererParticles.gpuParticlesParamBuffer = gGpuParticlesParamDef.createBuffer();
			renElement.is3D = false;
		}
		else
		{
			switch (settings.renderMode)
			{
			case ParticleRenderMode::Billboard:
				gpuParams->getTextureParam(GPT_VERTEX_PROGRAM, "gPositionAndRotTex",
					renElement.paramsCPUBillboard.positionAndRotTexture);
				gpuParams->getTextureParam(GPT_VERTEX_PROGRAM, "gColorTex",
					renElement.paramsCPUBillboard.colorTexture);
				gpuParams->getTextureParam(GPT_VERTEX_PROGRAM, "gSizeAndFrameIdxTex",
					renElement.paramsCPUBillboard.sizeAndFrameIdxTexture);

				renElement.is3D = false;
				break;
			case ParticleRenderMode::Mesh:
				gpuParams->getTextureParam(GPT_VERTEX_PROGRAM, "gPositionTex",
					renElement.paramsCPUMesh.positionTexture);
				gpuParams->getTextureParam(GPT_VERTEX_PROGRAM, "gColorTex",
					renElement.paramsCPUMesh.colorTexture);
				gpuParams->getTextureParam(GPT_VERTEX_PROGRAM, "gSizeTex",
					renElement.paramsCPUMesh.sizeTexture);
				gpuParams->getTextureParam(GPT_VERTEX_PROGRAM, "gRotationTex",
					renElement.paramsCPUMesh.rotationTexture);

				renElement.is3D = true;
				renElement.mesh = settings.mesh;
				break;
			default:
				break;
			}

			rendererParticles.gpuParticlesParamBuffer = nullptr;
		}

		// Note: Perhaps perform buffer validation to ensure expected buffer has the same size and layout as the
		// provided buffer, and show a warning otherwise. But this is perhaps better handled on a higher level.
		gpuParams->setParamBlockBuffer("ParticleParams", rendererParticles.particlesParamBuffer);
		gpuParams->setParamBlockBuffer("PerObject", rendererParticles.perObjectParamBuffer);
		gpuParams->setParamBlockBuffer("GpuParticleParams", rendererParticles.gpuParticlesParamBuffer);

		gpuParams->getBufferParam(GPT_VERTEX_PROGRAM, "gIndices", renElement.indicesBuffer);

		gpuParams->getParamInfo()->getBindings(
			GpuPipelineParamInfoBase::ParamType::ParamBlock,
			"PerCamera",
			renElement.perCameraBindings
		);

		if (gpu)
		{
			// Allocate curves
			GpuParticleCurves& curves = GpuParticleSimulation::instance().getResources().getCurveTexture();
			curves.free(rendererParticles.colorCurveAlloc);
			curves.free(rendererParticles.sizeScaleFrameIdxCurveAlloc);

			static constexpr UINT32 NUM_CURVE_SAMPLES = 128;
			Color samples[NUM_CURVE_SAMPLES];

			const ParticleGpuSimulationSettings& gpuSimSettings = particleSystem->getGpuSimulationSettings();

			// Write color over lifetime curve
			LookupTable colorLookup = gpuSimSettings.colorOverLifetime.toLookupTable(NUM_CURVE_SAMPLES, true);

			for (UINT32 i = 0; i < NUM_CURVE_SAMPLES; i++)
			{
				const float* sample = colorLookup.getSample(i);
				samples[i] = Color(sample[0], sample[1], sample[2], sample[3]);
			}

			rendererParticles.colorCurveAlloc = curves.alloc(samples, NUM_CURVE_SAMPLES);

			// Write size over lifetime / sprite animation curve
			LookupTable sizeLookup = gpuSimSettings.sizeScaleOverLifetime.toLookupTable(NUM_CURVE_SAMPLES, true);

			float frameSamples[NUM_CURVE_SAMPLES];
			if (spriteTexture && spriteTexture->getAnimationPlayback() != SpriteAnimationPlayback::None)
			{
				const SpriteSheetGridAnimation& anim = spriteTexture->getAnimation();
				for (UINT32 i = 0; i < NUM_CURVE_SAMPLES; i++)
				{
					const float t = i / (float)(NUM_CURVE_SAMPLES - 1);
					frameSamples[i] = t * (anim.count - 1);
				}
			}
			else
				memset(frameSamples, 0, sizeof(frameSamples));

			for (UINT32 i = 0; i < NUM_CURVE_SAMPLES; i++)
			{
				const float* sample = sizeLookup.getSample(i);
				samples[i] = Color(sample[0], sample[1], frameSamples[i], 0.0f);
			}

			rendererParticles.sizeScaleFrameIdxCurveAlloc = curves.alloc(samples, NUM_CURVE_SAMPLES);

			const Vector2 colorUVOffset = GpuParticleCurves::getUVOffset(rendererParticles.colorCurveAlloc);
			const float colorUVScale = GpuParticleCurves::getUVScale(rendererParticles.colorCurveAlloc);

			const Vector2 sizeScaleFrameIdxUVOffset =
				GpuParticleCurves::getUVOffset(rendererParticles.sizeScaleFrameIdxCurveAlloc);
			const float sizeScaleFrameIdxUVScale =
				GpuParticleCurves::getUVScale(rendererParticles.sizeScaleFrameIdxCurveAlloc);

			const SPtr<GpuParamBlockBuffer>& gpuParticlesParamBuffer = rendererParticles.gpuParticlesParamBuffer;
			gGpuParticlesParamDef.gColorCurveOffset.set(gpuParticlesParamBuffer, colorUVOffset);
			gGpuParticlesParamDef.gColorCurveScale.set(gpuParticlesParamBuffer, Vector2(colorUVScale, 0.0f));
			gGpuParticlesParamDef.gSizeScaleFrameIdxCurveOffset.set(gpuParticlesParamBuffer,
				sizeScaleFrameIdxUVOffset);
			gGpuParticlesParamDef.gSizeScaleFrameIdxCurveScale.set(gpuParticlesParamBuffer,
				Vector2(sizeScaleFrameIdxUVScale, 0.0f));

			// Write sprite animation curve
			if (spriteTexture)
			{
				gParticlesParamDef.gUVOffset.set(particlesParamBuffer, spriteTexture->getOffset());
				gParticlesParamDef.gUVScale.set(particlesParamBuffer, spriteTexture->getScale());

				const SpriteSheetGridAnimation& anim = spriteTexture->getAnimation();
				gParticlesParamDef.gSubImageSize.set(particlesParamBuffer,
					Vector4((float)anim.numColumns, (float)anim.numRows, 1.0f / anim.numColumns, 1.0f / anim.numRows));
			}
		}

		// Set up buffers for lighting
		const bool useForwardRendering = shaderFlags.isSet(ShaderFlag::Forward);
		if (useForwardRendering)
		{
			renElement.forwardLightingParams.populate(gpuParams, supportsClusteredForward);
			renElement.imageBasedParams.populate(gpuParams, GPT_FRAGMENT_PROGRAM, true, supportsClusteredForward,
				supportsClusteredForward);
		}

		const bool isTransparent = shaderFlags.isSet(ShaderFlag::Transparent);
		if(isTransparent)
		{
			// Optional depth buffer input if requested
			if (gpuParams->hasTexture(GPT_FRAGMENT_PROGRAM, "gDepthBufferTex"))
				gpuParams->getTextureParam(GPT_FRAGMENT_PROGRAM, "gDepthBufferTex", renElement.depthInputTexture);
		}
	}

	void RendererScene::unregisterParticleSystem(ParticleSystem* particleSystem)
	{
		const UINT32 rendererId = particleSystem->getRendererId();
		RendererParticles& rendererParticles = mInfo.particleSystems[rendererId];

		// Free curves
		GpuParticleCurves& curves = GpuParticleSimulation::instance().getResources().getCurveTexture();
		curves.free(rendererParticles.colorCurveAlloc);
		curves.free(rendererParticles.sizeScaleFrameIdxCurveAlloc);

		if (rendererParticles.gpuParticleSystem)
		{
			bs_pool_delete(rendererParticles.gpuParticleSystem);
			rendererParticles.gpuParticleSystem = nullptr;
		}

		ParticleSystem* lastSystem = mInfo.particleSystems.back().particleSystem;
		const UINT32 lastRendererId = lastSystem->getRendererId();

		if (rendererId != lastRendererId)
		{
			// Swap current last element with the one we want to erase
			std::swap(mInfo.particleSystems[rendererId], mInfo.particleSystems[lastRendererId]);
			std::swap(mInfo.particleSystemCullInfos[rendererId], mInfo.particleSystemCullInfos[lastRendererId]);

			lastSystem->setRendererId(rendererId);
		}

		// Last element is the one we want to erase
		mInfo.particleSystems.erase(mInfo.particleSystems.end() - 1);
		mInfo.particleSystemCullInfos.erase(mInfo.particleSystemCullInfos.end() - 1);
	}

	void RendererScene::registerDecal(Decal* decal)
	{
		const auto renderableId = (UINT32)mInfo.decals.size();
		decal->setRendererId(renderableId);

		mInfo.decals.emplace_back();
		mInfo.decalCullInfos.push_back(CullInfo(decal->getBounds(), decal->getLayer()));

		RendererDecal& rendererDecal = mInfo.decals.back();
		rendererDecal.decal = decal;
		rendererDecal.updatePerObjectBuffer();

		DecalRenderElement& renElement = rendererDecal.renderElement;
		renElement.type = (UINT32)RenderElementType::Decal;
		renElement.mesh = RendererUtility::instance().getBoxStencil();
		renElement.subMesh = renElement.mesh->getProperties().getSubMesh();

		renElement.material = decal->getMaterial();

		if (renElement.material != nullptr && renElement.material->getShader() == nullptr)
			renElement.material = nullptr;

		// If no material use the default material
		if (renElement.material == nullptr)
			renElement.material = Material::create(DefaultDecalMat::get()->getShader());

		for(UINT32 i = 0; i < 2; i++)
		{
			for(UINT32 j = 0; j < 3; j++)
			{
				FIND_TECHNIQUE_DESC findDesc;
				findDesc.variation = DECAL_VAR_LOOKUP[i][j];
				findDesc.override = true;

				UINT32 techniqueIdx = renElement.material->findTechnique(findDesc);
				if(techniqueIdx == (UINT32)-1)
					techniqueIdx = 0;

				const SPtr<Technique>& technique = renElement.material->getTechnique(techniqueIdx);
				if (technique)
					technique->compile();

				renElement.techniqueIndices[i][j] = techniqueIdx;
			}
		}

		renElement.defaultTechniqueIdx = renElement.techniqueIndices[0][0];

		// Generate or assigned renderer specific data for the material
		// Note: This makes the assumption that all variations of the material share the same parameter set
		renElement.params = renElement.material->createParamsSet(renElement.defaultTechniqueIdx);
		renElement.material->updateParamsSet(renElement.params, 0.0f, true);

		// Generate or assign sampler state overrides
		renElement.samplerOverrides = allocSamplerStateOverrides(renElement);

		// Prepare all parameter bindings
		SPtr<GpuParams> gpuParams = renElement.params->getGpuParams();

		// Note: Perhaps perform buffer validation to ensure expected buffer has the same size and layout as the
		// provided buffer, and show a warning otherwise. But this is perhaps better handled on a higher level.
		gpuParams->setParamBlockBuffer("PerFrame", mPerFrameParamBuffer);
		gpuParams->setParamBlockBuffer("DecalParams", rendererDecal.decalParamBuffer);
		gpuParams->setParamBlockBuffer("PerObject", rendererDecal.perObjectParamBuffer);
		gpuParams->setParamBlockBuffer("PerCall", rendererDecal.perCallParamBuffer);

		gpuParams->getParamInfo()->getBindings(
			GpuPipelineParamInfoBase::ParamType::ParamBlock,
			"PerCamera",
			renElement.perCameraBindings
		);

		if (gpuParams->hasTexture(GPT_FRAGMENT_PROGRAM, "gDepthBufferTex"))
			gpuParams->getTextureParam(GPT_FRAGMENT_PROGRAM, "gDepthBufferTex", renElement.depthInputTexture);

		if (gpuParams->hasTexture(GPT_FRAGMENT_PROGRAM, "gMaskTex"))
			gpuParams->getTextureParam(GPT_FRAGMENT_PROGRAM, "gMaskTex", renElement.maskInputTexture);
	}

	void RendererScene::updateDecal(Decal* decal)
	{
		const UINT32 rendererId = decal->getRendererId();

		mInfo.decals[rendererId].updatePerObjectBuffer();
		mInfo.decalCullInfos[rendererId].bounds = decal->getBounds();
	}

	void RendererScene::unregisterDecal(Decal* decal)
	{
		const UINT32 rendererId = decal->getRendererId();
		Decal* lastDecal = mInfo.decals.back().decal;
		const UINT32 lastDecalId = lastDecal->getRendererId();

		RendererDecal& rendererDecal = mInfo.decals[rendererId];
		DecalRenderElement& renElement = rendererDecal.renderElement;

		// Unregister sampler overrides
		freeSamplerStateOverrides(renElement);
		renElement.samplerOverrides = nullptr;

		if (rendererId != lastDecalId)
		{
			// Swap current last element with the one we want to erase
			std::swap(mInfo.decals[rendererId], mInfo.decals[lastDecalId]);
			std::swap(mInfo.decalCullInfos[rendererId], mInfo.decalCullInfos[lastDecalId]);

			lastDecal->setRendererId(rendererId);
		}

		// Last element is the one we want to erase
		mInfo.decals.erase(mInfo.decals.end() - 1);
		mInfo.decalCullInfos.erase(mInfo.decalCullInfos.end() - 1);
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

		viewDesc.mainView = camera->isMain();
		viewDesc.triggerCallbacks = true;
		viewDesc.runPostProcessing = true;
		viewDesc.capturingReflections = false;
		viewDesc.onDemand = camera->getFlags().isSet(CameraFlag::OnDemand);

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

	void RendererScene::setParamFrameParams(float time)
	{
		gPerFrameParamDef.gTime.set(mPerFrameParamBuffer, time);
	}

	void RendererScene::prepareRenderable(UINT32 idx, const FrameInfo& frameInfo)
	{
		RendererRenderable* rendererRenderable = mInfo.renderables[idx];
		
		for (auto& element : rendererRenderable->elements)
			element.materialAnimationTime += frameInfo.timings.timeDelta;

		if (frameInfo.perFrameData.animation != nullptr)
			rendererRenderable->renderable->updatePrevFrameAnimationBuffers();

		if (rendererRenderable->prevFrameDirtyState != PrevFrameDirtyState::Clean)
		{
			if (rendererRenderable->prevFrameDirtyState == PrevFrameDirtyState::Updated)
				rendererRenderable->prevFrameDirtyState = PrevFrameDirtyState::CopyMostRecent;
			else if (rendererRenderable->prevFrameDirtyState == PrevFrameDirtyState::CopyMostRecent)
			{
				rendererRenderable->prevWorldTfrm = mInfo.renderables[idx]->worldTfrm;
				rendererRenderable->prevFrameDirtyState = PrevFrameDirtyState::Clean;
				rendererRenderable->updatePerObjectBuffer();
			}
		}
	}

	void RendererScene::prepareVisibleRenderable(UINT32 idx, const FrameInfo& frameInfo)
	{
		if (mInfo.renderableReady[idx])
			return;

		RendererRenderable* rendererRenderable = mInfo.renderables[idx];
		
		// Note: Before uploading bone matrices perhaps check if they has actually been changed since last frame
		if(frameInfo.perFrameData.animation != nullptr)
			rendererRenderable->renderable->updateAnimationBuffers(*frameInfo.perFrameData.animation);
		
		// Note: Could this step be moved in notifyRenderableUpdated, so it only triggers when material actually gets
		// changed? Although it shouldn't matter much because if the internal versions keeping track of dirty params.
		for (auto& element : rendererRenderable->elements)
			element.material->updateParamsSet(element.params, element.materialAnimationTime);

		mInfo.renderables[idx]->perObjectParamBuffer->flushToGPU();
		mInfo.renderableReady[idx] = true;
	}

	void RendererScene::prepareParticleSystem(UINT32 idx, const FrameInfo& frameInfo)
	{
		RendererParticles& rendererParticles = mInfo.particleSystems[idx];
		
		if (rendererParticles.prevFrameDirtyState != PrevFrameDirtyState::Clean)
		{
			if (rendererParticles.prevFrameDirtyState == PrevFrameDirtyState::Updated)
				rendererParticles.prevFrameDirtyState = PrevFrameDirtyState::CopyMostRecent;
			else if (rendererParticles.prevFrameDirtyState == PrevFrameDirtyState::CopyMostRecent)
			{
				rendererParticles.prevLocalToWorld = rendererParticles.localToWorld;
				rendererParticles.prevFrameDirtyState = PrevFrameDirtyState::Clean;
				rendererParticles.updatePerObjectBuffer();
			}
		}
		
		ParticlesRenderElement& renElement = mInfo.particleSystems[idx].renderElement;
		renElement.material->updateParamsSet(renElement.params, 0.0f);
		
		mInfo.particleSystems[idx].perObjectParamBuffer->flushToGPU();
	}

	void RendererScene::prepareDecal(UINT32 idx, const FrameInfo& frameInfo)
	{
		DecalRenderElement& renElement = mInfo.decals[idx].renderElement;
		renElement.materialAnimationTime += frameInfo.timings.timeDelta;
		renElement.material->updateParamsSet(renElement.params, renElement.materialAnimationTime);
		
		mInfo.decals[idx].perObjectParamBuffer->flushToGPU();
	}

	void RendererScene::updateParticleSystemBounds(const ParticlePerFrameData* particleRenderData)
	{
		// Note: Avoid updating bounds for deterministic particle systems every frame. Also see if this can be copied
		// over in a faster way (or ideally just assigned)

		for(auto& entry : mInfo.particleSystems)
		{
			const UINT32 rendererId = entry.particleSystem->getRendererId();

			AABox worldAABox = AABox::INF_BOX;
			const auto iterFind = particleRenderData->cpuData.find(entry.particleSystem->getId());
			if(iterFind != particleRenderData->cpuData.end())
				worldAABox = iterFind->second->bounds;
			else if(entry.gpuParticleSystem)
				worldAABox = entry.gpuParticleSystem->getBounds();

			const ParticleSystemSettings& settings = entry.particleSystem->getSettings();
			if (settings.simulationSpace == ParticleSimulationSpace::Local)
				worldAABox.transformAffine(entry.localToWorld);

			const Sphere worldSphere(worldAABox.getCenter(), worldAABox.getRadius());
			mInfo.particleSystemCullInfos[rendererId].bounds = Bounds(worldAABox, worldSphere);
		}
	}

	MaterialSamplerOverrides* RendererScene::allocSamplerStateOverrides(RenderElement& elem)
	{
		SamplerOverrideKey samplerKey(elem.material, elem.defaultTechniqueIdx);
		auto iterFind = mSamplerOverrides.find(samplerKey);
		if (iterFind != mSamplerOverrides.end())
		{
			iterFind->second->refCount++;
			return iterFind->second;
		}
		else
		{
			SPtr<Shader> shader = elem.material->getShader();
			MaterialSamplerOverrides* samplerOverrides = SamplerOverrideUtility::generateSamplerOverrides(shader,
				elem.material->_getInternalParams(), elem.params, mOptions);

			mSamplerOverrides[samplerKey] = samplerOverrides;

			samplerOverrides->refCount++;
			return samplerOverrides;
		}
	}

	void RendererScene::freeSamplerStateOverrides(RenderElement& elem)
	{
		SamplerOverrideKey samplerKey(elem.material, elem.defaultTechniqueIdx);

		auto iterFind = mSamplerOverrides.find(samplerKey);
		assert(iterFind != mSamplerOverrides.end());

		MaterialSamplerOverrides* samplerOverrides = iterFind->second;
		samplerOverrides->refCount--;
		if (samplerOverrides->refCount == 0)
		{
			SamplerOverrideUtility::destroySamplerOverrides(samplerOverrides);
			mSamplerOverrides.erase(iterFind);
		}
	}
}}
