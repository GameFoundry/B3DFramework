//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DRenderCompositor.h"
#include "Renderer/B3DRendererExtension.h"
#include "Components/B3DSkybox.h"
#include "Components/B3DCamera.h"
#include "Renderer/B3DRendererUtility.h"
#include "Utility/B3DBitwise.h"
#include "Mesh/B3DMesh.h"
#include "Material/B3DMaterialParameterAdapter.h"
#include "Renderer/B3DGpuResourcePool.h"
#include "Utility/B3DRendererTextures.h"
#include "Shading/B3DStandardDeferred.h"
#include "Shading/B3DTiledDeferred.h"
#include "Shading/B3DLightProbes.h"
#include "Shading/B3DPostProcessing.h"
#include "Shading/B3DShadowRendering.h"
#include "Shading/B3DLightGrid.h"
#include "B3DRendererView.h"
#include "B3DRenderBeastOptions.h"
#include "B3DRenderBeastScene.h"
#include "B3DRenderBeast.h"
#include "Particles/B3DParticleScene.h"
#include "Components/B3DParticleSystem.h"
#include "Profiling/B3DProfilerGPU.h"
#include "Shading/B3DGpuParticleSimulation.h"
#include "Profiling/B3DProfilerCPU.h"
#include "RenderAPI/B3DGpuCommandBuffer.h"
#include "RenderAPI/B3DRenderTexture.h"

