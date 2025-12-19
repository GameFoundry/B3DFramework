//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DRenderBeastScene.h"
#include "Components/B3DCamera.h"
#include "Components/B3DLight.h"
#include "Components/B3DSkybox.h"
#include "Components/B3DReflectionProbe.h"
#include "Renderer/B3DRenderer.h"
#include "Particles/B3DParticleScene.h"
#include "Mesh/B3DMesh.h"
#include "Material/B3DPass.h"
#include "Material/B3DMaterialParameterAdapter.h"
#include "Utility/B3DSamplerOverrides.h"
#include "B3DRenderBeastOptions.h"
#include "B3DRenderBeast.h"
#include "B3DRendererDecal.h"
#include "Image/B3DSpriteTexture.h"
#include "RenderAPI/B3DVertexDescription.h"
#include "Shading/B3DGpuParticleSimulation.h"
#include "Components/B3DDecal.h"
#include "Renderer/B3DIBLUtility.h"
#include "Renderer/B3DRendererUtility.h"
#include "Utility/B3DBitwise.h"

namespace b3d { namespace render {

PerFrameParamDef gPerFrameParamDef;

static const ShaderVariationParameters* DECAL_VAR_LOOKUP[2][3] = {
	{ &GetDecalShaderVariation<false, MSAAMode::None>(),
	  &GetDecalShaderVariation<false, MSAAMode::Single>(),
	  &GetDecalShaderVariation<false, MSAAMode::Full>() },
	{ &GetDecalShaderVariation<true, MSAAMode::None>(),
	  &GetDecalShaderVariation<true, MSAAMode::Single>(),
	  &GetDecalShaderVariation<true, MSAAMode::Full>() }
};

/** Returns a specific forward rendering shader variation. */
template <bool SKINNED, bool MORPH, bool CLUSTERED, bool WRITE_VELOCITY>
static const ShaderVariationParameters& GetForwardRenderingVariation(bool supportsVelocityWrites)
{
	if(!supportsVelocityWrites)
	{
		static ShaderVariationParameters variation = ShaderVariationParameters(
			{
				ShaderVariationParameter("SKINNED", SKINNED),
				ShaderVariationParameter("MORPH", MORPH),
				ShaderVariationParameter("CLUSTERED", CLUSTERED),
			});

		return variation;
	}
	else
	{
		static ShaderVariationParameters variation = ShaderVariationParameters(
			{
				ShaderVariationParameter("SKINNED", SKINNED),
				ShaderVariationParameter("MORPH", MORPH),
				ShaderVariationParameter("CLUSTERED", CLUSTERED),
				ShaderVariationParameter("WRITE_VELOCITY", WRITE_VELOCITY),
			});

		return variation;
	}
}

/** Returns a specific forward rendering shader variation. */
template <bool CLUSTERED, bool WRITE_VELOCITY>
static const ShaderVariationParameters* GetClusteredForwardRenderingVariation(RenderableAnimType animType, bool shaderCanWriteVelocity)
{
	const ShaderVariationParameters* VAR_LOOKUP[4];
	VAR_LOOKUP[0] = &GetForwardRenderingVariation<false, false, CLUSTERED, WRITE_VELOCITY>(shaderCanWriteVelocity);
	VAR_LOOKUP[1] = &GetForwardRenderingVariation<true, false, CLUSTERED, WRITE_VELOCITY>(shaderCanWriteVelocity);
	VAR_LOOKUP[2] = &GetForwardRenderingVariation<false, true, CLUSTERED, WRITE_VELOCITY>(shaderCanWriteVelocity);
	VAR_LOOKUP[3] = &GetForwardRenderingVariation<true, true, CLUSTERED, WRITE_VELOCITY>(shaderCanWriteVelocity);

	return VAR_LOOKUP[(int)animType];
}

/** Returns a specific base pass shader variation. */
template <bool WRITE_VELOCITY>
static const ShaderVariationParameters* GetBasePassVariation(bool useForwardRendering, bool supportsClusteredForward, bool shaderCanWriteVelocity, RenderableAnimType animType)
{
	if(useForwardRendering)
	{
		if(supportsClusteredForward)
			return GetClusteredForwardRenderingVariation<true, WRITE_VELOCITY>(animType, shaderCanWriteVelocity);
		else
			return GetClusteredForwardRenderingVariation<false, WRITE_VELOCITY>(animType, shaderCanWriteVelocity);
	}
	else
	{
		const ShaderVariationParameters* VAR_LOOKUP[4];
		VAR_LOOKUP[0] = &GetVertexInputVariation<false, false, WRITE_VELOCITY>(shaderCanWriteVelocity);
		VAR_LOOKUP[1] = &GetVertexInputVariation<true, false, WRITE_VELOCITY>(shaderCanWriteVelocity);
		VAR_LOOKUP[2] = &GetVertexInputVariation<false, true, WRITE_VELOCITY>(shaderCanWriteVelocity);
		VAR_LOOKUP[3] = &GetVertexInputVariation<true, true, WRITE_VELOCITY>(shaderCanWriteVelocity);

		return VAR_LOOKUP[(int)animType];
	}
}

/** Initializes a specific base pass technique on the provided material and returns the technique index. */
static u32 InitAndRetrieveBasePassTechnique(Material& material, bool useForwardRendering, bool supportsClusteredForward, bool shaderCanWriteVelocity, bool writeVelocity, RenderableAnimType animType)
{
	const ShaderVariationParameters* const variationParameters = writeVelocity ? GetBasePassVariation<true>(useForwardRendering, supportsClusteredForward, shaderCanWriteVelocity, animType) : GetBasePassVariation<false>(useForwardRendering, supportsClusteredForward, shaderCanWriteVelocity, animType);

	FindVariationInformation findInformation;
	findInformation.VariationParameters = variationParameters;
	findInformation.Override = true;

	u32 variationIndex = material.FindVariation(findInformation);

	if(variationIndex == ~0u)
		variationIndex = material.GetDefaultVariation();

	// Make sure the variation is compiled
	const SPtr<Variation>& variation = material.GetVariation(variationIndex);
	if(variation != nullptr)
		variation->Compile();

	return variationIndex;
}

static void ValidateBasePassMaterial(Material& material, RenderableAnimType animType, u32 techniqueIdx, VertexDescription& vertexBufferDescription)
{
	// Validate mesh <-> shader vertex bindings
	u32 numPasses = material.GetPassCount(techniqueIdx);
	for(u32 j = 0; j < numPasses; j++)
	{
		SPtr<Pass> pass = material.GetPass(j, techniqueIdx);
		SPtr<GpuGraphicsPipelineState> graphicsPipeline = pass->GetGraphicsPipelineState();

		SPtr<VertexDescription> shaderVertexDescription = graphicsPipeline->GetVertexProgram()->GetVertexInputDescription();
		if(shaderVertexDescription && !VertexDescription::IsCompatibleWithShaderInputs(vertexBufferDescription, *shaderVertexDescription))
		{
			TInlineArray<VertexElement, 8> missingElements = VertexDescription::GetMissingElementsForShaderInput(vertexBufferDescription, *shaderVertexDescription);

			// If using morph shapes ignore POSITION1 and NORMAL1 missing since we assign them from within the renderer
			if(animType == RenderableAnimType::Morph || animType == RenderableAnimType::SkinnedMorph)
			{
				auto removeIter = std::remove_if(missingElements.begin(), missingElements.end(), [](const VertexElement& x)
												 { return (x.GetSemantic() == VES_POSITION && x.GetSemanticIndex() == 1) ||
													   (x.GetSemantic() == VES_NORMAL && x.GetSemanticIndex() == 1); });

				missingElements.erase(removeIter, missingElements.end());
			}

			if(!missingElements.Empty())
			{
				StringStream wrnStream;
				wrnStream << "Provided mesh is missing required vertex attributes to render with the \
									provided shader. Missing elements: "
						  << std::endl;

				for(auto& entry : missingElements)
					wrnStream << "\t" << ToString(entry.GetSemantic()) << entry.GetSemanticIndex() << std::endl;

				B3D_LOG(Warning, Renderer, "{0}", wrnStream.str());
				break;
			}
		}
	}
}

RenderBeastScene::RenderBeastScene(const SPtr<RenderBeastOptions>& options)
	: mOptions(options)
{
}

void RenderBeastScene::RegisterCamera(Camera* camera)
{
	RendererViewCreateInformation viewDesc = CreateViewDesc(camera);

	RendererView* view = B3DNew<RendererView>(viewDesc);
	view->SetRenderSettings(camera->GetRenderSettings());
	view->UpdatePerViewBuffer();

	u32 viewIdx = (u32)mInfo.Views.size();
	mInfo.Views.push_back(view);

	mInfo.CameraToView[camera] = viewIdx;
	camera->SetRendererId(viewIdx);

	UpdateCameraRenderTargets(camera);
}

void RenderBeastScene::UpdateCamera(Camera* camera, u32 updateFlag)
{
	u32 cameraId = camera->GetRendererId();
	RendererView* view = mInfo.Views[cameraId];

	if((updateFlag & (u32)CameraDirtyFlag::Redraw) != 0)
		view->NotifyNeedsRedraw();

	u32 updateEverythingFlag = (u32)ComponentDirtyFlag::Everything | (u32)ComponentDirtyFlag::Active | (u32)CameraDirtyFlag::Viewport;

	if((updateFlag & updateEverythingFlag) != 0)
	{
		RendererViewCreateInformation viewDesc = CreateViewDesc(camera);

		view->SetView(viewDesc);
		view->SetRenderSettings(camera->GetRenderSettings());
		view->UpdatePerViewBuffer();

		UpdateCameraRenderTargets(camera);
		return;
	}

	if((updateFlag & (u32)CameraDirtyFlag::RenderSettings) != 0)
		view->SetRenderSettings(camera->GetRenderSettings());

	const Transform& tfrm = camera->GetWorldTransform();
	view->SetTransform(
		tfrm.GetPosition(),
		tfrm.GetForward(),
		camera->GetViewMatrix(),
		camera->GetProjectionMatrix(),
		camera->GetWorldFrustum());

	view->UpdatePerViewBuffer();
}

void RenderBeastScene::UnregisterCamera(Camera* camera)
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
	B3DDelete(view);

