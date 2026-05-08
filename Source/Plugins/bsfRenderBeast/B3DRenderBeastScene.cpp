//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
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
#include "GpuBackend/B3DVertexDescription.h"
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
	const TShared<Variation>& variation = material.GetVariation(variationIndex);
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
		TShared<Pass> pass = material.GetPass(passIndex, variationIndex);
		TShared<GpuGraphicsPipelineState> graphicsPipeline = pass->GetGraphicsPipelineState();

		TShared<VertexDescription> shaderVertexDescription = graphicsPipeline->GetVertexProgram()->GetVertexInputDescription();
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

	for(const PackedRendererId renderableId : ids)
	{
		RenderableProxy& proxy = mRenderableProxies[renderableId];

		proxy.SetRendererId(renderableId);

		B3D_ASSERT(renderableId < (PackedRendererId)mRenderables.size());
		B3D_ASSERT(mRenderables[renderableId] == nullptr);

		mRenderables[renderableId] = B3DNew<RenderableRenderState>();
		mRenderableCullInfos[renderableId] = CullInfo(proxy.GetBounds(), proxy.GetLayer(), proxy.GetCullDistanceFactor());

		RenderableRenderState* renderState = mRenderables[renderableId];
		renderState->UpdatePerObjectData(proxy);
		renderState->PrevWorldTransform = renderState->WorldTransform;
		renderState->PrevFrameDirtyState = PrevFrameDirtyState::Clean;

		TShared<Mesh> mesh = proxy.GetMesh();
		if(mesh != nullptr)
		{
			const MeshProperties& meshProps = mesh->GetProperties();
			TShared<VertexDescription> vertexDescription = mesh->GetVertexData()->VertexDescription;

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

				const TShared<Shader>& shader = drawCommand.Material->GetShader();
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
			TShared<Shader> shader = drawCommand.Material->GetShader();
			if(shader == nullptr)
			{
				B3D_LOG(Warning, LogRenderer, "Missing shader on material.");
				continue;
			}

			// Store shared parameter set and buffer offset for render-time binding
			drawCommand.SharedPerObjectParameterSet = renderState->PerObjectParameterSet;
			drawCommand.PerObjectBufferOffset = renderState->PerObjectSuballocation.GetSuballocationOffset();

			TShared<GpuParameterSet> gpuParameterSet = drawCommand.ParameterAdapter->GetGpuParameterSet();

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
	if(mRenderBeastScene->mRenderableReady[id])
		return;

	RenderableRenderState* renderState = mRenderables[id];
	RenderableProxy& proxy = GetRenderableProxy(id);

	// Note: Before uploading bone matrices perhaps check if they has actually been changed since last frame
	TShared<GpuBuffer> boneMatrixBuffer;
	TShared<GpuBuffer> previousBoneMatrixBuffer;
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

	mRenderBeastScene->mRenderableReady[id] = true;
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

			const LightType type = mLightProxies[id].GetType();
			const u32 typeArrayIndex = mLightRenderStates[id].TypeArrayIndex;
			if(type == LightType::Directional)
				mDirectionalLightIds[typeArrayIndex] = id;
			else if(type == LightType::Radial)
				mRadialLightIds[typeArrayIndex] = id;
			else
				mSpotLightIds[typeArrayIndex] = id;
		},
		[this](TArrayView<const PackedRendererId> ids) { DestroyRenderState(ids); },
		mLightProxies, mLightRenderStates);
}

void LightObjectStorage::CreateRenderState(TArrayView<const PackedRendererId> ids)
{
	for(const PackedRendererId packedId : ids)
	{
		const LightProxy& proxy = mLightProxies[packedId];
		const LightType type = proxy.GetType();

		LightRenderState& renderState = mLightRenderStates[packedId];

		if(type == LightType::Directional)
		{
			renderState.TypeArrayIndex = (u32)mDirectionalLightIds.size();
			mDirectionalLightIds.push_back(packedId);
		}
		else if(type == LightType::Radial)
		{
			renderState.TypeArrayIndex = (u32)mRadialLightIds.size();
			mRadialLightIds.push_back(packedId);
			mRadialLightWorldBounds.push_back(proxy.GetBounds());
		}
		else // Spot
		{
			renderState.TypeArrayIndex = (u32)mSpotLightIds.size();
			mSpotLightIds.push_back(packedId);
			mSpotLightWorldBounds.push_back(proxy.GetBounds());
		}
	}
}

