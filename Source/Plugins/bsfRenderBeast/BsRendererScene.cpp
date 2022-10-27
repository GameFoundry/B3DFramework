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

namespace bs { namespace ct {

PerFrameParamDef gPerFrameParamDef;

static const ShaderVariation* DECAL_VAR_LOOKUP[2][3] = {
	{ &getDecalShaderVariation<false, MSAAMode::None>(),
	  &getDecalShaderVariation<false, MSAAMode::Single>(),
	  &getDecalShaderVariation<false, MSAAMode::Full>() },
	{ &getDecalShaderVariation<true, MSAAMode::None>(),
	  &getDecalShaderVariation<true, MSAAMode::Single>(),
	  &getDecalShaderVariation<true, MSAAMode::Full>() }
};

/** Returns a specific forward rendering shader variation. */
template <bool SKINNED, bool MORPH, bool CLUSTERED, bool WRITE_VELOCITY>
static const ShaderVariation& getForwardRenderingVariation(bool supportsVelocityWrites)
{
	if(!supportsVelocityWrites)
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
template <bool CLUSTERED, bool WRITE_VELOCITY>
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
template <bool WRITE_VELOCITY>
static const ShaderVariation* getBasePassVariation(bool useForwardRendering, bool supportsClusteredForward, bool shaderCanWriteVelocity, RenderableAnimType animType)
{
	if(useForwardRendering)
	{
		if(supportsClusteredForward)
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
static u32 initAndRetrieveBasePassTechnique(Material& material, bool useForwardRendering, bool supportsClusteredForward, bool shaderCanWriteVelocity, bool writeVelocity, RenderableAnimType animType)
{
	const ShaderVariation* variation = writeVelocity ? getBasePassVariation<true>(useForwardRendering, supportsClusteredForward, shaderCanWriteVelocity, animType) : getBasePassVariation<false>(useForwardRendering, supportsClusteredForward, shaderCanWriteVelocity, animType);

	FIND_TECHNIQUE_DESC findDesc;
	findDesc.Variation = variation;
	findDesc.Override = true;

	u32 techniqueIdx = material.FindTechnique(findDesc);

	if(techniqueIdx == (u32)-1)
		techniqueIdx = material.GetDefaultTechnique();

	// Make sure the technique shaders are compiled
	const SPtr<Technique>& technique = material.GetTechnique(techniqueIdx);
	if(technique)
		technique->Compile();

	return techniqueIdx;
}

static void validateBasePassMaterial(Material& material, RenderableAnimType animType, u32 techniqueIdx, VertexDeclaration& vertexDecl)
{
	// Validate mesh <-> shader vertex bindings
	u32 numPasses = material.GetNumPasses(techniqueIdx);
	for(u32 j = 0; j < numPasses; j++)
	{
		SPtr<Pass> pass = material.GetPass(j, techniqueIdx);
		SPtr<GraphicsPipelineState> graphicsPipeline = pass->GetGraphicsPipelineState();

		SPtr<VertexDeclaration> shaderDecl = graphicsPipeline->GetVertexProgram()->GetInputDeclaration();
		if(shaderDecl && !vertexDecl.IsCompatible(shaderDecl))
		{
			Vector<VertexElement> missingElements = vertexDecl.GetMissingElements(shaderDecl);

			// If using morph shapes ignore POSITION1 and NORMAL1 missing since we assign them from within the renderer
			if(animType == RenderableAnimType::Morph || animType == RenderableAnimType::SkinnedMorph)
			{
				auto removeIter = std::remove_if(missingElements.begin(), missingElements.end(), [](const VertexElement& x)
												 { return (x.GetSemantic() == VES_POSITION && x.GetSemanticIdx() == 1) ||
													   (x.GetSemantic() == VES_NORMAL && x.GetSemanticIdx() == 1); });

				missingElements.erase(removeIter, missingElements.end());
			}

			if(!missingElements.empty())
			{
				StringStream wrnStream;
				wrnStream << "Provided mesh is missing required vertex attributes to render with the \
									provided shader. Missing elements: "
						  << std::endl;

				for(auto& entry : missingElements)
					wrnStream << "\t" << toString(entry.GetSemantic()) << entry.GetSemanticIdx() << std::endl;

				BS_LOG(Warning, Renderer, wrnStream.str());
				break;
			}
		}
	}
}

RendererScene::RendererScene(const SPtr<RenderBeastOptions>& options)
	: mOptions(options)
{
	mPerFrameParamBuffer = gPerFrameParamDef.CreateBuffer();
}

RendererScene::~RendererScene()
{
	for(auto& entry : mInfo.Renderables)
		bs_delete(entry);

	for(auto& entry : mInfo.Views)
		bs_delete(entry);

	assert(mSamplerOverrides.empty());
}

void RendererScene::RegisterCamera(Camera* camera)
{
	RENDERER_VIEW_DESC viewDesc = CreateViewDesc(camera);

	RendererView* view = bs_new<RendererView>(viewDesc);
	view->SetRenderSettings(camera->GetRenderSettings());
	view->UpdatePerViewBuffer();

	u32 viewIdx = (u32)mInfo.Views.size();
	mInfo.Views.push_back(view);

	mInfo.CameraToView[camera] = viewIdx;
	camera->SetRendererId(viewIdx);

	UpdateCameraRenderTargets(camera);
}

void RendererScene::UpdateCamera(Camera* camera, u32 updateFlag)
{
	u32 cameraId = camera->GetRendererId();
	RendererView* view = mInfo.Views[cameraId];

	if((updateFlag & (u32)CameraDirtyFlag::Redraw) != 0)
		view->NotifyNeedsRedrawInternal();

	u32 updateEverythingFlag = (u32)ActorDirtyFlag::Everything | (u32)ActorDirtyFlag::Active | (u32)CameraDirtyFlag::Viewport;

	if((updateFlag & updateEverythingFlag) != 0)
	{
		RENDERER_VIEW_DESC viewDesc = CreateViewDesc(camera);

		view->SetView(viewDesc);
		view->SetRenderSettings(camera->GetRenderSettings());

		UpdateCameraRenderTargets(camera);
		return;
	}

	if((updateFlag & (u32)CameraDirtyFlag::RenderSettings) != 0)
		view->SetRenderSettings(camera->GetRenderSettings());

	const Transform& tfrm = camera->GetTransform();
	view->SetTransform(
		tfrm.GetPosition(),
		tfrm.GetForward(),
		camera->GetViewMatrix(),
		camera->GetProjectionMatrixRs(),
		camera->GetWorldFrustum());

	view->UpdatePerViewBuffer();
}

void RendererScene::UnregisterCamera(Camera* camera)
{
	u32 cameraId = camera->GetRendererId();

	Camera* lastCamera = mInfo.Views.back()->GetSceneCamera();
	u32 lastCameraId = lastCamera->GetRendererId();

	if(cameraId != lastCameraId)
	{
		// Swap current last element with the one we want to erase
		std::swap(mInfo.Views[cameraId], mInfo.Views[lastCameraId]);
		lastCamera->SetRendererId(cameraId);

		mInfo.CameraToView[lastCamera] = cameraId;
	}

	// Last element is the one we want to erase
	RendererView* view = mInfo.Views[mInfo.Views.size() - 1];
	bs_delete(view);

	mInfo.Views.erase(mInfo.Views.end() - 1);

	auto iterFind = mInfo.CameraToView.find(camera);
	if(iterFind != mInfo.CameraToView.end())
		mInfo.CameraToView.erase(iterFind);

	UpdateCameraRenderTargets(camera, true);
}

void RendererScene::RegisterLight(Light* light)
{
	if(light->GetType() == LightType::Directional)
	{
		u32 lightId = (u32)mInfo.DirectionalLights.size();
		light->SetRendererId(lightId);

		mInfo.DirectionalLights.push_back(RendererLight(light));
	}
	else
	{
		if(light->GetType() == LightType::Radial)
		{
			u32 lightId = (u32)mInfo.RadialLights.size();
			light->SetRendererId(lightId);

			mInfo.RadialLights.push_back(RendererLight(light));
			mInfo.RadialLightWorldBounds.push_back(light->GetBounds());
		}
		else // Spot
		{
			u32 lightId = (u32)mInfo.SpotLights.size();
			light->SetRendererId(lightId);

			mInfo.SpotLights.push_back(RendererLight(light));
			mInfo.SpotLightWorldBounds.push_back(light->GetBounds());
		}
	}
}

void RendererScene::UpdateLight(Light* light)
{
	u32 lightId = light->GetRendererId();

	if(light->GetType() == LightType::Radial)
		mInfo.RadialLightWorldBounds[lightId] = light->GetBounds();
	else if(light->GetType() == LightType::Spot)
		mInfo.SpotLightWorldBounds[lightId] = light->GetBounds();
}

void RendererScene::UnregisterLight(Light* light)
{
	u32 lightId = light->GetRendererId();
	if(light->GetType() == LightType::Directional)
	{
		Light* lastLight = mInfo.DirectionalLights.back().Internal;
		u32 lastLightId = lastLight->GetRendererId();

		if(lightId != lastLightId)
		{
			// Swap current last element with the one we want to erase
			std::swap(mInfo.DirectionalLights[lightId], mInfo.DirectionalLights[lastLightId]);
			lastLight->SetRendererId(lightId);
		}

		// Last element is the one we want to erase
		mInfo.DirectionalLights.erase(mInfo.DirectionalLights.end() - 1);
	}
	else
	{
		if(light->GetType() == LightType::Radial)
		{
			Light* lastLight = mInfo.RadialLights.back().Internal;
			u32 lastLightId = lastLight->GetRendererId();

			if(lightId != lastLightId)
			{
				// Swap current last element with the one we want to erase
				std::swap(mInfo.RadialLights[lightId], mInfo.RadialLights[lastLightId]);
				std::swap(mInfo.RadialLightWorldBounds[lightId], mInfo.RadialLightWorldBounds[lastLightId]);

				lastLight->SetRendererId(lightId);
			}

			// Last element is the one we want to erase
			mInfo.RadialLights.erase(mInfo.RadialLights.end() - 1);
			mInfo.RadialLightWorldBounds.erase(mInfo.RadialLightWorldBounds.end() - 1);
		}
		else // Spot
		{
			Light* lastLight = mInfo.SpotLights.back().Internal;
			u32 lastLightId = lastLight->GetRendererId();

			if(lightId != lastLightId)
			{
				// Swap current last element with the one we want to erase
				std::swap(mInfo.SpotLights[lightId], mInfo.SpotLights[lastLightId]);
				std::swap(mInfo.SpotLightWorldBounds[lightId], mInfo.SpotLightWorldBounds[lastLightId]);

				lastLight->SetRendererId(lightId);
			}

			// Last element is the one we want to erase
			mInfo.SpotLights.erase(mInfo.SpotLights.end() - 1);
			mInfo.SpotLightWorldBounds.erase(mInfo.SpotLightWorldBounds.end() - 1);
		}
	}
}

void RendererScene::RegisterRenderable(Renderable* renderable)
{
	u32 renderableId = (u32)mInfo.Renderables.size();

	renderable->SetRendererId(renderableId);

	mInfo.Renderables.push_back(bs_new<RendererRenderable>());
	mInfo.RenderableCullInfos.push_back(CullInfo(renderable->GetBounds(), renderable->GetLayer(), renderable->GetCullDistanceFactor()));

	RendererRenderable* rendererRenderable = mInfo.Renderables.back();
	rendererRenderable->Renderable = renderable;
	rendererRenderable->WorldTfrm = renderable->GetMatrix();
	rendererRenderable->PrevWorldTfrm = rendererRenderable->WorldTfrm;
	rendererRenderable->PrevFrameDirtyState = PrevFrameDirtyState::Clean;
	rendererRenderable->UpdatePerObjectBuffer();

	SPtr<Mesh> mesh = renderable->GetMesh();
	if(mesh != nullptr)
	{
		const MeshProperties& meshProps = mesh->GetProperties();
		SPtr<VertexDeclaration> vertexDecl = mesh->GetVertexData()->VertexDeclaration;

		for(u32 i = 0; i < meshProps.GetNumSubMeshes(); i++)
		{
			rendererRenderable->Elements.push_back(RenderableElement());
			RenderableElement& renElement = rendererRenderable->Elements.back();

			renElement.Type = (u32)RenderElementType::Renderable;
			renElement.Mesh = mesh;
			renElement.SubMesh = meshProps.GetSubMesh(i);
			renElement.AnimType = renderable->GetAnimType();
			renElement.AnimationId = renderable->GetAnimationId();
			renElement.MorphShapeVersion = 0;
			renElement.MorphShapeBuffer = renderable->GetMorphShapeBuffer();
			renElement.BoneMatrixBuffer = renderable->GetBoneMatrixBuffer();
			renElement.BonePrevMatrixBuffer = renderable->GetBonePrevMatrixBuffer();
			renElement.MorphVertexDeclaration = renderable->GetMorphVertexDeclaration();

			renElement.Material = renderable->GetMaterial(i);
			if(renElement.Material == nullptr)
				renElement.Material = renderable->GetMaterial(0);

			if(renElement.Material != nullptr && renElement.Material->GetShader() == nullptr)
				renElement.Material = nullptr;

			// If no material use the default material
			if(renElement.Material == nullptr)
				renElement.Material = Material::Create(DefaultMaterial::Get()->GetShader());

			// Determine which technique to use
			static_assert((u32)RenderableAnimType::Count == 4, "RenderableAnimType is expected to have four sequential entries.");

			const SPtr<Shader>& shader = renElement.Material->GetShader();
			ShaderFlags shaderFlags = shader->GetFlags();
			const bool useForwardRendering = shaderFlags.IsSet(ShaderFlag::Forward) || shaderFlags.IsSet(ShaderFlag::Transparent);
			bool supportsClusteredForward = gRenderBeast()->GetFeatureSet() == RenderBeastFeatureSet::Desktop;

			const Vector<ShaderVariationParamInfo>& variationParams = shader->GetVariationParams();
			const bool shaderCanWriteVelocity = std::find_if(variationParams.begin(), variationParams.end(), [](const ShaderVariationParamInfo& x)
															 { return x.Identifier == "WRITE_VELOCITY"; }) != variationParams.end();

			const bool writeVelocity = shaderCanWriteVelocity && renderable->GetWriteVelocity();

			RenderableAnimType animType = renderable->GetAnimType();

			renElement.DefaultTechniqueIdx = initAndRetrieveBasePassTechnique(*renElement.Material, useForwardRendering, supportsClusteredForward, shaderCanWriteVelocity, false, animType);

#if BS_DEBUG_MODE
			validateBasePassMaterial(*renElement.Material, animType, renElement.DefaultTechniqueIdx, *vertexDecl);
#endif

			// Generate or assigned renderer specific data for the material
			renElement.Params = renElement.Material->CreateParamsSet(renElement.DefaultTechniqueIdx);
			renElement.Material->UpdateParamsSet(renElement.Params, 0.0f, true);

			if(writeVelocity)
			{
				renElement.WriteVelocityTechniqueIdx = initAndRetrieveBasePassTechnique(*renElement.Material, useForwardRendering, supportsClusteredForward, shaderCanWriteVelocity, true, animType);

#if BS_DEBUG_MODE
				validateBasePassMaterial(*renElement.Material, animType, renElement.WriteVelocityTechniqueIdx, *vertexDecl);
#endif

				// Note: Using the same params as the non-velocity technique. There are assumed to be no differences
			}
			else
				renElement.WriteVelocityTechniqueIdx = (u32)-1;

			// Generate or assign sampler state overrides
			renElement.SamplerOverrides = AllocSamplerStateOverrides(renElement);
		}
	}

	// Prepare all parameter bindings
	for(auto& element : rendererRenderable->Elements)
	{
		SPtr<Shader> shader = element.Material->GetShader();
		if(shader == nullptr)
		{
			BS_LOG(Warning, Renderer, "Missing shader on material.");
			continue;
		}

		SPtr<GpuParams> gpuParams = element.Params->GetGpuParams();

		// Note: Perhaps perform buffer validation to ensure expected buffer has the same size and layout as the
		// provided buffer, and show a warning otherwise. But this is perhaps better handled on a higher level.
		gpuParams->SetParamBlockBuffer("PerFrame", mPerFrameParamBuffer);
		gpuParams->SetParamBlockBuffer("PerObject", rendererRenderable->PerObjectParamBuffer);
		gpuParams->SetParamBlockBuffer("PerCall", rendererRenderable->PerCallParamBuffer);

		gpuParams->GetParamInfo()->GetBindings(
			GpuPipelineParamInfoBase::ParamType::ParamBlock,
			"PerCamera",
			element.PerCameraBindings);

		if(gpuParams->HasBuffer(GPT_VERTEX_PROGRAM, "boneMatrices"))
			gpuParams->SetBuffer(GPT_VERTEX_PROGRAM, "boneMatrices", element.BoneMatrixBuffer);

		if(gpuParams->HasBuffer(GPT_VERTEX_PROGRAM, "prevBoneMatrices"))
			gpuParams->SetBuffer(GPT_VERTEX_PROGRAM, "prevBoneMatrices", element.BonePrevMatrixBuffer);

		ShaderFlags shaderFlags = shader->GetFlags();
		const bool useForwardRendering = shaderFlags.IsSet(ShaderFlag::Forward) || shaderFlags.IsSet(ShaderFlag::Transparent);

		if(useForwardRendering)
		{
			const bool supportsClusteredForward = gRenderBeast()->GetFeatureSet() == RenderBeastFeatureSet::Desktop;

			element.ForwardLightingParams.Populate(gpuParams, supportsClusteredForward);
			element.ImageBasedParams.Populate(gpuParams, GPT_FRAGMENT_PROGRAM, true, supportsClusteredForward, supportsClusteredForward);
		}
	}
}

void RendererScene::UpdateRenderable(Renderable* renderable)
{
	u32 renderableId = renderable->GetRendererId();

	RendererRenderable* rendererRenderable = mInfo.Renderables[renderableId];

	if(rendererRenderable->PrevFrameDirtyState != PrevFrameDirtyState::Updated)
		rendererRenderable->PrevWorldTfrm = rendererRenderable->WorldTfrm;

	rendererRenderable->WorldTfrm = renderable->GetMatrix();
	rendererRenderable->PrevFrameDirtyState = PrevFrameDirtyState::Updated;

	mInfo.Renderables[renderableId]->UpdatePerObjectBuffer();
	mInfo.RenderableCullInfos[renderableId].Bounds = renderable->GetBounds();
	mInfo.RenderableCullInfos[renderableId].CullDistanceFactor = renderable->GetCullDistanceFactor();
}

void RendererScene::UnregisterRenderable(Renderable* renderable)
{
	u32 renderableId = renderable->GetRendererId();
	Renderable* lastRenerable = mInfo.Renderables.back()->Renderable;
	u32 lastRenderableId = lastRenerable->GetRendererId();

	RendererRenderable* rendererRenderable = mInfo.Renderables[renderableId];
	Vector<RenderableElement>& elements = rendererRenderable->Elements;
	for(auto& element : elements)
	{
		FreeSamplerStateOverrides(element);
		element.SamplerOverrides = nullptr;
	}

	if(renderableId != lastRenderableId)
	{
		// Swap current last element with the one we want to erase
		std::swap(mInfo.Renderables[renderableId], mInfo.Renderables[lastRenderableId]);
		std::swap(mInfo.RenderableCullInfos[renderableId], mInfo.RenderableCullInfos[lastRenderableId]);

		lastRenerable->SetRendererId(renderableId);
	}

	// Last element is the one we want to erase
	mInfo.Renderables.erase(mInfo.Renderables.end() - 1);
	mInfo.RenderableCullInfos.erase(mInfo.RenderableCullInfos.end() - 1);

	bs_delete(rendererRenderable);
}

void RendererScene::RegisterReflectionProbe(ReflectionProbe* probe)
{
	u32 probeId = (u32)mInfo.ReflProbes.size();
	probe->SetRendererId(probeId);

	mInfo.ReflProbes.push_back(RendererReflectionProbe(probe));
	RendererReflectionProbe& probeInfo = mInfo.ReflProbes.back();

	mInfo.ReflProbeWorldBounds.push_back(probe->GetBounds());

	// Find a spot in cubemap array
	u32 numArrayEntries = (u32)mInfo.ReflProbeCubemapArrayUsedSlots.size();
	for(u32 i = 0; i < numArrayEntries; i++)
	{
		if(!mInfo.ReflProbeCubemapArrayUsedSlots[i])
		{
			SetReflectionProbeArrayIndex(probeId, i, false);
			mInfo.ReflProbeCubemapArrayUsedSlots[i] = true;
			break;
		}
	}

	// No empty slot was found
	if(probeInfo.ArrayIdx == (u32)-1)
	{
		SetReflectionProbeArrayIndex(probeId, numArrayEntries, false);
		mInfo.ReflProbeCubemapArrayUsedSlots.push_back(true);
	}

	if(probeInfo.ArrayIdx > MaxReflectionCubemaps)
	{
		BS_LOG(Error, Renderer, "Reached the maximum number of allowed reflection probe cubemaps at once. "
								"Ignoring reflection probe data.");
	}
}

void RendererScene::UpdateReflectionProbe(ReflectionProbe* probe, bool texture)
{
	// Should only get called if transform changes, any other major changes and ReflProbeInfo entry gets rebuild
	u32 probeId = probe->GetRendererId();
	mInfo.ReflProbeWorldBounds[probeId] = probe->GetBounds();

	if(texture)
	{
		RendererReflectionProbe& probeInfo = mInfo.ReflProbes[probeId];
		probeInfo.ArrayDirty = true;
	}
}

void RendererScene::UnregisterReflectionProbe(ReflectionProbe* probe)
{
	u32 probeId = probe->GetRendererId();
	u32 arrayIdx = mInfo.ReflProbes[probeId].ArrayIdx;

	if(arrayIdx != (u32)-1)
		mInfo.ReflProbeCubemapArrayUsedSlots[arrayIdx] = false;

	ReflectionProbe* lastProbe = mInfo.ReflProbes.back().Probe;
	u32 lastProbeId = lastProbe->GetRendererId();

	if(probeId != lastProbeId)
	{
		// Swap current last element with the one we want to erase
		std::swap(mInfo.ReflProbes[probeId], mInfo.ReflProbes[lastProbeId]);
		std::swap(mInfo.ReflProbeWorldBounds[probeId], mInfo.ReflProbeWorldBounds[lastProbeId]);

		lastProbe->SetRendererId(probeId);
	}

	// Last element is the one we want to erase
	mInfo.ReflProbes.erase(mInfo.ReflProbes.end() - 1);
	mInfo.ReflProbeWorldBounds.erase(mInfo.ReflProbeWorldBounds.end() - 1);
}

void RendererScene::SetReflectionProbeArrayIndex(u32 probeIdx, u32 arrayIdx, bool markAsClean)
{
	RendererReflectionProbe* probe = &mInfo.ReflProbes[probeIdx];
	probe->ArrayIdx = arrayIdx;

	if(markAsClean)
		probe->ArrayDirty = false;
}

void RendererScene::RegisterLightProbeVolume(LightProbeVolume* volume)
{
	mInfo.LightProbes.NotifyAdded(volume);
}

void RendererScene::UpdateLightProbeVolume(LightProbeVolume* volume)
{
	mInfo.LightProbes.NotifyDirty(volume);
}

void RendererScene::UnregisterLightProbeVolume(LightProbeVolume* volume)
{
	mInfo.LightProbes.NotifyRemoved(volume);
}

void RendererScene::UpdateLightProbes()
{
	mInfo.LightProbes.UpdateProbes();
}

void RendererScene::RegisterSkybox(Skybox* skybox)
{
	mInfo.Skybox = skybox;
}

void RendererScene::UnregisterSkybox(Skybox* skybox)
{
	if(mInfo.Skybox == skybox)
		mInfo.Skybox = nullptr;
}

void RendererScene::RegisterParticleSystem(ParticleSystem* particleSystem)
{
	const auto rendererId = (u32)mInfo.ParticleSystems.size();
	particleSystem->SetRendererId(rendererId);

	mInfo.ParticleSystems.push_back(RendererParticles());
	mInfo.ParticleSystemCullInfos.push_back(CullInfo(Bounds(), particleSystem->GetLayer()));

	RendererParticles& rendererParticles = mInfo.ParticleSystems.back();
	rendererParticles.ParticleSystem = particleSystem;

	UpdateParticleSystem(particleSystem, false);

	rendererParticles.PrevLocalToWorld = rendererParticles.LocalToWorld;
	rendererParticles.PrevFrameDirtyState = PrevFrameDirtyState::Clean;
}

void RendererScene::UpdateParticleSystem(ParticleSystem* particleSystem, bool tfrmOnly)
{
	const u32 rendererId = particleSystem->GetRendererId();
	RendererParticles& rendererParticles = mInfo.ParticleSystems[rendererId];

	rendererParticles.PrevLocalToWorld = rendererParticles.LocalToWorld;
	rendererParticles.PrevFrameDirtyState = PrevFrameDirtyState::Updated;

	const ParticleSystemSettings& settings = particleSystem->GetSettings();
	if(settings.SimulationSpace == ParticleSimulationSpace::Local)
	{
		const Transform& tfrm = particleSystem->GetTransform();
		rendererParticles.LocalToWorld = tfrm.GetMatrix();
	}
	else
		rendererParticles.LocalToWorld = Matrix4::IDENTITY;

	if(tfrmOnly)
	{
		rendererParticles.UpdatePerObjectBuffer();
		return;
	}

	rendererParticles.PerObjectParamBuffer = gPerObjectParamDef.CreateBuffer();
	rendererParticles.UpdatePerObjectBuffer();

	SPtr<GpuParamBlockBuffer> particlesParamBuffer = gParticlesParamDef.CreateBuffer();
	rendererParticles.ParticlesParamBuffer = particlesParamBuffer;

	Vector3 axisForward = settings.OrientationPlaneNormal;

	Vector3 axisUp = Vector3::UNIT_Y;
	if(axisForward.Dot(axisUp) > 0.9998f)
		axisUp = Vector3::UNIT_Z;

	Vector3 axisRight = axisUp.Cross(axisForward);
	Vector3::Orthonormalize(axisRight, axisUp, axisForward);

	gParticlesParamDef.gAxisUp.Set(particlesParamBuffer, axisUp);
	gParticlesParamDef.gAxisRight.Set(particlesParamBuffer, axisRight);

	// Initialize the variant of the particle system for GPU simulation, if needed
	if(settings.GpuSimulation)
	{
		if(!rendererParticles.GpuParticleSystem)
			rendererParticles.GpuParticleSystem = bs_pool_new<GpuParticleSystem>(particleSystem);
	}
	else
	{
		if(rendererParticles.GpuParticleSystem)
		{
			bs_pool_delete(rendererParticles.GpuParticleSystem);
			rendererParticles.GpuParticleSystem = nullptr;
		}
	}

	ParticlesRenderElement& renElement = rendererParticles.RenderElement;
	renElement.Type = (u32)RenderElementType::Particle;

	renElement.Material = settings.Material;

	if(renElement.Material != nullptr && renElement.Material->GetShader() == nullptr)
		renElement.Material = nullptr;

	// If no material use the default material
	if(renElement.Material == nullptr)
		renElement.Material = Material::Create(DefaultParticlesMat::Get()->GetShader());

	const SPtr<Shader> shader = renElement.Material->GetShader();

	SpriteTexture* spriteTexture = nullptr;
	if(shader->HasTextureParam("gTexture"))
		spriteTexture = renElement.Material->GetSpriteTexture("gTexture").get();

	if(!spriteTexture && shader->HasTextureParam("gAlbedoTex"))
		spriteTexture = renElement.Material->GetSpriteTexture("gAlbedoTex").get();

	if(spriteTexture)
	{
		gParticlesParamDef.gUVOffset.Set(particlesParamBuffer, spriteTexture->GetOffset());
		gParticlesParamDef.gUVScale.Set(particlesParamBuffer, spriteTexture->GetScale());

		const SpriteSheetGridAnimation& anim = spriteTexture->GetAnimation();
		gParticlesParamDef.gSubImageSize.Set(particlesParamBuffer, Vector4((float)anim.NumColumns, (float)anim.NumRows, 1.0f / anim.NumColumns, 1.0f / anim.NumRows));
	}
	else
	{
		gParticlesParamDef.gUVOffset.Set(particlesParamBuffer, Vector2::ZERO);
		gParticlesParamDef.gUVScale.Set(particlesParamBuffer, Vector2::ONE);
		gParticlesParamDef.gSubImageSize.Set(particlesParamBuffer, Vector4(1.0f, 1.0f, 1.0f, 1.0f));
	}

	const ParticleOrientation orientation = settings.Orientation;
	const bool lockY = settings.OrientationLockY;
	const bool gpu = settings.GpuSimulation;
	const bool is3d = settings.RenderMode == ParticleRenderMode::Mesh;

	ShaderFlags shaderFlags = shader->GetFlags();
	const bool requiresForwardLighting = shaderFlags.IsSet(ShaderFlag::Forward);
	const bool supportsClusteredForward = gRenderBeast()->GetFeatureSet() == RenderBeastFeatureSet::Desktop;

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
	findDesc.Variation = variation;
	findDesc.Override = true;

	u32 techniqueIdx = renElement.Material->FindTechnique(findDesc);

	if(techniqueIdx == (u32)-1)
		techniqueIdx = renElement.Material->GetDefaultTechnique();

	renElement.DefaultTechniqueIdx = techniqueIdx;

	// Make sure the technique shaders are compiled
	const SPtr<Technique>& technique = renElement.Material->GetTechnique(techniqueIdx);
	if(technique)
		technique->Compile();

	// Generate or assigned renderer specific data for the material
	renElement.Params = renElement.Material->CreateParamsSet(techniqueIdx);
	renElement.Material->UpdateParamsSet(renElement.Params, 0.0f, true);

	SPtr<GpuParams> gpuParams = renElement.Params->GetGpuParams();

	if(gpu)
	{
		gpuParams->GetTextureParam(GPT_VERTEX_PROGRAM, "gPositionTimeTex", renElement.ParamsGpu.PositionTimeTexture);
		gpuParams->GetTextureParam(GPT_VERTEX_PROGRAM, "gSizeRotationTex", renElement.ParamsGpu.SizeRotationTexture);
		gpuParams->GetTextureParam(GPT_VERTEX_PROGRAM, "gCurvesTex", renElement.ParamsGpu.CurvesTexture);

		rendererParticles.GpuParticlesParamBuffer = gGpuParticlesParamDef.CreateBuffer();
		renElement.Is3D = false;
	}
	else
	{
		switch(settings.RenderMode)
		{
		case ParticleRenderMode::Billboard:
			gpuParams->GetTextureParam(GPT_VERTEX_PROGRAM, "gPositionAndRotTex", renElement.ParamsCpuBillboard.PositionAndRotTexture);
			gpuParams->GetTextureParam(GPT_VERTEX_PROGRAM, "gColorTex", renElement.ParamsCpuBillboard.ColorTexture);
			gpuParams->GetTextureParam(GPT_VERTEX_PROGRAM, "gSizeAndFrameIdxTex", renElement.ParamsCpuBillboard.SizeAndFrameIdxTexture);

			renElement.Is3D = false;
			break;
		case ParticleRenderMode::Mesh:
			gpuParams->GetTextureParam(GPT_VERTEX_PROGRAM, "gPositionTex", renElement.ParamsCpuMesh.PositionTexture);
			gpuParams->GetTextureParam(GPT_VERTEX_PROGRAM, "gColorTex", renElement.ParamsCpuMesh.ColorTexture);
			gpuParams->GetTextureParam(GPT_VERTEX_PROGRAM, "gSizeTex", renElement.ParamsCpuMesh.SizeTexture);
			gpuParams->GetTextureParam(GPT_VERTEX_PROGRAM, "gRotationTex", renElement.ParamsCpuMesh.RotationTexture);

			renElement.Is3D = true;
			renElement.Mesh = settings.Mesh;
			break;
		default:
			break;
		}

		rendererParticles.GpuParticlesParamBuffer = nullptr;
	}

	// Note: Perhaps perform buffer validation to ensure expected buffer has the same size and layout as the
	// provided buffer, and show a warning otherwise. But this is perhaps better handled on a higher level.
	gpuParams->SetParamBlockBuffer("ParticleParams", rendererParticles.ParticlesParamBuffer);
	gpuParams->SetParamBlockBuffer("PerObject", rendererParticles.PerObjectParamBuffer);
	gpuParams->SetParamBlockBuffer("GpuParticleParams", rendererParticles.GpuParticlesParamBuffer);

	gpuParams->GetBufferParam(GPT_VERTEX_PROGRAM, "gIndices", renElement.IndicesBuffer);

	gpuParams->GetParamInfo()->GetBindings(
		GpuPipelineParamInfoBase::ParamType::ParamBlock,
		"PerCamera",
		renElement.PerCameraBindings);

	if(gpu)
	{
		// Allocate curves
		GpuParticleCurves& curves = GpuParticleSimulation::Instance().GetResources().GetCurveTexture();
		curves.Free(rendererParticles.ColorCurveAlloc);
		curves.Free(rendererParticles.SizeScaleFrameIdxCurveAlloc);

		static constexpr u32 NUM_CURVE_SAMPLES = 128;
		Color samples[NUM_CURVE_SAMPLES];

		const ParticleGpuSimulationSettings& gpuSimSettings = particleSystem->GetGpuSimulationSettings();

		// Write color over lifetime curve
		LookupTable colorLookup = gpuSimSettings.ColorOverLifetime.ToLookupTable(NUM_CURVE_SAMPLES, true);

		for(u32 i = 0; i < NUM_CURVE_SAMPLES; i++)
		{
			const float* sample = colorLookup.GetSample(i);
			samples[i] = Color(sample[0], sample[1], sample[2], sample[3]);
		}

		rendererParticles.ColorCurveAlloc = curves.Alloc(samples, NUM_CURVE_SAMPLES);

		// Write size over lifetime / sprite animation curve
		LookupTable sizeLookup = gpuSimSettings.SizeScaleOverLifetime.ToLookupTable(NUM_CURVE_SAMPLES, true);

		float frameSamples[NUM_CURVE_SAMPLES];
		if(spriteTexture && spriteTexture->GetAnimationPlayback() != SpriteAnimationPlayback::None)
		{
			const SpriteSheetGridAnimation& anim = spriteTexture->GetAnimation();
			for(u32 i = 0; i < NUM_CURVE_SAMPLES; i++)
			{
				const float t = i / (float)(NUM_CURVE_SAMPLES - 1);
				frameSamples[i] = t * (anim.Count - 1);
			}
		}
		else
			memset(frameSamples, 0, sizeof(frameSamples));

		for(u32 i = 0; i < NUM_CURVE_SAMPLES; i++)
		{
			const float* sample = sizeLookup.GetSample(i);
			samples[i] = Color(sample[0], sample[1], frameSamples[i], 0.0f);
		}

		rendererParticles.SizeScaleFrameIdxCurveAlloc = curves.Alloc(samples, NUM_CURVE_SAMPLES);

		const Vector2 colorUVOffset = GpuParticleCurves::GetUvOffset(rendererParticles.ColorCurveAlloc);
		const float colorUVScale = GpuParticleCurves::GetUvScale(rendererParticles.ColorCurveAlloc);

		const Vector2 sizeScaleFrameIdxUVOffset =
			GpuParticleCurves::GetUvOffset(rendererParticles.SizeScaleFrameIdxCurveAlloc);
		const float sizeScaleFrameIdxUVScale =
			GpuParticleCurves::GetUvScale(rendererParticles.SizeScaleFrameIdxCurveAlloc);

		const SPtr<GpuParamBlockBuffer>& gpuParticlesParamBuffer = rendererParticles.GpuParticlesParamBuffer;
		gGpuParticlesParamDef.gColorCurveOffset.Set(gpuParticlesParamBuffer, colorUVOffset);
		gGpuParticlesParamDef.gColorCurveScale.Set(gpuParticlesParamBuffer, Vector2(colorUVScale, 0.0f));
		gGpuParticlesParamDef.gSizeScaleFrameIdxCurveOffset.Set(gpuParticlesParamBuffer, sizeScaleFrameIdxUVOffset);
		gGpuParticlesParamDef.gSizeScaleFrameIdxCurveScale.Set(gpuParticlesParamBuffer, Vector2(sizeScaleFrameIdxUVScale, 0.0f));

		// Write sprite animation curve
		if(spriteTexture)
		{
			gParticlesParamDef.gUVOffset.Set(particlesParamBuffer, spriteTexture->GetOffset());
			gParticlesParamDef.gUVScale.Set(particlesParamBuffer, spriteTexture->GetScale());

			const SpriteSheetGridAnimation& anim = spriteTexture->GetAnimation();
			gParticlesParamDef.gSubImageSize.Set(particlesParamBuffer, Vector4((float)anim.NumColumns, (float)anim.NumRows, 1.0f / anim.NumColumns, 1.0f / anim.NumRows));
		}
	}

	// Set up buffers for lighting
	const bool useForwardRendering = shaderFlags.IsSet(ShaderFlag::Forward);
	if(useForwardRendering)
	{
		renElement.ForwardLightingParams.Populate(gpuParams, supportsClusteredForward);
		renElement.ImageBasedParams.Populate(gpuParams, GPT_FRAGMENT_PROGRAM, true, supportsClusteredForward, supportsClusteredForward);
	}

	const bool isTransparent = shaderFlags.IsSet(ShaderFlag::Transparent);
	if(isTransparent)
	{
		// Optional depth buffer input if requested
		if(gpuParams->HasTexture(GPT_FRAGMENT_PROGRAM, "gDepthBufferTex"))
			gpuParams->GetTextureParam(GPT_FRAGMENT_PROGRAM, "gDepthBufferTex", renElement.DepthInputTexture);
	}
}

void RendererScene::UnregisterParticleSystem(ParticleSystem* particleSystem)
{
	const u32 rendererId = particleSystem->GetRendererId();
	RendererParticles& rendererParticles = mInfo.ParticleSystems[rendererId];

	// Free curves
	GpuParticleCurves& curves = GpuParticleSimulation::Instance().GetResources().GetCurveTexture();
	curves.Free(rendererParticles.ColorCurveAlloc);
	curves.Free(rendererParticles.SizeScaleFrameIdxCurveAlloc);

	if(rendererParticles.GpuParticleSystem)
	{
		bs_pool_delete(rendererParticles.GpuParticleSystem);
		rendererParticles.GpuParticleSystem = nullptr;
	}

	ParticleSystem* lastSystem = mInfo.ParticleSystems.back().ParticleSystem;
	const u32 lastRendererId = lastSystem->GetRendererId();

	if(rendererId != lastRendererId)
	{
		// Swap current last element with the one we want to erase
		std::swap(mInfo.ParticleSystems[rendererId], mInfo.ParticleSystems[lastRendererId]);
		std::swap(mInfo.ParticleSystemCullInfos[rendererId], mInfo.ParticleSystemCullInfos[lastRendererId]);

		lastSystem->SetRendererId(rendererId);
	}

	// Last element is the one we want to erase
	mInfo.ParticleSystems.erase(mInfo.ParticleSystems.end() - 1);
	mInfo.ParticleSystemCullInfos.erase(mInfo.ParticleSystemCullInfos.end() - 1);
}

void RendererScene::RegisterDecal(Decal* decal)
{
	const auto renderableId = (u32)mInfo.Decals.size();
	decal->SetRendererId(renderableId);

	mInfo.Decals.emplace_back();
	mInfo.DecalCullInfos.push_back(CullInfo(decal->GetBounds(), decal->GetLayer()));

	RendererDecal& rendererDecal = mInfo.Decals.back();
	rendererDecal.Decal = decal;
	rendererDecal.UpdatePerObjectBuffer();

	DecalRenderElement& renElement = rendererDecal.RenderElement;
	renElement.Type = (u32)RenderElementType::Decal;
	renElement.Mesh = RendererUtility::Instance().GetBoxStencil();
	renElement.SubMesh = renElement.Mesh->GetProperties().GetSubMesh();

	renElement.Material = decal->GetMaterial();

	if(renElement.Material != nullptr && renElement.Material->GetShader() == nullptr)
		renElement.Material = nullptr;

	// If no material use the default material
	if(renElement.Material == nullptr)
		renElement.Material = Material::Create(DefaultDecalMat::Get()->GetShader());

	for(u32 i = 0; i < 2; i++)
	{
		for(u32 j = 0; j < 3; j++)
		{
			FIND_TECHNIQUE_DESC findDesc;
			findDesc.Variation = DECAL_VAR_LOOKUP[i][j];
			findDesc.Override = true;

			u32 techniqueIdx = renElement.Material->FindTechnique(findDesc);
			if(techniqueIdx == (u32)-1)
				techniqueIdx = 0;

			const SPtr<Technique>& technique = renElement.Material->GetTechnique(techniqueIdx);
			if(technique)
				technique->Compile();

			renElement.TechniqueIndices[i][j] = techniqueIdx;
		}
	}

	renElement.DefaultTechniqueIdx = renElement.TechniqueIndices[0][0];

	// Generate or assigned renderer specific data for the material
	// Note: This makes the assumption that all variations of the material share the same parameter set
	renElement.Params = renElement.Material->CreateParamsSet(renElement.DefaultTechniqueIdx);
	renElement.Material->UpdateParamsSet(renElement.Params, 0.0f, true);

	// Generate or assign sampler state overrides
	renElement.SamplerOverrides = AllocSamplerStateOverrides(renElement);

	// Prepare all parameter bindings
	SPtr<GpuParams> gpuParams = renElement.Params->GetGpuParams();

	// Note: Perhaps perform buffer validation to ensure expected buffer has the same size and layout as the
	// provided buffer, and show a warning otherwise. But this is perhaps better handled on a higher level.
	gpuParams->SetParamBlockBuffer("PerFrame", mPerFrameParamBuffer);
	gpuParams->SetParamBlockBuffer("DecalParams", rendererDecal.DecalParamBuffer);
	gpuParams->SetParamBlockBuffer("PerObject", rendererDecal.PerObjectParamBuffer);
	gpuParams->SetParamBlockBuffer("PerCall", rendererDecal.PerCallParamBuffer);

	gpuParams->GetParamInfo()->GetBindings(
		GpuPipelineParamInfoBase::ParamType::ParamBlock,
		"PerCamera",
		renElement.PerCameraBindings);

	if(gpuParams->HasTexture(GPT_FRAGMENT_PROGRAM, "gDepthBufferTex"))
		gpuParams->GetTextureParam(GPT_FRAGMENT_PROGRAM, "gDepthBufferTex", renElement.DepthInputTexture);

	if(gpuParams->HasTexture(GPT_FRAGMENT_PROGRAM, "gMaskTex"))
		gpuParams->GetTextureParam(GPT_FRAGMENT_PROGRAM, "gMaskTex", renElement.MaskInputTexture);
}

void RendererScene::UpdateDecal(Decal* decal)
{
	const u32 rendererId = decal->GetRendererId();

	mInfo.Decals[rendererId].UpdatePerObjectBuffer();
	mInfo.DecalCullInfos[rendererId].Bounds = decal->GetBounds();
}

void RendererScene::UnregisterDecal(Decal* decal)
{
	const u32 rendererId = decal->GetRendererId();
	Decal* lastDecal = mInfo.Decals.back().Decal;
	const u32 lastDecalId = lastDecal->GetRendererId();

	RendererDecal& rendererDecal = mInfo.Decals[rendererId];
	DecalRenderElement& renElement = rendererDecal.RenderElement;

	// Unregister sampler overrides
	FreeSamplerStateOverrides(renElement);
	renElement.SamplerOverrides = nullptr;

	if(rendererId != lastDecalId)
	{
		// Swap current last element with the one we want to erase
		std::swap(mInfo.Decals[rendererId], mInfo.Decals[lastDecalId]);
		std::swap(mInfo.DecalCullInfos[rendererId], mInfo.DecalCullInfos[lastDecalId]);

		lastDecal->SetRendererId(rendererId);
	}

	// Last element is the one we want to erase
	mInfo.Decals.erase(mInfo.Decals.end() - 1);
	mInfo.DecalCullInfos.erase(mInfo.DecalCullInfos.end() - 1);
}

void RendererScene::SetOptions(const SPtr<RenderBeastOptions>& options)
{
	mOptions = options;

	for(auto& entry : mInfo.Views)
		entry->SetStateReductionMode(mOptions->StateReductionMode);
}

RENDERER_VIEW_DESC RendererScene::CreateViewDesc(Camera* camera) const
{
	SPtr<Viewport> viewport = camera->GetViewport();
	ClearFlags clearFlags = viewport->GetClearFlags();
	RENDERER_VIEW_DESC viewDesc;

	viewDesc.Target.ClearFlags = 0;
	if(clearFlags.IsSet(ClearFlagBits::Color))
		viewDesc.Target.ClearFlags |= FBT_COLOR;

	if(clearFlags.IsSet(ClearFlagBits::Depth))
		viewDesc.Target.ClearFlags |= FBT_DEPTH;

	if(clearFlags.IsSet(ClearFlagBits::Stencil))
		viewDesc.Target.ClearFlags |= FBT_STENCIL;

	viewDesc.Target.ClearColor = viewport->GetClearColorValue();
	viewDesc.Target.ClearDepthValue = viewport->GetClearDepthValue();
	viewDesc.Target.ClearStencilValue = viewport->GetClearStencilValue();

	viewDesc.Target.Target = viewport->GetTarget();
	viewDesc.Target.NrmViewRect = viewport->GetArea();
	viewDesc.Target.ViewRect = viewport->GetPixelArea();

	if(viewDesc.Target.Target != nullptr)
	{
		viewDesc.Target.TargetWidth = viewDesc.Target.Target->GetProperties().Width;
		viewDesc.Target.TargetHeight = viewDesc.Target.Target->GetProperties().Height;
	}
	else
	{
		viewDesc.Target.TargetWidth = 0;
		viewDesc.Target.TargetHeight = 0;
	}

	viewDesc.Target.NumSamples = camera->GetMsaaCount();

	viewDesc.MainView = camera->IsMain();
	viewDesc.TriggerCallbacks = true;
	viewDesc.RunPostProcessing = true;
	viewDesc.CapturingReflections = false;
	viewDesc.OnDemand = camera->GetFlags().IsSet(CameraFlag::OnDemand);

	viewDesc.CullFrustum = camera->GetWorldFrustum();
	viewDesc.VisibleLayers = camera->GetLayers();
	viewDesc.NearPlane = camera->GetNearClipDistance();
	viewDesc.FarPlane = camera->GetFarClipDistance();
	viewDesc.FlipView = false;

	const Transform& tfrm = camera->GetTransform();
	viewDesc.ViewOrigin = tfrm.GetPosition();
	viewDesc.ViewDirection = tfrm.GetForward();
	viewDesc.ProjTransform = camera->GetProjectionMatrixRs();
	viewDesc.ViewTransform = camera->GetViewMatrix();
	viewDesc.ProjType = camera->GetProjectionType();

	viewDesc.StateReduction = mOptions->StateReductionMode;
	viewDesc.SceneCamera = camera;

	return viewDesc;
}

void RendererScene::UpdateCameraRenderTargets(Camera* camera, bool remove)
{
	SPtr<RenderTarget> renderTarget = camera->GetViewport()->GetTarget();

	// Remove from render target list
	int rtChanged = 0; // 0 - No RT, 1 - RT found, 2 - RT changed
	for(auto iterTarget = mInfo.RenderTargets.begin(); iterTarget != mInfo.RenderTargets.end(); ++iterTarget)
	{
		RendererRenderTarget& target = *iterTarget;
		for(auto iterCam = target.Cameras.begin(); iterCam != target.Cameras.end(); ++iterCam)
		{
			if(camera == *iterCam)
			{
				if(remove)
				{
					target.Cameras.erase(iterCam);
					rtChanged = 1;
				}
				else
				{
					if(renderTarget != target.Target)
					{
						target.Cameras.erase(iterCam);
						rtChanged = 2;
					}
					else
						rtChanged = 1;
				}

				break;
			}
		}

		if(target.Cameras.empty())
		{
			mInfo.RenderTargets.erase(iterTarget);
			break;
		}
	}

	// Register in render target list
	if(renderTarget != nullptr && !remove && (rtChanged == 0 || rtChanged == 2))
	{
		auto findIter = std::find_if(mInfo.RenderTargets.begin(), mInfo.RenderTargets.end(), [&](const RendererRenderTarget& x)
									 { return x.Target == renderTarget; });

		if(findIter != mInfo.RenderTargets.end())
		{
			findIter->Cameras.push_back(camera);
		}
		else
		{
			mInfo.RenderTargets.push_back(RendererRenderTarget());
			RendererRenderTarget& renderTargetData = mInfo.RenderTargets.back();

			renderTargetData.Target = renderTarget;
			renderTargetData.Cameras.push_back(camera);
		}

		// Sort render targets based on priority
		auto cameraComparer = [&](const Camera* a, const Camera* b)
		{ return a->GetPriority() > b->GetPriority(); };
		auto renderTargetInfoComparer = [&](const RendererRenderTarget& a, const RendererRenderTarget& b)
		{ return a.Target->GetProperties().Priority > b.Target->GetProperties().Priority; };
		std::sort(begin(mInfo.RenderTargets), end(mInfo.RenderTargets), renderTargetInfoComparer);

		for(auto& camerasPerTarget : mInfo.RenderTargets)
		{
			Vector<Camera*>& cameras = camerasPerTarget.Cameras;

			std::sort(begin(cameras), end(cameras), cameraComparer);
		}
	}
}

void RendererScene::RefreshSamplerOverrides(bool force)
{
	bool anyDirty = false;
	for(auto& entry : mSamplerOverrides)
	{
		SPtr<MaterialParams> materialParams = entry.first.Material->GetInternalParamsInternal();

		MaterialSamplerOverrides* materialOverrides = entry.second;
		for(u32 i = 0; i < materialOverrides->NumOverrides; i++)
		{
			SamplerOverride& override = materialOverrides->Overrides[i];
			const MaterialParamsBase::ParamData* materialParamData = materialParams->GetParamData(override.ParamIdx);

			SPtr<SamplerState> samplerState;
			materialParams->GetSamplerState(*materialParamData, samplerState);

			u64 hash = 0;
			if(samplerState != nullptr)
				hash = samplerState->GetProperties().GetHash();

			if(hash != override.OriginalStateHash || force)
			{
				if(samplerState != nullptr)
					override.State = SamplerOverrideUtility::GenerateSamplerOverride(samplerState, mOptions);
				else
					override.State = SamplerOverrideUtility::GenerateSamplerOverride(SamplerState::GetDefault(), mOptions);

				override.OriginalStateHash = override.State->GetProperties().GetHash();
				materialOverrides->IsDirty = true;
			}

			// Dirty flag can also be set externally, so check here even though we assign it above
			if(materialOverrides->IsDirty)
				anyDirty = true;
		}
	}

	// Early exit if possible
	if(!anyDirty)
		return;

	u32 numRenderables = (u32)mInfo.Renderables.size();
	for(u32 i = 0; i < numRenderables; i++)
	{
		for(auto& element : mInfo.Renderables[i]->Elements)
		{
			MaterialSamplerOverrides* overrides = element.SamplerOverrides;
			if(overrides != nullptr && overrides->IsDirty)
			{
				u32 numPasses = element.Material->GetNumPasses();
				for(u32 j = 0; j < numPasses; j++)
				{
					SPtr<GpuParams> params = element.Params->GetGpuParams(j);

					const u32 numStages = 6;
					for(u32 k = 0; k < numStages; k++)
					{
						GpuProgramType type = (GpuProgramType)k;

						SPtr<GpuParamDesc> paramDesc = params->GetParamDesc(type);
						if(paramDesc == nullptr)
							continue;

						for(auto& samplerDesc : paramDesc->Samplers)
						{
							u32 set = samplerDesc.second.Set;
							u32 slot = samplerDesc.second.Slot;

							u32 overrideIndex = overrides->Passes[j].StateOverrides[set][slot];
							if(overrideIndex == (u32)-1)
								continue;

							params->SetSamplerState(set, slot, overrides->Overrides[overrideIndex].State);
						}
					}
				}
			}
		}
	}

	for(auto& entry : mSamplerOverrides)
		entry.second->IsDirty = false;
}

void RendererScene::SetParamFrameParams(float time)
{
	gPerFrameParamDef.gTime.Set(mPerFrameParamBuffer, time);
}

void RendererScene::PrepareRenderable(u32 idx, const FrameInfo& frameInfo)
{
	RendererRenderable* rendererRenderable = mInfo.Renderables[idx];

	for(auto& element : rendererRenderable->Elements)
		element.MaterialAnimationTime += frameInfo.Timings.TimeDelta;

	if(frameInfo.PerFrameData.Animation != nullptr)
		rendererRenderable->Renderable->UpdatePrevFrameAnimationBuffers();

	if(rendererRenderable->PrevFrameDirtyState != PrevFrameDirtyState::Clean)
	{
		if(rendererRenderable->PrevFrameDirtyState == PrevFrameDirtyState::Updated)
			rendererRenderable->PrevFrameDirtyState = PrevFrameDirtyState::CopyMostRecent;
		else if(rendererRenderable->PrevFrameDirtyState == PrevFrameDirtyState::CopyMostRecent)
		{
			rendererRenderable->PrevWorldTfrm = mInfo.Renderables[idx]->WorldTfrm;
			rendererRenderable->PrevFrameDirtyState = PrevFrameDirtyState::Clean;
			rendererRenderable->UpdatePerObjectBuffer();
		}
	}
}

void RendererScene::PrepareVisibleRenderable(u32 idx, const FrameInfo& frameInfo)
{
	if(mInfo.RenderableReady[idx])
		return;

	RendererRenderable* rendererRenderable = mInfo.Renderables[idx];

	// Note: Before uploading bone matrices perhaps check if they has actually been changed since last frame
	if(frameInfo.PerFrameData.Animation != nullptr)
		rendererRenderable->Renderable->UpdateAnimationBuffers(*frameInfo.PerFrameData.Animation);

	// Note: Could this step be moved in notifyRenderableUpdated, so it only triggers when material actually gets
	// changed? Although it shouldn't matter much because if the internal versions keeping track of dirty params.
	for(auto& element : rendererRenderable->Elements)
		element.Material->UpdateParamsSet(element.Params, element.MaterialAnimationTime);

	mInfo.Renderables[idx]->PerObjectParamBuffer->FlushToGpu();
	mInfo.RenderableReady[idx] = true;
}

void RendererScene::PrepareParticleSystem(u32 idx, const FrameInfo& frameInfo)
{
	RendererParticles& rendererParticles = mInfo.ParticleSystems[idx];

	if(rendererParticles.PrevFrameDirtyState != PrevFrameDirtyState::Clean)
	{
		if(rendererParticles.PrevFrameDirtyState == PrevFrameDirtyState::Updated)
			rendererParticles.PrevFrameDirtyState = PrevFrameDirtyState::CopyMostRecent;
		else if(rendererParticles.PrevFrameDirtyState == PrevFrameDirtyState::CopyMostRecent)
		{
			rendererParticles.PrevLocalToWorld = rendererParticles.LocalToWorld;
			rendererParticles.PrevFrameDirtyState = PrevFrameDirtyState::Clean;
			rendererParticles.UpdatePerObjectBuffer();
		}
	}

	ParticlesRenderElement& renElement = mInfo.ParticleSystems[idx].RenderElement;
	renElement.Material->UpdateParamsSet(renElement.Params, 0.0f);

	mInfo.ParticleSystems[idx].PerObjectParamBuffer->FlushToGpu();
}

void RendererScene::PrepareDecal(u32 idx, const FrameInfo& frameInfo)
{
	DecalRenderElement& renElement = mInfo.Decals[idx].RenderElement;
	renElement.MaterialAnimationTime += frameInfo.Timings.TimeDelta;
	renElement.Material->UpdateParamsSet(renElement.Params, renElement.MaterialAnimationTime);

	mInfo.Decals[idx].PerObjectParamBuffer->FlushToGpu();
}

void RendererScene::UpdateParticleSystemBounds(const ParticlePerFrameData* particleRenderData)
{
	// Note: Avoid updating bounds for deterministic particle systems every frame. Also see if this can be copied
	// over in a faster way (or ideally just assigned)

	for(auto& entry : mInfo.ParticleSystems)
	{
		const u32 rendererId = entry.ParticleSystem->GetRendererId();

		AABox worldAABox = AABox::INF_BOX;
		const auto iterFind = particleRenderData->CpuData.find(entry.ParticleSystem->GetId());
		if(iterFind != particleRenderData->CpuData.end())
			worldAABox = iterFind->second->Bounds;
		else if(entry.GpuParticleSystem)
			worldAABox = entry.GpuParticleSystem->GetBounds();

		const ParticleSystemSettings& settings = entry.ParticleSystem->GetSettings();
		if(settings.SimulationSpace == ParticleSimulationSpace::Local)
			worldAABox.TransformAffine(entry.LocalToWorld);

		const Sphere worldSphere(worldAABox.GetCenter(), worldAABox.GetRadius());
		mInfo.ParticleSystemCullInfos[rendererId].Bounds = Bounds(worldAABox, worldSphere);
	}
}

MaterialSamplerOverrides* RendererScene::AllocSamplerStateOverrides(RenderElement& elem)
{
	SamplerOverrideKey samplerKey(elem.Material, elem.DefaultTechniqueIdx);
	auto iterFind = mSamplerOverrides.find(samplerKey);
	if(iterFind != mSamplerOverrides.end())
	{
		iterFind->second->RefCount++;
		return iterFind->second;
	}
	else
	{
		SPtr<Shader> shader = elem.Material->GetShader();
		MaterialSamplerOverrides* samplerOverrides = SamplerOverrideUtility::GenerateSamplerOverrides(shader, elem.Material->GetInternalParamsInternal(), elem.Params, mOptions);

		mSamplerOverrides[samplerKey] = samplerOverrides;

		samplerOverrides->RefCount++;
		return samplerOverrides;
	}
}

void RendererScene::FreeSamplerStateOverrides(RenderElement& elem)
{
	SamplerOverrideKey samplerKey(elem.Material, elem.DefaultTechniqueIdx);

	auto iterFind = mSamplerOverrides.find(samplerKey);
	assert(iterFind != mSamplerOverrides.end());

	MaterialSamplerOverrides* samplerOverrides = iterFind->second;
	samplerOverrides->RefCount--;
	if(samplerOverrides->RefCount == 0)
	{
		PassSamplerOverrides(samplerOverrides);
		mSamplerOverrides.erase(iterFind);
	}
}
}}