	mInfo.Views.erase(mInfo.Views.end() - 1);

	auto iterFind = mInfo.CameraToView.find(camera);
	if(iterFind != mInfo.CameraToView.end())
		mInfo.CameraToView.erase(iterFind);

	UpdateCameraRenderTargets(camera, true);
}

void RenderBeastScene::RegisterLight(Light* light)
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

void RenderBeastScene::UpdateLight(Light* light)
{
	u32 lightId = light->GetRendererId();

	if(light->GetType() == LightType::Radial)
		mInfo.RadialLightWorldBounds[lightId] = light->GetBounds();
	else if(light->GetType() == LightType::Spot)
		mInfo.SpotLightWorldBounds[lightId] = light->GetBounds();
}

void RenderBeastScene::UnregisterLight(Light* light)
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

void RenderBeastScene::RegisterRenderable(Renderable* renderable)
{
	u32 renderableId = (u32)mInfo.Renderables.size();

	renderable->SetRendererId(renderableId);

	mInfo.Renderables.push_back(B3DNew<RendererRenderable>());
	mInfo.RenderableCullInfos.push_back(CullInfo(renderable->GetBounds(), renderable->GetLayer(), renderable->GetCullDistanceFactor()));

	RendererRenderable* rendererRenderable = mInfo.Renderables.back();
	rendererRenderable->Renderable = renderable;
	rendererRenderable->UpdatePerObjectData();
	rendererRenderable->PrevWorldTransform = rendererRenderable->WorldTransform;
	rendererRenderable->PrevFrameDirtyState = PrevFrameDirtyState::Clean;

	SPtr<Mesh> mesh = renderable->GetMesh();
	if(mesh != nullptr)
	{
		const MeshProperties& meshProps = mesh->GetProperties();
		SPtr<VertexDescription> vertexDescription = mesh->GetVertexData()->VertexDescription;

		for(u32 i = 0; i < (u32)meshProps.SubMeshes.size(); i++)
		{
			rendererRenderable->Elements.push_back(RenderableElement());
			RenderableElement& renElement = rendererRenderable->Elements.back();

			renElement.Type = (u32)RenderElementType::Renderable;
			renElement.Mesh = mesh;
			renElement.SubMesh = meshProps.SubMeshes[i];
			renElement.AnimType = renderable->GetAnimType();
			renElement.AnimationId = renderable->GetAnimationId();
			renElement.MorphShapeVersion = 0;
			renElement.MorphShapeBuffer = renderable->GetMorphShapeBuffer();
			renElement.MorphVertexDefinition = renderable->GetMorphVertexDescription();

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
			bool supportsClusteredForward = GetRenderBeast()->GetFeatureSet() == RenderBeastFeatureSet::Desktop;

			const Vector<ShaderVariationParameterInformation>& variationParams = shader->GetVariationParams();
			const bool shaderCanWriteVelocity = std::find_if(variationParams.begin(), variationParams.end(), [](const ShaderVariationParameterInformation& x)
															 { return x.Identifier == "WRITE_VELOCITY"; }) != variationParams.end();

			const bool writeVelocity = shaderCanWriteVelocity && renderable->GetWriteVelocity();

			RenderableAnimType animType = renderable->GetAnimType();

			renElement.DefaultVariationIndex = InitAndRetrieveBasePassTechnique(*renElement.Material, useForwardRendering, supportsClusteredForward, shaderCanWriteVelocity, false, animType);



#if B3D_DEBUG
			ValidateBasePassMaterial(*renElement.Material, animType, renElement.DefaultVariationIndex, *vertexDescription);
#endif

			// Generate or assigned renderer specific data for the material
			renElement.ParameterAdapter = renElement.Material->CreateParameterAdapter(renElement.DefaultVariationIndex);
			renElement.ParameterAdapter->Update(renElement.Material, 0.0f, true);

			if(writeVelocity)
			{
				renElement.WriteVelocityVariationIndex = InitAndRetrieveBasePassTechnique(*renElement.Material, useForwardRendering, supportsClusteredForward, shaderCanWriteVelocity, true, animType);

#if B3D_DEBUG
				ValidateBasePassMaterial(*renElement.Material, animType, renElement.WriteVelocityVariationIndex, *vertexDescription);
#endif

				// Note: Using the same params as the non-velocity technique. There are assumed to be no differences
			}
			else
				renElement.WriteVelocityVariationIndex = (u32)-1;

			// Generate or assign sampler state overrides
			renElement.SamplerOverrides = AllocSamplerStateOverrides(renElement);
		}
	}

	// Allocate from the uniform buffer manager
	if(!rendererRenderable->Elements.empty())
	{
		auto result = mUniformBufferPools.Allocate();
		rendererRenderable->PerObjectBufferAllocationHandle = result.Handle;
		rendererRenderable->PerObjectParameterSet = result.ParameterSet;
		rendererRenderable->PerObjectSuballocation = result.GetSuballocation(UniformBufferPools::PerObjectBuffer);
	}

	mUniformBufferPools.UpdatePerObjectBuffer(*rendererRenderable);

	// Prepare all parameter bindings
	for(auto& element : rendererRenderable->Elements)
	{
		SPtr<Shader> shader = element.Material->GetShader();
		if(shader == nullptr)
		{
			B3D_LOG(Warning, Renderer, "Missing shader on material.");
			continue;
		}

		// Store shared parameter set and buffer offset for render-time binding
		element.SharedPerObjectParameterSet = rendererRenderable->PerObjectParameterSet;
		element.PerObjectBufferOffset = rendererRenderable->PerObjectSuballocation.GetSuballocationOffset();

		SPtr<GpuParameterSet> gpuParameterSet = element.ParameterAdapter->GetGpuParameterSet();

		// Note: Perhaps perform buffer validation to ensure expected buffer has the same size and layout as the
		// provided buffer, and show a warning otherwise. But this is perhaps better handled on a higher level.
		gpuParameterSet->TryGetUniformBufferParameter("PerFrame", element.PerFrameUniformBufferParameter);
		gpuParameterSet->TryGetUniformBufferParameter("PerCamera", element.PerCameraUniformBufferParameter);
		gpuParameterSet->TryGetStorageBufferParameter("boneMatrices", element.BoneMatrixBufferParameter);
		gpuParameterSet->TryGetStorageBufferParameter("prevBoneMatrices", element.PreviousBoneMatrixBufferParameter);

		ShaderFlags shaderFlags = shader->GetFlags();
		const bool useForwardRendering = shaderFlags.IsSet(ShaderFlag::Forward) || shaderFlags.IsSet(ShaderFlag::Transparent);

		if(useForwardRendering)
		{
			const bool supportsClusteredForward = GetRenderBeast()->GetFeatureSet() == RenderBeastFeatureSet::Desktop;

			element.ForwardLightingParams.Populate(gpuParameterSet, supportsClusteredForward);
			element.ImageBasedParams.Initialize(gpuParameterSet, GPT_FRAGMENT_PROGRAM, true, supportsClusteredForward, supportsClusteredForward);
		}
	}
}

void RenderBeastScene::UpdateRenderable(Renderable* renderable)
{
	u32 renderableId = renderable->GetRendererId();

	RendererRenderable* rendererRenderable = mInfo.Renderables[renderableId];

	if(rendererRenderable->PrevFrameDirtyState != PrevFrameDirtyState::Updated)
		rendererRenderable->PrevWorldTransform = rendererRenderable->WorldTransform;

	rendererRenderable->UpdatePerObjectData();
	rendererRenderable->PrevFrameDirtyState = PrevFrameDirtyState::Updated;

	mUniformBufferPools.UpdatePerObjectBuffer(*rendererRenderable);

	mInfo.RenderableCullInfos[renderableId].Bounds = renderable->GetBounds();
	mInfo.RenderableCullInfos[renderableId].CullDistanceFactor = renderable->GetCullDistanceFactor();
}

void RenderBeastScene::UnregisterRenderable(Renderable* renderable)
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