void LightObjectStorage::DestroyRenderState(TArrayView<const PackedRendererId> ids)
{
	for(const PackedRendererId packedId : ids)
	{
		const LightType type = mLightProxies[packedId].GetType();
		const u32 indexInTypeArray = mLightRenderStates[packedId].TypeArrayIndex;

		if(type == LightType::Directional)
		{
			const u32 lastIndex = (u32)mDirectionalLightIds.size() - 1;
			if(indexInTypeArray != lastIndex)
			{
				std::swap(mDirectionalLightIds[indexInTypeArray], mDirectionalLightIds[lastIndex]);

				PackedRendererId swappedPackedId = mDirectionalLightIds[indexInTypeArray];
				mLightRenderStates[swappedPackedId].TypeArrayIndex = indexInTypeArray;
			}
			mDirectionalLightIds.pop_back();
		}
		else if(type == LightType::Radial)
		{
			const u32 lastIndex = (u32)mRadialLightIds.size() - 1;
			if(indexInTypeArray != lastIndex)
			{
				std::swap(mRadialLightWorldBounds[indexInTypeArray], mRadialLightWorldBounds[lastIndex]);
				std::swap(mRadialLightIds[indexInTypeArray], mRadialLightIds[lastIndex]);

				PackedRendererId swappedPackedId = mRadialLightIds[indexInTypeArray];
				mLightRenderStates[swappedPackedId].TypeArrayIndex = indexInTypeArray;
			}
			mRadialLightWorldBounds.pop_back();
			mRadialLightIds.pop_back();
		}
		else // Spot
		{
			const u32 lastIndex = (u32)mSpotLightIds.size() - 1;
			if(indexInTypeArray != lastIndex)
			{
				std::swap(mSpotLightWorldBounds[indexInTypeArray], mSpotLightWorldBounds[lastIndex]);
				std::swap(mSpotLightIds[indexInTypeArray], mSpotLightIds[lastIndex]);

				PackedRendererId swappedPackedId = mSpotLightIds[indexInTypeArray];
				mLightRenderStates[swappedPackedId].TypeArrayIndex = indexInTypeArray;
			}
			mSpotLightWorldBounds.pop_back();
			mSpotLightIds.pop_back();
		}
	}
}

void LightObjectStorage::UpdateRenderState(TArrayView<const PackedRendererId> ids)
{
	for(const PackedRendererId packedId : ids)
	{
		const LightProxy& proxy = mLightProxies[packedId];
		const LightType type = proxy.GetType();
		const u32 typeArrayIndex = mLightRenderStates[packedId].TypeArrayIndex;

		if(type == LightType::Radial)
			mRadialLightWorldBounds[typeArrayIndex] = proxy.GetBounds();
		else if(type == LightType::Spot)
			mSpotLightWorldBounds[typeArrayIndex] = proxy.GetBounds();
	}
}

// ---- DecalObjectStorage ----

DecalObjectStorage::DecalObjectStorage() = default;

void DecalObjectStorage::ApplyCommands(const CommandBatch& commands, FrameAllocator& allocator)
{
	RendererObjectStorage::ApplyCommandsHelper(
		commands,
		allocator,
		[this](PackedRendererId id) { mDecalProxies[id].SetRendererId(id); },
		[this](TArrayView<const PackedRendererId> ids) { DestroyRenderState(ids); },
		mDecalProxies, mDecals, mDecalCullInfos);
}

