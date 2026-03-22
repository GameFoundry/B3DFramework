//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DRenderBeastScene.h"
#include "Components/B3DRenderable.h"
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
#include "RenderState/B3DDecalRenderState.h"
#include "Image/B3DSpriteTexture.h"
#include "RenderAPI/B3DVertexDescription.h"
#include "Shading/B3DGpuParticleSimulation.h"
#include "Components/B3DDecal.h"
#include "Renderer/B3DIBLUtility.h"
#include "Renderer/B3DRendererUtility.h"
#include "Renderer/B3DRendererObjectStorage.h"
#include "Utility/B3DBitwise.h"

namespace b3d { namespace render {

PerFrameUniformDefinition gPerFrameUniformDefinition;

static const ShaderVariationParameters* kDecalVariationParameterLookup[2][3] = {
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

/** Initializes a specific base pass variation on the provided material and returns the variation index. */
static u32 InitAndRetrieveBasePassVariation(Material& material, bool useForwardRendering, bool supportsClusteredForward, bool shaderCanWriteVelocity, bool writeVelocity, RenderableAnimType animType)
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

static void ValidateBasePassMaterial(Material& material, RenderableAnimType animType, u32 variationIndex, VertexDescription& vertexBufferDescription)
{
	// Validate mesh <-> shader vertex bindings
	u32 passCount = material.GetPassCount(variationIndex);
	for(u32 passIndex = 0; passIndex < passCount; passIndex++)
	{
		SPtr<Pass> pass = material.GetPass(passIndex, variationIndex);
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

				B3D_LOG(Warning, LogRenderer, "{0}", wrnStream.str());
				break;
			}
		}
	}
}

RenderableObjectStorage::RenderableObjectStorage() = default;

void RenderableObjectStorage::ApplyCommands(const CommandBatch& commands, FrameAllocator& allocator)
{
	RendererObjectStorage::ApplyCommandsHelper(
		commands,
		allocator,
		[this](PackedRendererId id) { mRenderableProxies[id].SetRendererId(id); },
		[this](TArrayView<const PackedRendererId> ids) { DestroyRenderState(ids); },
		mRenderableProxies, mRenderables, mRenderableCullInfos);
}

void RenderableObjectStorage::CreateRenderState(TArrayView<const PackedRendererId> ids)
{
	UniformBufferPools& uniformBufferPools = mRenderBeastScene->GetUniformBufferPools();

	for(const PackedRendererId packedId : ids)
	{
		RenderableProxy& proxy = mRenderableProxies[packedId];

		proxy.SetRendererId(packedId);

		B3D_ASSERT(renderableId < (PackedRendererId)mRenderables.size());
		B3D_ASSERT(mRenderables[renderableId] == nullptr);

		mRenderables[packedId] = B3DNew<RenderableRenderState>();
		mRenderableCullInfos[packedId] = CullInfo(proxy.GetBounds(), proxy.GetLayer(), proxy.GetCullDistanceFactor());

		RenderableRenderState* renderState = mRenderables[packedId];
		renderState->UpdatePerObjectData(proxy);
		renderState->PrevWorldTransform = renderState->WorldTransform;
		renderState->PrevFrameDirtyState = PrevFrameDirtyState::Clean;

		SPtr<Mesh> mesh = proxy.GetMesh();
		if(mesh != nullptr)
		{
			const MeshProperties& meshProps = mesh->GetProperties();
			SPtr<VertexDescription> vertexDescription = mesh->GetVertexData()->VertexDescription;

			for(u32 subMeshIndex = 0; subMeshIndex < (u32)meshProps.SubMeshes.size(); subMeshIndex++)
			{
				renderState->DrawCommands.push_back(RenderableDrawCommand());
				RenderableDrawCommand& drawCommand = renderState->DrawCommands.back();

				drawCommand.Type = (u32)DrawCommandType::Renderable;
				drawCommand.Mesh = mesh;
				drawCommand.SubMesh = meshProps.SubMeshes[subMeshIndex];
				drawCommand.AnimType = proxy.GetAnimType();
				drawCommand.AnimationId = proxy.GetAnimationId();
				drawCommand.MorphShapeVersion = 0;
				drawCommand.MorphShapeBuffer = proxy.GetMorphShapeBuffer();
				drawCommand.MorphVertexDefinition = proxy.GetMorphVertexDescription();

				drawCommand.Material = proxy.GetMaterial(subMeshIndex);
				if(drawCommand.Material == nullptr)
					drawCommand.Material = proxy.GetMaterial(0);

				if(drawCommand.Material != nullptr && drawCommand.Material->GetShader() == nullptr)
					drawCommand.Material = nullptr;

				// If no material use the default material
				if(drawCommand.Material == nullptr)
					drawCommand.Material = Material::Create(DefaultMaterial::Get()->GetShader());

				// Determine which variation to use
				static_assert((u32)RenderableAnimType::Count == 4, "RenderableAnimType is expected to have four sequential entries.");

				const SPtr<Shader>& shader = drawCommand.Material->GetShader();
				ShaderFlags shaderFlags = shader->GetFlags();
				const bool useForwardRendering = shaderFlags.IsSet(ShaderFlag::Forward) || shaderFlags.IsSet(ShaderFlag::Transparent);
				bool supportsClusteredForward = GetRenderBeast()->GetFeatureSet() == RenderBeastFeatureSet::Desktop;

				const Vector<ShaderVariationParameterInformation>& variationParams = shader->GetVariationParameters();
				const bool shaderCanWriteVelocity = std::find_if(variationParams.begin(), variationParams.end(), [](const ShaderVariationParameterInformation& x)
																 { return x.Identifier == "WRITE_VELOCITY"; }) != variationParams.end();

				const bool writeVelocity = shaderCanWriteVelocity && proxy.GetWriteVelocity();

				RenderableAnimType animType = proxy.GetAnimType();

				drawCommand.DefaultVariationIndex = InitAndRetrieveBasePassVariation(*drawCommand.Material, useForwardRendering, supportsClusteredForward, shaderCanWriteVelocity, false, animType);

#if B3D_DEBUG
				ValidateBasePassMaterial(*drawCommand.Material, animType, drawCommand.DefaultVariationIndex, *vertexDescription);
#endif

				// Generate or assigned renderer specific data for the material
				drawCommand.ParameterAdapter = drawCommand.Material->CreateParameterAdapter(drawCommand.DefaultVariationIndex);
				drawCommand.ParameterAdapter->Update(drawCommand.Material, 0.0f, true);

				if(writeVelocity)
				{
					drawCommand.WriteVelocityVariationIndex = InitAndRetrieveBasePassVariation(*drawCommand.Material, useForwardRendering, supportsClusteredForward, shaderCanWriteVelocity, true, animType);

#if B3D_DEBUG
					ValidateBasePassMaterial(*drawCommand.Material, animType, drawCommand.WriteVelocityVariationIndex, *vertexDescription);
#endif

					// Note: Using the same params as the non-velocity variation. There are assumed to be no differences
				}
				else
					drawCommand.WriteVelocityVariationIndex = (u32)-1;

				// Generate or assign sampler state overrides
				drawCommand.SamplerOverrides = mRenderBeastScene->AllocSamplerStateOverrides(drawCommand);
			}
		}

		// Allocate from the uniform buffer manager
		if(!renderState->DrawCommands.empty())
		{
			auto result = uniformBufferPools.Allocate();
			renderState->PerObjectBufferAllocationHandle = result.Handle;
			renderState->PerObjectParameterSet = result.ParameterSet;
			renderState->PerObjectSuballocation = result.GetSuballocation(UniformBufferPools::PerObjectBuffer);
		}

		uniformBufferPools.UpdatePerObjectBuffer(*renderState);

		// Prepare all parameter bindings
		for(auto& drawCommand : renderState->DrawCommands)
		{
			SPtr<Shader> shader = drawCommand.Material->GetShader();
			if(shader == nullptr)
			{
				B3D_LOG(Warning, LogRenderer, "Missing shader on material.");
				continue;
			}

			// Store shared parameter set and buffer offset for render-time binding
			drawCommand.SharedPerObjectParameterSet = renderState->PerObjectParameterSet;
			drawCommand.PerObjectBufferOffset = renderState->PerObjectSuballocation.GetSuballocationOffset();

			SPtr<GpuParameterSet> gpuParameterSet = drawCommand.ParameterAdapter->GetGpuParameterSet();

			// Note: Perhaps perform buffer validation to ensure expected buffer has the same size and layout as the
			// provided buffer, and show a warning otherwise. But this is perhaps better handled on a higher level.
			gpuParameterSet->TryGetUniformBufferParameter("PerFrame", drawCommand.PerFrameUniformBufferParameter);
			gpuParameterSet->TryGetUniformBufferParameter("PerCamera", drawCommand.PerCameraUniformBufferParameter);
			gpuParameterSet->TryGetStorageBufferParameter("boneMatrices", drawCommand.BoneMatrixBufferParameter);
			gpuParameterSet->TryGetStorageBufferParameter("prevBoneMatrices", drawCommand.PreviousBoneMatrixBufferParameter);

			ShaderFlags shaderFlags = shader->GetFlags();
			const bool useForwardRendering = shaderFlags.IsSet(ShaderFlag::Forward) || shaderFlags.IsSet(ShaderFlag::Transparent);

			if(useForwardRendering)
			{
				const bool supportsClusteredForward = GetRenderBeast()->GetFeatureSet() == RenderBeastFeatureSet::Desktop;

				drawCommand.ForwardLightingParams.Populate(gpuParameterSet, supportsClusteredForward);
				drawCommand.ImageBasedParams.Initialize(gpuParameterSet, GPT_FRAGMENT_PROGRAM, true, supportsClusteredForward, supportsClusteredForward);
			}
		}
	}
}

void RenderableObjectStorage::DestroyRenderState(TArrayView<const PackedRendererId> ids)
{
	UniformBufferPools& uniformBufferPools = mRenderBeastScene->GetUniformBufferPools();

	for(const PackedRendererId packedId : ids)
	{
		RenderableRenderState* renderState = mRenderables[packedId];
		if(renderState == nullptr)
			continue;

		for(auto& element : renderState->DrawCommands)
		{
			mRenderBeastScene->FreeSamplerStateOverrides(element);
			element.SamplerOverrides = nullptr;
		}

		uniformBufferPools.Release(renderState->PerObjectBufferAllocationHandle);
		B3DDelete(renderState);

		mRenderables[packedId] = nullptr;
		mRenderableProxies[packedId].SetRendererId(kInvalidPackedRendererId);
	}
}

void RenderableObjectStorage::UpdateRenderState(TArrayView<const PackedRendererId> ids)
{
	UniformBufferPools& uniformBufferPools = mRenderBeastScene->GetUniformBufferPools();

	for(const PackedRendererId packedId : ids)
	{
		RenderableProxy& proxy = mRenderableProxies[packedId];
		RenderableRenderState* renderState = mRenderables[packedId];

		if(renderState->PrevFrameDirtyState != PrevFrameDirtyState::Updated)
			renderState->PrevWorldTransform = renderState->WorldTransform;

		renderState->UpdatePerObjectData(proxy);
		renderState->PrevFrameDirtyState = PrevFrameDirtyState::Updated;

		uniformBufferPools.UpdatePerObjectBuffer(*renderState);

		mRenderableCullInfos[packedId].Bounds = proxy.GetBounds();
		mRenderableCullInfos[packedId].CullDistanceFactor = proxy.GetCullDistanceFactor();
	}
}

void RenderableObjectStorage::PrepareRenderable(PackedRendererId id, const FrameInfo& frameInfo)
{
	RenderableRenderState* renderState = mRenderables[id];

	for(auto& drawCommand : renderState->DrawCommands)
		drawCommand.MaterialAnimationTime += frameInfo.Timings.TimeDelta;

	if(renderState->PrevFrameDirtyState != PrevFrameDirtyState::Clean)
	{
		if(renderState->PrevFrameDirtyState == PrevFrameDirtyState::Updated)
			renderState->PrevFrameDirtyState = PrevFrameDirtyState::CopyMostRecent;
		else if(renderState->PrevFrameDirtyState == PrevFrameDirtyState::CopyMostRecent)
		{
			renderState->PrevWorldTransform = mRenderables[id]->WorldTransform;
			renderState->PrevFrameDirtyState = PrevFrameDirtyState::Clean;

			mRenderBeastScene->GetUniformBufferPools().UpdatePerObjectBuffer(*renderState);
		}
	}
}

void RenderableObjectStorage::PrepareVisibleRenderable(PackedRendererId id, const FrameInfo& frameInfo)
{
	SceneInfo& sceneInfo = mRenderBeastScene->GetSceneInfo();
	if(sceneInfo.RenderableReady[id])
		return;

	RenderableRenderState* renderState = mRenderables[id];
	RenderableProxy& proxy = GetRenderableProxy(id);

	// Note: Before uploading bone matrices perhaps check if they has actually been changed since last frame
	SPtr<GpuBuffer> boneMatrixBuffer;
	SPtr<GpuBuffer> previousBoneMatrixBuffer;
	bool isAnimated = false;
	if(frameInfo.PerSceneFrameData.Animation != nullptr)
	{
		isAnimated = proxy.GetAnimationId() != (u64)-1;

		if(isAnimated)
		{
			proxy.UpdateAnimationBuffers(*frameInfo.PerSceneFrameData.Animation);
			boneMatrixBuffer = proxy.GetBoneMatrixBuffer();
			previousBoneMatrixBuffer = proxy.GetPreviousBoneMatrixBuffer();
		}
	}

	// Note: Could this step be moved in notifyRenderableUpdated, so it only triggers when material actually gets
	// changed? Although it shouldn't matter much because if the internal versions keeping track of dirty params.
	for(auto& element : renderState->DrawCommands)
	{
		element.ParameterAdapter->Update(element.Material, element.MaterialAnimationTime);

		// Note: If renderable is not writing to velocity, then these buffer don't have to be rebound every frame. Potential optimization for future.
		if(isAnimated)
		{
			element.BoneMatrixBufferParameter.Set(boneMatrixBuffer);
			element.PreviousBoneMatrixBufferParameter.Set(previousBoneMatrixBuffer);
		}
	}

	sceneInfo.RenderableReady[id] = true;
}

// ---- LightObjectStorage ----

LightObjectStorage::LightObjectStorage() = default;

void LightObjectStorage::ApplyCommands(const CommandBatch& commands, FrameAllocator& allocator)
{
	RendererObjectStorage::ApplyCommandsHelper(
		commands,
		allocator,
		[this](PackedRendererId id)
		{
			mLightProxies[id].SetRendererId(id);

			const TypeArrayMapping& mapping = mPackedIndexToTypeArrayIndex[id];
			if(mapping.Type == LightType::Directional)
				mDirectionalLightPackedIds[mapping.TypeArrayIndex] = id;
			else if(mapping.Type == LightType::Radial)
				mRadialLightPackedIds[mapping.TypeArrayIndex] = id;
			else
				mSpotLightPackedIds[mapping.TypeArrayIndex] = id;
		},
		[this](TArrayView<const PackedRendererId> ids) { DestroyRenderState(ids); },
		mLightProxies, mPackedIndexToTypeArrayIndex);
}

void LightObjectStorage::CreateRenderState(TArrayView<const PackedRendererId> ids)
{
	for(const PackedRendererId packedId : ids)
	{
		const LightProxy& proxy = mLightProxies[packedId];
		const LightType type = proxy.GetType();

		LightRenderState renderState;

		TypeArrayMapping perTypeArrayMapping;
		perTypeArrayMapping.Type = type;

		if(type == LightType::Directional)
		{
			perTypeArrayMapping.TypeArrayIndex = (u32)mDirectionalLights.size();
			mDirectionalLights.push_back(renderState);
			mDirectionalLightPackedIds.push_back(packedId);
		}
		else if(type == LightType::Radial)
		{
			perTypeArrayMapping.TypeArrayIndex = (u32)mRadialLights.size();
			mRadialLights.push_back(renderState);
			mRadialLightPackedIds.push_back(packedId);
			mRadialLightWorldBounds.push_back(proxy.GetBounds());
		}
		else // Spot
		{
			perTypeArrayMapping.TypeArrayIndex = (u32)mSpotLights.size();
			mSpotLights.push_back(renderState);
			mSpotLightPackedIds.push_back(packedId);
			mSpotLightWorldBounds.push_back(proxy.GetBounds());
		}

		mPackedIndexToTypeArrayIndex[packedId] = perTypeArrayMapping;
	}
}

void LightObjectStorage::DestroyRenderState(TArrayView<const PackedRendererId> ids)
{
	for(const PackedRendererId packedId : ids)
	{
		const TypeArrayMapping& info = mPackedIndexToTypeArrayIndex[packedId];
		const u32 indexInTypeArray = info.TypeArrayIndex;

		if(info.Type == LightType::Directional)
		{
			const u32 lastIndex = (u32)mDirectionalLights.size() - 1;
			if(indexInTypeArray != lastIndex)
			{
				std::swap(mDirectionalLights[indexInTypeArray], mDirectionalLights[lastIndex]);
				std::swap(mDirectionalLightPackedIds[indexInTypeArray], mDirectionalLightPackedIds[lastIndex]);

				PackedRendererId swappedPackedId = mDirectionalLightPackedIds[indexInTypeArray];
				mPackedIndexToTypeArrayIndex[swappedPackedId].TypeArrayIndex = indexInTypeArray;
			}
			mDirectionalLights.pop_back();
			mDirectionalLightPackedIds.pop_back();
		}
		else if(info.Type == LightType::Radial)
		{
			const u32 lastIndex = (u32)mRadialLights.size() - 1;
			if(indexInTypeArray != lastIndex)
			{
				std::swap(mRadialLights[indexInTypeArray], mRadialLights[lastIndex]);
				std::swap(mRadialLightWorldBounds[indexInTypeArray], mRadialLightWorldBounds[lastIndex]);
				std::swap(mRadialLightPackedIds[indexInTypeArray], mRadialLightPackedIds[lastIndex]);

				PackedRendererId swappedPackedId = mRadialLightPackedIds[indexInTypeArray];
				mPackedIndexToTypeArrayIndex[swappedPackedId].TypeArrayIndex = indexInTypeArray;
			}
			mRadialLights.pop_back();
			mRadialLightWorldBounds.pop_back();
			mRadialLightPackedIds.pop_back();
		}
		else // Spot
		{
			const u32 lastIndex = (u32)mSpotLights.size() - 1;
			if(indexInTypeArray != lastIndex)
			{
				std::swap(mSpotLights[indexInTypeArray], mSpotLights[lastIndex]);
				std::swap(mSpotLightWorldBounds[indexInTypeArray], mSpotLightWorldBounds[lastIndex]);
				std::swap(mSpotLightPackedIds[indexInTypeArray], mSpotLightPackedIds[lastIndex]);

				PackedRendererId swappedPackedId = mSpotLightPackedIds[indexInTypeArray];
				mPackedIndexToTypeArrayIndex[swappedPackedId].TypeArrayIndex = indexInTypeArray;
			}
			mSpotLights.pop_back();
			mSpotLightWorldBounds.pop_back();
			mSpotLightPackedIds.pop_back();
		}
	}
}

void LightObjectStorage::UpdateRenderState(TArrayView<const PackedRendererId> ids)
{
	for(const PackedRendererId packedId : ids)
	{
		const LightProxy& proxy = mLightProxies[packedId];
		const TypeArrayMapping& info = mPackedIndexToTypeArrayIndex[packedId];

		if(info.Type == LightType::Radial)
			mRadialLightWorldBounds[info.TypeArrayIndex] = proxy.GetBounds();
		else if(info.Type == LightType::Spot)
			mSpotLightWorldBounds[info.TypeArrayIndex] = proxy.GetBounds();
	}
}

// ---- RenderBeastScene ----

RenderBeastScene::RenderBeastScene(const SPtr<RenderBeastOptions>& options)
	: mOptions(options)
{
	mRenderableStorage = B3DMakeShared<RenderableObjectStorage>();
	mLightStorage = B3DMakeShared<LightObjectStorage>();
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
	LightObjectStorage& lightStorage = GetLightStorage();

	if(light->GetType() == LightType::Directional)
	{
		u32 lightId = (u32)lightStorage.GetDirectionalLights().size();
		light->SetRendererId(lightId);

		lightStorage.GetDirectionalLights().push_back(LightRenderState(light));
	}
	else if(light->GetType() == LightType::Radial)
	{
		u32 lightId = (u32)lightStorage.GetRadialLights().size();
		light->SetRendererId(lightId);

		lightStorage.GetRadialLights().push_back(LightRenderState(light));
		lightStorage.GetRadialLightWorldBounds().push_back(light->GetBounds());
	}
	else // Spot
	{
		u32 lightId = (u32)lightStorage.GetSpotLights().size();
		light->SetRendererId(lightId);

		lightStorage.GetSpotLights().push_back(LightRenderState(light));
		lightStorage.GetSpotLightWorldBounds().push_back(light->GetBounds());
	}
}

void RenderBeastScene::UpdateLight(Light* light)
{
	LightObjectStorage& lightStorage = GetLightStorage();
	u32 lightId = light->GetRendererId();

	if(light->GetType() == LightType::Radial)
		lightStorage.GetRadialLightWorldBounds()[lightId] = light->GetBounds();
	else if(light->GetType() == LightType::Spot)
		lightStorage.GetSpotLightWorldBounds()[lightId] = light->GetBounds();
}

void RenderBeastScene::UnregisterLight(Light* light)
{
	LightObjectStorage& lightStorage = GetLightStorage();
	u32 lightId = light->GetRendererId();

	if(light->GetType() == LightType::Directional)
	{
		auto& dirLights = lightStorage.GetDirectionalLights();
		Light* lastLight = dirLights.back().Light;
		u32 lastLightId = lastLight->GetRendererId();

		if(lightId != lastLightId)
		{
			std::swap(dirLights[lightId], dirLights[lastLightId]);
			lastLight->SetRendererId(lightId);
		}

		dirLights.pop_back();
	}
	else if(light->GetType() == LightType::Radial)
	{
		auto& radialLights = lightStorage.GetRadialLights();
		auto& radialBounds = lightStorage.GetRadialLightWorldBounds();
		Light* lastLight = radialLights.back().Light;
		u32 lastLightId = lastLight->GetRendererId();

		if(lightId != lastLightId)
		{
			std::swap(radialLights[lightId], radialLights[lastLightId]);
			std::swap(radialBounds[lightId], radialBounds[lastLightId]);
			lastLight->SetRendererId(lightId);
		}

		radialLights.pop_back();
		radialBounds.pop_back();
	}
	else // Spot
	{
		auto& spotLights = lightStorage.GetSpotLights();
		auto& spotBounds = lightStorage.GetSpotLightWorldBounds();
		Light* lastLight = spotLights.back().Light;
		u32 lastLightId = lastLight->GetRendererId();

		if(lightId != lastLightId)
		{
			std::swap(spotLights[lightId], spotLights[lastLightId]);
			std::swap(spotBounds[lightId], spotBounds[lastLightId]);
			lastLight->SetRendererId(lightId);
		}

		spotLights.pop_back();
		spotBounds.pop_back();
	}
}

void RenderBeastScene::RegisterReflectionProbe(ReflectionProbe* probe)
{
	u32 probeId = (u32)mInfo.ReflProbes.size();
	probe->SetRendererId(probeId);

	mInfo.ReflProbes.push_back(ReflectionProbeRenderState(probe));
	ReflectionProbeRenderState& probeInfo = mInfo.ReflProbes.back();

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
		B3D_LOG(Error, LogRenderer, "Reached the maximum number of allowed reflection probe cubemaps at once. "
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
		ReflectionProbeRenderState& probeInfo = mInfo.ReflProbes[probeId];
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
			const ReflectionProbeRenderState& probeInfo = sceneInfo.ReflProbes[i];

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
						B3D_LOG(Error, LogRenderer, "Cubemap texture invalid to use as a reflection cubemap. "
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
	ReflectionProbeRenderState* probe = &mInfo.ReflProbes[probeIdx];
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

	mInfo.ParticleSystems.push_back(ParticleRenderState());
	mInfo.ParticleSystemCullInfos.push_back(CullInfo(Bounds(kZeroTag), particleSystem->GetLayer()));

	ParticleRenderState& renderState = mInfo.ParticleSystems.back();
	renderState.ParticleSystem = particleSystem;

	UpdateParticleSystem(particleSystem, false);

	renderState.PrevWorldTransform = renderState.WorldTransform;
	renderState.PrevFrameDirtyState = PrevFrameDirtyState::Clean;
}

void RenderBeastScene::UpdateParticleSystem(ParticleSystem* particleSystem, bool tfrmOnly)
{
	const u32 rendererId = particleSystem->GetRendererId();
	ParticleRenderState& renderState = mInfo.ParticleSystems[rendererId];

	renderState.PrevWorldTransform = renderState.WorldTransform;
	renderState.PrevFrameDirtyState = PrevFrameDirtyState::Updated;

	renderState.UpdatePerObjectData();

	if(tfrmOnly)
	{
		mUniformBufferPools.UpdatePerObjectBuffer(renderState);
		return;
	}

	const ParticleSystemSettings& settings = particleSystem->GetSettings();

	// Initialize the variant of the particle system for GPU simulation, if needed
	if(settings.GpuSimulation)
	{
		if(!renderState.GpuParticleSystem)
			renderState.GpuParticleSystem = B3DPoolNew<GpuParticleSystem>(particleSystem);
	}
	else
	{
		if(renderState.GpuParticleSystem)
		{
			B3DPoolDelete(renderState.GpuParticleSystem);
			renderState.GpuParticleSystem = nullptr;
		}
	}

	ParticlesDrawCommand& drawCommand = renderState.DrawCommand;
	drawCommand.Type = (u32)DrawCommandType::Particle;

	drawCommand.Material = settings.Material;

	if(drawCommand.Material != nullptr && drawCommand.Material->GetShader() == nullptr)
		drawCommand.Material = nullptr;

	// If no material use the default material
	if(drawCommand.Material == nullptr)
		drawCommand.Material = Material::Create(DefaultParticleMaterial::Get()->GetShader());

	const SPtr<Shader> shader = drawCommand.Material->GetShader();

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

	const ShaderVariationParameters* variationParameters = &GetParticleShaderVariationParameters(orientation, lockY, gpu, is3d, forwardLightingType);

	FindVariationInformation findVariationInformation;
	findVariationInformation.VariationParameters = variationParameters;
	findVariationInformation.Override = true;

	u32 variationIndex = drawCommand.Material->FindVariation(findVariationInformation);

	if(variationIndex == (u32)-1)
		variationIndex = drawCommand.Material->GetDefaultVariation();

	drawCommand.DefaultVariationIndex = variationIndex;

	// Make sure the variation is compiled
	const SPtr<Variation>& variation = drawCommand.Material->GetVariation(variationIndex);
	if(variation)
		variation->Compile();

	// Generate or assigned renderer specific data for the material
	drawCommand.ParameterAdapter = drawCommand.Material->CreateParameterAdapter(variationIndex);
	drawCommand.ParameterAdapter->Update(drawCommand.Material, 0.0f, true);

	SPtr<GpuParameterSet> gpuParameterSet = drawCommand.ParameterAdapter->GetGpuParameterSet();

	// Allocate from the uniform buffer manager after ParameterAdapter is created
	// Only allocate if no allocation exists or if the particle type changed (GPU to CPU or vice-versa)
	const bool typeChanged = renderState.PerObjectBufferAllocationHandle.IsValid() && (gpu != drawCommand.IsGpuSimulated);

	// If the type changed, release the old allocation first
	if(typeChanged)
		mUniformBufferPools.Release(renderState.PerObjectBufferAllocationHandle);

	// Allocate if no allocation exists or if type changed
	if(!renderState.PerObjectBufferAllocationHandle.IsValid() || typeChanged)
	{
		if(gpu)
		{
			// GPU particles use GpuParticlesPool (PerObject + GpuParticleParams)
			UniformBufferPools::AllocationResult result = mUniformBufferPools.Allocate(UniformBufferPools::GpuParticlesPool);
			renderState.PerObjectBufferAllocationHandle = result.Handle;
			renderState.PerObjectParameterSet = result.ParameterSet;
			renderState.PerObjectSuballocation = result.GetSuballocation(UniformBufferPools::PerObjectBuffer);
			renderState.GpuParticlesParamSuballocation = result.GetSuballocation(UniformBufferPools::GpuParticlesBuffer);
		}
		else
		{
			// CPU particles use RenderablePool (PerObject only)
			UniformBufferPools::AllocationResult result = mUniformBufferPools.Allocate(UniformBufferPools::RenderablePool);
			renderState.PerObjectBufferAllocationHandle = result.Handle;
			renderState.PerObjectParameterSet = result.ParameterSet;
			renderState.PerObjectSuballocation = result.GetSuballocation(UniformBufferPools::PerObjectBuffer);
		}
	}

	// Update GPU-specific fields based on current state
	if(gpu)
	{
		drawCommand.GpuParticlesParamBufferOffset = renderState.GpuParticlesParamSuballocation.GetSuballocationOffset();
		drawCommand.IsGpuSimulated = true;
	}
	else
		drawCommand.IsGpuSimulated = false;

	// Store shared parameter set and buffer offset for render-time binding
	drawCommand.SharedPerObjectParameterSet = renderState.PerObjectParameterSet;
	drawCommand.PerObjectBufferOffset = renderState.PerObjectSuballocation.GetSuballocationOffset();

	// Now update the per-object buffer (allocation is ready)
	mUniformBufferPools.UpdatePerObjectBuffer(renderState);

	if(gpu)
	{
		gpuParameterSet->GetSampledTextureParameter("gPositionTimeTex", drawCommand.ParamsGpu.PositionTimeTexture);
		gpuParameterSet->GetSampledTextureParameter("gSizeRotationTex", drawCommand.ParamsGpu.SizeRotationTexture);
		gpuParameterSet->GetSampledTextureParameter("gCurvesTex", drawCommand.ParamsGpu.CurvesTexture);

		drawCommand.Is3D = false;
	}
	else
	{
		switch(settings.RenderMode)
		{
		case ParticleRenderMode::Billboard:
			gpuParameterSet->GetSampledTextureParameter("gPositionAndRotTex", drawCommand.ParamsCpuBillboard.PositionAndRotTexture);
			gpuParameterSet->GetSampledTextureParameter("gColorTex", drawCommand.ParamsCpuBillboard.ColorTexture);
			gpuParameterSet->GetSampledTextureParameter("gSizeAndFrameIdxTex", drawCommand.ParamsCpuBillboard.SizeAndFrameIdxTexture);

			drawCommand.Is3D = false;
			break;
		case ParticleRenderMode::Mesh:
			gpuParameterSet->GetSampledTextureParameter("gPositionTex", drawCommand.ParamsCpuMesh.PositionTexture);
			gpuParameterSet->GetSampledTextureParameter("gColorTex", drawCommand.ParamsCpuMesh.ColorTexture);
			gpuParameterSet->GetSampledTextureParameter("gSizeTex", drawCommand.ParamsCpuMesh.SizeTexture);
			gpuParameterSet->GetSampledTextureParameter("gRotationTex", drawCommand.ParamsCpuMesh.RotationTexture);

			drawCommand.Is3D = true;
			drawCommand.Mesh = settings.Mesh;
			break;
		default:
			break;
		}
	}

	gpuParameterSet->GetStorageBufferParameter("gIndices", drawCommand.IndicesBuffer);
	gpuParameterSet->TryGetUniformBufferParameter("PerCamera", drawCommand.PerCameraUniformBufferParameter);
	gpuParameterSet->TryGetUniformBufferParameter("ParticleParams", drawCommand.ParticlesUniformBufferParameter);

	if(gpu)
	{
		// Get sprite image for animation curve generation
		SpriteImage* spriteImage = nullptr;
		if(shader->HasTextureParameter("gTexture"))
			spriteImage = drawCommand.Material->GetSpriteImage("gTexture").get();

		if(!spriteImage && shader->HasTextureParameter("gAlbedoTex"))
			spriteImage = drawCommand.Material->GetSpriteImage("gAlbedoTex").get();

		// Allocate curves
		GpuParticleCurves& curves = GpuParticleSimulation::Instance().GetResources().GetCurveTexture();
		curves.Free(renderState.ColorCurveAlloc);
		curves.Free(renderState.SizeScaleFrameIdxCurveAlloc);

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

		renderState.ColorCurveAlloc = curves.Alloc(samples, kNumCurveSamples);

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

		renderState.SizeScaleFrameIdxCurveAlloc = curves.Alloc(samples, kNumCurveSamples);

		// Update the GPU particles param buffer via staging
		mUniformBufferPools.UpdateGpuParticlesParamBuffer(renderState);
	}

	// Set up buffers for lighting
	const bool useForwardRendering = shaderFlags.IsSet(ShaderFlag::Forward);
	if(useForwardRendering)
	{
		drawCommand.ForwardLightingParams.Populate(gpuParameterSet, supportsClusteredForward);
		drawCommand.ImageBasedParams.Initialize(gpuParameterSet, GPT_FRAGMENT_PROGRAM, true, supportsClusteredForward, supportsClusteredForward);
	}

	const bool isTransparent = shaderFlags.IsSet(ShaderFlag::Transparent);
	if(isTransparent)
	{
		// Optional depth buffer input if requested
		if(gpuParameterSet->HasSampledTexture("gDepthBufferTex"))
			gpuParameterSet->GetSampledTextureParameter("gDepthBufferTex", drawCommand.DepthInputTexture);
	}
}

void RenderBeastScene::UnregisterParticleSystem(ParticleSystem* particleSystem)
{
	const u32 rendererId = particleSystem->GetRendererId();
	ParticleRenderState& renderState = mInfo.ParticleSystems[rendererId];

	// Free curves
	GpuParticleCurves& curves = GpuParticleSimulation::Instance().GetResources().GetCurveTexture();
	curves.Free(renderState.ColorCurveAlloc);
	curves.Free(renderState.SizeScaleFrameIdxCurveAlloc);

	if(renderState.GpuParticleSystem)
	{
		B3DPoolDelete(renderState.GpuParticleSystem);
		renderState.GpuParticleSystem = nullptr;
	}

	// Release the buffer allocation
	mUniformBufferPools.Release(renderState.PerObjectBufferAllocationHandle);

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

	DecalRenderState& renderState = mInfo.Decals.back();
	renderState.Decal = decal;

	DecalDrawCommand& drawCommand = renderState.DrawCommand;
	drawCommand.Type = (u32)DrawCommandType::Decal;
	drawCommand.Mesh = RendererUtility::Instance().GetBoxStencil();
	drawCommand.SubMesh = drawCommand.Mesh->GetProperties().SubMeshes[0];

	drawCommand.Material = decal->GetMaterial();

	if(drawCommand.Material != nullptr && drawCommand.Material->GetShader() == nullptr)
		drawCommand.Material = nullptr;

	// If no material use the default material
	if(drawCommand.Material == nullptr)
		drawCommand.Material = Material::Create(DefaultDecalMaterial::Get()->GetShader());

	for(u32 i = 0; i < 2; i++)
	{
		for(u32 j = 0; j < 3; j++)
		{
			FindVariationInformation findVariationINformation;
			findVariationINformation.VariationParameters = kDecalVariationParameterLookup[i][j];
			findVariationINformation.Override = true;

			u32 variationIndex = drawCommand.Material->FindVariation(findVariationINformation);
			if(variationIndex == ~0u)
				variationIndex = 0;

			const SPtr<Variation>& variation = drawCommand.Material->GetVariation(variationIndex);
			if(variation)
				variation->Compile();

			drawCommand.VariationIndices[i][j] = variationIndex;
		}
	}

	drawCommand.DefaultVariationIndex = drawCommand.VariationIndices[0][0];

	// Generate or assigned renderer specific data for the material
	// Note: This makes the assumption that all variations of the material share the same parameter set
	drawCommand.ParameterAdapter = drawCommand.Material->CreateParameterAdapter(drawCommand.DefaultVariationIndex);
	drawCommand.ParameterAdapter->Update(drawCommand.Material, 0.0f, true);

	// Generate or assign sampler state overrides
	drawCommand.SamplerOverrides = AllocSamplerStateOverrides(drawCommand);

	// Prepare all parameter bindings
	SPtr<GpuParameterSet> gpuParameterSet = drawCommand.ParameterAdapter->GetGpuParameterSet();

	// Allocate from the uniform buffer manager after ParameterAdapter is created
	{
		UniformBufferPools::AllocationResult result = mUniformBufferPools.Allocate(UniformBufferPools::DecalPool);
		renderState.PerObjectBufferAllocationHandle = result.Handle;
		renderState.PerObjectParameterSet = result.ParameterSet;
		renderState.PerObjectSuballocation = result.GetSuballocation(UniformBufferPools::PerObjectBuffer);
		renderState.DecalParamSuballocation = result.GetSuballocation(UniformBufferPools::DecalBuffer);
	}

	// Store shared parameter set and buffer offsets for render-time binding
	drawCommand.SharedPerObjectParameterSet = renderState.PerObjectParameterSet;
	drawCommand.PerObjectBufferOffset = renderState.PerObjectSuballocation.GetSuballocationOffset();
	drawCommand.DecalParamBufferOffset = renderState.DecalParamSuballocation.GetSuballocationOffset();

	// Now update the uniform buffers (allocation is ready)
	renderState.UpdatePerObjectData();
	mUniformBufferPools.UpdatePerObjectBuffer(renderState);
	mUniformBufferPools.UpdateDecalParamBuffer(renderState);

	gpuParameterSet->TryGetUniformBufferParameter("PerFrame", drawCommand.PerFrameUniformBufferParameter);
	gpuParameterSet->TryGetUniformBufferParameter("PerCamera", drawCommand.PerCameraUniformBufferParameter);
	gpuParameterSet->TryGetSampledTextureParameter("gDepthBufferTex", drawCommand.DepthInputTexture);
	gpuParameterSet->TryGetSampledTextureParameter("gMaskTex", drawCommand.MaskInputTexture);
}

void RenderBeastScene::UpdateDecal(Decal* decal)
{
	const u32 rendererId = decal->GetRendererId();
	DecalRenderState& renderState = mInfo.Decals[rendererId];

	renderState.UpdatePerObjectData();
	mUniformBufferPools.UpdatePerObjectBuffer(renderState);
	mUniformBufferPools.UpdateDecalParamBuffer(renderState);

	mInfo.DecalCullInfos[rendererId].Bounds = decal->GetBounds();
}

void RenderBeastScene::UnregisterDecal(Decal* decal)
{
	const u32 rendererId = decal->GetRendererId();
	Decal* lastDecal = mInfo.Decals.back().Decal;
	const u32 lastDecalId = lastDecal->GetRendererId();

	DecalRenderState& renderState = mInfo.Decals[rendererId];
	DecalDrawCommand& drawCommand = renderState.DrawCommand;

	// Unregister sampler overrides
	FreeSamplerStateOverrides(drawCommand);
	drawCommand.SamplerOverrides = nullptr;

	// Release the buffer allocation
	mUniformBufferPools.Release(renderState.PerObjectBufferAllocationHandle);

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

	RenderableObjectStorage& renderableStorage = GetRenderableStorage();
	renderableStorage.SetScene(*this);

	mInfo.Renderables = &renderableStorage.GetRenderables();
	mInfo.RenderableCullInfos = &renderableStorage.GetRenderableCullInfos();

	LightObjectStorage& lightStorage = GetLightStorage();
	mInfo.DirectionalLights = &lightStorage.GetDirectionalLights();
	mInfo.RadialLights = &lightStorage.GetRadialLights();
	mInfo.SpotLights = &lightStorage.GetSpotLights();
	mInfo.RadialLightWorldBounds = &lightStorage.GetRadialLightWorldBounds();
	mInfo.SpotLightWorldBounds = &lightStorage.GetSpotLightWorldBounds();

	// Register all types
	for (const auto& config : GetRenderBeast()->GetPerObjectUniformTypeConfigurations())
		mUniformBufferPools.RegisterType(config);

	// Initialize after registration
	mUniformBufferPools.Initialize(*mGpuDevice);
	RendererScene::Initialize();
}

void RenderBeastScene::Destroy()
{
	RenderableObjectStorage& renderableStorage = GetRenderableStorage();
	for(auto& entry : renderableStorage.GetRenderables())
		B3DDelete(entry);

	for(auto& entry : mInfo.Views)
		B3DDelete(entry);

	B3D_ASSERT(mSamplerOverrides.empty());

	mUniformBufferPools.Destroy();

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

	// Rebind sampler overrides for renderables
	RenderableObjectStorage& renderableStorage = GetRenderableStorage();
	for(const auto& renderState : renderableStorage.GetRenderables())
	{
		for(auto& drawCommand : renderState->DrawCommands)
		{
			MaterialSamplerOverrides* overrides = drawCommand.SamplerOverrides;
			if(overrides != nullptr && overrides->IsDirty)
			{
				const u32 passCount = drawCommand.Material->GetPassCount(drawCommand.DefaultVariationIndex);
				for(u32 passIndex = 0; passIndex < passCount; passIndex++)
				{
					const u32 setCount = drawCommand.ParameterAdapter->GetSetCount(passIndex);
					for(u32 setIndex = 0; setIndex < setCount; setIndex++)
					{
						const SPtr<GpuParameterSet>& parameterSet = drawCommand.ParameterAdapter->GetGpuParameterSet(passIndex, setIndex);
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
	mPerFrameSuballocation = gPerFrameUniformDefinition.AllocateTransient();

	// Update SceneInfo pointer for compositor access
	mInfo.PerFrameSuballocation = &mPerFrameSuballocation;

	// Map and set the time parameter
	GpuBufferMappedScope mappedScope = mPerFrameSuballocation.Map();
	gPerFrameUniformDefinition.gTime.Set(mappedScope, time);
}

void RenderBeastScene::PrepareParticleSystem(u32 idx, const FrameInfo& frameInfo)
{
	ParticleRenderState& renderState = mInfo.ParticleSystems[idx];

	if(renderState.PrevFrameDirtyState != PrevFrameDirtyState::Clean)
	{
		if(renderState.PrevFrameDirtyState == PrevFrameDirtyState::Updated)
			renderState.PrevFrameDirtyState = PrevFrameDirtyState::CopyMostRecent;
		else if(renderState.PrevFrameDirtyState == PrevFrameDirtyState::CopyMostRecent)
		{
			renderState.PrevWorldTransform = renderState.WorldTransform;
			renderState.PrevFrameDirtyState = PrevFrameDirtyState::Clean;

			mUniformBufferPools.UpdatePerObjectBuffer(renderState);
		}
	}

	ParticlesDrawCommand& drawCommand = mInfo.ParticleSystems[idx].DrawCommand;
	drawCommand.ParameterAdapter->Update(drawCommand.Material, 0.0f);
}

void RenderBeastScene::PrepareDecal(u32 idx, const FrameInfo& frameInfo)
{
	DecalDrawCommand& drawCommand = mInfo.Decals[idx].DrawCommand;
	drawCommand.MaterialAnimationTime += frameInfo.Timings.TimeDelta;
	drawCommand.ParameterAdapter->Update(drawCommand.Material, drawCommand.MaterialAnimationTime);
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

MaterialSamplerOverrides* RenderBeastScene::AllocSamplerStateOverrides(DrawCommand& drawCommand)
{
	SamplerOverrideKey samplerKey(drawCommand.Material, drawCommand.DefaultVariationIndex);
	auto iterFind = mSamplerOverrides.find(samplerKey);
	if(iterFind != mSamplerOverrides.end())
	{
		iterFind->second->RefCount++;
		return iterFind->second;
	}
	else
	{
		SPtr<Shader> shader = drawCommand.Material->GetShader();
		MaterialSamplerOverrides* samplerOverrides = SamplerOverrideUtility::GenerateSamplerOverrides(*mGpuDevice, shader, drawCommand.Material->GetMaterialParameters(), drawCommand.ParameterAdapter, mOptions);

		mSamplerOverrides[samplerKey] = samplerOverrides;

		samplerOverrides->RefCount++;
		return samplerOverrides;
	}
}

void RenderBeastScene::FreeSamplerStateOverrides(DrawCommand& drawCommand)
{
	SamplerOverrideKey samplerKey(drawCommand.Material, drawCommand.DefaultVariationIndex);

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