	// Release the buffer allocation
	mUniformBufferPools.Release(rendererRenderable->PerObjectBufferAllocationHandle);

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

	B3DDelete(rendererRenderable);
}

void RenderBeastScene::RegisterReflectionProbe(ReflectionProbe* probe)
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

	if(probeInfo.ArrayIdx > kMaxReflectionCubemaps)
	{
		B3D_LOG(Error, Renderer, "Reached the maximum number of allowed reflection probe cubemaps at once. "
								"Ignoring reflection probe data.");
	}
}

void RenderBeastScene::UpdateReflectionProbe(ReflectionProbe* probe, bool texture)
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

void RenderBeastScene::UnregisterReflectionProbe(ReflectionProbe* probe)
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

void RenderBeastScene::UpdateReflectionProbes(GpuCommandBuffer& commandBuffer)
{
	SceneInfo& sceneInfo = GetSceneInfo();
	const u32 probeCount = (u32)sceneInfo.ReflProbes.size();

	B3DMarkAllocatorFrame();
	{
		u32 currentCubeArraySize = 0;

		if(sceneInfo.ReflProbeCubemapsTex != nullptr)
			currentCubeArraySize = sceneInfo.ReflProbeCubemapsTex->GetProperties().ArraySliceCount;

		bool forceArrayUpdate = false;
		if(sceneInfo.ReflProbeCubemapsTex == nullptr || (currentCubeArraySize < probeCount && currentCubeArraySize != kMaxReflectionCubemaps))
		{
			TextureCreateInformation cubeMapDesc;
			cubeMapDesc.Name = "Reflection Probe Cubemap Array";
			cubeMapDesc.Type = TEX_TYPE_CUBE_MAP;
			cubeMapDesc.Format = PF_RG11B10F;
			cubeMapDesc.Width = IBLUtility::kReflectionCubemapSize;
			cubeMapDesc.Height = IBLUtility::kReflectionCubemapSize;
			cubeMapDesc.MipMapCount = PixelUtility::GetMipmapCount(cubeMapDesc.Width, cubeMapDesc.Height, 1, cubeMapDesc.Format);
			cubeMapDesc.ArraySliceCount = std::min(kMaxReflectionCubemaps, probeCount + 4); // Keep a few empty entries

			sceneInfo.ReflProbeCubemapsTex = mGpuDevice->CreateTexture(cubeMapDesc);

			forceArrayUpdate = true;
		}

		auto& cubemapArrayProps = sceneInfo.ReflProbeCubemapsTex->GetProperties();

		FrameQueue<u32> emptySlots;
		for(u32 i = 0; i < probeCount; i++)
		{
			const RendererReflectionProbe& probeInfo = sceneInfo.ReflProbes[i];

			if(probeInfo.ArrayIdx > kMaxReflectionCubemaps)
				continue;

			if(probeInfo.ArrayDirty || forceArrayUpdate)
			{
				SPtr<Texture> texture = probeInfo.Probe->GetFilteredTexture();
				if(texture == nullptr)
					continue;

				auto& srcProps = texture->GetProperties();
				bool isValid = srcProps.Width == IBLUtility::kReflectionCubemapSize &&
					srcProps.Height == IBLUtility::kReflectionCubemapSize &&
					srcProps.MipMapCount == cubemapArrayProps.MipMapCount &&
					srcProps.Type == TEX_TYPE_CUBE_MAP;

				if(!isValid)
				{
					if(!probeInfo.ErrorFlagged)
					{
						B3D_LOG(Error, Renderer, "Cubemap texture invalid to use as a reflection cubemap. "
												"Check texture size (must be {0}x{0}) and mip-map count",
							   IBLUtility::kReflectionCubemapSize);

						probeInfo.ErrorFlagged = true;
					}
				}
				else
				{
					for(u32 face = 0; face < 6; face++)
					{
						for(u32 mip = 0; mip <= srcProps.MipMapCount; mip++)
						{
							TextureCopyInformation copyDesc;
							copyDesc.SourceFace = face;
							copyDesc.SourceMip = mip;
							copyDesc.DestinationFace = probeInfo.ArrayIdx * 6 + face;
							copyDesc.DestinationMip = mip;

							commandBuffer.CopyTexture(texture, sceneInfo.ReflProbeCubemapsTex, copyDesc);
						}
					}
				}

				SetReflectionProbeArrayIndex(i, probeInfo.ArrayIdx, true);
			}

			// Note: Consider pruning the reflection cubemap array if empty slot count becomes too high
		}
	}
	B3DClearAllocatorFrame();
}