void DecalObjectStorage::CreateRenderState(TArrayView<const PackedRendererId> ids)
{
	UniformBufferPools& uniformBufferPools = mRenderBeastScene->GetUniformBufferPools();

	for(const PackedRendererId packedId : ids)
	{
		const render::DecalProxy& proxy = GetDecalProxy(packedId);

		DecalRenderState& renderState = mDecals[packedId];

		mDecalCullInfos[packedId] = CullInfo(proxy.GetBounds(), proxy.GetLayer());

		DecalDrawCommand& drawCommand = renderState.DrawCommand;
		drawCommand.Type = (u32)DrawCommandType::Decal;
		drawCommand.Mesh = RendererUtility::Instance().GetBoxStencil();
		drawCommand.SubMesh = drawCommand.Mesh->GetProperties().SubMeshes[0];

		drawCommand.Material = proxy.GetMaterial();

		if(drawCommand.Material != nullptr && drawCommand.Material->GetShader() == nullptr)
			drawCommand.Material = nullptr;

		// If no material use the default material
		if(drawCommand.Material == nullptr)
			drawCommand.Material = Material::Create(DefaultDecalMaterial::Get()->GetShader());

		for(u32 insideGeometryIndex = 0; insideGeometryIndex < 2; insideGeometryIndex++)
		{
			for(u32 msaaModeIndex = 0; msaaModeIndex < 3; msaaModeIndex++)
			{
				FindVariationInformation findVariationInformation;
				findVariationInformation.VariationParameters = kDecalVariationParameterLookup[insideGeometryIndex][msaaModeIndex];
				findVariationInformation.Override = true;

				u32 variationIndex = drawCommand.Material->FindVariation(findVariationInformation);
				if(variationIndex == ~0u)
					variationIndex = 0;

				const TShared<Variation>& variation = drawCommand.Material->GetVariation(variationIndex);
				if(variation)
					variation->Compile();

				drawCommand.VariationIndices[insideGeometryIndex][msaaModeIndex] = variationIndex;
			}
		}

		drawCommand.DefaultVariationIndex = drawCommand.VariationIndices[0][0];

		// Generate or assign renderer specific data for the material
		drawCommand.ParameterAdapter = drawCommand.Material->CreateParameterAdapter(drawCommand.DefaultVariationIndex);
		drawCommand.ParameterAdapter->Update(drawCommand.Material, 0.0f, true);

		// Generate or assign sampler state overrides
		drawCommand.SamplerOverrides = mRenderBeastScene->AllocSamplerStateOverrides(drawCommand);

		// Prepare all parameter bindings
		TShared<GpuParameterSet> gpuParameterSet = drawCommand.ParameterAdapter->GetGpuParameterSet();

		// Allocate from the uniform buffer manager after ParameterAdapter is created
		{
			UniformBufferPools::AllocationResult result = uniformBufferPools.Allocate(UniformBufferPools::DecalPool);
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
		renderState.UpdatePerObjectData(proxy);
		uniformBufferPools.UpdatePerObjectBuffer(renderState);
		uniformBufferPools.UpdateDecalParamBuffer(renderState, proxy);

		gpuParameterSet->TryGetUniformBufferParameter("PerFrame", drawCommand.PerFrameUniformBufferParameter);
		gpuParameterSet->TryGetUniformBufferParameter("PerCamera", drawCommand.PerCameraUniformBufferParameter);
		gpuParameterSet->TryGetSampledTextureParameter("gDepthBufferTex", drawCommand.DepthInputTexture);
		gpuParameterSet->TryGetSampledTextureParameter("gMaskTex", drawCommand.MaskInputTexture);
	}
}

void DecalObjectStorage::DestroyRenderState(TArrayView<const PackedRendererId> ids)
{
	UniformBufferPools& uniformBufferPools = mRenderBeastScene->GetUniformBufferPools();

	for(const PackedRendererId packedId : ids)
	{
		DecalRenderState& renderState = mDecals[packedId];
		DecalDrawCommand& drawCommand = renderState.DrawCommand;

		// Unregister sampler overrides
		mRenderBeastScene->FreeSamplerStateOverrides(drawCommand);
		drawCommand.SamplerOverrides = nullptr;

		// Release the buffer allocation
		uniformBufferPools.Release(renderState.PerObjectBufferAllocationHandle);

	}
}

void DecalObjectStorage::UpdateRenderState(TArrayView<const PackedRendererId> ids)
{
	UniformBufferPools& uniformBufferPools = mRenderBeastScene->GetUniformBufferPools();

	for(const PackedRendererId packedId : ids)
	{
		const render::DecalProxy& proxy = GetDecalProxy(packedId);
		DecalRenderState& renderState = mDecals[packedId];

		renderState.UpdatePerObjectData(proxy);
		uniformBufferPools.UpdatePerObjectBuffer(renderState);
		uniformBufferPools.UpdateDecalParamBuffer(renderState, proxy);

		mDecalCullInfos[packedId] = CullInfo(proxy.GetBounds(), proxy.GetLayer());
	}
}

// ---- ParticleSystemObjectStorage ----

ParticleSystemObjectStorage::ParticleSystemObjectStorage() = default;

void ParticleSystemObjectStorage::ApplyCommands(const CommandBatch& commands, FrameAllocator& allocator)
{
	RendererObjectStorage::ApplyCommandsHelper(
		commands,
		allocator,
		[this](PackedRendererId id)
		{
			mParticleSystemProxies[id].SetRendererId(id);

			ParticleRenderState& renderState = mParticleRenderStates[id];
			if(renderState.GpuParticleSystem)
				mGpuSimulatedIds[renderState.GpuSimulatedParticleArrayIndex] = id;
		},
		[this](TArrayView<const PackedRendererId> ids) { DestroyRenderState(ids); },
		mParticleSystemProxies, mParticleRenderStates, mParticleSystemCullInfos);
}

void ParticleSystemObjectStorage::CreateRenderState(TArrayView<const PackedRendererId> ids)
{
	for(PackedRendererId id : ids)
	{
		ParticleRenderState& renderState = mParticleRenderStates[id];
		const render::ParticleSystemProxy& proxy = mParticleSystemProxies[id];

		renderState.UpdatePerObjectData(proxy);

		const render::ParticleSystemSettings& settings = proxy.GetSettings();

		// Initialize GPU simulation system if needed
		if(settings.GpuSimulation)
		{
			if(!renderState.GpuParticleSystem)
			{
				renderState.GpuParticleSystem = B3DPoolNew<GpuParticleSystem>();
				renderState.GpuSimulatedParticleArrayIndex = (u32)mGpuSimulatedIds.size();
				mGpuSimulatedIds.push_back(id);
			}
		}
		else
		{
			if(renderState.GpuParticleSystem)
			{
				B3DPoolDelete(renderState.GpuParticleSystem);
				renderState.GpuParticleSystem = nullptr;
			}
		}

		// Set up draw command
		ParticlesDrawCommand& drawCommand = renderState.DrawCommand;
		drawCommand.Type = (u32)DrawCommandType::Particle;

		drawCommand.Material = settings.Material;

		if(drawCommand.Material != nullptr && drawCommand.Material->GetShader() == nullptr)
			drawCommand.Material = nullptr;

		// If no material use the default material
		if(drawCommand.Material == nullptr)
			drawCommand.Material = Material::Create(DefaultParticleMaterial::Get()->GetShader());

		const TShared<Shader> shader = drawCommand.Material->GetShader();

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
		const TShared<Variation>& variation = drawCommand.Material->GetVariation(variationIndex);
		if(variation)
			variation->Compile();

		// Generate or assigned renderer specific data for the material
		drawCommand.ParameterAdapter = drawCommand.Material->CreateParameterAdapter(variationIndex);
		drawCommand.ParameterAdapter->Update(drawCommand.Material, 0.0f, true);

		TShared<GpuParameterSet> gpuParameterSet = drawCommand.ParameterAdapter->GetGpuParameterSet();

		// Allocate per-object uniform buffer
		const bool typeChanged = renderState.PerObjectBufferAllocationHandle.IsValid() && (gpu != drawCommand.IsGpuSimulated);

		if(typeChanged)
			mRenderBeastScene->GetUniformBufferPools().Release(renderState.PerObjectBufferAllocationHandle);

		if(!renderState.PerObjectBufferAllocationHandle.IsValid() || typeChanged)
		{
			if(gpu)
			{
				UniformBufferPools::AllocationResult result = mRenderBeastScene->GetUniformBufferPools().Allocate(UniformBufferPools::GpuParticlesPool);
				renderState.PerObjectBufferAllocationHandle = result.Handle;
				renderState.PerObjectParameterSet = result.ParameterSet;
				renderState.PerObjectSuballocation = result.GetSuballocation(UniformBufferPools::PerObjectBuffer);
				renderState.GpuParticlesParamSuballocation = result.GetSuballocation(UniformBufferPools::GpuParticlesBuffer);
			}
			else
			{
				UniformBufferPools::AllocationResult result = mRenderBeastScene->GetUniformBufferPools().Allocate(UniformBufferPools::RenderablePool);
				renderState.PerObjectBufferAllocationHandle = result.Handle;
				renderState.PerObjectParameterSet = result.ParameterSet;
				renderState.PerObjectSuballocation = result.GetSuballocation(UniformBufferPools::PerObjectBuffer);
			}
		}

		drawCommand.IsGpuSimulated = gpu;

		if(gpu)
			drawCommand.GpuParticlesParamBufferOffset = renderState.GpuParticlesParamSuballocation.GetSuballocationOffset();

		drawCommand.SharedPerObjectParameterSet = renderState.PerObjectParameterSet;
		drawCommand.PerObjectBufferOffset = renderState.PerObjectSuballocation.GetSuballocationOffset();

		mRenderBeastScene->GetUniformBufferPools().UpdatePerObjectBuffer(renderState);

		// Allocate and populate GPU particle system curves
		if(gpu)
		{
			// Get sprite image for animation curve generation
			SpriteImage* spriteImage = nullptr;
			if(shader->HasTextureParameter("gTexture"))
				spriteImage = drawCommand.Material->GetSpriteImage("gTexture").get();

			if(!spriteImage && shader->HasTextureParameter("gAlbedoTex"))
				spriteImage = drawCommand.Material->GetSpriteImage("gAlbedoTex").get();

			GpuParticleCurves& curves = GpuParticleSimulation::Instance().GetResources().GetCurveTexture();
			curves.Free(renderState.ColorCurveAlloc);
			curves.Free(renderState.SizeScaleFrameIdxCurveAlloc);

			static constexpr u32 kNumCurveSamples = 128;
			Color samples[kNumCurveSamples];

			const render::ParticleGpuSimulationSettings& gpuSimSettings = proxy.GetGpuSimulationSettings();

			// Write color over lifetime curve
			LookupTable colorLookup = gpuSimSettings.ColorOverLifetime.ToLookupTable(kNumCurveSamples, true);

			for(u32 sampleIndex = 0; sampleIndex < kNumCurveSamples; sampleIndex++)
			{
				const float* sample = colorLookup.GetSample(sampleIndex);
				samples[sampleIndex] = Color(sample[0], sample[1], sample[2], sample[3]);
			}

			renderState.ColorCurveAlloc = curves.Alloc(samples, kNumCurveSamples);

			// Write size over lifetime / sprite animation curve
			LookupTable sizeLookup = gpuSimSettings.SizeScaleOverLifetime.ToLookupTable(kNumCurveSamples, true);

			float frameSamples[kNumCurveSamples];
			if(spriteImage && spriteImage->GetAnimationPlayback() != SpriteAnimationPlayback::None)
			{
				const SpriteSheetGridAnimation& anim = spriteImage->GetAnimation();
				for(u32 sampleIndex = 0; sampleIndex < kNumCurveSamples; sampleIndex++)
				{
					const float t = sampleIndex / (float)(kNumCurveSamples - 1);
					frameSamples[sampleIndex] = t * (anim.FrameCount - 1);
				}
			}
			else
				memset(frameSamples, 0, sizeof(frameSamples));

			for(u32 sampleIndex = 0; sampleIndex < kNumCurveSamples; sampleIndex++)
			{
				const float* sample = sizeLookup.GetSample(sampleIndex);
				samples[sampleIndex] = Color(sample[0], sample[1], frameSamples[sampleIndex], 0.0f);
			}

			renderState.SizeScaleFrameIdxCurveAlloc = curves.Alloc(samples, kNumCurveSamples);

			// Update the GPU particles param buffer via staging
			mRenderBeastScene->GetUniformBufferPools().UpdateGpuParticlesParamBuffer(renderState);
		}

		// Bind additional parameters
		gpuParameterSet->TryGetUniformBufferParameter("PerCamera", drawCommand.PerCameraUniformBufferParameter);
		gpuParameterSet->TryGetUniformBufferParameter("ParticleParams", drawCommand.ParticlesUniformBufferParameter);
		gpuParameterSet->GetStorageBufferParameter("gIndices", drawCommand.IndicesBuffer);

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

		const bool isTransparent = shaderFlags.IsSet(ShaderFlag::Transparent);
		if(isTransparent)
		{
			if(gpuParameterSet->HasSampledTexture("gDepthBufferTex"))
				gpuParameterSet->GetSampledTextureParameter("gDepthBufferTex", drawCommand.DepthInputTexture);
		}

		// Set up buffers for lighting
		const bool useForwardRendering = shaderFlags.IsSet(ShaderFlag::Forward);
		if(useForwardRendering)
		{
			drawCommand.ForwardLightingParams.Populate(gpuParameterSet, supportsClusteredForward);
			drawCommand.ImageBasedParams.Initialize(gpuParameterSet, GPT_FRAGMENT_PROGRAM, true, supportsClusteredForward, supportsClusteredForward);
		}

		// Initialize cull info
		mParticleSystemCullInfos[id] = CullInfo(Bounds(kZeroTag), proxy.GetLayer());

		renderState.PrevWorldTransform = renderState.WorldTransform;
		renderState.PrevFrameDirtyState = PrevFrameDirtyState::Clean;
	}
}

void ParticleSystemObjectStorage::DestroyRenderState(TArrayView<const PackedRendererId> ids)
{
	for(PackedRendererId id : ids)
	{
		ParticleRenderState& renderState = mParticleRenderStates[id];

		// Free curves
		GpuParticleCurves& curves = GpuParticleSimulation::Instance().GetResources().GetCurveTexture();
		curves.Free(renderState.ColorCurveAlloc);
		curves.Free(renderState.SizeScaleFrameIdxCurveAlloc);

		if(renderState.GpuParticleSystem != nullptr)
		{
			const u32 gpuSimulatedParticleArrayIndex = renderState.GpuSimulatedParticleArrayIndex;
			const u32 lastIndex = (u32)mGpuSimulatedIds.size() - 1;
			if(gpuSimulatedParticleArrayIndex != lastIndex)
			{
				std::swap(mGpuSimulatedIds[gpuSimulatedParticleArrayIndex], mGpuSimulatedIds[lastIndex]);

				PackedRendererId swappedPackedId = mGpuSimulatedIds[gpuSimulatedParticleArrayIndex];
				mParticleRenderStates[swappedPackedId].GpuSimulatedParticleArrayIndex = gpuSimulatedParticleArrayIndex;
			}
			mGpuSimulatedIds.pop_back();

			B3DPoolDelete(renderState.GpuParticleSystem);
			renderState.GpuParticleSystem = nullptr;
		}

		// Release the buffer allocation
		mRenderBeastScene->GetUniformBufferPools().Release(renderState.PerObjectBufferAllocationHandle);
	}
}

void ParticleSystemObjectStorage::UpdateRenderState(TArrayView<const PackedRendererId> ids)
{
	for(PackedRendererId id : ids)
	{
		ParticleRenderState& renderState = mParticleRenderStates[id];
		const render::ParticleSystemProxy& proxy = mParticleSystemProxies[id];

		renderState.PrevWorldTransform = renderState.WorldTransform;
		renderState.PrevFrameDirtyState = PrevFrameDirtyState::Updated;

		renderState.UpdatePerObjectData(proxy);
		mRenderBeastScene->GetUniformBufferPools().UpdatePerObjectBuffer(renderState);
	}
}

// ---- ReflectionProbeObjectStorage ----

ReflectionProbeObjectStorage::ReflectionProbeObjectStorage() = default;

void ReflectionProbeObjectStorage::ApplyCommands(const CommandBatch& commands, FrameAllocator& allocator)
{
	RendererObjectStorage::ApplyCommandsHelper(
		commands,
		allocator,
		[this](PackedRendererId id)
		{
			mReflectionProbeProxies[id].SetRendererId(id);
		},
		[this](TArrayView<const PackedRendererId> ids) { DestroyRenderState(ids); },
		mReflectionProbeProxies, mReflectionProbeRenderStates, mReflProbeWorldBounds);
}

void ReflectionProbeObjectStorage::CreateRenderState(TArrayView<const PackedRendererId> slotIds)
{
	for(const PackedRendererId packedId : slotIds)
	{
		ReflectionProbeRenderState& renderState = mReflectionProbeRenderStates[packedId];
		renderState.ArrayIdx = (u32)-1;
		renderState.ArrayDirty = true;
		renderState.ErrorFlagged = false;

		mReflProbeWorldBounds[packedId] = mReflectionProbeProxies[packedId].GetBounds();

		// Find a free cubemap array slot
		u32 numArrayEntries = (u32)mReflProbeCubemapArrayUsedSlots.size();
		for(u32 slotIndex = 0; slotIndex < numArrayEntries; slotIndex++)
		{
			if(!mReflProbeCubemapArrayUsedSlots[slotIndex])
			{
				SetReflectionProbeArrayIndex(packedId, slotIndex, false);
				mReflProbeCubemapArrayUsedSlots[slotIndex] = true;
				break;
			}
		}

		// No empty slot was found
		if(renderState.ArrayIdx == (u32)-1)
		{
			SetReflectionProbeArrayIndex(packedId, numArrayEntries, false);
			mReflProbeCubemapArrayUsedSlots.push_back(true);
		}

		if(renderState.ArrayIdx > kMaxReflectionCubemaps)
		{
			B3D_LOG(Error, LogRenderer, "Reached the maximum number of allowed reflection probe cubemaps at once. "
									"Ignoring reflection probe data.");
		}
	}
}

void ReflectionProbeObjectStorage::DestroyRenderState(TArrayView<const PackedRendererId> slotIds)
{
	for(const PackedRendererId packedId : slotIds)
	{
		u32 arrayIdx = mReflectionProbeRenderStates[packedId].ArrayIdx;

		if(arrayIdx != (u32)-1 && arrayIdx < (u32)mReflProbeCubemapArrayUsedSlots.size())
			mReflProbeCubemapArrayUsedSlots[arrayIdx] = false;
	}
}

void ReflectionProbeObjectStorage::UpdateRenderState(TArrayView<const PackedRendererId> slotIds)
{
	for(const PackedRendererId packedId : slotIds)
		mReflProbeWorldBounds[packedId] = mReflectionProbeProxies[packedId].GetBounds();
}

void ReflectionProbeObjectStorage::OnFilteredTextureUpdated(PackedRendererId slotId)
{
	mReflectionProbeRenderStates[slotId].ArrayDirty = true;
}

void ReflectionProbeObjectStorage::SetReflectionProbeArrayIndex(PackedRendererId packedId, u32 arrayIdx, bool markAsClean)
{
	ReflectionProbeRenderState& renderState = mReflectionProbeRenderStates[packedId];
	renderState.ArrayIdx = arrayIdx;

	if(markAsClean)
		renderState.ArrayDirty = false;
}

void ReflectionProbeObjectStorage::UpdateReflectionProbes(GpuCommandBuffer& commandBuffer)
{
	const u32 probeCount = (u32)mReflectionProbeProxies.size();

	B3DMarkAllocatorFrame();
	{
		u32 currentCubeArraySize = 0;

		if(mReflProbeCubemapsTex != nullptr)
			currentCubeArraySize = mReflProbeCubemapsTex->GetProperties().ArraySliceCount;

		bool forceArrayUpdate = false;
		if(mReflProbeCubemapsTex == nullptr || (currentCubeArraySize < probeCount && currentCubeArraySize != kMaxReflectionCubemaps))
		{
			TextureCreateInformation cubeMapDesc;
			cubeMapDesc.Name = "Reflection Probe Cubemap Array";
			cubeMapDesc.Type = TEX_TYPE_CUBE_MAP;
			cubeMapDesc.Format = PF_RG11B10F;
			cubeMapDesc.Width = IBLUtility::kReflectionCubemapSize;
			cubeMapDesc.Height = IBLUtility::kReflectionCubemapSize;
			cubeMapDesc.MipMapCount = PixelUtility::GetMipmapCount(cubeMapDesc.Width, cubeMapDesc.Height, 1, cubeMapDesc.Format);
			cubeMapDesc.ArraySliceCount = std::min(kMaxReflectionCubemaps, probeCount + 4);

			mReflProbeCubemapsTex = mGpuDevice->CreateTexture(cubeMapDesc);

			forceArrayUpdate = true;
		}

		if(probeCount > 0)
		{
			auto& cubemapArrayProps = mReflProbeCubemapsTex->GetProperties();

			for(u32 probeIndex = 0; probeIndex < probeCount; probeIndex++)
			{
				const ReflectionProbeRenderState& probeRenderState = mReflectionProbeRenderStates[probeIndex];

				if(probeRenderState.ArrayIdx > kMaxReflectionCubemaps)
					continue;

				if(probeRenderState.ArrayDirty || forceArrayUpdate)
				{
					const TShared<Texture>& texture = mReflectionProbeProxies[probeIndex].GetFilteredTexture();
					if(texture == nullptr)
						continue;

					auto& srcProps = texture->GetProperties();
					bool isValid = srcProps.Width == IBLUtility::kReflectionCubemapSize &&
						srcProps.Height == IBLUtility::kReflectionCubemapSize &&
						srcProps.MipMapCount == cubemapArrayProps.MipMapCount &&
						srcProps.Type == TEX_TYPE_CUBE_MAP;

					if(!isValid)
					{
						if(!probeRenderState.ErrorFlagged)
						{
							B3D_LOG(Error, LogRenderer, "Reflection cubemap texture is not a valid cubemap of the expected size and "
								"mip count. Ignoring.");
							probeRenderState.ErrorFlagged = true;
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
								copyDesc.DestinationFace = probeRenderState.ArrayIdx * 6 + face;
								copyDesc.DestinationMip = mip;

								commandBuffer.CopyTexture(texture, mReflProbeCubemapsTex, copyDesc);
							}
						}
					}

					SetReflectionProbeArrayIndex(probeIndex, probeRenderState.ArrayIdx, true);
				}
			}
		}
	}
	B3DClearAllocatorFrame();
}