namespace b3d { namespace render {

UnorderedMap<StringID, RenderCompositor::NodeDescriptor*> RenderCompositor::mNodeDescriptors;

/** Prepares a render pass by registering all required parameter sets from the render queue elements. */
void PrepareRenderQueuePass(RenderPassCreateInformation& passInfo, const Vector<RenderQueueElement>& elements)
{
	TInlineArray<SPtr<GpuParameterSet>, 4> perObjectParameterSets;

	for(const auto& element : elements)
	{
		SPtr<GpuParameterSet> parameterSet0 = element.RenderElem->ParameterAdapter->GetGpuParameterSet(element.PassIdx);
		if(parameterSet0 != nullptr)
			passInfo.Parameters.Add(parameterSet0);

		const SPtr<GpuParameterSet>& parameterSet1 = element.RenderElem->SharedPerObjectParameterSet;
		auto found = std::find(perObjectParameterSets.begin(), perObjectParameterSets.end(), parameterSet1);
		if(found == perObjectParameterSets.end())
		{
			passInfo.Parameters.Add(parameterSet1);
			perObjectParameterSets.Add(parameterSet1);
		}
	}
}

/** Renders all elements in a render queue. */
void RenderQueueElements(GpuCommandBuffer& commandBuffer, const Vector<RenderQueueElement>& elements)
{
	// TODO: Consider sorting elements by SharedPerObjectParameterSet to minimize parameter set rebinds.
	// Currently respecting existing RenderQueue sorting rules (material, distance, etc.).

	const u32 perObjectDynamicOffsetIndex = GetRenderBeast()->GetRenderableParameterSetInfo().PerObjectDynamicOffsetIndex;
	SPtr<GpuParameterSet> lastBoundPerObjectSet;

	for(auto& entry : elements)
	{
		if(entry.ApplyPass)
			GetRendererUtility().SetPass(commandBuffer, entry.RenderElem->Material, entry.PassIdx, entry.TechniqueIdx);

		// Bind shared per-object parameter set if changed (using SetGpuParameterSet directly, not SetPassParams)
		if(entry.RenderElem->SharedPerObjectParameterSet != lastBoundPerObjectSet)
		{
			if(entry.RenderElem->SharedPerObjectParameterSet)
			{
				commandBuffer.SetGpuParameterSet(entry.RenderElem->SharedPerObjectParameterSet);
				lastBoundPerObjectSet = entry.RenderElem->SharedPerObjectParameterSet;
			}
		}

		// Set the dynamic buffer offset for this element's per-object data
		commandBuffer.SetDynamicBufferOffset(GpuPipelineSet::kPerObject, perObjectDynamicOffsetIndex, entry.RenderElem->PerObjectBufferOffset);

		GetRendererUtility().SetPassParams(commandBuffer, entry.RenderElem->ParameterAdapter, entry.PassIdx);

		entry.RenderElem->Draw(commandBuffer);
	}
}

RenderCompositor::~RenderCompositor()
{
	Clear();
}

void RenderCompositor::Build(const RendererView& view, const TArray<StringID>& primaryNodes)
{
	Clear();

	FrameAllocatorScope frameScope;
	FrameUnorderedMap<StringID, u32> processedNodes;
	mIsValid = true;

	std::function<bool(const StringID&, RenderCompositorNodeCategory)> fnRegisterNode = [&](const StringID& nodeId, RenderCompositorNodeCategory requiredNodeCategory)
	{
		// Find node type
		auto foundNodeDescriptor = mNodeDescriptors.find(nodeId);
		if(!B3D_ENSURE(foundNodeDescriptor != mNodeDescriptors.end()))
			return false;

		const NodeDescriptor* const nodeDescriptor = foundNodeDescriptor->second;

		// Register current node
		auto foundNodeInstance = processedNodes.find(nodeId);

		// New node
		if(foundNodeInstance == processedNodes.end())
		{
			if(!B3D_ENSURE(nodeDescriptor->GetCategory() == requiredNodeCategory))
				return false;

			// Mark it as invalid for now
			processedNodes[nodeId] = -1;
		}

		// Register node dependencies
		TInlineArray<StringID, 4> nodeDependencyIds = nodeDescriptor->GetDependencies(view);
		TInlineArray<StringID, 4> allDependencyIds = nodeDescriptor->GetAllDependencyIds();

		for(const auto& entry : nodeDependencyIds)
		{
			if(!fnRegisterNode(entry, RenderCompositorNodeCategory::Utility))
				return false;
		}

		// Register current node
		u32 currentNodeIndex;

		// New node, properly populate its index
		if(foundNodeInstance == processedNodes.end())
		{
			foundNodeInstance = processedNodes.find(nodeId);

			currentNodeIndex = (u32)mRegisteredNodes.size();
			mRegisteredNodes.push_back(NodeInfo());
			processedNodes[nodeId] = currentNodeIndex;

			NodeInfo& nodeInfo = mRegisteredNodes.back();
			nodeInfo.Node = nodeDescriptor->Create();
			nodeInfo.NodeDescriptor = nodeDescriptor;
			nodeInfo.LastUsedByNodeIndex = -1;

			// Pad Inputs array with nulls for disabled dependencies
			for(const auto& entry : allDependencyIds)
			{
				// Check if this dependency is enabled
				bool isEnabled = false;
				for(const auto& enabledId : nodeDependencyIds)
				{
					if(enabledId == entry)
					{
						isEnabled = true;
						break;
					}
				}

				if(isEnabled)
				{
					foundNodeInstance = processedNodes.find(entry);
					NodeInfo& nodeDependencyNode = mRegisteredNodes[foundNodeInstance->second];
					nodeInfo.Inputs.Add(nodeDependencyNode.Node);
				}
				else
				{
					nodeInfo.Inputs.Add(nullptr);
				}
			}
		}
		else // Existing node
		{
			currentNodeIndex = foundNodeInstance->second;

			// Check if invalid
			if(currentNodeIndex == ~0u)
			{
				B3D_LOG(Error, LogRenderer, "Render compositor nodes recursion detected. Node \"{0}\" "
										"depends on node \"{1}\" which is not available at this stage.",
					   String(nodeId.CStr()), String(foundNodeDescriptor->first.CStr()));
				return false;
			}
		}

		// Update dependency last use counters
		for(const auto& entry : nodeDependencyIds)
		{
			foundNodeInstance = processedNodes.find(entry);

			NodeInfo& depNodeInfo = mRegisteredNodes[foundNodeInstance->second];
			if(depNodeInfo.LastUsedByNodeIndex == ~0u)
				depNodeInfo.LastUsedByNodeIndex = currentNodeIndex;
			else
				depNodeInfo.LastUsedByNodeIndex = std::max(depNodeInfo.LastUsedByNodeIndex, currentNodeIndex);
		}

		return true;
	};

	mIsValid = true;
	for(const auto& entry : primaryNodes)
	{
		if(!fnRegisterNode(entry, RenderCompositorNodeCategory::Primary))
		{
			mIsValid = false;
			break;
		}
	}

	if(!mIsValid)
		Clear();
}

void RenderCompositor::Execute(RenderCompositorNodeInputs& inputs) const
{
	if(!mIsValid)
		return;

	B3DMarkAllocatorFrame();
	{
		FrameVector<const NodeInfo*> activeNodes;

		u32 idx = 0;
		for(auto& entry : mRegisteredNodes)
		{
			inputs.InputNodes = entry.Inputs;

#if B3D_PROFILING_ENABLED
			const ProfilerString sampleName = ProfilerString("RC: ") + entry.NodeDescriptor->Id.CStr();
			if(inputs.CommandBufferProfiler != nullptr)
				inputs.CommandBufferProfiler->BeginSample(*inputs.ActiveCommandBuffer, sampleName);

			GetProfilerCPU().BeginSample(sampleName.c_str());
#endif

			inputs.ActiveCommandBuffer->BeginLabel(entry.NodeDescriptor->Id.CStr());
			entry.Node->Render(inputs);
			inputs.ActiveCommandBuffer->EndLabel();

#if B3D_PROFILING_ENABLED
			GetProfilerCPU().EndSample(sampleName.c_str());

			if(inputs.CommandBufferProfiler != nullptr)
				inputs.CommandBufferProfiler->EndSample(*inputs.ActiveCommandBuffer);
#endif

			activeNodes.push_back(&entry);

			for(u32 i = 0; i < (u32)activeNodes.size(); ++i)
			{
				if(activeNodes[i] == nullptr)
					continue;

				if(activeNodes[i]->LastUsedByNodeIndex <= idx)
				{
					activeNodes[i]->Node->Clear();
					activeNodes[i] = nullptr;
				}
			}

			idx++;
		}
	}
	B3DClearAllocatorFrame();

	if(!mRegisteredNodes.empty())
		mRegisteredNodes.back().Node->Clear();
}

void RenderCompositor::Clear()
{
	for(auto& entry : mRegisteredNodes)
		B3DDelete(entry.Node);

	mRegisteredNodes.clear();
	mIsValid = false;
}

void RCNodeSceneDepth::Render(const RenderCompositorNodeInputs& inputs)
{
	const RendererViewProperties& viewProps = inputs.View.GetProperties();

	u32 width = viewProps.Target.ViewRect.Width;
	u32 height = viewProps.Target.ViewRect.Height;
	u32 numSamples = viewProps.Target.NumSamples;

	DepthTex = GetGpuResourcePool().Get(PooledRenderTextureCreateInformation::Create2D(PF_D32_S8X24, width, height, TextureUsageFlag::DepthStencil, numSamples, false));
}

void RCNodeSceneDepth::Clear()
{
	DepthTex = nullptr;
}

RCNodeSceneDepth::DependencyDefinition RCNodeSceneDepth::GetDependencyDefinition()
{
	static const DependencyDefinition kDependencyDefinition;
	return kDependencyDefinition;
}

void RCNodeBasePass::Render(const RenderCompositorNodeInputs& inputs)
{
	// Allocate necessary textures & targets
	GpuResourcePool& resPool = GetGpuResourcePool();
	const RendererViewProperties& viewProps = inputs.View.GetProperties();

	const u32 width = viewProps.Target.ViewRect.Width;
	const u32 height = viewProps.Target.ViewRect.Height;
	const u32 numSamples = viewProps.Target.NumSamples;

	bool needsVelocity = inputs.View.RequiresVelocityWrites();

	// Note: Consider customizable formats. e.g. for testing if quality can be improved with higher precision normals.
	AlbedoTex = resPool.Get(PooledRenderTextureCreateInformation::Create2D(PF_RGBA8, width, height, TextureUsageFlag::RenderTarget, numSamples, true));
	NormalTex = resPool.Get(PooledRenderTextureCreateInformation::Create2D(PF_RGB10A2, width, height, TextureUsageFlag::RenderTarget, numSamples, false));
	RoughMetalTex = resPool.Get(PooledRenderTextureCreateInformation::Create2D(PF_RG16F, width, height, TextureUsageFlag::RenderTarget, numSamples, false)); // Note: Metal doesn't need 16-bit float
	IdTex = resPool.Get(PooledRenderTextureCreateInformation::Create2D(PF_R8, width, height, TextureUsageFlag::RenderTarget, numSamples, false));

	if(needsVelocity)
	{
		VelocityTex = resPool.Get(PooledRenderTextureCreateInformation::Create2D(PF_RG16S, width, height, TextureUsageFlag::RenderTarget, numSamples, false));
	}

	auto dependencies = GetDependencyDefinition().ResolveDependencies(inputs);
	auto sceneDepthNode = dependencies.Get<RCNodeSceneDepth>();
	auto sceneColorNode = dependencies.Get<RCNodeSceneColor>();
	SPtr<PooledRenderTexture> sceneDepthTex = sceneDepthNode->DepthTex;
	SPtr<PooledRenderTexture> sceneColorTex = sceneColorNode->SceneColorTex;

	bool rebuildRT = false;
	if(RenderTarget != nullptr)
	{
		u32 targetIdx = 0;
		rebuildRT |= RenderTarget->GetColorTexture(targetIdx++) != sceneColorTex->Texture;
		rebuildRT |= RenderTarget->GetColorTexture(targetIdx++) != AlbedoTex->Texture;
		rebuildRT |= RenderTarget->GetColorTexture(targetIdx++) != NormalTex->Texture;
		rebuildRT |= RenderTarget->GetColorTexture(targetIdx++) != RoughMetalTex->Texture;
		if(needsVelocity) rebuildRT |= RenderTarget->GetColorTexture(targetIdx++) != VelocityTex->Texture;
		rebuildRT |= RenderTarget->GetColorTexture(targetIdx++) != IdTex->Texture;
		rebuildRT |= RenderTarget->GetDepthStencilTexture() != sceneDepthTex->Texture;
	}
	else
		rebuildRT = true;

	if(RenderTarget == nullptr || rebuildRT)
	{
		u32 targetIdx = 0;

		RenderTextureCreateInformation gbufferDesc;
		gbufferDesc.ColorSurfaces[targetIdx].Texture = sceneColorTex->Texture;
		gbufferDesc.ColorSurfaces[targetIdx].Face = 0;
		gbufferDesc.ColorSurfaces[targetIdx].FaceCount = 1;
		gbufferDesc.ColorSurfaces[targetIdx].MipLevel = 0;
		targetIdx++;

		gbufferDesc.ColorSurfaces[targetIdx].Texture = AlbedoTex->Texture;
		gbufferDesc.ColorSurfaces[targetIdx].Face = 0;
		gbufferDesc.ColorSurfaces[targetIdx].FaceCount = 1;
		gbufferDesc.ColorSurfaces[targetIdx].MipLevel = 0;
		targetIdx++;

		gbufferDesc.ColorSurfaces[targetIdx].Texture = NormalTex->Texture;
		gbufferDesc.ColorSurfaces[targetIdx].Face = 0;
		gbufferDesc.ColorSurfaces[targetIdx].FaceCount = 1;
		gbufferDesc.ColorSurfaces[targetIdx].MipLevel = 0;
		targetIdx++;

		gbufferDesc.ColorSurfaces[targetIdx].Texture = RoughMetalTex->Texture;
		gbufferDesc.ColorSurfaces[targetIdx].Face = 0;
		gbufferDesc.ColorSurfaces[targetIdx].FaceCount = 1;
		gbufferDesc.ColorSurfaces[targetIdx].MipLevel = 0;
		targetIdx++;

		if(needsVelocity)
		{
			gbufferDesc.ColorSurfaces[targetIdx].Texture = VelocityTex->Texture;
			gbufferDesc.ColorSurfaces[targetIdx].Face = 0;
			gbufferDesc.ColorSurfaces[targetIdx].FaceCount = 1;
			gbufferDesc.ColorSurfaces[targetIdx].MipLevel = 0;
			targetIdx++;
		}

		gbufferDesc.DepthStencilSurface.Texture = sceneDepthTex->Texture;
		gbufferDesc.DepthStencilSurface.Face = 0;
		gbufferDesc.DepthStencilSurface.MipLevel = 0;

		RenderTargetNoMask = RenderTexture::Create(gbufferDesc);

		gbufferDesc.ColorSurfaces[targetIdx].Texture = IdTex->Texture;
		gbufferDesc.ColorSurfaces[targetIdx].Face = 0;
		gbufferDesc.ColorSurfaces[targetIdx].FaceCount = 1;
		gbufferDesc.ColorSurfaces[targetIdx].MipLevel = 0;
		targetIdx++;

		RenderTarget = RenderTexture::Create(gbufferDesc);
	}

	// Prepare all visible objects. Note that this also prepares non-opaque objects.
	//// Prepare normal renderables
	const VisibilityInfo& visibility = inputs.View.GetVisibilityMasks();
	const auto renderableCount = (u32)inputs.Scene.Renderables.size();
	for(u32 i = 0; i < renderableCount; i++)
	{
		if(!visibility.Renderables[i])
			continue;

		for(auto& element : inputs.Scene.Renderables[i]->Elements)
		{
			element.PerFrameUniformBufferParameter.Set(*inputs.Scene.PerFrameSuballocation);
			element.PerCameraUniformBufferParameter.Set(inputs.View.GetPerViewBuffer());
		}
	}

	//// Prepare particle systems
	const EvaluatedParticleData* particleData = inputs.FrameInfo.PerSceneFrameData.Particles;
	if(particleData)
	{
		const auto particleSystemCount = (u32)inputs.Scene.ParticleSystems.size();
		const GpuParticleResources& gpuSimResources = GpuParticleSimulation::Instance().GetResources();

		bool isGpuSortingUsed = false;
		for(u32 i = 0; i < particleSystemCount; i++)
		{
			if(!visibility.ParticleSystems[i])
				continue;

			const RendererParticles& rendererParticles = inputs.Scene.ParticleSystems[i];
			ParticlesRenderElement& renderElement = rendererParticles.RenderElement;

			if(!renderElement.IsValid())
				continue;

			ParticleSystem* particleSystem = rendererParticles.ParticleSystem;

			// Bind textures/buffers from CPU simulation
			const auto iterFind = particleData->CpuData.find(particleSystem->GetId());
			if(iterFind != particleData->CpuData.end())
			{
				ParticleRenderData* renderData = iterFind->second;
				rendererParticles.BindCpuSimulatedInputs(renderData, inputs.View);
			}
			// Bind textures/buffers from GPU simulation
			else if(rendererParticles.GpuParticleSystem)
			{
				rendererParticles.BindGpuSimulatedInputs(gpuSimResources, inputs.View);

				if(rendererParticles.GpuParticleSystem->HasSortInfo())
					isGpuSortingUsed = true;
			}
		}
	}

	//// Prepare decals
	const auto decalCount = (u32)inputs.Scene.Decals.size();
	for(u32 i = 0; i < decalCount; i++)
	{
		if(!visibility.Decals[i])
			continue;

		const RendererDecal& rendererDecal = inputs.Scene.Decals[i];
		DecalRenderElement& renderElement = rendererDecal.RenderElement;

		renderElement.PerFrameUniformBufferParameter.Set(*inputs.Scene.PerFrameSuballocation);
		renderElement.PerCameraUniformBufferParameter.Set(inputs.View.GetPerViewBuffer());
		renderElement.DepthInputTexture.Set(sceneDepthTex->Texture);
		renderElement.MaskInputTexture.Set(IdTex->Texture);
	}

	Camera* sceneCamera = inputs.View.GetSceneCamera();

	// Trigger prepare callbacks
	if(sceneCamera != nullptr)
	{
		for(auto& extension : inputs.ExtPrepare)
		{
			RendererViewContext context;
			context.CurrentTarget = inputs.View.GetCompositorRenderTarget();
			context.CommandBuffer = inputs.ActiveCommandBuffer;

			extension->Render(*sceneCamera, context);
		}
	}

	// Render base pass
	GpuCommandBuffer& commandBuffer = *inputs.ActiveCommandBuffer;

	// Clear the GBuffer
	commandBuffer.BeginRenderPass(RenderPassCreateInformation(RenderTarget));

	Area2 area(0.0f, 0.0f, 1.0f, 1.0f);
	commandBuffer.SetViewport(area);

	commandBuffer.ClearViewport(RT_ALL, Color::kZero);

	commandBuffer.EndRenderPass();

	// Trigger pre-base-pass callbacks
	if(sceneCamera != nullptr)
	{
		inputs.View.NotifyCompositorTargetChangedInternal(RenderTarget);

		for(auto& extension : inputs.ExtPreBasePass)
		{
			RendererViewContext context;
			context.CurrentTarget = inputs.View.GetCompositorRenderTarget();
			context.CommandBuffer = inputs.ActiveCommandBuffer;

			extension->Render(*sceneCamera, context);
		}
	}

	// Collect all GpuParameters that will be used in the base pass
	const Vector<RenderQueueElement>& opaqueElements = inputs.View.GetOpaqueQueue(false)->GetSortedElements();
	RenderPassCreateInformation basePassInfo(RenderTarget, RT_NONE, RT_ALL);
	PrepareRenderQueuePass(basePassInfo, opaqueElements);

	commandBuffer.BeginRenderPass(basePassInfo);

	// Render all visible opaque elements that use the deferred pipeline
	RenderQueueElements(commandBuffer, opaqueElements);

	commandBuffer.EndRenderPass();

	// Determine MSAA coverage if required
	if(viewProps.Target.NumSamples > 1)
	{
		auto msaaCoverageNode = static_cast<RCNodeMSAACoverage*>(inputs.InputNodes[3]);

		GBufferTextures gbuffer;
		gbuffer.Albedo = AlbedoTex->Texture;
		gbuffer.Normals = NormalTex->Texture;
		gbuffer.RoughMetal = RoughMetalTex->Texture;
		gbuffer.Depth = sceneDepthNode->DepthTex->Texture;

		MSAACoverageMaterial* const msaaCoverageMaterial = MSAACoverageMaterial::GetVariation(viewProps.Target.NumSamples);
		msaaCoverageMaterial->Prepare(inputs.View, gbuffer);

		RenderPassCreateInformation msaaCoverageInfo(msaaCoverageNode->Output->RenderTexture, msaaCoverageMaterial->GetGpuParameterSet());
		commandBuffer.BeginRenderPass(msaaCoverageInfo);
		msaaCoverageMaterial->Execute(commandBuffer, inputs.View);
		commandBuffer.EndRenderPass();

		MSAACoverageStencilMaterial* msaaCoverageStencilMaterial = MSAACoverageStencilMaterial::Get();
		msaaCoverageStencilMaterial->Prepare(msaaCoverageNode->Output->Texture);

		RenderPassCreateInformation msaaStencilInfo(sceneDepthNode->DepthTex->RenderTexture, msaaCoverageStencilMaterial->GetGpuParameterSet());
		commandBuffer.BeginRenderPass(msaaStencilInfo);
		msaaCoverageStencilMaterial->Execute(commandBuffer, inputs.View);
		commandBuffer.EndRenderPass();
	}

	// Render decals after all normal objects, using a read-only depth buffer
	const Vector<RenderQueueElement>& decalElements = inputs.View.GetDecalQueue()->GetSortedElements();
	RenderPassCreateInformation decalPassInfo(RenderTargetNoMask, RT_DEPTH, RT_ALL);
	PrepareRenderQueuePass(decalPassInfo, decalElements);

	commandBuffer.BeginRenderPass(decalPassInfo);

	RenderQueueElements(commandBuffer, decalElements);

	// Trigger post-base-pass callbacks
	if(sceneCamera != nullptr)
	{
		inputs.View.NotifyCompositorTargetChangedInternal(RenderTargetNoMask);

		for(auto& extension : inputs.ExtPostBasePass)
		{
			RendererViewContext context;
			context.CurrentTarget = inputs.View.GetCompositorRenderTarget();
			context.CommandBuffer = inputs.ActiveCommandBuffer;

			extension->Render(*sceneCamera, context);
		}
	}

	// Make sure that any compute shaders are able to read g-buffer by unbinding it
	commandBuffer.EndRenderPass();
}

void RCNodeBasePass::Clear()
{
	AlbedoTex = nullptr;
	NormalTex = nullptr;
	RoughMetalTex = nullptr;
	IdTex = nullptr;
}

RCNodeBasePass::DependencyDefinition RCNodeBasePass::GetDependencyDefinition()
{
	static const DependencyDefinition kDependencyDefinition;
	return kDependencyDefinition;
}

void RCNodeSceneColor::Render(const RenderCompositorNodeInputs& inputs)
{
	GpuResourcePool& resPool = GetGpuResourcePool();
	const RendererViewProperties& viewProps = inputs.View.GetProperties();

	u32 width = viewProps.Target.ViewRect.Width;
	u32 height = viewProps.Target.ViewRect.Height;
	u32 numSamples = viewProps.Target.NumSamples;

	TextureUsageFlags usageFlags = TextureUsageFlag::RenderTarget;

	bool tiledDeferredSupported = inputs.FeatureSet != RenderBeastFeatureSet::DesktopMacOS;
	if(tiledDeferredSupported && numSamples == 1)
		usageFlags |= TextureUsageFlag::AllowUnorderedAccessOnTheGPU;

	// Note: Consider customizable HDR format via options? e.g. smaller PF_FLOAT_R11G11B10 or larger 32-bit format
	SceneColorTex = resPool.Get(PooledRenderTextureCreateInformation::Create2D(PF_RGBA16F, width, height, usageFlags, numSamples, false));

	auto dependencies = GetDependencyDefinition().ResolveDependencies(inputs);
	RCNodeSceneDepth* sceneDepthNode = dependencies.Get<RCNodeSceneDepth>();
	SPtr<PooledRenderTexture> sceneDepthTex = sceneDepthNode->DepthTex;

	if(tiledDeferredSupported && viewProps.Target.NumSamples > 1)
	{
		SceneColorTexArray = resPool.Get(PooledRenderTextureCreateInformation::Create2D(PF_RGBA16F, width, height, TextureUsageFlag::AllowUnorderedAccessOnTheGPU, 1, false, viewProps.Target.NumSamples));
	}
	else
		SceneColorTexArray = nullptr;

	bool rebuildRT = false;
	if(RenderTarget != nullptr)
	{
		rebuildRT |= RenderTarget->GetColorTexture(0) != SceneColorTex->Texture;
		rebuildRT |= RenderTarget->GetDepthStencilTexture() != sceneDepthTex->Texture;
	}
	else
		rebuildRT = true;

	if(rebuildRT)
	{
		RenderTextureCreateInformation sceneColorDesc;
		sceneColorDesc.ColorSurfaces[0].Texture = SceneColorTex->Texture;
		sceneColorDesc.ColorSurfaces[0].Face = 0;
		sceneColorDesc.ColorSurfaces[0].FaceCount = 1;
		sceneColorDesc.ColorSurfaces[0].MipLevel = 0;

		sceneColorDesc.DepthStencilSurface.Texture = sceneDepthTex->Texture;
		sceneColorDesc.DepthStencilSurface.Face = 0;
		sceneColorDesc.DepthStencilSurface.FaceCount = 1;
		sceneColorDesc.DepthStencilSurface.MipLevel = 0;

		RenderTarget = RenderTexture::Create(sceneColorDesc);
	}
}

void RCNodeSceneColor::Clear()
{
	SceneColorTex = nullptr;
	SceneColorTexArray = nullptr;
}

void RCNodeSceneColor::MsaaTexArrayToTexture(GpuCommandBuffer& commandBuffer)
{
	TextureArrayToMSAATexture* material = TextureArrayToMSAATexture::Get();
	material->Prepare(SceneColorTexArray->Texture, SceneColorTex->Texture);

	commandBuffer.BeginRenderPass(RenderPassCreateInformation(RenderTarget, material->GetGpuParameterSet(), RT_DEPTH_STENCIL, RT_DEPTH_STENCIL));

	Area2 area(0.0f, 0.0f, 1.0f, 1.0f);
	commandBuffer.SetViewport(area);

	material->Execute(commandBuffer, SceneColorTex->Texture);

	commandBuffer.EndRenderPass();
	SceneColorTexArray = nullptr;
}

void RCNodeSceneColor::Swap(RCNodeLightAccumulation* lightAccumNode)
{
	lightAccumNode->LightAccumulationTex.swap(SceneColorTex);
	lightAccumNode->LightAccumulationTexArray.swap(SceneColorTexArray);
	lightAccumNode->RenderTarget.swap(RenderTarget);
}

void RCNodeSceneColor::SetExternalTexture(const SPtr<PooledRenderTexture>& texture)
{
	B3D_ASSERT(SceneColorTexArray == nullptr);

	SceneColorTex = texture;
}

RCNodeSceneColor::DependencyDefinition RCNodeSceneColor::GetDependencyDefinition()
{
	static const DependencyDefinition kDependencyDefinition;
	return kDependencyDefinition;
}

void RCNodeMSAACoverage::Render(const RenderCompositorNodeInputs& inputs)
{
	const RendererViewProperties& viewProps = inputs.View.GetProperties();
	if(viewProps.Target.NumSamples <= 1)
	{
		// No need for MSAA coverage
		Output = nullptr;
		return;
	}

	u32 width = viewProps.Target.ViewRect.Width;
	u32 height = viewProps.Target.ViewRect.Height;

	// We just allocate the texture, while the base pass is responsible for filling it out
	Output = GetGpuResourcePool().Get(PooledRenderTextureCreateInformation::Create2D(PF_R8, width, height, TextureUsageFlag::RenderTarget));
}

void RCNodeMSAACoverage::Clear()
{
	Output = nullptr;
}

RCNodeMSAACoverage::DependencyDefinition RCNodeMSAACoverage::GetDependencyDefinition()
{
	static const DependencyDefinition kDependencyDefinition;
	return kDependencyDefinition;
}

void RCNodeParticleSimulate::Render(const RenderCompositorNodeInputs& inputs)
{
	// Only simulate particles for the first view in the main render pass
	if(inputs.ViewGroup.IsMainPass() && inputs.View.GetViewIdx() == 0)
	{
		auto dependencies = GetDependencyDefinition().ResolveDependencies(inputs);
		RCNodeBasePass* gbufferNode = dependencies.Get<RCNodeBasePass>();
		RCNodeSceneDepth* sceneDepthNode = dependencies.Get<RCNodeSceneDepth>();

		GBufferTextures gbuffer;
		gbuffer.Albedo = gbufferNode->AlbedoTex->Texture;
		gbuffer.Normals = gbufferNode->NormalTex->Texture;
		gbuffer.RoughMetal = gbufferNode->RoughMetalTex->Texture;
		gbuffer.Depth = sceneDepthNode->DepthTex->Texture;

		GpuParticleSimulation::Instance().Simulate(*inputs.ActiveCommandBuffer, inputs.Scene, inputs.FrameInfo.PerSceneFrameData.Particles, inputs.View.GetPerViewBuffer(), gbuffer, inputs.FrameInfo.Timings.TimeDelta);
	}

	GpuParticleSimulation::Instance().Sort(*inputs.ActiveCommandBuffer, inputs.View);
}

void RCNodeParticleSimulate::Clear()
{
	// Do nothing
}

RCNodeParticleSimulate::DependencyDefinition RCNodeParticleSimulate::GetDependencyDefinition()
{
	static const DependencyDefinition kDependencyDefinition;
	return kDependencyDefinition;
}

void RCNodeParticleSort::Render(const RenderCompositorNodeInputs& inputs)
{
	const EvaluatedParticleData* particleData = inputs.FrameInfo.PerSceneFrameData.Particles;
	if(!particleData)
		return;

	const RendererViewProperties& viewProps = inputs.View.GetProperties();
	const VisibilityInfo& visibility = inputs.View.GetVisibilityMasks();
	const auto numParticleSystems = (u32)inputs.Scene.ParticleSystems.size();

	// Sort particles
	B3DMarkAllocatorFrame();
	{
		struct SortData
		{
			ParticleSystem* System;
			ParticleRenderData* RenderData;
		};

		FrameVector<SortData> systemsToSort;
		for(u32 i = 0; i < numParticleSystems; i++)
		{
			if(!visibility.ParticleSystems[i])
				continue;

			const RendererParticles& rendererParticles = inputs.Scene.ParticleSystems[i];

			ParticleSystem* particleSystem = rendererParticles.ParticleSystem;
			const auto iterFind = particleData->CpuData.find(particleSystem->GetId());
			if(iterFind == particleData->CpuData.end())
				continue;

			ParticleRenderData* simulationData = iterFind->second;
			if(particleSystem->GetSettings().SortMode == ParticleSortMode::Distance)
				systemsToSort.push_back({ particleSystem, simulationData });
		}

		WaitGroup waitGroup((u32)systemsToSort.size());
		const auto fnSortWorker = [&waitGroup, viewOrigin = viewProps.ViewOrigin](const SortData& data)
		{
			Vector3 refPoint = viewOrigin;

			// Transform the view point into particle system's local space
			const ParticleSystemSettings& settings = data.System->GetSettings();
			if(settings.SimulationSpace == ParticleSimulationSpace::Local)
				refPoint = data.System->GetWorldTransform().GetInvMatrix().MultiplyAffine(refPoint);

			if(settings.RenderMode == ParticleRenderMode::Billboard)
			{
				auto renderData = static_cast<ParticleBillboardRenderData*>(data.RenderData);
				ParticleRenderer::SortByDistance(refPoint, renderData->PositionAndRotation, renderData->NumParticles, 4, renderData->Indices);
			}
			else
			{
				auto renderData = static_cast<ParticleMeshRenderData*>(data.RenderData);
				ParticleRenderer::SortByDistance(refPoint, renderData->Position, renderData->NumParticles, 3, renderData->Indices);
			}

			waitGroup.NotifyDone();
		};

		Scheduler& taskScheduler = GetApplication().GetTaskScheduler();
		for (const auto& data : systemsToSort)
		{
			taskScheduler.Post(SchedulerTask([&fnSortWorker, &data] { fnSortWorker(data); }, "ParticleSort"));
		}

		waitGroup.Wait();
	}
	B3DClearAllocatorFrame();
}

void RCNodeParticleSort::Clear()
{
	// Do nothing
}

RCNodeParticleSort::DependencyDefinition RCNodeParticleSort::GetDependencyDefinition()
{
	static const DependencyDefinition kDependencyDefinition;
	return kDependencyDefinition;
}

void RCNodeLightAccumulation::Render(const RenderCompositorNodeInputs& inputs)
{
	auto dependencies = GetDependencyDefinition().ResolveDependencies(inputs);

	bool supportsTiledDeferred = GetRenderBeast()->GetFeatureSet() != RenderBeastFeatureSet::DesktopMacOS;
	if(!supportsTiledDeferred)
	{
		// If tiled deferred is not supported, we don't need a separate texture for light accumulation, instead we
		// use scene color directly
		RCNodeSceneColor* sceneColorNode = dependencies.Get<RCNodeSceneColor>();
		LightAccumulationTex = sceneColorNode->SceneColorTex;
		RenderTarget = sceneColorNode->RenderTarget;

		return;
	}

	GpuResourcePool& resPool = GetGpuResourcePool();
	const RendererViewProperties& viewProps = inputs.View.GetProperties();

	RCNodeSceneDepth* depthNode = dependencies.Get<RCNodeSceneDepth>();

	u32 width = viewProps.Target.ViewRect.Width;
	u32 height = viewProps.Target.ViewRect.Height;
	u32 numSamples = viewProps.Target.NumSamples;

	TextureUsageFlags usage = TextureUsageFlag::RenderTarget;
	if(numSamples > 1)
	{
		resPool.Get(LightAccumulationTexArray, PooledRenderTextureCreateInformation::Create2D(PF_RGBA16F, width, height, TextureUsageFlag::AllowUnorderedAccessOnTheGPU, 1, false, numSamples));

		ClearLoadStoreMaterial* clearMat = ClearLoadStoreMaterial::GetVariation(ClearLoadStoreType::TextureArray, ClearLoadStoreDataType::Float, 4);

		for(u32 i = 0; i < numSamples; i++)
		{
			TextureSurface surface;
			surface.Face = i;
			surface.FaceCount = 1;
			surface.MipLevel = 0;
			surface.MipLevelCount = 1;

			clearMat->Execute(*inputs.ActiveCommandBuffer, LightAccumulationTexArray->Texture, Color::kZero, surface);
		}
	}
	else
	{
		usage |= TextureUsageFlag::AllowUnorderedAccessOnTheGPU;
		LightAccumulationTexArray = nullptr;
	}

	resPool.Get(LightAccumulationTex, PooledRenderTextureCreateInformation::Create2D(PF_RGBA16F, width, height, usage, numSamples, false));

	bool rebuildRT;
	if(RenderTarget != nullptr)
	{
		rebuildRT = RenderTarget->GetColorTexture(0) != LightAccumulationTex->Texture;
		rebuildRT |= RenderTarget->GetDepthStencilTexture() != depthNode->DepthTex->Texture;
	}
	else
		rebuildRT = true;

	if(rebuildRT)
	{
		RenderTextureCreateInformation lightAccumulationRTDesc;
		lightAccumulationRTDesc.ColorSurfaces[0].Texture = LightAccumulationTex->Texture;
		lightAccumulationRTDesc.ColorSurfaces[0].Face = 0;
		lightAccumulationRTDesc.ColorSurfaces[0].FaceCount = 1;
		lightAccumulationRTDesc.ColorSurfaces[0].MipLevel = 0;

		lightAccumulationRTDesc.DepthStencilSurface.Texture = depthNode->DepthTex->Texture;
		lightAccumulationRTDesc.DepthStencilSurface.Face = 0;
		lightAccumulationRTDesc.DepthStencilSurface.FaceCount = 1;
		lightAccumulationRTDesc.DepthStencilSurface.MipLevel = 0;

		RenderTarget = RenderTexture::Create(lightAccumulationRTDesc);
	}
}

void RCNodeLightAccumulation::MsaaTexArrayToTexture(GpuCommandBuffer& commandBuffer)
{
	TextureArrayToMSAATexture* material = TextureArrayToMSAATexture::Get();
	material->Prepare(LightAccumulationTexArray->Texture, LightAccumulationTex->Texture);
	
	commandBuffer.BeginRenderPass(RenderPassCreateInformation(RenderTarget, material->GetGpuParameterSet(), RT_DEPTH_STENCIL, RT_DEPTH_STENCIL));

	material->Execute(commandBuffer, LightAccumulationTex->Texture);

	commandBuffer.EndRenderPass();
}

void RCNodeLightAccumulation::Clear()
{
	RenderTarget = nullptr;
	LightAccumulationTex = nullptr;
	LightAccumulationTexArray = nullptr;
}

RCNodeLightAccumulation::DependencyDefinition RCNodeLightAccumulation::GetDependencyDefinition()
{
	auto fnSupportsTiledDeferred = [](const RendererView& view)
	{
		return GetRenderBeast()->GetFeatureSet() != RenderBeastFeatureSet::DesktopMacOS;
	};

	static const DependencyDefinition kDependencyDefinition = DependencyDefinition()
		.ConditionalEnable<RCNodeSceneColor>([fnSupportsTiledDeferred](const RendererView& view) { return !fnSupportsTiledDeferred(view); })
		.ConditionalEnable<RCNodeSceneDepth>(fnSupportsTiledDeferred);

	return kDependencyDefinition;
}

void RCNodeDeferredDirectLighting::Render(const RenderCompositorNodeInputs& inputs)
{
	auto dependencies = GetDependencyDefinition().ResolveDependencies(inputs);

	Output = dependencies.Get<RCNodeLightAccumulation>();

	auto gbufferNode = dependencies.Get<RCNodeBasePass>();
	auto sceneDepthNode = dependencies.Get<RCNodeSceneDepth>();
	auto sceneColorNode = dependencies.Get<RCNodeSceneColor>();

	GBufferTextures gbuffer;
	gbuffer.Albedo = gbufferNode->AlbedoTex->Texture;
	gbuffer.Normals = gbufferNode->NormalTex->Texture;
	gbuffer.RoughMetal = gbufferNode->RoughMetalTex->Texture;
	gbuffer.Depth = sceneDepthNode->DepthTex->Texture;

	GpuCommandBuffer& commandBuffer = *inputs.ActiveCommandBuffer;
	const RendererViewProperties& viewProps = inputs.View.GetProperties();

	if(!inputs.View.GetRenderSettings().EnableShadows)
		mLightOcclusionRT = nullptr;

	bool tiledDeferredSupported = inputs.FeatureSet != RenderBeastFeatureSet::DesktopMacOS;
	if(tiledDeferredSupported)
	{
		SPtr<Texture> msaaCoverage;
		if(viewProps.Target.NumSamples > 1)
		{
			RCNodeMSAACoverage* coverageNode = dependencies.Get<RCNodeMSAACoverage>();
			msaaCoverage = coverageNode->Output->Texture;
		}

		TiledDeferredLightingMaterial* tiledDeferredMat =
			TiledDeferredLightingMaterial::GetVariation(viewProps.Target.NumSamples);

		const VisibleLightData& lightData = inputs.ViewGroup.GetVisibleLightData();

		SPtr<Texture> lightAccumTexArray;
		if(Output->LightAccumulationTexArray)
			lightAccumTexArray = Output->LightAccumulationTexArray->Texture;

		tiledDeferredMat->Execute(commandBuffer, inputs.View, lightData, gbuffer, sceneColorNode->SceneColorTex->Texture, Output->LightAccumulationTex->Texture, lightAccumTexArray, msaaCoverage);

		if(viewProps.Target.NumSamples > 1)
			Output->MsaaTexArrayToTexture(commandBuffer);

		// If shadows are disabled we handle all lights through tiled deferred so we can exit immediately
		if(!inputs.View.GetRenderSettings().EnableShadows)
			return;
	}

	// Standard deferred used for shadowed lights, or when tiled deferred isn't supported
	u32 width = viewProps.Target.ViewRect.Width;
	u32 height = viewProps.Target.ViewRect.Height;
	u32 numSamples = viewProps.Target.NumSamples;

	const VisibleLightData& lightData = inputs.ViewGroup.GetVisibleLightData();

	// Render unshadowed lights
	if(!tiledDeferredSupported)
	{
		ProfileGPUBlock sampleBlock(commandBuffer, "Standard deferred unshadowed lights");

		// Collect all unshadowed lights
		Vector<const RendererLight*> unshadowedLights;
		for(u32 i = 0; i < (u32)LightType::Count; i++)
		{
			LightType lightType = (LightType)i;
			auto& lights = lightData.GetLights(lightType);
			u32 count = lightData.GetUnshadowedLightCount(lightType);

			for(u32 j = 0; j < count; j++)
				unshadowedLights.push_back(lights[j]);
		}

		// Prepare batch (groups lights and creates instanced uniform buffers)
		StandardDeferred::LightBatches baches = StandardDeferred::Instance().PrepareLightBatches(unshadowedLights, inputs.View, gbuffer, Texture::kBlack);

		// Begin render pass with pre-declared parameters
		RenderPassCreateInformation passInfo(Output->RenderTarget, RT_DEPTH_STENCIL, RT_DEPTH_STENCIL);
		for(const auto& [key, value] : baches.Batches)
			passInfo.Parameters.Add(value.GpuParameters);

		commandBuffer.BeginRenderPass(passInfo);

		// Render all lights using dynamic offsets
		StandardDeferred::Instance().RenderLightBatches(commandBuffer, baches);

		commandBuffer.EndRenderPass();
	}

	// Allocate light occlusion
	SPtr<PooledRenderTexture> lightOcclusionTex = GetGpuResourcePool().Get(
		PooledRenderTextureCreateInformation::Create2D(PF_R8, width, height, TextureUsageFlag::RenderTarget, numSamples, false));

	bool rebuildRT = false;
	if(mLightOcclusionRT != nullptr)
	{
		rebuildRT |= mLightOcclusionRT->GetColorTexture(0) != lightOcclusionTex->Texture;
		rebuildRT |= mLightOcclusionRT->GetDepthStencilTexture() != sceneDepthNode->DepthTex->Texture;
	}
	else
		rebuildRT = true;

	if(rebuildRT)
	{
		RenderTextureCreateInformation lightOcclusionRTDesc;
		lightOcclusionRTDesc.ColorSurfaces[0].Texture = lightOcclusionTex->Texture;
		lightOcclusionRTDesc.ColorSurfaces[0].Face = 0;
		lightOcclusionRTDesc.ColorSurfaces[0].FaceCount = 1;
		lightOcclusionRTDesc.ColorSurfaces[0].MipLevel = 0;

		lightOcclusionRTDesc.DepthStencilSurface.Texture = sceneDepthNode->DepthTex->Texture;
		lightOcclusionRTDesc.DepthStencilSurface.Face = 0;
		lightOcclusionRTDesc.DepthStencilSurface.FaceCount = 1;
		lightOcclusionRTDesc.DepthStencilSurface.MipLevel = 0;

		mLightOcclusionRT = RenderTexture::Create(lightOcclusionRTDesc);
	}

	// Render shadowed lights
	{
		ProfileGPUBlock sampleBlock(commandBuffer, "Standard deferred shadowed lights");

		const ShadowRendering& shadowRenderer = inputs.ViewGroup.GetShadowRenderer();
		for(u32 lightTypeIndex = 0; lightTypeIndex < (u32)LightType::Count; lightTypeIndex++)
		{
			const LightType lightType = (LightType)lightTypeIndex;

			auto& lights = lightData.GetLights(lightType);
			u32 lightCount = lightData.GetShadowedLightCount(lightType);
			u32 offset = lightData.GetUnshadowedLightCount(lightType);

			for(u32 shadowedLightIndex = 0; shadowedLightIndex < lightCount; shadowedLightIndex++)
			{
				u32 rendererLightId = offset + shadowedLightIndex;
				const RendererLight& light = *lights[rendererLightId];

				ShadowRendering::ProjectedShadowRenderingBatchInformation shadowProjectionRenderingBatch =
					shadowRenderer.PrepareParametersForRenderShadowProjection(commandBuffer.GetGpuDevice(), inputs.View, light, gbuffer);

				RenderPassCreateInformation shadowProjectionPassInfo(mLightOcclusionRT, RT_DEPTH, RT_DEPTH_STENCIL);
				for(const auto& shadowProjectionRenderingInfo : shadowProjectionRenderingBatch.Shadows)
				{
					shadowProjectionPassInfo.Parameters.Add(shadowProjectionRenderingInfo.PrimaryGpuParameters);

					if(shadowProjectionRenderingInfo.StencilGpuParameters != nullptr)
						shadowProjectionPassInfo.Parameters.Add(shadowProjectionRenderingInfo.StencilGpuParameters);
				}

				shadowProjectionPassInfo.ClearMask = RT_COLOR_ALL;
				shadowProjectionPassInfo.ClearColor = Color::kZero;

				commandBuffer.BeginRenderPass(shadowProjectionPassInfo);

				Area2 area(0.0f, 0.0f, 1.0f, 1.0f);
				commandBuffer.SetViewport(area);

				shadowRenderer.RenderShadowProjectionBatch(commandBuffer, inputs.View, light, shadowProjectionRenderingBatch);
				commandBuffer.EndRenderPass();

				StandardDeferred::LightBatches batches = StandardDeferred::Instance().PrepareLightBatches({ &light }, inputs.View, gbuffer, lightOcclusionTex->Texture);

				RenderPassCreateInformation lightingPassInfo(Output->RenderTarget, RT_DEPTH_STENCIL, RT_COLOR0 | RT_DEPTH_STENCIL);
				for(const auto& [key, value] : batches.Batches)
					lightingPassInfo.Parameters.Add(value.GpuParameters);

				commandBuffer.BeginRenderPass(lightingPassInfo);
				StandardDeferred::Instance().RenderLightBatches(commandBuffer, batches);
				commandBuffer.EndRenderPass();
			}
		}
	}
}

void RCNodeDeferredDirectLighting::Clear()
{
	Output = nullptr;
}

RCNodeDeferredDirectLighting::DependencyDefinition RCNodeDeferredDirectLighting::GetDependencyDefinition()
{
	static const DependencyDefinition kDependencyDefinition = []
	{
		return DependencyDefinition().ConditionalEnable<RCNodeMSAACoverage>([](const RendererView& view) { return view.GetProperties().Target.NumSamples > 1; });
	}();

	return kDependencyDefinition;
}

void RCNodeIndirectDiffuseLighting::Render(const RenderCompositorNodeInputs& inputs)
{
	if(!inputs.View.GetRenderSettings().EnableIndirectLighting)
		return;

	auto dependencies = GetDependencyDefinition().ResolveDependencies(inputs);
	RCNodeBasePass* gbufferNode = dependencies.Get<RCNodeBasePass>();
	RCNodeSceneDepth* sceneDepthNode = dependencies.Get<RCNodeSceneDepth>();
	RCNodeLightAccumulation* lightAccumNode = dependencies.Get<RCNodeLightAccumulation>();
	RCNodeSSAO* ssaoNode = dependencies.Get<RCNodeSSAO>();

	GpuCommandBuffer& commandBuffer = *inputs.ActiveCommandBuffer;
	GpuResourcePool& resPool = GetGpuResourcePool();
	const RendererViewProperties& viewProps = inputs.View.GetProperties();

	const LightProbes& lightProbes = inputs.Scene.LightProbes;
	LightProbesInfo lpInfo = lightProbes.GetInfo();

	IrradianceEvaluateMaterial* evaluateMat;
	SPtr<PooledRenderTexture> volumeIndices;
	if(lightProbes.HasAnyProbes())
	{
		PooledRenderTextureCreateInformation volumeIndicesDesc;
		PooledRenderTextureCreateInformation depthDesc;
		TetrahedraRenderMaterial::GetOutputDesc(inputs.View, volumeIndicesDesc, depthDesc);

		volumeIndices = resPool.Get(volumeIndicesDesc);
		SPtr<PooledRenderTexture> depthTex = resPool.Get(depthDesc);

		RenderTextureCreateInformation rtDesc;
		rtDesc.ColorSurfaces[0].Texture = volumeIndices->Texture;
		rtDesc.DepthStencilSurface.Texture = depthTex->Texture;

		SPtr<RenderTexture> rt = RenderTexture::Create(rtDesc);

		RenderPassCreateInformation clearRenderPassCreateInformation(rt);
		clearRenderPassCreateInformation.ClearMask = RT_DEPTH;

		commandBuffer.BeginRenderPass(clearRenderPassCreateInformation);
		GetRendererUtility().Clear(commandBuffer, -1);
		commandBuffer.EndRenderPass();

		TetrahedraRenderMaterial* renderTetrahedra =
			TetrahedraRenderMaterial::GetVariation(viewProps.Target.NumSamples > 1, true);
		renderTetrahedra->Prepare(inputs.View, sceneDepthNode->DepthTex->Texture);
		renderTetrahedra->Execute(commandBuffer, lpInfo.TetrahedraVolume, rt);

		rt = nullptr;
		depthTex = nullptr;

		evaluateMat = IrradianceEvaluateMaterial::GetVariation(viewProps.Target.NumSamples > 1, true, false);
	}
	else // Sky only
	{
		evaluateMat = IrradianceEvaluateMaterial::GetVariation(viewProps.Target.NumSamples > 1, true, true);
	}

	GBufferTextures gbuffer;
	gbuffer.Albedo = gbufferNode->AlbedoTex->Texture;
	gbuffer.Normals = gbufferNode->NormalTex->Texture;
	gbuffer.RoughMetal = gbufferNode->RoughMetalTex->Texture;
	gbuffer.Depth = sceneDepthNode->DepthTex->Texture;

	SPtr<Texture> volumeIndicesTex;
	if(volumeIndices)
		volumeIndicesTex = volumeIndices->Texture;

	Skybox* skybox = nullptr;
	if(inputs.View.GetRenderSettings().EnableSkybox)
		skybox = inputs.Scene.Skybox;

	evaluateMat->Execute(commandBuffer, inputs.View, gbuffer, volumeIndicesTex, lpInfo, skybox, ssaoNode->Output, lightAccumNode->RenderTarget);

	volumeIndices = nullptr;
}

void RCNodeIndirectDiffuseLighting::Clear()
{
	// Do nothing
}

RCNodeIndirectDiffuseLighting::DependencyDefinition RCNodeIndirectDiffuseLighting::GetDependencyDefinition()
{
	static const DependencyDefinition kDependencyDefinition;
	return kDependencyDefinition;
}

void RCNodeDeferredIndirectSpecularLighting::Render(const RenderCompositorNodeInputs& inputs)
{
	auto dependencies = GetDependencyDefinition().ResolveDependencies(inputs);
	RCNodeSceneColor* sceneColorNode = dependencies.Get<RCNodeSceneColor>();
	RCNodeBasePass* gbufferNode = dependencies.Get<RCNodeBasePass>();
	RCNodeSceneDepth* sceneDepthNode = dependencies.Get<RCNodeSceneDepth>();
	RCNodeLightAccumulation* lightAccumNode = dependencies.Get<RCNodeLightAccumulation>();
	RCNodeSSR* ssrNode = dependencies.Get<RCNodeSSR>();
	RCNodeSSAO* ssaoNode = dependencies.Get<RCNodeSSAO>();

	GpuCommandBuffer& commandBuffer = *inputs.ActiveCommandBuffer;

	GBufferTextures gbuffer;
	gbuffer.Albedo = gbufferNode->AlbedoTex->Texture;
	gbuffer.Normals = gbufferNode->NormalTex->Texture;
	gbuffer.RoughMetal = gbufferNode->RoughMetalTex->Texture;
	gbuffer.Depth = sceneDepthNode->DepthTex->Texture;

	const RendererViewProperties& viewProps = inputs.View.GetProperties();

	bool tiledDeferredSupported = inputs.FeatureSet != RenderBeastFeatureSet::DesktopMacOS;
	if(tiledDeferredSupported)
	{
		SPtr<Texture> msaaCoverage;
		if(viewProps.Target.NumSamples > 1)
		{
			RCNodeMSAACoverage* coverageNode = static_cast<RCNodeMSAACoverage*>(inputs.InputNodes[6]);
			msaaCoverage = coverageNode->Output->Texture;
		}

		TiledDeferredImageBasedLightingMaterial* material =
			TiledDeferredImageBasedLightingMaterial::GetVariation(viewProps.Target.NumSamples);

		TiledDeferredImageBasedLightingMaterial::Inputs iblInputs;
		iblInputs.Gbuffer = gbuffer;
		iblInputs.SceneColorTex = sceneColorNode->SceneColorTex->Texture;
		iblInputs.LightAccumulation = lightAccumNode->LightAccumulationTex->Texture;
		iblInputs.PreIntegratedGf = RendererTextures::preintegratedEnvGF;
		iblInputs.AmbientOcclusion = ssaoNode->Output;
		iblInputs.Ssr = ssrNode->Output;
		iblInputs.MsaaCoverage = msaaCoverage;

		if(sceneColorNode->SceneColorTexArray)
			iblInputs.SceneColorTexArray = sceneColorNode->SceneColorTexArray->Texture;

		material->Execute(*inputs.ActiveCommandBuffer, inputs.View, inputs.Scene, inputs.ViewGroup.GetVisibleReflProbeData(), iblInputs);

		if(viewProps.Target.NumSamples > 1)
			sceneColorNode->MsaaTexArrayToTexture(*inputs.ActiveCommandBuffer);
	}
	else // Standard deferred
	{
		SPtr<RenderTexture> outputRT = lightAccumNode->RenderTarget;

		u32 width = viewProps.Target.ViewRect.Width;
		u32 height = viewProps.Target.ViewRect.Height;
		u32 sampleCount = viewProps.Target.NumSamples;

		bool isMSAA = viewProps.Target.NumSamples > 1;

		SPtr<PooledRenderTexture> iblRadianceTex = GetGpuResourcePool().Get(
			PooledRenderTextureCreateInformation::Create2D(PF_RGBA16F, width, height, TextureUsageFlag::RenderTarget, sampleCount, false));

		RenderTextureCreateInformation renderTextureCreateInformation;
		renderTextureCreateInformation.ColorSurfaces[0].Texture = iblRadianceTex->Texture;
		renderTextureCreateInformation.DepthStencilSurface.Texture = sceneDepthNode->DepthTex->Texture;

		const GpuBufferSuballocation& perViewBuffer = inputs.View.GetPerViewBuffer();

		SPtr<RenderTexture> iblRadianceRT = RenderTexture::Create(renderTextureCreateInformation);

		const VisibleReflectionProbeData& probeData = inputs.ViewGroup.GetVisibleReflProbeData();

		Skybox* skybox = nullptr;
		if(inputs.View.GetRenderSettings().EnableSkybox)
			skybox = inputs.Scene.Skybox;

		DeferredIBLSetupMaterial* const setupMaterial = DeferredIBLSetupMaterial::GetVariation(isMSAA, true);
		DeferredIBLSkyMaterial* const skyMaterial = DeferredIBLSkyMaterial::GetVariation(isMSAA, true);

		DeferredIBLSetupMaterial* msaaSetupMaterial = nullptr;
		DeferredIBLSkyMaterial* msaaSkyMaterial = nullptr;
		if(isMSAA)
		{
			msaaSetupMaterial  = DeferredIBLSetupMaterial::GetVariation(true, false);
			msaaSkyMaterial = DeferredIBLSkyMaterial::GetVariation(true, false);
		}

		// Prepare buffers and parameters for rendering
		GpuBufferSuballocation reflProbeParamsBuffer = gGlobalReflectionProbeUniformBufferDefinition.AllocateTransient();
		RendererReflectionProbe::PopulateGlobalReflectionProbeUniformBuffer(reflProbeParamsBuffer, skybox, probeData.GetProbeCount(), inputs.Scene.ReflProbeCubemapsTex, viewProps.CapturingReflections);

		RenderPassCreateInformation mainPassCreateInformation(iblRadianceRT, RT_DEPTH_STENCIL, RT_DEPTH_STENCIL);
		{
			setupMaterial->Prepare(gbuffer, perViewBuffer, ssrNode->Output, ssaoNode->Output, reflProbeParamsBuffer);
			mainPassCreateInformation.Parameters.Add(setupMaterial->GetGpuParameterSet());

			if(isMSAA)
			{
				msaaSetupMaterial->Prepare(gbuffer, perViewBuffer, ssrNode->Output, ssaoNode->Output, reflProbeParamsBuffer);
				mainPassCreateInformation.Parameters.Add(msaaSetupMaterial->GetGpuParameterSet());
			}
		}

		SPtr<Texture> skyFilteredRadiance;
		if(skybox)
			skyFilteredRadiance = skybox->GetFilteredRadiance();

		TArray<StandardDeferred::ReflectionProbeRenderInformation> reflectionProbeRenderInformations;
		if(!viewProps.CapturingReflections)
		{
			reflectionProbeRenderInformations = StandardDeferred::Instance().PrepareReflectionProbes(commandBuffer.GetGpuDevice(), probeData, inputs.View, gbuffer, inputs.Scene, reflProbeParamsBuffer);

			for(const auto& entry : reflectionProbeRenderInformations)
				mainPassCreateInformation.Parameters.Add(entry.GpuParameters);

			if(skyFilteredRadiance)
			{
				skyMaterial->Prepare(gbuffer, perViewBuffer, skybox, reflProbeParamsBuffer);
				mainPassCreateInformation.Parameters.Add(skyMaterial->GetGpuParameterSet());

				if(isMSAA)
				{
					msaaSkyMaterial->Prepare(gbuffer, perViewBuffer, skybox, reflProbeParamsBuffer);
					mainPassCreateInformation.Parameters.Add(msaaSkyMaterial->GetGpuParameterSet());
				}
			}
		}

		commandBuffer.BeginRenderPass(mainPassCreateInformation);

		// Prepare the texture for refl. probe and skybox rendering
		{
			DeferredIBLSetupMaterial* const setupMaterial = DeferredIBLSetupMaterial::GetVariation(isMSAA, true);
			setupMaterial->Bind(commandBuffer);

			GetRendererUtility().DrawScreenQuad(commandBuffer);

			// Draw pixels requiring per-sample evaluation
			if(isMSAA)
			{
				DeferredIBLSetupMaterial* msaaSetupMaterial = DeferredIBLSetupMaterial::GetVariation(true, false);
				msaaSetupMaterial->Bind(commandBuffer);

				GetRendererUtility().DrawScreenQuad(commandBuffer);
			}
		}

		if(!viewProps.CapturingReflections)
		{
			// Render refl. probes
			StandardDeferred::Instance().RenderReflectionProbes(commandBuffer, reflectionProbeRenderInformations, inputs.View);

			// Render sky
			if(skyFilteredRadiance)
			{
				skyMaterial->Bind(commandBuffer);

				GetRendererUtility().DrawScreenQuad(commandBuffer);

				// Draw pixels requiring per-sample evaluation
				if(isMSAA)
				{
					msaaSkyMaterial->Bind(commandBuffer);

					GetRendererUtility().DrawScreenQuad(commandBuffer);
				}
			}
		}

		commandBuffer.EndRenderPass();

		// Finalize rendered reflections and output them to main render target
		{
			RenderPassCreateInformation finalizePassCreateInformation(outputRT, RT_DEPTH_STENCIL, RT_COLOR0 | RT_DEPTH_STENCIL);

			DeferredIBLFinalizeMaterial* const finalizeMaterial = DeferredIBLFinalizeMaterial::GetVariation(isMSAA, true);
			finalizeMaterial->Prepare(gbuffer, perViewBuffer, iblRadianceTex->Texture, RendererTextures::preintegratedEnvGF, reflProbeParamsBuffer);

			finalizePassCreateInformation.Parameters.Add(finalizeMaterial->GetGpuParameterSet());

			DeferredIBLFinalizeMaterial* msaaFinalizeMaterial = nullptr;

			if(isMSAA)
			{
				msaaFinalizeMaterial = DeferredIBLFinalizeMaterial::GetVariation(true, false);
				msaaFinalizeMaterial->Prepare(gbuffer, perViewBuffer, iblRadianceTex->Texture, RendererTextures::preintegratedEnvGF, reflProbeParamsBuffer);

				finalizePassCreateInformation.Parameters.Add(msaaFinalizeMaterial->GetGpuParameterSet());
			}

			commandBuffer.BeginRenderPass(finalizePassCreateInformation);

			finalizeMaterial->Bind(commandBuffer);

			GetRendererUtility().DrawScreenQuad(commandBuffer);

			// Draw pixels requiring per-sample evaluation
			if(isMSAA)
			{
				msaaFinalizeMaterial->Bind(commandBuffer);

				GetRendererUtility().DrawScreenQuad(commandBuffer);
			}

			commandBuffer.EndRenderPass();
		}
	}
}

void RCNodeDeferredIndirectSpecularLighting::Clear()
{
	Output = nullptr;
}

RCNodeDeferredIndirectSpecularLighting::DependencyDefinition RCNodeDeferredIndirectSpecularLighting::GetDependencyDefinition()
{
	static const DependencyDefinition kDependencyDefinition;
	return kDependencyDefinition;
}

void RCNodeClusteredForward::Render(const RenderCompositorNodeInputs& inputs)
{
	const SceneInfo& sceneInfo = inputs.Scene;
	const RendererViewProperties& viewProps = inputs.View.GetProperties();

	const VisibleLightData& visibleLightData = inputs.ViewGroup.GetVisibleLightData();
	const VisibleReflectionProbeData& visibleReflProbeData = inputs.ViewGroup.GetVisibleReflProbeData();

	LightGridOutputs lightGridOutputs;

	struct StandardForwardBuffers
	{
		GpuBufferSuballocation LightsUniformBuffer;
		GpuBufferSuballocation ReflProbesUniformBuffer;
		GpuBufferSuballocation LightAndReflProbeParamsUniformBuffer;
	} standardForwardBuffers;

	const bool supportsClusteredForward = GetRenderBeast()->GetFeatureSet() == RenderBeastFeatureSet::Desktop;
	if(supportsClusteredForward)
	{
		const LightGrid& lightGrid = inputs.View.GetLightGrid();
		lightGridOutputs = lightGrid.GetOutputs();
	}

	Skybox* skybox = nullptr;
	if(inputs.View.GetRenderSettings().EnableSkybox)
		skybox = sceneInfo.Skybox;

	// Prepare refl. probe param buffer
	GpuBufferSuballocation reflProbeParamBuffer = gGlobalReflectionProbeUniformBufferDefinition.AllocateTransient();
	RendererReflectionProbe::PopulateGlobalReflectionProbeUniformBuffer(reflProbeParamBuffer, skybox, visibleReflProbeData.GetProbeCount(), sceneInfo.ReflProbeCubemapsTex, viewProps.CapturingReflections);

	SPtr<Texture> skyFilteredRadiance;
	if(skybox)
		skyFilteredRadiance = skybox->GetFilteredRadiance();

	const auto fnBindClusteredForwardParameters = [&lightGridOutputs, &visibleLightData, &visibleReflProbeData](MaterialParameterAdapter& parameterAdapter, const ForwardLightingParams& fwdParams, const ImageBasedLightingParameterBinding& iblParams)
	{
		fwdParams.GridUniformBufferParameter.Set(lightGridOutputs.UniformBuffer);

		fwdParams.GridLightOffsetsAndSizeParameter.Set(lightGridOutputs.GridLightOffsetsAndSize);
		fwdParams.GridProbeOffsetsAndSizeParameter.Set(lightGridOutputs.GridProbeOffsetsAndSize);

		fwdParams.GridLightIndicesParameter.Set(lightGridOutputs.GridLightIndices);
		iblParams.ReflectionProbeIndicesParameter.Set(lightGridOutputs.GridProbeIndices);

		fwdParams.LightsBufferParameter.Set(visibleLightData.GetLightBuffer());
		iblParams.ReflectionProbesParameter.Set(visibleReflProbeData.GetProbeBuffer());
	};

	const auto fnBindStandardDeferredParameters = [&standardForwardBuffers, &visibleLightData, &visibleReflProbeData](MaterialParameterAdapter& parameterAdapter, const Bounds& bounds, const ForwardLightingParams& fwdParams, const ImageBasedLightingParameterBinding& iblParams)
	{
		// Allocate transient uniform buffers
		standardForwardBuffers.LightsUniformBuffer = gLightsUniformDefinition.AllocateTransient();
		standardForwardBuffers.ReflProbesUniformBuffer = gReflProbesUniformDefinition.AllocateTransient();
		standardForwardBuffers.LightAndReflProbeParamsUniformBuffer = gLightAndReflProbeParamsUniformDefinition.AllocateTransient();

		// Populate light & probe buffers
		Vector3I lightCounts;
		const LightData* lights[kStandardForwardMaxNumLights];
		visibleLightData.GatherInfluencingLights(bounds, lights, lightCounts);

		Vector4I lightOffsets;
		lightOffsets.X = lightCounts.X;
		lightOffsets.Y = lightCounts.X;
		lightOffsets.Z = lightOffsets.Y + lightCounts.Y;
		lightOffsets.W = lightOffsets.Z + lightCounts.Z;

		GpuBufferMappedScope lightsUniforms = standardForwardBuffers.LightsUniformBuffer.Map();
		GpuBufferMappedScope reflectionProbesUniforms = standardForwardBuffers.ReflProbesUniformBuffer.Map();
		GpuBufferMappedScope lightAndReflectionProbeUniforms = standardForwardBuffers.LightAndReflProbeParamsUniformBuffer.Map();

		for(i32 j = 0; j < lightOffsets.W; j++)
			gLightsUniformDefinition.gLights.Set(lightsUniforms, *lights[j], j);

		i32 numReflProbes = std::min(visibleReflProbeData.GetProbeCount(), kStandardForwardMaxNumProbes);
		for(i32 j = 0; j < numReflProbes; j++)
		{
			gReflProbesUniformDefinition.gReflectionProbes.Set(reflectionProbesUniforms, visibleReflProbeData.GetProbeData(j), j);
		}

		gLightAndReflProbeParamsUniformDefinition.gLightOffsets.Set(lightAndReflectionProbeUniforms, lightOffsets);
		gLightAndReflProbeParamsUniformDefinition.gReflProbeCount.Set(lightAndReflectionProbeUniforms, numReflProbes);

		iblParams.ReflectionProbesUniformBufferParameter.Set(standardForwardBuffers.ReflProbesUniformBuffer);
		fwdParams.LightsUniformBufferParameter.Set(standardForwardBuffers.LightsUniformBuffer);
		fwdParams.LightAndReflectionProbeUniformBufferParameter.Set(standardForwardBuffers.LightAndReflProbeParamsUniformBuffer);
	};

	const auto fnBindCommonIBLParameters = [&reflProbeParamBuffer, &skyFilteredRadiance, &sceneInfo](MaterialParameterAdapter& parameterAdapter, ImageBasedLightingParameterBinding& iblParams)
	{
		// Note: Ideally these should be bound once (they are the same for all renderables)
		iblParams.ReflectionProbeUniformBufferParameter.Set(reflProbeParamBuffer);

		iblParams.SkyReflectionsTexParam.Set(skyFilteredRadiance);
		iblParams.AmbientOcclusionTexParam.Set(Texture::kWhite); // Note: Add SSAO here?
		iblParams.SsrTexParameter.Set(Texture::kBlack); // Note: Add SSR here?

		iblParams.ReflectionProbeCubemapsTexParameter.Set(sceneInfo.ReflProbeCubemapsTex);
		iblParams.PreintegratedEnvBrdfParameter.Set(RendererTextures::preintegratedEnvGF);
	};

	// Prepare render target
	auto dependencies = GetDependencyDefinition().ResolveDependencies(inputs);
	auto sceneColorNode = dependencies.Get<RCNodeSceneColor>();
	auto sceneDepthNode = dependencies.Get<RCNodeSceneDepth>();
	auto resolvedSceneDepthNode = dependencies.Get<RCNodeResolvedSceneDepth>();

	bool rebuildRT;
	if(renderTarget != nullptr)
	{
		rebuildRT = renderTarget->GetColorTexture(0) != sceneColorNode->SceneColorTex->Texture;
		rebuildRT |= renderTarget->GetDepthStencilTexture() != sceneDepthNode->DepthTex->Texture;
	}
	else
		rebuildRT = true;

	if(rebuildRT)
	{
		RenderTextureCreateInformation rtDesc;
		rtDesc.ColorSurfaces[0].Texture = sceneColorNode->SceneColorTex->Texture;
		rtDesc.ColorSurfaces[0].Face = 0;
		rtDesc.ColorSurfaces[0].FaceCount = 1;
		rtDesc.ColorSurfaces[0].MipLevel = 0;

		rtDesc.DepthStencilSurface.Texture = sceneDepthNode->DepthTex->Texture;
		rtDesc.DepthStencilSurface.Face = 0;
		rtDesc.DepthStencilSurface.FaceCount = 1;
		rtDesc.DepthStencilSurface.MipLevel = 0;

		renderTarget = RenderTexture::Create(rtDesc);
	}

	// Prepare objects for rendering by binding forward lighting data
	//// Normal renderables
	const VisibilityInfo& visibility = inputs.View.GetVisibilityMasks();
	const auto numRenderables = (u32)sceneInfo.Renderables.size();
	for(u32 i = 0; i < numRenderables; i++)
	{
		if(!visibility.Renderables[i])
			continue;

		for(auto& element : sceneInfo.Renderables[i]->Elements)
		{
			ShaderFlags shaderFlags = element.Material->GetShader()->GetFlags();

			const bool useForwardRendering = shaderFlags.IsSet(ShaderFlag::Forward) || shaderFlags.IsSet(ShaderFlag::Transparent);
			if(!useForwardRendering)
				continue;

			// Note: It would be nice to be able to set this once and keep it, only updating if the buffers actually
			// change (e.g. when growing).
			if(supportsClusteredForward)
				fnBindClusteredForwardParameters(*element.ParameterAdapter, element.ForwardLightingParams, element.ImageBasedParams);
			else
			{
				// Populate light & probe buffers
				const Bounds& bounds = sceneInfo.RenderableCullInfos[i].Bounds;
				fnBindStandardDeferredParameters(*element.ParameterAdapter, bounds, element.ForwardLightingParams, element.ImageBasedParams);
			}

			fnBindCommonIBLParameters(*element.ParameterAdapter, element.ImageBasedParams);
		}
	}

	//// Particle systems
	const EvaluatedParticleData* particleData = inputs.FrameInfo.PerSceneFrameData.Particles;
	if(particleData)
	{
		const auto numParticleSystems = (u32)inputs.Scene.ParticleSystems.size();

		for(u32 i = 0; i < numParticleSystems; i++)
		{
			if(!visibility.ParticleSystems[i])
				continue;

			const RendererParticles& rendererParticles = inputs.Scene.ParticleSystems[i];
			ParticlesRenderElement& renderElement = rendererParticles.RenderElement;

			ShaderFlags shaderFlags = renderElement.Material->GetShader()->GetFlags();

			if(shaderFlags.IsSet(ShaderFlag::Transparent))
				renderElement.DepthInputTexture.Set(resolvedSceneDepthNode->Output->Texture);

			const bool requiresForwardLighting = shaderFlags.IsSet(ShaderFlag::Forward);
			if(!requiresForwardLighting)
				continue;

			if(!renderElement.IsValid())
				continue;

			// Note: It would be nice to be able to set this once and keep it, only updating if the buffers actually
			// change (e.g. when growing).
			if(supportsClusteredForward)
				fnBindClusteredForwardParameters(*renderElement.ParameterAdapter, renderElement.ForwardLightingParams, renderElement.ImageBasedParams);
			else
			{
				// Populate light & probe buffers
				const Bounds& bounds = sceneInfo.ParticleSystemCullInfos[i].Bounds;
				fnBindStandardDeferredParameters(*renderElement.ParameterAdapter, bounds, renderElement.ForwardLightingParams, renderElement.ImageBasedParams);
			}

			fnBindCommonIBLParameters(*renderElement.ParameterAdapter, renderElement.ImageBasedParams);
		}
	}

	// TODO: Forward pipeline rendering doesn't support shadows. In order to support this I'd have to render the light
	// occlusion for all lights affecting this object into a single (or a few) textures. I can likely use texture
	// arrays for this, or to avoid sampling many textures, perhaps just jam it all in one or few texture channels.

	// Render everything
	GpuCommandBuffer& commandBuffer = *inputs.ActiveCommandBuffer;

	RenderQueue* opaqueQueue = inputs.View.GetOpaqueQueue(true).get();
	RenderQueue* transparentQueue = inputs.View.GetTransparentQueue().get();

	// Collect all GpuParameters that will be used in the opaque pass
	const Vector<RenderQueueElement>& opaqueElements = opaqueQueue->GetSortedElements();
	RenderPassCreateInformation opaquePassInfo(renderTarget, RT_NONE, RT_ALL);
	PrepareRenderQueuePass(opaquePassInfo, opaqueElements);

	commandBuffer.BeginRenderPass(opaquePassInfo);
	RenderQueueElements(commandBuffer, opaqueElements);
	commandBuffer.EndRenderPass();

	// Collect all GpuParameters that will be used in the transparent pass
	const Vector<RenderQueueElement>& transparentElements = transparentQueue->GetSortedElements();
	RenderPassCreateInformation transparentPassInfo(renderTarget, RT_DEPTH, RT_ALL);
	PrepareRenderQueuePass(transparentPassInfo, transparentElements);

	commandBuffer.BeginRenderPass(transparentPassInfo);
	RenderQueueElements(commandBuffer, transparentElements);
	commandBuffer.EndRenderPass();

	// Note: Perhaps delay clearing this one frame, so previous frame textures have a better chance of being done
	ParticleRenderer::Instance().GetTexturePool().Clear();

	// Trigger post-lighting callbacks
	Camera* sceneCamera = inputs.View.GetSceneCamera();
	if(sceneCamera != nullptr)
	{
		inputs.View.NotifyCompositorTargetChangedInternal(renderTarget);

		for(auto& extension : inputs.ExtPostLighting)
		{
			RendererViewContext context;
			context.CurrentTarget = inputs.View.GetCompositorRenderTarget();
			context.CommandBuffer = inputs.ActiveCommandBuffer;

			// TODO - RenderPass
			extension->Render(*sceneCamera, context);
		}
	}
}

void RCNodeClusteredForward::Clear()
{
	// Do nothing
}

RCNodeClusteredForward::DependencyDefinition RCNodeClusteredForward::GetDependencyDefinition()
{
	static const DependencyDefinition kDependencyDefinition;
	return kDependencyDefinition;
}

void RCNodeSkybox::Render(const RenderCompositorNodeInputs& inputs)
{
	Skybox* skybox = nullptr;
	if(inputs.View.GetRenderSettings().EnableSkybox)
		skybox = inputs.Scene.Skybox;

	GpuCommandBuffer& commandBuffer = *inputs.ActiveCommandBuffer;
	SPtr<Texture> radiance = skybox ? skybox->GetTexture() : nullptr;

	SkyboxMaterial* material;
	if(radiance != nullptr)
	{
		material = SkyboxMaterial::GetVariation(false);
		material->Bind(commandBuffer, inputs.View.GetPerViewBuffer(), radiance, Color::kWhite);
	}
	else
	{
		// Cancel out the linear->SRGB conversion
		Color clearColor = inputs.View.GetProperties().Target.ClearColor.GetLinear();

		material = SkyboxMaterial::GetVariation(true);
		material->Bind(commandBuffer, inputs.View.GetPerViewBuffer(), nullptr, clearColor);
	}

	auto dependencies = GetDependencyDefinition().ResolveDependencies(inputs);
	RCNodeSceneColor* sceneColorNode = dependencies.Get<RCNodeSceneColor>();

	commandBuffer.BeginRenderPass(RenderPassCreateInformation(sceneColorNode->RenderTarget, material->GetGpuParameterSet(), RT_DEPTH_STENCIL, RT_COLOR0 | RT_DEPTH_STENCIL));

	Area2 area(0.0f, 0.0f, 1.0f, 1.0f);
	commandBuffer.SetViewport(area);

	SPtr<Mesh> mesh = GetRendererUtility().GetSkyBoxMesh();
	GetRendererUtility().Draw(commandBuffer, mesh, mesh->GetProperties().SubMeshes[0]);

	commandBuffer.EndRenderPass();
}

void RCNodeSkybox::Clear()
{}

RCNodeSkybox::DependencyDefinition RCNodeSkybox::GetDependencyDefinition()
{
	static const DependencyDefinition kDependencyDefinition;
	return kDependencyDefinition;
}

void RCNodeFinalResolve::Render(const RenderCompositorNodeInputs& inputs)
{
	auto dependencies = GetDependencyDefinition().ResolveDependencies(inputs);
	const RendererViewProperties& viewProperties = inputs.View.GetProperties();

	SPtr<Texture> input;
	if(viewProperties.RunPostProcessing)
	{
		RCNodePostProcess* postProcessNode = dependencies.Get<RCNodePostProcess>();

		// Note: Ideally the last PP effect could write directly to the final target and we could avoid this copy
		input = postProcessNode->GetLastOutput();
	}
	else
	{
		RCNodeSceneColor* sceneColorNode = dependencies.Get<RCNodeSceneColor>();
		input = sceneColorNode->SceneColorTex->Texture;
	}

	SPtr<RenderTarget> target = viewProperties.Target.Target;

	GpuCommandBuffer& commandBuffer = *inputs.ActiveCommandBuffer;

	BlitInformation blitInformation = BlitInformation::BlitColor(input, target);
	blitInformation.OutputArea = viewProperties.Target.NrmViewRect;
	blitInformation.FlipUV = viewProperties.FlipView;

	GetRendererUtility().Blit(commandBuffer, blitInformation);

	if(viewProperties.EncodeDepth)
	{
		RCNodeResolvedSceneDepth* resolvedSceneDepthNode = dependencies.Get<RCNodeResolvedSceneDepth>();

		EncodeDepthMaterial* const encodeDepthMat = EncodeDepthMaterial::Get();
		encodeDepthMat->Prepare(resolvedSceneDepthNode->Output->Texture, viewProperties.DepthEncodeNear, viewProperties.DepthEncodeFar);
		encodeDepthMat->Execute(commandBuffer, target);
	}

	// Trigger overlay callbacks
	Camera* sceneCamera = inputs.View.GetSceneCamera();
	if(sceneCamera != nullptr)
	{
		inputs.View.NotifyCompositorTargetChangedInternal(target);

		for(auto& extension : inputs.ExtOverlay)
		{
			RendererViewContext context;
			context.CurrentTarget = inputs.View.GetCompositorRenderTarget();
			context.CommandBuffer = inputs.ActiveCommandBuffer;

			// TODO - RenderPass
			extension->Render(*sceneCamera, context);
		}
	}

	// Process pending frame captures (after overlays, before clearing target)
	inputs.View.ResolveSceneCaptures(*inputs.ActiveCommandBuffer, input);

	inputs.View.NotifyCompositorTargetChangedInternal(nullptr);
}

void RCNodeFinalResolve::Clear()
{}

RCNodeFinalResolve::DependencyDefinition RCNodeFinalResolve::GetDependencyDefinition()
{
	auto fnRunPostProcessing = [](const RendererView& view)
	{
		return view.GetProperties().RunPostProcessing;
	};

	auto fnEncodeDepth = [](const RendererView& view)
	{
		return view.GetProperties().EncodeDepth;
	};

	static const DependencyDefinition kDependencyDefinition = DependencyDefinition()
		.ConditionalEnable<RCNodePostProcess>(fnRunPostProcessing)
		.ConditionalEnable<RCNodeFilmGrain>(fnRunPostProcessing)
		.ConditionalEnable<RCNodeSceneColor>([fnRunPostProcessing](const RendererView& view) { return !fnRunPostProcessing(view); })
		.ConditionalEnable<RCNodeClusteredForward>([fnRunPostProcessing](const RendererView& view) { return !fnRunPostProcessing(view); })
		.ConditionalEnable<RCNodeResolvedSceneDepth>(fnEncodeDepth);

	return kDependencyDefinition;
}

void RCNodePostProcess::GetAndSwitch(const RendererView& view, SPtr<RenderTexture>& output, SPtr<Texture>& lastFrame) const
{
	const RendererViewProperties& viewProps = view.GetProperties();
	u32 width = viewProps.Target.ViewRect.Width;
	u32 height = viewProps.Target.ViewRect.Height;

	if(!mOutput[mCurrentIdx])
	{
		mOutput[mCurrentIdx] = GetGpuResourcePool().Get(
			PooledRenderTextureCreateInformation::Create2D(PF_RGBA8, width, height, TextureUsageFlag::RenderTarget, 1, false));
	}

	output = mOutput[mCurrentIdx]->RenderTexture;

	u32 otherIdx = (mCurrentIdx + 1) % 2;
	if(mOutput[otherIdx])
		lastFrame = mOutput[otherIdx]->Texture;

	mCurrentIdx = otherIdx;
}

SPtr<Texture> RCNodePostProcess::GetLastOutput() const
{
	u32 otherIdx = (mCurrentIdx + 1) % 2;
	if(mOutput[otherIdx])
		return mOutput[otherIdx]->Texture;

	return nullptr;
}

void RCNodePostProcess::Render(const RenderCompositorNodeInputs& inputs)
{
	// Do nothing, this is just a helper node
}

void RCNodePostProcess::Clear()
{
	mOutput[0] = nullptr;
	mOutput[1] = nullptr;
	mCurrentIdx = 0;
}

RCNodePostProcess::DependencyDefinition RCNodePostProcess::GetDependencyDefinition()
{
	static const DependencyDefinition kDependencyDefinition;
	return kDependencyDefinition;
}

void RCNodeEyeAdaptation::Render(const RenderCompositorNodeInputs& inputs)
{
	auto dependencies = GetDependencyDefinition().ResolveDependencies(inputs);
	GpuResourcePool& resPool = GetGpuResourcePool();

	GpuCommandBuffer& commandBuffer = *inputs.ActiveCommandBuffer;
	const RenderSettings& settings = inputs.View.GetRenderSettings();

	const bool hdr = settings.EnableHdr;

	if(hdr && settings.EnableAutoExposure)
	{
		// Get downsample scene
		auto* halfSceneColorNode = dependencies.Get<RCNodeHalfSceneColor>();
		const SPtr<PooledRenderTexture>& downsampledScene = halfSceneColorNode->Output;

		if(UseHistogramEyeAdapatation(inputs))
		{
			// Generate histogram
			SPtr<PooledRenderTexture> eyeAdaptHistogram =
				resPool.Get(EyeAdaptHistogramMaterial::GetOutputDesc(downsampledScene->Texture));
			EyeAdaptHistogramMaterial* eyeAdaptHistogramMat = EyeAdaptHistogramMaterial::Get();
			eyeAdaptHistogramMat->Execute(commandBuffer, downsampledScene->Texture, eyeAdaptHistogram->Texture, settings.AutoExposure);

			// Reduce histogram
			SPtr<PooledRenderTexture> reducedHistogram = resPool.Get(EyeAdaptHistogramReduceMaterial::GetOutputDesc());

			SPtr<Texture> prevFrameEyeAdaptation;
			if(previous != nullptr)
				prevFrameEyeAdaptation = previous->Texture;

			EyeAdaptHistogramReduceMaterial* eyeAdaptHistogramReduce = EyeAdaptHistogramReduceMaterial::Get();
			eyeAdaptHistogramReduce->Prepare(downsampledScene->Texture, eyeAdaptHistogram->Texture, prevFrameEyeAdaptation);
			eyeAdaptHistogramReduce->Execute(commandBuffer, reducedHistogram->RenderTexture);

			eyeAdaptHistogram = nullptr;

			// Generate eye adaptation value
			Output = resPool.Get(EyeAdaptationMaterial::GetOutputDesc());
			EyeAdaptationMaterial* eyeAdaptationMat = EyeAdaptationMaterial::Get();
			eyeAdaptationMat->Prepare(
				reducedHistogram->Texture,
				inputs.FrameInfo.Timings.TimeDelta,
				settings.AutoExposure,
				settings.ExposureScale);
			eyeAdaptationMat->Execute(commandBuffer, Output->RenderTexture);
		}
		else
		{
			// Populate alpha values of the downsampled texture with luminance
			SPtr<PooledRenderTexture> luminanceTex =
				resPool.Get(EyeAdaptationBasicSetupMaterial::GetOutputDesc(downsampledScene->Texture));

			EyeAdaptationBasicSetupMaterial* setupMat = EyeAdaptationBasicSetupMaterial::Get();
			setupMat->Prepare(
				downsampledScene->Texture,
				inputs.FrameInfo.Timings.TimeDelta,
				settings.AutoExposure,
				settings.ExposureScale);
			setupMat->Execute(commandBuffer, luminanceTex->RenderTexture);

			SPtr<Texture> downsampleInput = luminanceTex->Texture;
			luminanceTex = nullptr;

			// Downsample some more
			for(u32 i = 0; i < 5; i++)
			{
				DownsampleMaterial* downsampleMat = DownsampleMaterial::GetVariation(1, false);
				SPtr<PooledRenderTexture> downsampledLuminance =
					resPool.Get(DownsampleMaterial::GetOutputDesc(downsampleInput));

				downsampleMat->Execute(commandBuffer, downsampleInput, downsampledLuminance->RenderTexture);
				downsampleInput = downsampledLuminance->Texture;
			}

			// Generate eye adaptation value
			EyeAdaptationBasicMaterial* eyeAdaptationMat = EyeAdaptationBasicMaterial::Get();

			SPtr<Texture> prevFrameEyeAdaptation;
			if(previous != nullptr)
				prevFrameEyeAdaptation = previous->Texture;

			Output = resPool.Get(EyeAdaptationBasicMaterial::GetOutputDesc());
			eyeAdaptationMat->Prepare(
				downsampleInput,
				prevFrameEyeAdaptation,
				inputs.FrameInfo.Timings.TimeDelta,
				settings.AutoExposure,
				settings.ExposureScale);
			eyeAdaptationMat->Execute(commandBuffer, Output->RenderTexture);
		}

		const RendererView& view = inputs.View;

		// Notify the view eye adaptation value will change
		view.NotifyLuminanceUpdated(inputs.FrameInfo.Timings.FrameIndex, inputs.ActiveCommandBuffer, Output);
	}
	else
	{
		previous = nullptr;
		Output = nullptr;
	}
}

void RCNodeEyeAdaptation::Clear()
{
	std::swap(Output, previous);
	Output = nullptr;
}

bool RCNodeEyeAdaptation::UseHistogramEyeAdapatation(const RenderCompositorNodeInputs& inputs)
{
	return inputs.FeatureSet == RenderBeastFeatureSet::Desktop;
}

RCNodeEyeAdaptation::DependencyDefinition RCNodeEyeAdaptation::GetDependencyDefinition()
{
	auto fnEnableAutoExposure = [](const RendererView& view)
	{
		const RenderSettings& settings = view.GetRenderSettings();
		return settings.EnableHdr && settings.EnableAutoExposure;
	};

	static const DependencyDefinition kDependencyDefinition = DependencyDefinition()
		.ConditionalEnable<RCNodeHalfSceneColor>(fnEnableAutoExposure);

	return kDependencyDefinition;
}

void RCNodeTonemapping::Render(const RenderCompositorNodeInputs& inputs)
{
	auto dependencies = GetDependencyDefinition().ResolveDependencies(inputs);

	GpuCommandBuffer& commandBuffer = *inputs.ActiveCommandBuffer;
	const RenderSettings& settings = inputs.View.GetRenderSettings();

	auto* eyeAdaptationNode = dependencies.Get<RCNodeEyeAdaptation>();
	auto* sceneColorNode = dependencies.Get<RCNodeSceneColor>();
	auto* postProcessNode = dependencies.Get<RCNodePostProcess>();
	const SPtr<Texture>& sceneColor = sceneColorNode->SceneColorTex->Texture;

	const bool hdr = settings.EnableHdr;
	const bool msaa = sceneColor->GetProperties().SampleCount > 1;

	const bool volumeLUT = inputs.FeatureSet == RenderBeastFeatureSet::Desktop;
	bool gammaOnly;
	bool autoExposure;
	if(hdr)
	{
		if(settings.EnableTonemapping)
		{
			const u64 latestHash = inputs.View.GetRenderSettingsHash();
			const bool tonemapLUTDirty = mTonemapLastUpdateHash != latestHash;

			if(tonemapLUTDirty) // Rebuild LUT if PP settings changed
			{
				if(volumeLUT)
				{
					CreateTonemap3DLUTMaterial* createLUT = CreateTonemap3DLUTMaterial::Get();
					if(mTonemapLUT == nullptr)
						mTonemapLUT = GetGpuResourcePool().Get(createLUT->GetOutputDesc());

					createLUT->Execute(commandBuffer, mTonemapLUT->Texture, settings);
				}
				else
				{
					CreateTonemap2DLUTMaterial* createLUT = CreateTonemap2DLUTMaterial::Get();
					if(mTonemapLUT == nullptr)
						mTonemapLUT = GetGpuResourcePool().Get(createLUT->GetOutputDesc());

					createLUT->Prepare(settings);
					createLUT->Execute(commandBuffer, mTonemapLUT->RenderTexture);
				}

				mTonemapLastUpdateHash = latestHash;
			}

			gammaOnly = false;
		}
		else
			gammaOnly = true;

		autoExposure = settings.EnableAutoExposure;
	}
	else
	{
		gammaOnly = true;
		autoExposure = false;
	}

	if(gammaOnly)
		mTonemapLUT = nullptr;

	TonemappingMaterial* tonemapping = TonemappingMaterial::GetVariation(volumeLUT, gammaOnly, autoExposure, msaa);

	SPtr<RenderTexture> ppOutput;
	SPtr<Texture> ppLastFrame;
	postProcessNode->GetAndSwitch(inputs.View, ppOutput, ppLastFrame);

	SPtr<Texture> eyeAdaptationTex;
	if(eyeAdaptationNode->Output)
		eyeAdaptationTex = eyeAdaptationNode->Output->Texture;

	SPtr<Texture> tonemapLUTTex;
	if(mTonemapLUT)
		tonemapLUTTex = mTonemapLUT->Texture;

	SPtr<Texture> bloomTex;
	if(settings.Bloom.Enabled)
	{
		auto* bloomNode = dependencies.Get<RCNodeBloom>();
		bloomTex = bloomNode->Output;
	}

	tonemapping->Prepare(sceneColor, eyeAdaptationTex, bloomTex, tonemapLUTTex, settings);
	tonemapping->Execute(commandBuffer, ppOutput);
}

void RCNodeTonemapping::Clear()
{
	// Do nothing
}

RCNodeTonemapping::DependencyDefinition RCNodeTonemapping::GetDependencyDefinition()
{
	static const DependencyDefinition kDependencyDefinition = []
	{
		return DependencyDefinition()
			.ConditionalEnable<RCNodeBloom>([](const RendererView& view) { return view.GetRenderSettings().Bloom.Enabled; }) 
			.ConditionalEnable<RCNodeScreenSpaceLensFlare>([](const RendererView& view) { return view.GetRenderSettings().ScreenSpaceLensFlare.Enabled; });
	}();

	return kDependencyDefinition;
}

void RCNodeBokehDOF::Render(const RenderCompositorNodeInputs& inputs)
{
	auto dependencies = GetDependencyDefinition().ResolveDependencies(inputs);
	const DepthOfFieldSettings& settings = inputs.View.GetRenderSettings().DepthOfField;
	if(!settings.Enabled || settings.Type != DepthOfFieldType::Bokeh)
		return;

	GpuCommandBuffer& commandBuffer = *inputs.ActiveCommandBuffer;
	const RendererViewProperties& viewProps = inputs.View.GetProperties();
	const bool msaa = viewProps.Target.NumSamples > 1;

	RCNodeSceneColor* sceneColorNode = dependencies.Get<RCNodeSceneColor>();
	RCNodeSceneDepth* sceneDepthNode = dependencies.Get<RCNodeSceneDepth>();
	RCNodeLightAccumulation* lightAccumNode = dependencies.Get<RCNodeLightAccumulation>();

	BokehDOFPrepareMaterial* prepareMat = BokehDOFPrepareMaterial::GetVariation(msaa);
	BokehDOFMaterial* renderMat = BokehDOFMaterial::GetVariation(settings.BokehOcclusion);
	BokehDOFCombineMaterial* combineMat = BokehDOFCombineMaterial::GetVariation(msaa ? MSAAMode::Full : MSAAMode::None);

	SPtr<Texture> depth = sceneDepthNode->DepthTex->Texture;

	// Downsample scene and store depth in .w
	SPtr<PooledRenderTexture> halfResSceneAndDepth =
		GetGpuResourcePool().Get(BokehDOFPrepareMaterial::GetOutputDesc(sceneColorNode->SceneColorTex->Texture));

	prepareMat->Prepare(sceneColorNode->SceneColorTex->Texture, depth, inputs.View, settings);
	prepareMat->Execute(commandBuffer, halfResSceneAndDepth->RenderTexture);

	SPtr<PooledRenderTexture> unfocusedTex =
		GetGpuResourcePool().Get(BokehDOFMaterial::GetOutputDesc(halfResSceneAndDepth->Texture));

	renderMat->Prepare(halfResSceneAndDepth->Texture, inputs.View, settings, unfocusedTex->RenderTexture);
	renderMat->Execute(commandBuffer, halfResSceneAndDepth->Texture, unfocusedTex->RenderTexture);
	halfResSceneAndDepth = nullptr;

	// Combine the unfocused and focused textures to form the final image
	combineMat->Prepare(unfocusedTex->Texture, sceneColorNode->SceneColorTex->Texture, depth, inputs.View, settings);
	combineMat->Execute(commandBuffer, lightAccumNode->LightAccumulationTex->RenderTexture);

	// TODO - This might be incorrect when not supporting tiled deferred? As light accum is the same as scene color
	sceneColorNode->Swap(lightAccumNode);
}

void RCNodeBokehDOF::Clear()
{
	// Do nothing
}

RCNodeBokehDOF::DependencyDefinition RCNodeBokehDOF::GetDependencyDefinition()
{
	static const DependencyDefinition kDependencyDefinition;
	return kDependencyDefinition;
}

RCNodeTemporalAA::~RCNodeTemporalAA()
{
	DeallocOutputs();
}

void RCNodeTemporalAA::Render(const RenderCompositorNodeInputs& inputs)
{
	auto dependencies = GetDependencyDefinition().ResolveDependencies(inputs);
	auto* sceneColorNode = dependencies.Get<RCNodeSceneColor>();
	SPtr<PooledRenderTexture> sceneColor = sceneColorNode->SceneColorTex;

	const TemporalAASettings& settings = inputs.View.GetRenderSettings().TemporalAa;
	if(!settings.Enabled)
	{
		DeallocOutputs();

		mPooledOutput = nullptr;
		Output = sceneColor->Texture;
		return;
	}

	GpuCommandBuffer& commandBuffer = *inputs.ActiveCommandBuffer;

	// TODO - Resolve scene color MSAA (in a way that can be shared by multiple effects)

	auto* sceneDepthNode = dependencies.Get<RCNodeSceneDepth>();
	auto* basePassNode = dependencies.Get<RCNodeBasePass>();

	GpuResourcePool& resPool = GetGpuResourcePool();
	const RendererViewProperties& viewProps = inputs.View.GetProperties();

	u32 width = viewProps.Target.ViewRect.Width;
	u32 height = viewProps.Target.ViewRect.Height;

	// Resolve multiple samples if MSAA is used
	SPtr<PooledRenderTexture> resolvedSceneColor;
	if(viewProps.Target.NumSamples > 1)
	{
		resolvedSceneColor = resPool.Get(PooledRenderTextureCreateInformation::Create2D(PF_RGBA16F, width, height, TextureUsageFlag::RenderTarget));

		GetRendererUtility().Blit(commandBuffer, BlitInformation::BlitColor(sceneColor->Texture, resolvedSceneColor->RenderTexture));
		sceneColor = resolvedSceneColor;
	}

	if(mPrevFrame)
	{
		mPooledOutput = resPool.Get(PooledRenderTextureCreateInformation::Create2D(PF_RGBA16F, width, height, TextureUsageFlag::RenderTarget));

		SPtr<Texture> velocityTex;
		if(basePassNode->VelocityTex)
			velocityTex = basePassNode->VelocityTex->Texture;

		float exposure = inputs.View.GetCurrentExposure();

		TemporalFilteringMaterial* temporalFilteringMat =
			TemporalFilteringMaterial::GetVariation(TemporalFilteringType::FullScreenAA, true, viewProps.Target.NumSamples > 1);
		temporalFilteringMat->Prepare(inputs.View, mPrevFrame->Texture, sceneColor->Texture, velocityTex, sceneDepthNode->DepthTex->Texture, viewProps.TemporalJitter, exposure);
		temporalFilteringMat->Execute(commandBuffer, inputs.View, mPooledOutput->RenderTexture);

		sceneColorNode->SetExternalTexture(mPooledOutput);
	}
	else
		mPooledOutput = sceneColor;

	Output = mPooledOutput->Texture;
}

void RCNodeTemporalAA::Clear()
{
	mPrevFrame = mPooledOutput;
	mPooledOutput = nullptr;
	Output = nullptr;
}

void RCNodeTemporalAA::DeallocOutputs()
{
	mPrevFrame = nullptr;
	Output = nullptr;
}

RCNodeTemporalAA::DependencyDefinition RCNodeTemporalAA::GetDependencyDefinition()
{
	static const DependencyDefinition kDependencyDefinition;
	return kDependencyDefinition;
}

void RCNodeMotionBlur::Render(const RenderCompositorNodeInputs& inputs)
{
	const MotionBlurSettings& settings = inputs.View.GetRenderSettings().MotionBlur;
	if(!settings.Enabled)
		return;

	// TODO - Account for settings such as filter type and domain by grabbing correct shader variations

	// TODO - WIP
	// const RendererViewProperties& viewProps = inputs.view.getProperties();

	// RCNodeSceneColor* sceneColorNode = static_cast<RCNodeSceneColor*>(inputs.inputNodes[1]);
	// RCNodeSceneDepth* sceneDepthNode = static_cast<RCNodeSceneDepth*>(inputs.inputNodes[2]);
	// RCNodeLightAccumulation* lightAccumNode = static_cast<RCNodeLightAccumulation*>(inputs.inputNodes[3]);

	// MotionBlurMaterial* motionBlurMat = MotionBlurMaterial::Get();

	// SPtr<Texture> depth = sceneDepthNode->depthTex->texture;
	// motionBlurMat->Prepare(sceneColorNode->sceneColorTex->texture, depth, inputs.view, settings);
	// motionBlurMat->Execute(commandBuffer, lightAccumNode->lightAccumulationTex->renderTexture);

	// sceneColorNode->swap(lightAccumNode);
}

void RCNodeMotionBlur::Clear()
{
	// Do nothing
}

RCNodeMotionBlur::DependencyDefinition RCNodeMotionBlur::GetDependencyDefinition()
{
	static const DependencyDefinition kDependencyDefinition;
	return kDependencyDefinition;
}

void RCNodeGaussianDOF::Render(const RenderCompositorNodeInputs& inputs)
{
	auto dependencies = GetDependencyDefinition().ResolveDependencies(inputs);
	GpuCommandBuffer& commandBuffer = *inputs.ActiveCommandBuffer;
	RCNodeSceneDepth* sceneDepthNode = dependencies.Get<RCNodeSceneDepth>();
	RCNodePostProcess* postProcessNode = dependencies.Get<RCNodePostProcess>();

	const DepthOfFieldSettings& settings = inputs.View.GetRenderSettings().DepthOfField;
	bool near = settings.NearBlurAmount > 0.0f;
	bool far = settings.FarBlurAmount > 0.0f;

	bool enabled = settings.Enabled && settings.Type == DepthOfFieldType::Gaussian && (near || far);
	if(!enabled)
		return;

	GaussianDOFSeparateMaterial* separateMat = GaussianDOFSeparateMaterial::GetVariation(near, far);
	GaussianDOFCombineMaterial* combineMat = GaussianDOFCombineMaterial::GetVariation(near, far);
	GaussianBlurMaterial* blurMat = GaussianBlurMaterial::Get();

	SPtr<RenderTexture> ppOutput;
	SPtr<Texture> ppLastFrame;
	postProcessNode->GetAndSwitch(inputs.View, ppOutput, ppLastFrame);

	separateMat->Prepare(ppLastFrame, sceneDepthNode->DepthTex->Texture, inputs.View, settings);
	separateMat->Execute(commandBuffer, ppLastFrame);

	SPtr<PooledRenderTexture> nearTex, farTex;
	if(near && far)
	{
		nearTex = separateMat->GetOutput(0);
		farTex = separateMat->GetOutput(1);
	}
	else
	{
		if(near)
			nearTex = separateMat->GetOutput(0);
		else
			farTex = separateMat->GetOutput(0);
	}

	// Blur the out of focus pixels
	// Note: Perhaps set up stencil so I can avoid performing blur on unused parts of the textures?
	const TextureProperties& texProps = nearTex ? nearTex->Texture->GetProperties() : farTex->Texture->GetProperties();
	PooledRenderTextureCreateInformation tempTexDesc = PooledRenderTextureCreateInformation::Create2D(texProps.Format, texProps.Width, texProps.Height, TextureUsageFlag::RenderTarget);
	SPtr<PooledRenderTexture> tempTexture = GetGpuResourcePool().Get(tempTexDesc);

	SPtr<Texture> blurredNearTex;
	if(nearTex)
	{
		blurMat->Execute(commandBuffer, nearTex->Texture, settings.NearBlurAmount, tempTexture->RenderTexture);
		blurredNearTex = tempTexture->Texture;
	}

	SPtr<Texture> blurredFarTex;
	if(farTex)
	{
		// If temporary texture is used up, re-use the original near texture for the blurred result
		if(blurredNearTex)
		{
			blurMat->Execute(commandBuffer, farTex->Texture, settings.FarBlurAmount, nearTex->RenderTexture);
			blurredFarTex = nearTex->Texture;
		}
		else // Otherwise just use the temporary
		{
			blurMat->Execute(commandBuffer, farTex->Texture, settings.FarBlurAmount, tempTexture->RenderTexture);
			blurredFarTex = tempTexture->Texture;
		}
	}

	combineMat->Prepare(ppLastFrame, blurredNearTex, blurredFarTex, sceneDepthNode->DepthTex->Texture, inputs.View, settings);
	combineMat->Execute(commandBuffer, ppOutput);

	separateMat->Release();
}

void RCNodeGaussianDOF::Clear()
{
	// Do nothing
}

RCNodeGaussianDOF::DependencyDefinition RCNodeGaussianDOF::GetDependencyDefinition()
{
	static const DependencyDefinition kDependencyDefinition;
	return kDependencyDefinition;
}

void RCNodeFXAA::Render(const RenderCompositorNodeInputs& inputs)
{
	auto dependencies = GetDependencyDefinition().ResolveDependencies(inputs);
	const RenderSettings& settings = inputs.View.GetRenderSettings();
	if(!settings.EnableFxaa)
		return;

	RCNodePostProcess* postProcessNode = dependencies.Get<RCNodePostProcess>();

	SPtr<RenderTexture> ppOutput;
	SPtr<Texture> ppLastFrame;
	postProcessNode->GetAndSwitch(inputs.View, ppOutput, ppLastFrame);

	// Note: I could skip executing FXAA over DOF and motion blurred pixels
	FXAAMaterial* fxaa = FXAAMaterial::Get();
	fxaa->Prepare(ppLastFrame);
	fxaa->Execute(*inputs.ActiveCommandBuffer, ppOutput);
}

void RCNodeFXAA::Clear()
{
	// Do nothing
}

RCNodeFXAA::DependencyDefinition RCNodeFXAA::GetDependencyDefinition()
{
	static const DependencyDefinition kDependencyDefinition;
	return kDependencyDefinition;
}

void RCNodeChromaticAberration::Render(const RenderCompositorNodeInputs& inputs)
{
	auto dependencies = GetDependencyDefinition().ResolveDependencies(inputs);
	const RenderSettings& settings = inputs.View.GetRenderSettings();
	if(!settings.ChromaticAberration.Enabled)
		return;

	auto* postProcessNode = dependencies.Get<RCNodePostProcess>();

	SPtr<RenderTexture> ppOutput;
	SPtr<Texture> ppLastFrame;
	postProcessNode->GetAndSwitch(inputs.View, ppOutput, ppLastFrame);

	ChromaticAberrationMaterial* chromaticAberration = ChromaticAberrationMaterial::GetVariation(settings.ChromaticAberration.Type);
	chromaticAberration->Prepare(ppLastFrame, settings.ChromaticAberration);
	chromaticAberration->Execute(*inputs.ActiveCommandBuffer, ppOutput);
}

void RCNodeChromaticAberration::Clear()
{
	// Do nothing
}

RCNodeChromaticAberration::DependencyDefinition RCNodeChromaticAberration::GetDependencyDefinition()
{
	static const DependencyDefinition kDependencyDefinition;
	return kDependencyDefinition;
}

void RCNodeFilmGrain::Render(const RenderCompositorNodeInputs& inputs)
{
	auto dependencies = GetDependencyDefinition().ResolveDependencies(inputs);
	const RenderSettings& settings = inputs.View.GetRenderSettings();
	if(!settings.FilmGrain.Enabled)
		return;

	auto* postProcessNode = dependencies.Get<RCNodePostProcess>();

	SPtr<RenderTexture> ppOutput;
	SPtr<Texture> ppLastFrame;
	postProcessNode->GetAndSwitch(inputs.View, ppOutput, ppLastFrame);

	FilmGrainMaterial* filmGrain = FilmGrainMaterial::Get();
	filmGrain->Prepare(ppLastFrame, inputs.FrameInfo.Timings.Time, settings.FilmGrain);
	filmGrain->Execute(*inputs.ActiveCommandBuffer, ppOutput);
}

void RCNodeFilmGrain::Clear()
{
	// Do nothing
}

RCNodeFilmGrain::DependencyDefinition RCNodeFilmGrain::GetDependencyDefinition()
{
	static const DependencyDefinition kDependencyDefinition;
	return kDependencyDefinition;
}

void RCNodeHalfSceneColor::Render(const RenderCompositorNodeInputs& inputs)
{
	auto dependencies = GetDependencyDefinition().ResolveDependencies(inputs);
	const RendererViewProperties& viewProps = inputs.View.GetProperties();

	auto* sceneColorNode = dependencies.Get<RCNodeSceneColor>();
	const SPtr<Texture>& input = sceneColorNode->SceneColorTex->Texture;

	// Downsample scene
	const bool msaa = viewProps.Target.NumSamples > 1;
	DownsampleMaterial* downsampleMat = DownsampleMaterial::GetVariation(1, msaa);

	Output = GetGpuResourcePool().Get(DownsampleMaterial::GetOutputDesc(input));

	downsampleMat->Execute(*inputs.ActiveCommandBuffer, input, Output->RenderTexture);
}

void RCNodeHalfSceneColor::Clear()
{
	Output = nullptr;
}

RCNodeHalfSceneColor::DependencyDefinition RCNodeHalfSceneColor::GetDependencyDefinition()
{
	static const DependencyDefinition kDependencyDefinition;
	return kDependencyDefinition;
}

constexpr u32 RCNodeSceneColorDownsamples::kMaxNumDownsamples;

void RCNodeSceneColorDownsamples::Render(const RenderCompositorNodeInputs& inputs)
{
	auto dependencies = GetDependencyDefinition().ResolveDependencies(inputs);
	GpuResourcePool& resPool = GetGpuResourcePool();

	auto* halfSceneColorNode = dependencies.Get<RCNodeHalfSceneColor>();
	const TextureProperties& halfSceneProps = halfSceneColorNode->Output->Texture->GetProperties();

	const u32 totalDownsampleLevels = PixelUtility::GetMipmapCount(
										  halfSceneProps.Width,
										  halfSceneProps.Height,
										  1,
										  halfSceneProps.Format) +
		1;

	AvailableDownsamples = Math::Min(kMaxNumDownsamples, totalDownsampleLevels);

	{
		Output[0] = halfSceneColorNode->Output;

		DownsampleMaterial* downsampleMat = DownsampleMaterial::GetVariation(1, false);
		for(u32 i = 1; i < AvailableDownsamples; i++)
		{
			Output[i] = resPool.Get(DownsampleMaterial::GetOutputDesc(Output[i - 1]->Texture));
			downsampleMat->Execute(*inputs.ActiveCommandBuffer, Output[i - 1]->Texture, Output[i]->RenderTexture);
		}
	}
}

void RCNodeSceneColorDownsamples::Clear()
{
	for(u32 i = 0; i < kMaxNumDownsamples; i++)
		Output[i] = nullptr;
}

RCNodeSceneColorDownsamples::DependencyDefinition RCNodeSceneColorDownsamples::GetDependencyDefinition()
{
	static const DependencyDefinition kDependencyDefinition;
	return kDependencyDefinition;
}

void RCNodeResolvedSceneDepth::Render(const RenderCompositorNodeInputs& inputs)
{
	auto dependencies = GetDependencyDefinition().ResolveDependencies(inputs);
	const RendererViewProperties& viewProps = inputs.View.GetProperties();
	RCNodeSceneDepth* sceneDepthNode = dependencies.Get<RCNodeSceneDepth>();

	GpuCommandBuffer& commandBuffer = *inputs.ActiveCommandBuffer;
	if(viewProps.Target.NumSamples > 1)
	{
		u32 width = viewProps.Target.ViewRect.Width;
		u32 height = viewProps.Target.ViewRect.Height;

		Output = GetGpuResourcePool().Get(
			PooledRenderTextureCreateInformation::Create2D(PF_D32_S8X24, width, height, TextureUsageFlag::DepthStencil, 1, false));

		BlitInformation blitInformation = BlitInformation::BlitDepth(sceneDepthNode->DepthTex->Texture, Output->RenderTexture);
		blitInformation.ClearMask = RT_STENCIL;

		GetRendererUtility().Blit(commandBuffer, BlitInformation::BlitDepth(sceneDepthNode->DepthTex->Texture, Output->RenderTexture));
	}
	else
		Output = sceneDepthNode->DepthTex;
}

void RCNodeResolvedSceneDepth::Clear()
{
	Output = nullptr;
}

RCNodeResolvedSceneDepth::DependencyDefinition RCNodeResolvedSceneDepth::GetDependencyDefinition()
{
	static const DependencyDefinition kDependencyDefinition;
	return kDependencyDefinition;
}

void RCNodeHiZ::Render(const RenderCompositorNodeInputs& inputs)
{
	auto dependencies = GetDependencyDefinition().ResolveDependencies(inputs);
	GpuCommandBuffer& commandBuffer = *inputs.ActiveCommandBuffer;
	const RendererViewProperties& viewProps = inputs.View.GetProperties();

	RCNodeResolvedSceneDepth* resolvedSceneDepth = dependencies.Get<RCNodeResolvedSceneDepth>();

	u32 width = viewProps.Target.ViewRect.Width;
	u32 height = viewProps.Target.ViewRect.Height;

	u32 size = Bitwise::NextPow2(std::max(width, height));
	u32 numMips = PixelUtility::GetMipmapCount(size, size, 1, PF_R32F);
	size = 1 << numMips;

	// Note: Use the 32-bit buffer here as 16-bit causes too much banding (most of the scene gets assigned 4-5 different
	// depth values).
	//  - When I add UNORM 16-bit format I should be able to switch to that
	Output = GetGpuResourcePool().Get(
		PooledRenderTextureCreateInformation::Create2D(PF_R32F, size, size, TextureUsageFlag::RenderTarget, 1, false, 1, numMips));

	Area2 srcRect = viewProps.Target.NrmViewRect;

	// If viewport size is odd, adjust UV
	srcRect.Width += (viewProps.Target.ViewRect.Width % 2) * (1.0f / viewProps.Target.ViewRect.Width);
	srcRect.Height += (viewProps.Target.ViewRect.Height % 2) * (1.0f / viewProps.Target.ViewRect.Height);

	bool noTextureViews = !commandBuffer.GetGpuDevice().GetCapabilities().HasCapability(RSC_TEXTURE_VIEWS);

	BuildHiZMaterial* material = BuildHiZMaterial::GetVariation(noTextureViews);

	// Generate first mip
	RenderTextureCreateInformation rtDesc;
	rtDesc.ColorSurfaces[0].Texture = Output->Texture;
	rtDesc.ColorSurfaces[0].MipLevel = 0;

	SPtr<RenderTexture> renderTexture = RenderTexture::Create(rtDesc);

	Area2 destinationArea;
	bool downsampledFirstMip = false; // Not used currently
	if(downsampledFirstMip)
	{
		// Make sure that 1 pixel in HiZ maps to a 2x2 block in source
		destinationArea = Area2(0, 0, Math::CeilToInt(viewProps.Target.ViewRect.Width / 2.0f) / (float)size, Math::CeilToInt(viewProps.Target.ViewRect.Height / 2.0f) / (float)size);

		material->Prepare(resolvedSceneDepth->Output->Texture, 0);
		material->Execute(commandBuffer, renderTexture, srcRect, destinationArea);
	}
	else // First level is just a copy of the depth buffer
	{
		destinationArea = Area2(0, 0, viewProps.Target.ViewRect.Width / (float)size, viewProps.Target.ViewRect.Height / (float)size);

		Area2I sourceArea;
		sourceArea.X = (i32)(srcRect.X * viewProps.Target.ViewRect.Width);
		sourceArea.Y = (i32)(srcRect.Y * viewProps.Target.ViewRect.Height);
		sourceArea.Width = (u32)(srcRect.Width * viewProps.Target.ViewRect.Width);
		sourceArea.Height = (u32)(srcRect.Height * viewProps.Target.ViewRect.Height);

		BlitInformation blitInformation = BlitInformation::BlitColor(resolvedSceneDepth->Output->Texture, renderTexture, sourceArea);
		blitInformation.OutputArea = destinationArea;

		GetRendererUtility().Blit(commandBuffer, blitInformation);
		commandBuffer.SetViewport(Area2(0, 0, 1, 1));
	}

	// Generate remaining mip levels
	for(u32 i = 1; i <= numMips; i++)
	{
		rtDesc.ColorSurfaces[0].MipLevel = i;
		renderTexture = RenderTexture::Create(rtDesc);

		material->Prepare(Output->Texture, i - 1);
		material->Execute(commandBuffer, renderTexture, destinationArea, destinationArea);
	}
}

void RCNodeHiZ::Clear()
{
	Output = nullptr;
}

RCNodeHiZ::DependencyDefinition RCNodeHiZ::GetDependencyDefinition()
{
	static const DependencyDefinition kDependencyDefinition;
	return kDependencyDefinition;
}

void RCNodeSSAO::Render(const RenderCompositorNodeInputs& inputs)
{
	auto dependencies = GetDependencyDefinition().ResolveDependencies(inputs);
	/** Maximum valid depth range within samples in a sample set. In meters. */
	static const float kDepthRange = 1.0f;

	const AmbientOcclusionSettings& settings = inputs.View.GetRenderSettings().AmbientOcclusion;
	if(!settings.Enabled)
	{
		Output = Texture::kWhite;
		mPooledOutput = nullptr;
		return;
	}

	GpuCommandBuffer& commandBuffer = *inputs.ActiveCommandBuffer;
	GpuResourcePool& resPool = GetGpuResourcePool();
	const RendererViewProperties& viewProps = inputs.View.GetProperties();

	RCNodeResolvedSceneDepth* resolvedDepthNode = dependencies.Get<RCNodeResolvedSceneDepth>();
	RCNodeBasePass* gbufferNode = dependencies.Get<RCNodeBasePass>();

	SPtr<Texture> sceneDepth = resolvedDepthNode->Output->Texture;
	SPtr<Texture> sceneNormals = gbufferNode->NormalTex->Texture;

	const TextureProperties& normalsProps = sceneNormals->GetProperties();
	SPtr<PooledRenderTexture> resolvedNormals;

	if(sceneNormals->GetProperties().SampleCount > 1)
	{
		PooledRenderTextureCreateInformation desc = PooledRenderTextureCreateInformation::Create2D(normalsProps.Format, normalsProps.Width, normalsProps.Height, TextureUsageFlag::RenderTarget);
		resolvedNormals = resPool.Get(desc);

		GetRendererUtility().Blit(commandBuffer, BlitInformation::BlitColor(sceneNormals, resolvedNormals->RenderTexture));
		sceneNormals = resolvedNormals->Texture;
	}

	// Multiple downsampled AO levels are used to minimize cache trashing. Downsampled AO targets use larger radius,
	// whose contents are then blended with the higher level.
	u32 quality = settings.Quality;
	u32 numDownsampleLevels = 0;
	if(quality == 2)
		numDownsampleLevels = 1;
	else if(quality > 2)
		numDownsampleLevels = 2;

	SSAODownsampleMaterial* downsample = SSAODownsampleMaterial::Get();

	SPtr<PooledRenderTexture> setupTex0;
	if(numDownsampleLevels > 0)
	{
		Vector2I downsampledSize(
			std::max(1, Math::DivideAndRoundUp((i32)viewProps.Target.ViewRect.Width, 2)),
			std::max(1, Math::DivideAndRoundUp((i32)viewProps.Target.ViewRect.Height, 2)));

		PooledRenderTextureCreateInformation desc = PooledRenderTextureCreateInformation::Create2D(PF_RGBA16F, downsampledSize.X, downsampledSize.Y, TextureUsageFlag::RenderTarget);
		setupTex0 = resPool.Get(desc);

		downsample->Prepare(inputs.View, sceneDepth, sceneNormals, setupTex0->RenderTexture, kDepthRange);
		downsample->Execute(commandBuffer, setupTex0->RenderTexture);
	}

	SPtr<PooledRenderTexture> setupTex1;
	if(numDownsampleLevels > 1)
	{
		Vector2I downsampledSize(
			std::max(1, Math::DivideAndRoundUp((i32)viewProps.Target.ViewRect.Width, 4)),
			std::max(1, Math::DivideAndRoundUp((i32)viewProps.Target.ViewRect.Height, 4)));

		PooledRenderTextureCreateInformation desc = PooledRenderTextureCreateInformation::Create2D(PF_RGBA16F, downsampledSize.X, downsampledSize.Y, TextureUsageFlag::RenderTarget);
		setupTex1 = resPool.Get(desc);

		downsample->Prepare(inputs.View, sceneDepth, sceneNormals, setupTex1->RenderTexture, kDepthRange);
		downsample->Execute(commandBuffer, setupTex1->RenderTexture);
	}

	SSAOTextureInputs textures;
	textures.SceneDepth = sceneDepth;
	textures.SceneNormals = sceneNormals;
	textures.RandomRotations = RendererTextures::ssaoRandomization4x4;

	SPtr<PooledRenderTexture> downAOTex1;
	if(numDownsampleLevels > 1)
	{
		textures.AoSetup = setupTex1->Texture;

		Vector2I downsampledSize(
			std::max(1, Math::DivideAndRoundUp((i32)viewProps.Target.ViewRect.Width, 4)),
			std::max(1, Math::DivideAndRoundUp((i32)viewProps.Target.ViewRect.Height, 4)));

		PooledRenderTextureCreateInformation desc = PooledRenderTextureCreateInformation::Create2D(PF_R8, downsampledSize.X, downsampledSize.Y, TextureUsageFlag::RenderTarget);
		downAOTex1 = resPool.Get(desc);

		SSAOMaterial* ssaoMat = SSAOMaterial::GetVariation(false, false, quality);
		ssaoMat->Prepare(inputs.View, textures, downAOTex1->RenderTexture, settings);
		ssaoMat->Execute(commandBuffer, downAOTex1->RenderTexture);

		setupTex1 = nullptr;
	}

	SPtr<PooledRenderTexture> downAOTex0;
	if(numDownsampleLevels > 0)
	{
		textures.AoSetup = setupTex0->Texture;

		if(downAOTex1)
			textures.AoDownsampled = downAOTex1->Texture;

		Vector2I downsampledSize(
			std::max(1, Math::DivideAndRoundUp((i32)viewProps.Target.ViewRect.Width, 2)),
			std::max(1, Math::DivideAndRoundUp((i32)viewProps.Target.ViewRect.Height, 2)));

		PooledRenderTextureCreateInformation desc = PooledRenderTextureCreateInformation::Create2D(PF_R8, downsampledSize.X, downsampledSize.Y, TextureUsageFlag::RenderTarget);
		downAOTex0 = resPool.Get(desc);

		bool upsample = numDownsampleLevels > 1;
		SSAOMaterial* ssaoMat = SSAOMaterial::GetVariation(upsample, false, quality);
		ssaoMat->Prepare(inputs.View, textures, downAOTex0->RenderTexture, settings);
		ssaoMat->Execute(commandBuffer, downAOTex0->RenderTexture);

		if(upsample)
			downAOTex1 = nullptr;
	}

	u32 width = viewProps.Target.ViewRect.Width;
	u32 height = viewProps.Target.ViewRect.Height;
	mPooledOutput = resPool.Get(PooledRenderTextureCreateInformation::Create2D(PF_R8, width, height, TextureUsageFlag::RenderTarget));

	{
		if(setupTex0)
			textures.AoSetup = setupTex0->Texture;

		if(downAOTex0)
			textures.AoDownsampled = downAOTex0->Texture;

		bool upsample = numDownsampleLevels > 0;
		SSAOMaterial* ssaoMat = SSAOMaterial::GetVariation(upsample, true, quality);
		ssaoMat->Prepare(inputs.View, textures, mPooledOutput->RenderTexture, settings);
		ssaoMat->Execute(commandBuffer, mPooledOutput->RenderTexture);
	}

	resolvedNormals = nullptr;

	if(numDownsampleLevels > 0)
	{
		setupTex0 = nullptr;
		downAOTex0 = nullptr;
	}

	// Blur the output
	// Note: If I implement temporal AA then this can probably be avoided. I can instead jitter the sample offsets
	// each frame, and averaging them out should yield blurred AO.
	if(quality > 1) // On level 0 we don't blur at all, on level 1 we use the ad-hoc blur in shader
	{
		const RenderTargetProperties& rtProps = mPooledOutput->RenderTexture->GetProperties();

		PooledRenderTextureCreateInformation desc = PooledRenderTextureCreateInformation::Create2D(PF_R8, rtProps.Width, rtProps.Height, TextureUsageFlag::RenderTarget);
		SPtr<PooledRenderTexture> blurIntermediateTex = resPool.Get(desc);

		SSAOBlurMaterial* blurHorz = SSAOBlurMaterial::GetVariation(true);
		SSAOBlurMaterial* blurVert = SSAOBlurMaterial::GetVariation(false);

		blurHorz->Prepare(inputs.View, mPooledOutput->Texture, sceneDepth, kDepthRange);
		blurHorz->Execute(commandBuffer, blurIntermediateTex->RenderTexture);

		blurVert->Prepare(inputs.View, blurIntermediateTex->Texture, sceneDepth, kDepthRange);
		blurVert->Execute(commandBuffer, mPooledOutput->RenderTexture);
	}

	Output = mPooledOutput->Texture;
}

void RCNodeSSAO::Clear()
{
	mPooledOutput = nullptr;
	Output = nullptr;
}

RCNodeSSAO::DependencyDefinition RCNodeSSAO::GetDependencyDefinition()
{
	static const DependencyDefinition kDependencyDefinition;
	return kDependencyDefinition;
}

RCNodeSSR::~RCNodeSSR()
{
	DeallocOutputs();
}

void RCNodeSSR::Render(const RenderCompositorNodeInputs& inputs)
{
	const ScreenSpaceReflectionsSettings& settings = inputs.View.GetRenderSettings().ScreenSpaceReflections;
	if(!settings.Enabled)
	{
		DeallocOutputs();

		mPooledOutput = nullptr;
		Output = Texture::kBlack;
		return;
	}

	auto dependencies = GetDependencyDefinition().ResolveDependencies(inputs);
	RCNodeSceneDepth* sceneDepthNode = dependencies.Get<RCNodeSceneDepth>();
	RCNodeLightAccumulation* lightAccumNode = dependencies.Get<RCNodeLightAccumulation>();
	RCNodeBasePass* gbufferNode = dependencies.Get<RCNodeBasePass>();
	RCNodeHiZ* hiZNode = dependencies.Get<RCNodeHiZ>();
	RCNodeResolvedSceneDepth* resolvedSceneDepthNode = dependencies.Get<RCNodeResolvedSceneDepth>();

	GpuCommandBuffer& commandBuffer = *inputs.ActiveCommandBuffer;
	GpuResourcePool& resPool = GetGpuResourcePool();
	const RendererViewProperties& viewProps = inputs.View.GetProperties();

	u32 width = viewProps.Target.ViewRect.Width;
	u32 height = viewProps.Target.ViewRect.Height;

	SPtr<Texture> hiZ = hiZNode->Output->Texture;

	// This will be executing before scene color is resolved, so get the light accum buffer instead
	SPtr<Texture> sceneColor;

	// Resolve multiple samples if MSAA is used
	SPtr<PooledRenderTexture> resolvedSceneColor;
	if(viewProps.Target.NumSamples > 1)
	{
		resolvedSceneColor = resPool.Get(PooledRenderTextureCreateInformation::Create2D(PF_RGBA16F, width, height, TextureUsageFlag::RenderTarget));

		GetRendererUtility().Blit(commandBuffer, BlitInformation::BlitColor(sceneColor, resolvedSceneColor->RenderTexture));
		sceneColor = resolvedSceneColor->Texture;
	}
	else
	{
		sceneColor = lightAccumNode->LightAccumulationTex->Texture;
	}

	GBufferTextures gbuffer;
	gbuffer.Albedo = gbufferNode->AlbedoTex->Texture;
	gbuffer.Normals = gbufferNode->NormalTex->Texture;
	gbuffer.RoughMetal = gbufferNode->RoughMetalTex->Texture;
	gbuffer.Depth = sceneDepthNode->DepthTex->Texture;

	SSRStencilMaterial* stencilMat = SSRStencilMaterial::GetVariation(viewProps.Target.NumSamples > 1, true);
	stencilMat->Prepare(inputs.View, gbuffer, settings);

	// Note: Making the assumption that the stencil buffer is clear at this point
	RenderPassCreateInformation stencilInfo(resolvedSceneDepthNode->Output->RenderTexture, stencilMat->GetGpuParameterSet(), RT_DEPTH, RT_DEPTH_STENCIL);
	commandBuffer.BeginRenderPass(stencilInfo);
	stencilMat->Execute(commandBuffer, inputs.View);
	commandBuffer.EndRenderPass();

	SPtr<PooledRenderTexture> traceOutput = resPool.Get(PooledRenderTextureCreateInformation::Create2D(PF_RGBA16F, width, height, TextureUsageFlag::RenderTarget));

	RenderTextureCreateInformation traceRtDesc;
	traceRtDesc.ColorSurfaces[0].Texture = traceOutput->Texture;
	traceRtDesc.DepthStencilSurface.Texture = resolvedSceneDepthNode->Output->Texture;

	SPtr<RenderTexture> traceRt = RenderTexture::Create(traceRtDesc);

	SSRTraceMaterial* traceMat = SSRTraceMaterial::GetVariation(settings.Quality, viewProps.Target.NumSamples > 1, true);
	traceMat->Prepare(inputs.View, gbuffer, sceneColor, hiZ, settings);
	traceMat->Execute(commandBuffer, traceRt, inputs.View);

	resolvedSceneColor = nullptr;

	mUsingTemporalAA = inputs.View.GetRenderSettings().TemporalAa.Enabled;
	if(mPrevFrame && !mUsingTemporalAA)
	{
		mPooledOutput = resPool.Get(PooledRenderTextureCreateInformation::Create2D(PF_RGBA16F, width, height, TextureUsageFlag::RenderTarget));

		TemporalFilteringMaterial* temporalFilteringMat =
			TemporalFilteringMaterial::GetVariation(TemporalFilteringType::SSR, false, viewProps.Target.NumSamples > 1);
		temporalFilteringMat->Prepare(inputs.View, mPrevFrame->Texture, traceOutput->Texture, nullptr, sceneDepthNode->DepthTex->Texture, Vector2::kZero, 1.0f);
		temporalFilteringMat->Execute(commandBuffer, inputs.View, mPooledOutput->RenderTexture);

		traceOutput = nullptr;
	}
	else
		mPooledOutput = traceOutput;

	Output = mPooledOutput->Texture;
}

void RCNodeSSR::Clear()
{
	if(!mUsingTemporalAA)
		mPrevFrame = mPooledOutput;

	mPooledOutput = nullptr;
	Output = nullptr;
}

void RCNodeSSR::DeallocOutputs()
{
	mPrevFrame = nullptr;
	Output = nullptr;
}

RCNodeSSR::DependencyDefinition RCNodeSSR::GetDependencyDefinition()
{
	static const DependencyDefinition kDependencyDefinition = []()
	{
		DependencyDefinition definition;

		auto fnSSREnabled = [](const RendererView& view)
		{
			return view.GetRenderSettings().ScreenSpaceReflections.Enabled;
		};

		definition.ConditionalEnable<RCNodeSceneDepth>(fnSSREnabled)
			.ConditionalEnable<RCNodeLightAccumulation>(fnSSREnabled)
			.ConditionalEnable<RCNodeBasePass>(fnSSREnabled)
			.ConditionalEnable<RCNodeHiZ>(fnSSREnabled)
			.ConditionalEnable<RCNodeResolvedSceneDepth>(fnSSREnabled)
			.ConditionalEnable<RCNodeIndirectDiffuseLighting>(fnSSREnabled);

		return definition;
	}();

	return kDependencyDefinition;
}

void RCNodeBloom::Render(const RenderCompositorNodeInputs& inputs)
{
	auto dependencies = GetDependencyDefinition().ResolveDependencies(inputs);
	const RenderSettings& settings = inputs.View.GetRenderSettings();

	// Grab downsampled scene color to use as input
	auto* sceneDownsamplesNode = dependencies.Get<RCNodeSceneColorDownsamples>();

	constexpr u32 PREFERRED_NUM_DOWNSAMPLE_LEVELS = 6;
	const u32 availableDownsamples = sceneDownsamplesNode->AvailableDownsamples;
	const u32 numDownsamples = Math::Min(availableDownsamples, PREFERRED_NUM_DOWNSAMPLE_LEVELS);
	B3D_ASSERT(numDownsamples >= 1);

	// Blur & clip the downsampled entries and add them together
	const u32 quality = Math::Clamp(settings.Bloom.Quality, 0U, 3U);
	constexpr u32 NUM_STEPS_PER_QUALITY[] = { 3, 4, 5, 6 };

	GaussianBlurMaterial* filterMat = GaussianBlurMaterial::GetVariation(true);

	const bool autoExposure = settings.EnableHdr && settings.EnableAutoExposure;
	BloomClipMaterial* clipMat = BloomClipMaterial::GetVariation(autoExposure);

	SPtr<Texture> eyeAdaptationTex = nullptr;
	if(autoExposure)
	{
		auto* eyeAdapatationNode = dependencies.Get<RCNodeEyeAdaptation>();

		if(eyeAdapatationNode->Output)
			eyeAdaptationTex = eyeAdapatationNode->Output->Texture;
	}

	const u32 numSteps = NUM_STEPS_PER_QUALITY[quality];
	SPtr<PooledRenderTexture> prevOutput;
	for(u32 i = 0; i < numSteps; i++)
	{
		const u32 srcIdx = numDownsamples - i - 1;
		const SPtr<PooledRenderTexture> downsampledTex = sceneDownsamplesNode->Output[srcIdx];

		const TextureProperties& inputProps = downsampledTex->Texture->GetProperties();

		SPtr<PooledRenderTexture> filterOutput = GetGpuResourcePool().Get(
			PooledRenderTextureCreateInformation::Create2D(
				inputProps.Format,
				inputProps.Width,
				inputProps.Height,
				TextureUsageFlag::RenderTarget));

		SPtr<PooledRenderTexture> blurInput = downsampledTex;
		SPtr<PooledRenderTexture> blurOutput = filterOutput;
		if(settings.Bloom.Threshold > 0.0f)
		{
			clipMat->Prepare(downsampledTex->Texture, settings.Bloom.Threshold, eyeAdaptationTex, settings);
			clipMat->Execute(*inputs.ActiveCommandBuffer, filterOutput->RenderTexture);

			blurOutput = blurInput;
			blurInput = filterOutput;
		}

		SPtr<Texture> additiveInput;
		if(prevOutput)
			additiveInput = prevOutput->Texture;

		const Color tint = Color::kWhite * (settings.Bloom.Intensity / (float)numSteps);
		filterMat->Execute(*inputs.ActiveCommandBuffer, blurInput->Texture, settings.Bloom.FilterSize, blurOutput->RenderTexture, tint, additiveInput);
		prevOutput = blurOutput;
	}

	mPooledOutput = prevOutput;
	Output = mPooledOutput->Texture;
}

void RCNodeBloom::Clear()
{
	mPooledOutput = nullptr;
	Output = nullptr;
}

RCNodeBloom::DependencyDefinition RCNodeBloom::GetDependencyDefinition()
{
	static const DependencyDefinition kDependencyDefinition;
	return kDependencyDefinition;
}

void RCNodeScreenSpaceLensFlare::Render(const RenderCompositorNodeInputs& inputs)
{
	auto dependencies = GetDependencyDefinition().ResolveDependencies(inputs);
	GpuCommandBuffer& commandBuffer = *inputs.ActiveCommandBuffer;
	GpuResourcePool& resPool = GpuResourcePool::Instance();
	const RenderSettings& settings = inputs.View.GetRenderSettings();
	const ScreenSpaceLensFlareSettings& lensFlareSettings = settings.ScreenSpaceLensFlare;

	// Grab downsampled scene color to use as input
	auto* sceneDownsamplesNode = dependencies.Get<RCNodeSceneColorDownsamples>();

	const u32 availableDownsamples = sceneDownsamplesNode->AvailableDownsamples;
	const u32 numDownsamples = Math::Clamp(settings.ScreenSpaceLensFlare.DownsampleCount, 1U, availableDownsamples);
	B3D_ASSERT(numDownsamples >= 1);

	SPtr<PooledRenderTexture> downsampledTex = sceneDownsamplesNode->Output[numDownsamples - 1];
	const TextureProperties& sceneTexProps = downsampledTex->Texture->GetProperties();

	// Ghost features
	SPtr<PooledRenderTexture> featureTex = resPool.Get(
		PooledRenderTextureCreateInformation::Create2D(
			sceneTexProps.Format,
			sceneTexProps.Width,
			sceneTexProps.Height,
			TextureUsageFlag::RenderTarget));

	bool haloAspect = lensFlareSettings.HaloAspectRatio != 1.0f;
	ScreenSpaceLensFlareMaterial* lensFlareMat = ScreenSpaceLensFlareMaterial::GetVariation(
		lensFlareSettings.Halo,
		haloAspect,
		lensFlareSettings.ChromaticAberration);
	lensFlareMat->Prepare(downsampledTex->Texture, lensFlareSettings);
	lensFlareMat->Execute(commandBuffer, featureTex->RenderTexture);

	// Blur
	GaussianBlurMaterial* filterMat = GaussianBlurMaterial::Get();
	filterMat->Execute(commandBuffer, featureTex->Texture, lensFlareSettings.FilterSize, downsampledTex->RenderTexture, Color::kWhite);

	auto* sceneColorNode = static_cast<RCNodeSceneColor*>(inputs.InputNodes[1]);

	if(lensFlareSettings.BicubicUpsampling)
	{
		BicubicUpsampleMaterial* upsampleMat = BicubicUpsampleMaterial::GetVariation(false);
		upsampleMat->Execute(commandBuffer, 
			downsampledTex->Texture,
			sceneColorNode->RenderTarget,
			Color::kWhite * lensFlareSettings.Brightness);
	}
	else
	{
		CompositeMaterial* upsampleMat = CompositeMaterial::Get();
		upsampleMat->Execute(commandBuffer, 
			downsampledTex->Texture,
			sceneColorNode->RenderTarget,
			Color::kWhite * lensFlareSettings.Brightness);
	}
}

void RCNodeScreenSpaceLensFlare::Clear()
{
	// Do nothing
}

RCNodeScreenSpaceLensFlare::DependencyDefinition RCNodeScreenSpaceLensFlare::GetDependencyDefinition()
{
	static const DependencyDefinition kDependencyDefinition;
	return kDependencyDefinition;
}
}}