void RenderBeastScene::SetReflectionProbeArrayIndex(u32 probeIdx, u32 arrayIdx, bool markAsClean)
{
	RendererReflectionProbe* probe = &mInfo.ReflProbes[probeIdx];
	probe->ArrayIdx = arrayIdx;

	if(markAsClean)
		probe->ArrayDirty = false;
}

void RenderBeastScene::RegisterLightProbeVolume(LightProbeVolume* volume)
{
	mInfo.LightProbes.NotifyAdded(volume);
}

void RenderBeastScene::UpdateLightProbeVolume(LightProbeVolume* volume)
{
	mInfo.LightProbes.NotifyDirty(volume);
}

void RenderBeastScene::UnregisterLightProbeVolume(LightProbeVolume* volume)
{
	mInfo.LightProbes.NotifyRemoved(volume);
}

void RenderBeastScene::UpdateLightProbes(GpuCommandBuffer& commandBuffer)
{
	mInfo.LightProbes.UpdateProbes(commandBuffer);
}

void RenderBeastScene::RegisterSkybox(Skybox* skybox)
{
	mInfo.Skybox = skybox;
}

void RenderBeastScene::UnregisterSkybox(Skybox* skybox)
{
	if(mInfo.Skybox == skybox)
		mInfo.Skybox = nullptr;
}

void RenderBeastScene::RegisterParticleSystem(ParticleSystem* particleSystem)
{
	const auto rendererId = (u32)mInfo.ParticleSystems.size();
	particleSystem->SetRendererId(rendererId);

	mInfo.ParticleSystems.push_back(RendererParticles());
	mInfo.ParticleSystemCullInfos.push_back(CullInfo(Bounds(BsZero), particleSystem->GetLayer()));

	RendererParticles& rendererParticles = mInfo.ParticleSystems.back();
	rendererParticles.ParticleSystem = particleSystem;

	UpdateParticleSystem(particleSystem, false);

	rendererParticles.PrevWorldTransform = rendererParticles.WorldTransform;
	rendererParticles.PrevFrameDirtyState = PrevFrameDirtyState::Clean;
}