// ---- RenderBeastScene ----

RenderBeastScene::RenderBeastScene(const TShared<RenderBeastOptions>& options)
	: mOptions(options)
{
	mRenderableStorage = B3DMakeShared<RenderableObjectStorage>();
	mLightStorage = B3DMakeShared<LightObjectStorage>();
	mDecalStorage = B3DMakeShared<DecalObjectStorage>();
	mParticleSystemStorage = B3DMakeShared<ParticleSystemObjectStorage>();
	mReflectionProbeStorage = B3DMakeShared<ReflectionProbeObjectStorage>();
}

void RenderBeastScene::RegisterCamera(Camera* camera)
{
	RendererViewCreateInformation viewDesc = CreateViewDesc(camera);

	RendererView* view = B3DNew<RendererView>(viewDesc);
	view->SetRenderSettings(camera->GetRenderSettings());
	view->UpdatePerViewBuffer();

	u32 viewIdx = (u32)mViews.size();
	mViews.push_back(view);

	mCameraToView[camera] = viewIdx;
	camera->SetRendererId(viewIdx);

	UpdateCameraRenderTargets(camera);
}

void RenderBeastScene::UpdateCamera(Camera* camera, u32 updateFlag)
{
	u32 cameraId = camera->GetRendererId();
	RendererView* view = mViews[cameraId];

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

	Camera* lastCamera = mViews.back()->GetSceneCamera();
	u32 lastCameraId = lastCamera->GetRendererId();

	if(cameraId != lastCameraId)
	{
		// Swap current last element with the one we want to erase
		std::swap(mViews[cameraId], mViews[lastCameraId]);
		lastCamera->SetRendererId(cameraId);

		mCameraToView[lastCamera] = cameraId;
	}

	// Last element is the one we want to erase
	RendererView* view = mViews[mViews.size() - 1];
	B3DDelete(view);

	mViews.erase(mViews.end() - 1);

	auto iterFind = mCameraToView.find(camera);
	if(iterFind != mCameraToView.end())
		mCameraToView.erase(iterFind);

	UpdateCameraRenderTargets(camera, true);
}


void RenderBeastScene::UpdateReflectionProbes(GpuCommandBuffer& commandBuffer)
{
	GetReflectionProbeStorage().UpdateReflectionProbes(commandBuffer);
}

void RenderBeastScene::SetReflectionProbeArrayIndex(u32 probeIdx, u32 arrayIdx, bool markAsClean)
{
	GetReflectionProbeStorage().SetReflectionProbeArrayIndex(probeIdx, arrayIdx, markAsClean);
}

void RenderBeastScene::RegisterLightProbeVolume(LightProbeVolume* volume)
{
	mLightProbes.NotifyAdded(volume);
}

void RenderBeastScene::UpdateLightProbeVolume(LightProbeVolume* volume)
{
	mLightProbes.NotifyDirty(volume);
}

void RenderBeastScene::UnregisterLightProbeVolume(LightProbeVolume* volume)
{
	mLightProbes.NotifyRemoved(volume);
}

void RenderBeastScene::UpdateLightProbes(GpuCommandBuffer& commandBuffer)
{
	mLightProbes.UpdateProbes(commandBuffer);
}

void RenderBeastScene::RegisterSkybox(Skybox* skybox)
{
	mSkybox = skybox;
}

void RenderBeastScene::UnregisterSkybox(Skybox* skybox)
{
	if(mSkybox == skybox)
		mSkybox = nullptr;
}