void RenderBeastScene::UpdateParticleSystem(ParticleSystem* particleSystem, bool tfrmOnly)
{
	const u32 rendererId = particleSystem->GetRendererId();
	RendererParticles& rendererParticles = mInfo.ParticleSystems[rendererId];

	rendererParticles.PrevWorldTransform = rendererParticles.WorldTransform;
	rendererParticles.PrevFrameDirtyState = PrevFrameDirtyState::Updated;

	rendererParticles.UpdatePerObjectData();

	if(tfrmOnly)
	{
		mUniformBufferPools.UpdatePerObjectBuffer(rendererParticles);
		return;
	}

	const ParticleSystemSettings& settings = particleSystem->GetSettings();

	// Initialize the variant of the particle system for GPU simulation, if needed
	if(settings.GpuSimulation)
	{
		if(!rendererParticles.GpuParticleSystem)
			rendererParticles.GpuParticleSystem = B3DPoolNew<GpuParticleSystem>(particleSystem);
	}
	else
	{
		if(rendererParticles.GpuParticleSystem)
		{
			B3DPoolDelete(rendererParticles.GpuParticleSystem);
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

	const ParticleOrientation orientation = settings.Orientation;
	const bool lockY = settings.OrientationLockY;
	const bool gpu = settings.GpuSimulation;
	const bool is3d = settings.RenderMode == ParticleRenderMode::Mesh;

	ShaderFlags shaderFlags = shader->GetFlags();
	const bool requiresForwardLighting = shaderFlags.IsSet(ShaderFlag::Forward);
	const bool supportsClusteredForward = GetRenderBeast()->GetFeatureSet() == RenderBeastFeatureSet::Desktop;

	ParticleForwardLightingType forwardLightingType;
	if(requiresForwardLighting)
	{
		forwardLightingType = supportsClusteredForward
			? ParticleForwardLightingType::Clustered
			: ParticleForwardLightingType::Standard;
	}
	else
		forwardLightingType = ParticleForwardLightingType::None;

	const ShaderVariationParameters* variation = &GetParticleShaderVariation(orientation, lockY, gpu, is3d, forwardLightingType);

	FindVariationInformation findVariationInformation;
	findVariationInformation.VariationParameters = variation;
	findVariationInformation.Override = true;

	u32 variationIndex = renElement.Material->FindVariation(findVariationInformation);

	if(variationIndex == (u32)-1)
		variationIndex = renElement.Material->GetDefaultVariation();

	renElement.DefaultVariationIndex = variationIndex;

	// Make sure the technique shaders are compiled
	const SPtr<Variation>& technique = renElement.Material->GetVariation(variationIndex);
	if(technique)
		technique->Compile();

	// Generate or assigned renderer specific data for the material
	renElement.ParameterAdapter = renElement.Material->CreateParameterAdapter(variationIndex);
	renElement.ParameterAdapter->Update(renElement.Material, 0.0f, true);

	SPtr<GpuParameterSet> gpuParameterSet = renElement.ParameterAdapter->GetGpuParameterSet();

	// Allocate from the uniform buffer manager after ParameterAdapter is created
	if(gpu)
	{
		// GPU particles use GpuParticlesPool (PerObject + GpuParticleParams)
		UniformBufferPools::AllocationResult result = mUniformBufferPools.Allocate(UniformBufferPools::GpuParticlesPool);
		rendererParticles.PerObjectBufferAllocationHandle = result.Handle;
		rendererParticles.PerObjectParameterSet = result.ParameterSet;
		rendererParticles.PerObjectSuballocation = result.GetSuballocation(UniformBufferPools::PerObjectBuffer);
		rendererParticles.GpuParticlesParamSuballocation = result.GetSuballocation(UniformBufferPools::GpuParticlesBuffer);

		renElement.GpuParticlesParamBufferOffset = rendererParticles.GpuParticlesParamSuballocation.GetSuballocationOffset();
		renElement.IsGpuSimulated = true;
	}
	else
	{
		// CPU particles use RenderablePool (PerObject only)
		UniformBufferPools::AllocationResult result = mUniformBufferPools.Allocate(UniformBufferPools::RenderablePool);
		rendererParticles.PerObjectBufferAllocationHandle = result.Handle;
		rendererParticles.PerObjectParameterSet = result.ParameterSet;
		rendererParticles.PerObjectSuballocation = result.GetSuballocation(UniformBufferPools::PerObjectBuffer);
	}

	// Store shared parameter set and buffer offset for render-time binding
	renElement.SharedPerObjectParameterSet = rendererParticles.PerObjectParameterSet;
	renElement.PerObjectBufferOffset = rendererParticles.PerObjectSuballocation.GetSuballocationOffset();

	// Now update the per-object buffer (allocation is ready)
	mUniformBufferPools.UpdatePerObjectBuffer(rendererParticles);

	if(gpu)
	{
		gpuParameterSet->GetSampledTextureParameter("gPositionTimeTex", renElement.ParamsGpu.PositionTimeTexture);
		gpuParameterSet->GetSampledTextureParameter("gSizeRotationTex", renElement.ParamsGpu.SizeRotationTexture);
		gpuParameterSet->GetSampledTextureParameter("gCurvesTex", renElement.ParamsGpu.CurvesTexture);

		renElement.Is3D = false;
	}
	else
	{
		switch(settings.RenderMode)
		{
		case ParticleRenderMode::Billboard:
			gpuParameterSet->GetSampledTextureParameter("gPositionAndRotTex", renElement.ParamsCpuBillboard.PositionAndRotTexture);
			gpuParameterSet->GetSampledTextureParameter("gColorTex", renElement.ParamsCpuBillboard.ColorTexture);
			gpuParameterSet->GetSampledTextureParameter("gSizeAndFrameIdxTex", renElement.ParamsCpuBillboard.SizeAndFrameIdxTexture);

			renElement.Is3D = false;
			break;
		case ParticleRenderMode::Mesh:
			gpuParameterSet->GetSampledTextureParameter("gPositionTex", renElement.ParamsCpuMesh.PositionTexture);
			gpuParameterSet->GetSampledTextureParameter("gColorTex", renElement.ParamsCpuMesh.ColorTexture);
			gpuParameterSet->GetSampledTextureParameter("gSizeTex", renElement.ParamsCpuMesh.SizeTexture);
			gpuParameterSet->GetSampledTextureParameter("gRotationTex", renElement.ParamsCpuMesh.RotationTexture);

			renElement.Is3D = true;
			renElement.Mesh = settings.Mesh;
			break;
		default:
			break;
		}
	}

	gpuParameterSet->GetStorageBufferParameter("gIndices", renElement.IndicesBuffer);
	gpuParameterSet->TryGetUniformBufferParameter("PerCamera", renElement.PerCameraUniformBufferParameter);
	gpuParameterSet->TryGetUniformBufferParameter("ParticleParams", renElement.ParticlesUniformBufferParameter);

	if(gpu)
	{
		// Get sprite image for animation curve generation
		SpriteImage* spriteImage = nullptr;
		if(shader->HasTextureParam("gTexture"))
			spriteImage = renElement.Material->GetSpriteImage("gTexture").get();

		if(!spriteImage && shader->HasTextureParam("gAlbedoTex"))
			spriteImage = renElement.Material->GetSpriteImage("gAlbedoTex").get();

		// Allocate curves
		GpuParticleCurves& curves = GpuParticleSimulation::Instance().GetResources().GetCurveTexture();
		curves.Free(rendererParticles.ColorCurveAlloc);
		curves.Free(rendererParticles.SizeScaleFrameIdxCurveAlloc);

		static constexpr u32 kNumCurveSamples = 128;
		Color samples[kNumCurveSamples];

		const ParticleGpuSimulationSettings& gpuSimSettings = particleSystem->GetGpuSimulationSettings();

		// Write color over lifetime curve
		LookupTable colorLookup = gpuSimSettings.ColorOverLifetime.ToLookupTable(kNumCurveSamples, true);

		for(u32 i = 0; i < kNumCurveSamples; i++)
		{
			const float* sample = colorLookup.GetSample(i);
			samples[i] = Color(sample[0], sample[1], sample[2], sample[3]);
		}

		rendererParticles.ColorCurveAlloc = curves.Alloc(samples, kNumCurveSamples);

		// Write size over lifetime / sprite animation curve
		LookupTable sizeLookup = gpuSimSettings.SizeScaleOverLifetime.ToLookupTable(kNumCurveSamples, true);

		float frameSamples[kNumCurveSamples];
		if(spriteImage && spriteImage->GetAnimationPlayback() != SpriteAnimationPlayback::None)
		{
			const SpriteSheetGridAnimation& anim = spriteImage->GetAnimation();
			for(u32 i = 0; i < kNumCurveSamples; i++)
			{
				const float t = i / (float)(kNumCurveSamples - 1);
				frameSamples[i] = t * (anim.FrameCount - 1);
			}
		}
		else
			memset(frameSamples, 0, sizeof(frameSamples));

		for(u32 i = 0; i < kNumCurveSamples; i++)
		{
			const float* sample = sizeLookup.GetSample(i);
			samples[i] = Color(sample[0], sample[1], frameSamples[i], 0.0f);
		}

		rendererParticles.SizeScaleFrameIdxCurveAlloc = curves.Alloc(samples, kNumCurveSamples);

		// Update the GPU particles param buffer via staging
		mUniformBufferPools.UpdateGpuParticlesParamBuffer(rendererParticles);
	}

	// Set up buffers for lighting
	const bool useForwardRendering = shaderFlags.IsSet(ShaderFlag::Forward);
	if(useForwardRendering)
	{
		renElement.ForwardLightingParams.Populate(gpuParameterSet, supportsClusteredForward);
		renElement.ImageBasedParams.Initialize(gpuParameterSet, GPT_FRAGMENT_PROGRAM, true, supportsClusteredForward, supportsClusteredForward);
	}

	const bool isTransparent = shaderFlags.IsSet(ShaderFlag::Transparent);
	if(isTransparent)
	{
		// Optional depth buffer input if requested
		if(gpuParameterSet->HasSampledTexture("gDepthBufferTex"))
			gpuParameterSet->GetSampledTextureParameter("gDepthBufferTex", renElement.DepthInputTexture);
	}
}

void RenderBeastScene::UnregisterParticleSystem(ParticleSystem* particleSystem)
{
	const u32 rendererId = particleSystem->GetRendererId();
	RendererParticles& rendererParticles = mInfo.ParticleSystems[rendererId];

	// Free curves
	GpuParticleCurves& curves = GpuParticleSimulation::Instance().GetResources().GetCurveTexture();
	curves.Free(rendererParticles.ColorCurveAlloc);
	curves.Free(rendererParticles.SizeScaleFrameIdxCurveAlloc);

	if(rendererParticles.GpuParticleSystem)
	{
		B3DPoolDelete(rendererParticles.GpuParticleSystem);
		rendererParticles.GpuParticleSystem = nullptr;
	}

	// Release the buffer allocation
	mUniformBufferPools.Release(rendererParticles.PerObjectBufferAllocationHandle);

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

void RenderBeastScene::RegisterDecal(Decal* decal)
{
	const auto renderableId = (u32)mInfo.Decals.size();
	decal->SetRendererId(renderableId);

	mInfo.Decals.emplace_back();
	mInfo.DecalCullInfos.push_back(CullInfo(decal->GetBounds(), decal->GetLayer()));

	RendererDecal& rendererDecal = mInfo.Decals.back();
	rendererDecal.Decal = decal;

	DecalRenderElement& renElement = rendererDecal.RenderElement;
	renElement.Type = (u32)RenderElementType::Decal;
	renElement.Mesh = RendererUtility::Instance().GetBoxStencil();
	renElement.SubMesh = renElement.Mesh->GetProperties().SubMeshes[0];

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
			FindVariationInformation findDesc;
			findDesc.VariationParameters = DECAL_VAR_LOOKUP[i][j];
			findDesc.Override = true;

			u32 techniqueIdx = renElement.Material->FindVariation(findDesc);
			if(techniqueIdx == (u32)-1)
				techniqueIdx = 0;

			const SPtr<Variation>& technique = renElement.Material->GetVariation(techniqueIdx);
			if(technique)
				technique->Compile();

			renElement.TechniqueIndices[i][j] = techniqueIdx;
		}
	}

	renElement.DefaultVariationIndex = renElement.TechniqueIndices[0][0];

	// Generate or assigned renderer specific data for the material
	// Note: This makes the assumption that all variations of the material share the same parameter set
	renElement.ParameterAdapter = renElement.Material->CreateParameterAdapter(renElement.DefaultVariationIndex);
	renElement.ParameterAdapter->Update(renElement.Material, 0.0f, true);

	// Generate or assign sampler state overrides
	renElement.SamplerOverrides = AllocSamplerStateOverrides(renElement);

	// Prepare all parameter bindings
	SPtr<GpuParameterSet> gpuParameterSet = renElement.ParameterAdapter->GetGpuParameterSet();

	// Allocate from the uniform buffer manager after ParameterAdapter is created
	{
		UniformBufferPools::AllocationResult result = mUniformBufferPools.Allocate(UniformBufferPools::DecalPool);
		rendererDecal.PerObjectBufferAllocationHandle = result.Handle;
		rendererDecal.PerObjectParameterSet = result.ParameterSet;
		rendererDecal.PerObjectSuballocation = result.GetSuballocation(UniformBufferPools::PerObjectBuffer);
		rendererDecal.DecalParamSuballocation = result.GetSuballocation(UniformBufferPools::DecalBuffer);
	}

	// Store shared parameter set and buffer offsets for render-time binding
	renElement.SharedPerObjectParameterSet = rendererDecal.PerObjectParameterSet;
	renElement.PerObjectBufferOffset = rendererDecal.PerObjectSuballocation.GetSuballocationOffset();
	renElement.DecalParamBufferOffset = rendererDecal.DecalParamSuballocation.GetSuballocationOffset();

	// Now update the uniform buffers (allocation is ready)
	rendererDecal.UpdatePerObjectData();
	mUniformBufferPools.UpdatePerObjectBuffer(rendererDecal);
	mUniformBufferPools.UpdateDecalParamBuffer(rendererDecal);

	gpuParameterSet->TryGetUniformBufferParameter("PerFrame", renElement.PerFrameUniformBufferParameter);
	gpuParameterSet->TryGetUniformBufferParameter("PerCamera", renElement.PerCameraUniformBufferParameter);
	gpuParameterSet->TryGetSampledTextureParameter("gDepthBufferTex", renElement.DepthInputTexture);
	gpuParameterSet->TryGetSampledTextureParameter("gMaskTex", renElement.MaskInputTexture);
}

void RenderBeastScene::UpdateDecal(Decal* decal)
{
	const u32 rendererId = decal->GetRendererId();
	RendererDecal& rendererDecal = mInfo.Decals[rendererId];

	rendererDecal.UpdatePerObjectData();
	mUniformBufferPools.UpdatePerObjectBuffer(rendererDecal);
	mUniformBufferPools.UpdateDecalParamBuffer(rendererDecal);

	mInfo.DecalCullInfos[rendererId].Bounds = decal->GetBounds();
}

void RenderBeastScene::UnregisterDecal(Decal* decal)
{
	const u32 rendererId = decal->GetRendererId();
	Decal* lastDecal = mInfo.Decals.back().Decal;
	const u32 lastDecalId = lastDecal->GetRendererId();

	RendererDecal& rendererDecal = mInfo.Decals[rendererId];
	DecalRenderElement& renElement = rendererDecal.RenderElement;

	// Unregister sampler overrides
	FreeSamplerStateOverrides(renElement);
	renElement.SamplerOverrides = nullptr;

	// Release the buffer allocation
	mUniformBufferPools.Release(rendererDecal.PerObjectBufferAllocationHandle);

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

void RenderBeastScene::Initialize()
{
	GetRenderBeast()->NotifySceneCreated(std::static_pointer_cast<RenderBeastScene>(GetShared()));

	mGpuDevice = GetRenderBeast()->GetGpuDevice();

	// Register all types
	for (const auto& config : GetRenderBeast()->GetPerObjectUniformTypeConfigurations())
		mUniformBufferPools.RegisterType(config);

	// Initialize after registration
	mUniformBufferPools.Initialize(*mGpuDevice);
	RendererScene::Initialize();
}

void RenderBeastScene::Destroy()
{
	for(auto& entry : mInfo.Renderables)
		B3DDelete(entry);

	for(auto& entry : mInfo.Views)
		B3DDelete(entry);

	B3D_ASSERT(mSamplerOverrides.empty());

	GetRenderBeast()->NotifySceneDestroyed(this);
	RendererScene::Destroy();
}

void RenderBeastScene::SetOptions(const SPtr<RenderBeastOptions>& options)
{
	mOptions = options;

	for(auto& entry : mInfo.Views)
		entry->SetStateReductionMode(mOptions->StateReductionMode);
}

RendererViewCreateInformation RenderBeastScene::CreateViewDesc(Camera* camera) const
{
	SPtr<Viewport> viewport = camera->GetViewport();
	ClearFlags clearFlags = viewport->GetClearFlags();
	RendererViewCreateInformation viewDesc;

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

	viewDesc.Target.NumSamples = camera->GetSampleCount();

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

	const Transform& tfrm = camera->GetWorldTransform();
	viewDesc.ViewOrigin = tfrm.GetPosition();
	viewDesc.ViewDirection = tfrm.GetForward();
	viewDesc.ProjTransform = camera->GetProjectionMatrix();
	viewDesc.ViewTransform = camera->GetViewMatrix();
	viewDesc.ProjType = camera->GetProjectionType();

	viewDesc.StateReduction = mOptions->StateReductionMode;
	viewDesc.SceneCamera = camera;

	return viewDesc;
}

void RenderBeastScene::UpdateCameraRenderTargets(Camera* camera, bool remove)
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

void RenderBeastScene::RefreshSamplerOverrides(bool force)
{
	bool anyDirty = false;
	for(auto& entry : mSamplerOverrides)
	{
		SPtr<MaterialParameters> materialParams = entry.first.Material->GetMaterialParameters();

		MaterialSamplerOverrides* materialOverrides = entry.second;
		for(u32 i = 0; i < materialOverrides->NumOverrides; i++)
		{
			SamplerOverride& override = materialOverrides->Overrides[i];
			const MaterialParametersBase::ParamData* materialParamData = materialParams->GetParamData(override.ParamIdx);

			SPtr<SamplerState> samplerState;
			materialParams->GetSamplerState(*materialParamData, samplerState);

			u64 hash = 0;
			if (samplerState != nullptr)
				hash = B3DHash(samplerState->GetInformation());

			if(hash != override.OriginalStateHash || force)
			{
				if(samplerState != nullptr)
					override.State = SamplerOverrideUtility::GenerateSamplerOverride(*mGpuDevice, samplerState, mOptions);
				else
					override.State = SamplerOverrideUtility::GenerateSamplerOverride(*mGpuDevice, mGpuDevice->FindOrCreateSamplerState(SamplerStateCreateInformation()), mOptions);

				override.OriginalStateHash = B3DHash(override.State->GetInformation());
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

	u32 renderableCount = (u32)mInfo.Renderables.size();
	for(u32 renderableIndex = 0; renderableIndex < renderableCount; renderableIndex++)
	{
		for(auto& renderableElement : mInfo.Renderables[renderableIndex]->Elements)
		{
			MaterialSamplerOverrides* overrides = renderableElement.SamplerOverrides;
			if(overrides != nullptr && overrides->IsDirty)
			{
				const u32 passCount = renderableElement.Material->GetPassCount(renderableElement.DefaultVariationIndex);
				for(u32 passIndex = 0; passIndex < passCount; passIndex++)
				{
					const u32 setCount = renderableElement.ParameterAdapter->GetSetCount(passIndex);
					for(u32 setIndex = 0; setIndex < setCount; setIndex++)
					{
						const SPtr<GpuParameterSet>& parameterSet = renderableElement.ParameterAdapter->GetGpuParameterSet(passIndex, setIndex);
						const SPtr<GpuPipelineParameterSetLayout>& uniformLayoutSet = parameterSet->GetLayout();

						const u32 samplerCount = uniformLayoutSet->GetBindingCount(GpuParameterType::Sampler);
						for(u32 samplerIndex = 0; samplerIndex < samplerCount; ++samplerIndex)
						{
							const u32 slot = uniformLayoutSet->GetSlot(GpuParameterType::Sampler, samplerIndex);

							const u32& overrideIndex = overrides->Passes[passIndex].StateOverrides[setIndex][slot];
							if(overrideIndex == ~0u)
								continue;

							parameterSet->SetSamplerState(slot, overrides->Overrides[overrideIndex].State);
						}
					}
				}
			}
		}
	}

	for(auto& entry : mSamplerOverrides)
		entry.second->IsDirty = false;
}

void RenderBeastScene::SetParamFrameParams(float time)
{
	// Allocate a new transient buffer for this frame
	mPerFrameSuballocation = gPerFrameParamDef.AllocateTransient();

	// Update SceneInfo pointer for compositor access
	mInfo.PerFrameSuballocation = &mPerFrameSuballocation;

	// Map and set the time parameter
	GpuBufferMappedScope mappedScope = mPerFrameSuballocation.Map();
	gPerFrameParamDef.gTime.Set(mappedScope, time);
}

void RenderBeastScene::PrepareRenderable(u32 idx, const FrameInfo& frameInfo)
{
	RendererRenderable* rendererRenderable = mInfo.Renderables[idx];

	for(auto& element : rendererRenderable->Elements)
		element.MaterialAnimationTime += frameInfo.Timings.TimeDelta;

	if(rendererRenderable->PrevFrameDirtyState != PrevFrameDirtyState::Clean)
	{
		if(rendererRenderable->PrevFrameDirtyState == PrevFrameDirtyState::Updated)
			rendererRenderable->PrevFrameDirtyState = PrevFrameDirtyState::CopyMostRecent;
		else if(rendererRenderable->PrevFrameDirtyState == PrevFrameDirtyState::CopyMostRecent)
		{
			rendererRenderable->PrevWorldTransform = mInfo.Renderables[idx]->WorldTransform;
			rendererRenderable->PrevFrameDirtyState = PrevFrameDirtyState::Clean;

			mUniformBufferPools.UpdatePerObjectBuffer(*rendererRenderable);
		}
	}
}

void RenderBeastScene::PrepareVisibleRenderable(u32 idx, const FrameInfo& frameInfo)
{
	if(mInfo.RenderableReady[idx])
		return;

	RendererRenderable* rendererRenderable = mInfo.Renderables[idx];

	// Note: Before uploading bone matrices perhaps check if they has actually been changed since last frame
	SPtr<GpuBuffer> boneMatrixBuffer;
	SPtr<GpuBuffer> previousBoneMatrixBuffer;
	bool isAnimated = false;
	if(frameInfo.PerSceneFrameData.Animation != nullptr)
	{
		isAnimated = rendererRenderable->Renderable->GetAnimationId() != (u64)-1;

		if(isAnimated)
		{
			rendererRenderable->Renderable->UpdateAnimationBuffers(*frameInfo.PerSceneFrameData.Animation);
			boneMatrixBuffer = rendererRenderable->Renderable->GetBoneMatrixBuffer();
			previousBoneMatrixBuffer = rendererRenderable->Renderable->GetPreviousBoneMatrixBuffer();
		}
	}

	// Note: Could this step be moved in notifyRenderableUpdated, so it only triggers when material actually gets
	// changed? Although it shouldn't matter much because if the internal versions keeping track of dirty params.
	for(auto& element : rendererRenderable->Elements)
	{
		element.ParameterAdapter->Update(element.Material, element.MaterialAnimationTime);

		// Note: If renderable is not writing to velocity, then these buffer don't have to be rebound every frame. Potential optimization for future.
		if(isAnimated)
		{
			element.BoneMatrixBufferParameter.Set(boneMatrixBuffer);
			element.PreviousBoneMatrixBufferParameter.Set(previousBoneMatrixBuffer);
		}
	}

	mInfo.RenderableReady[idx] = true;
}

void RenderBeastScene::PrepareParticleSystem(u32 idx, const FrameInfo& frameInfo)
{
	RendererParticles& rendererParticles = mInfo.ParticleSystems[idx];

	if(rendererParticles.PrevFrameDirtyState != PrevFrameDirtyState::Clean)
	{
		if(rendererParticles.PrevFrameDirtyState == PrevFrameDirtyState::Updated)
			rendererParticles.PrevFrameDirtyState = PrevFrameDirtyState::CopyMostRecent;
		else if(rendererParticles.PrevFrameDirtyState == PrevFrameDirtyState::CopyMostRecent)
		{
			rendererParticles.PrevWorldTransform = rendererParticles.WorldTransform;
			rendererParticles.PrevFrameDirtyState = PrevFrameDirtyState::Clean;

			mUniformBufferPools.UpdatePerObjectBuffer(rendererParticles);
		}
	}

	ParticlesRenderElement& renElement = mInfo.ParticleSystems[idx].RenderElement;
	renElement.ParameterAdapter->Update(renElement.Material, 0.0f);
}

void RenderBeastScene::PrepareDecal(u32 idx, const FrameInfo& frameInfo)
{
	DecalRenderElement& renElement = mInfo.Decals[idx].RenderElement;
	renElement.MaterialAnimationTime += frameInfo.Timings.TimeDelta;
	renElement.ParameterAdapter->Update(renElement.Material, renElement.MaterialAnimationTime);
}

void RenderBeastScene::UpdateParticleSystemBounds(const EvaluatedParticleData* particleRenderData)
{
	// Note: Avoid updating bounds for deterministic particle systems every frame. Also see if this can be copied
	// over in a faster way (or ideally just assigned)

	for(auto& entry : mInfo.ParticleSystems)
	{
		const u32 rendererId = entry.ParticleSystem->GetRendererId();

		AABox worldAABox = AABox::kInfinite;
		const auto iterFind = particleRenderData->CpuData.find(entry.ParticleSystem->GetId());
		if(iterFind != particleRenderData->CpuData.end())
			worldAABox = iterFind->second->Bounds;
		else if(entry.GpuParticleSystem)
			worldAABox = entry.GpuParticleSystem->GetBounds();

		const ParticleSystemSettings& settings = entry.ParticleSystem->GetSettings();
		if(settings.SimulationSpace == ParticleSimulationSpace::Local)
			worldAABox.TransformAffine(entry.WorldTransform);

		const Sphere worldSphere(worldAABox.GetCenter(), worldAABox.GetRadius());
		mInfo.ParticleSystemCullInfos[rendererId].Bounds = Bounds(worldAABox, worldSphere);
	}
}

MaterialSamplerOverrides* RenderBeastScene::AllocSamplerStateOverrides(RenderElement& elem)
{
	SamplerOverrideKey samplerKey(elem.Material, elem.DefaultVariationIndex);
	auto iterFind = mSamplerOverrides.find(samplerKey);
	if(iterFind != mSamplerOverrides.end())
	{
		iterFind->second->RefCount++;
		return iterFind->second;
	}
	else
	{
		SPtr<Shader> shader = elem.Material->GetShader();
		MaterialSamplerOverrides* samplerOverrides = SamplerOverrideUtility::GenerateSamplerOverrides(*mGpuDevice, shader, elem.Material->GetMaterialParameters(), elem.ParameterAdapter, mOptions);

		mSamplerOverrides[samplerKey] = samplerOverrides;

		samplerOverrides->RefCount++;
		return samplerOverrides;
	}
}

void RenderBeastScene::FreeSamplerStateOverrides(RenderElement& elem)
{
	SamplerOverrideKey samplerKey(elem.Material, elem.DefaultVariationIndex);

	auto iterFind = mSamplerOverrides.find(samplerKey);
	B3D_ASSERT(iterFind != mSamplerOverrides.end());

	MaterialSamplerOverrides* samplerOverrides = iterFind->second;
	samplerOverrides->RefCount--;
	if(samplerOverrides->RefCount == 0)
	{
		SamplerOverrideUtility::DestroySamplerOverrides(samplerOverrides);
		mSamplerOverrides.erase(iterFind);
	}
}
}}