void RenderBeastScene::Initialize()
{
	GetRenderBeast()->NotifySceneCreated(std::static_pointer_cast<RenderBeastScene>(GetShared()));

	mGpuDevice = GetRenderBeast()->GetGpuDevice();

	RenderableObjectStorage& renderableStorage = GetRenderableStorage();
	renderableStorage.SetScene(*this);

	DecalObjectStorage& decalStorage = GetDecalStorage();
	decalStorage.SetScene(*this);

	ParticleSystemObjectStorage& particleSystemStorage = GetParticleSystemStorage();
	particleSystemStorage.SetScene(*this);

	ReflectionProbeObjectStorage& reflectionProbeStorage = GetReflectionProbeStorage();
	reflectionProbeStorage.SetGpuDevice(mGpuDevice);

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

	for(auto& entry : mViews)
		B3DDelete(entry);

	B3D_ASSERT(mSamplerOverrides.empty());

	mUniformBufferPools.Destroy();

	GetRenderBeast()->NotifySceneDestroyed(this);
	RendererScene::Destroy();
}

void RenderBeastScene::ResetRenderableReady()
{
	const u32 renderableCount = GetRenderableStorage().GetRenderableCount();
	mRenderableReady.resize(renderableCount, false);
	mRenderableReady.assign(renderableCount, false);
}

void RenderBeastScene::SetOptions(const TShared<RenderBeastOptions>& options)
{
	mOptions = options;

	for(auto& entry : mViews)
		entry->SetStateReductionMode(mOptions->StateReductionMode);
}

RendererViewCreateInformation RenderBeastScene::CreateViewDesc(Camera* camera) const
{
	TShared<Viewport> viewport = camera->GetViewport();
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
	TShared<RenderTarget> renderTarget = camera->GetViewport()->GetTarget();

	// Remove from render target list
	int rtChanged = 0; // 0 - No RT, 1 - RT found, 2 - RT changed
	for(auto iterTarget = mRenderTargets.begin(); iterTarget != mRenderTargets.end(); ++iterTarget)
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
			mRenderTargets.erase(iterTarget);
			break;
		}
	}

	// Register in render target list
	if(renderTarget != nullptr && !remove && (rtChanged == 0 || rtChanged == 2))
	{
		auto findIter = std::find_if(mRenderTargets.begin(), mRenderTargets.end(), [&](const RendererRenderTarget& x)
									 { return x.Target == renderTarget; });

		if(findIter != mRenderTargets.end())
		{
			findIter->Cameras.push_back(camera);
		}
		else
		{
			mRenderTargets.push_back(RendererRenderTarget());
			RendererRenderTarget& renderTargetData = mRenderTargets.back();

			renderTargetData.Target = renderTarget;
			renderTargetData.Cameras.push_back(camera);
		}

		// Sort render targets based on priority
		auto cameraComparer = [&](const Camera* a, const Camera* b)
		{ return a->GetPriority() > b->GetPriority(); };
		auto renderTargetInfoComparer = [&](const RendererRenderTarget& a, const RendererRenderTarget& b)
		{ return a.Target->GetProperties().Priority > b.Target->GetProperties().Priority; };
		std::sort(begin(mRenderTargets), end(mRenderTargets), renderTargetInfoComparer);

		for(auto& camerasPerTarget : mRenderTargets)
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
		TShared<MaterialParameters> materialParams = entry.first.Material->GetMaterialParameters();

		MaterialSamplerOverrides* materialOverrides = entry.second;
		for(u32 i = 0; i < materialOverrides->NumOverrides; i++)
		{
			SamplerOverride& override = materialOverrides->Overrides[i];
			const MaterialParametersBase::ParamData* materialParamData = materialParams->GetParamData(override.ParamIdx);

			TShared<SamplerState> samplerState;
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
						const TShared<GpuParameterSet>& parameterSet = drawCommand.ParameterAdapter->GetGpuParameterSet(passIndex, setIndex);
						const TShared<GpuPipelineParameterSetLayout>& uniformLayoutSet = parameterSet->GetLayout();

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

	// Map and set the time parameter
	GpuBufferMappedScope mappedScope = mPerFrameSuballocation.Map();
	gPerFrameUniformDefinition.gTime.Set(mappedScope, time);
}

void RenderBeastScene::PrepareParticleSystem(u32 idx, const FrameInfo& frameInfo)
{
	ParticleSystemObjectStorage& storage = GetParticleSystemStorage();
	ParticleRenderState& renderState = storage.GetParticleRenderState(idx);

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

	ParticlesDrawCommand& drawCommand = renderState.DrawCommand;
	drawCommand.ParameterAdapter->Update(drawCommand.Material, 0.0f);
}

void RenderBeastScene::PrepareDecal(u32 idx, const FrameInfo& frameInfo)
{
	DecalDrawCommand& drawCommand = GetDecalStorage().GetDecals()[idx].DrawCommand;
	drawCommand.MaterialAnimationTime += frameInfo.Timings.TimeDelta;
	drawCommand.ParameterAdapter->Update(drawCommand.Material, drawCommand.MaterialAnimationTime);
}

void RenderBeastScene::UpdateParticleSystemBounds(const EvaluatedParticleData* particleRenderData)
{
	// Note: Avoid updating bounds for deterministic particle systems every frame. Also see if this can be copied
	// over in a faster way (or ideally just assigned)

	ParticleSystemObjectStorage& storage = GetParticleSystemStorage();
	const u32 particleSystemCount = storage.GetParticleSystemCount();

	for(u32 particleSystemIndex = 0; particleSystemIndex < particleSystemCount; particleSystemIndex++)
	{
		ParticleRenderState& entry = storage.GetParticleRenderState(particleSystemIndex);
		const ParticleSystemProxy& proxy = storage.GetParticleSystemProxy(particleSystemIndex);
		const ParticleSystemSettings& settings = proxy.GetSettings();

		AABox worldAABox = AABox::kInfinite;
		const auto iterFind = particleRenderData->CpuData.find(proxy.GetId());
		if(iterFind != particleRenderData->CpuData.end())
			worldAABox = iterFind->second->Bounds;
		else if(entry.GpuParticleSystem)
		{
			if(settings.UseAutomaticBounds)
				worldAABox = AABox(-(float)kMaximumSceneExtent, (float)kMaximumSceneExtent);
			else
				worldAABox = settings.CustomBounds;
		}

		if(settings.SimulationSpace == ParticleSimulationSpace::Local)
			worldAABox.TransformAffine(entry.WorldTransform);

		const Sphere worldSphere(worldAABox.GetCenter(), worldAABox.GetRadius());
		storage.GetParticleSystemCullInfos()[particleSystemIndex].Bounds = Bounds(worldAABox, worldSphere);
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
		TShared<Shader> shader = drawCommand.Material->GetShader();
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
