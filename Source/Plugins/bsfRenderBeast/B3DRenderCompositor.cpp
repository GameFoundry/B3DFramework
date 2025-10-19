//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DRenderCompositor.h"
#include "Renderer/B3DRendererExtension.h"
#include "Components/B3DSkybox.h"
#include "Components/B3DCamera.h"
#include "Renderer/B3DRendererUtility.h"
#include "Utility/B3DBitwise.h"
#include "Mesh/B3DMesh.h"
#include "Material/B3DGpuParamsSet.h"
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

/** Renders all elements in a render queue. */
void RenderQueueElements(GpuCommandBuffer& commandBuffer, const Vector<RenderQueueElement>& elements)
{
	for(auto& entry : elements)
	{
		if(entry.ApplyPass)
			GetRendererUtility().SetPass(commandBuffer, entry.RenderElem->Material, entry.PassIdx, entry.TechniqueIdx);

		GetRendererUtility().SetPassParams(commandBuffer, entry.RenderElem->Params, entry.PassIdx);

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

	FrameScope frameScope;
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
				B3D_LOG(Error, Renderer, "Render compositor nodes recursion detected. Node \"{0}\" "
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

	DepthTex = GetGpuResourcePool().Get(PooledRenderTextureCreateInformation::Create2D(PF_D32_S8X24, width, height, TU_DEPTHSTENCIL, numSamples, false));
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
	AlbedoTex = resPool.Get(PooledRenderTextureCreateInformation::Create2D(PF_RGBA8, width, height, TU_RENDERTARGET, numSamples, true));
	NormalTex = resPool.Get(PooledRenderTextureCreateInformation::Create2D(PF_RGB10A2, width, height, TU_RENDERTARGET, numSamples, false));
	RoughMetalTex = resPool.Get(PooledRenderTextureCreateInformation::Create2D(PF_RG16F, width, height, TU_RENDERTARGET, numSamples, false)); // Note: Metal doesn't need 16-bit float
	IdTex = resPool.Get(PooledRenderTextureCreateInformation::Create2D(PF_R8, width, height, TU_RENDERTARGET, numSamples, false));

	if(needsVelocity)
	{
		VelocityTex = resPool.Get(PooledRenderTextureCreateInformation::Create2D(PF_RG16S, width, height, TU_RENDERTARGET, numSamples, false));
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
	const auto numRenderables = (u32)inputs.Scene.Renderables.size();
	for(u32 i = 0; i < numRenderables; i++)
	{
		if(!visibility.Renderables[i])
			continue;

		RendererRenderable* rendererRenderable = inputs.Scene.Renderables[i];
		rendererRenderable->UpdatePerCallBuffer(viewProps.ViewProjTransform);

		for(auto& element : inputs.Scene.Renderables[i]->Elements)
		{
			SPtr<GpuParameters> gpuParams = element.Params->GetGpuParams();
			const GpuParameterBinding& binding = element.PerCameraBinding;
			if(binding.Slot != (u32)-1)
				gpuParams->SetUniformBuffer(binding.Set, binding.Slot, inputs.View.GetPerViewBuffer());
		}
	}

	//// Prepare particle systems
	const EvaluatedParticleData* particleData = inputs.FrameInfo.PerSceneFrameData.Particles;
	if(particleData)
	{
		const auto numParticleSystems = (u32)inputs.Scene.ParticleSystems.size();

		const GpuParticleResources& gpuSimResources = GpuParticleSimulation::Instance().GetResources();
		for(u32 i = 0; i < numParticleSystems; i++)
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
				rendererParticles.BindGpuSimulatedInputs(gpuSimResources, inputs.View);
		}
	}

	//// Prepare decals
	const auto numDecals = (u32)inputs.Scene.Decals.size();
	for(u32 i = 0; i < numDecals; i++)
	{
		if(!visibility.Decals[i])
			continue;

		const RendererDecal& rendererDecal = inputs.Scene.Decals[i];
		DecalRenderElement& renderElement = rendererDecal.RenderElement;

		rendererDecal.UpdatePerCallBuffer(viewProps.ViewProjTransform);

		SPtr<GpuParameters> gpuParams = renderElement.Params->GetGpuParams();
		const GpuParameterBinding& binding = renderElement.PerCameraBinding;
		if(binding.Slot != (u32)-1)
			gpuParams->SetUniformBuffer(binding.Set, binding.Slot, inputs.View.GetPerViewBuffer());

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
	commandBuffer.BeginRenderPass(RenderTarget);

	Area2 area(0.0f, 0.0f, 1.0f, 1.0f);
	commandBuffer.SetViewport(area);

	// Clear all targets
	commandBuffer.ClearViewport(FBT_COLOR | FBT_DEPTH | FBT_STENCIL, Color::kZero, 1.0f, 0);

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

	// Render all visible opaque elements that use the deferred pipeline
	const Vector<RenderQueueElement>& opaqueElements = inputs.View.GetOpaqueQueue(false)->GetSortedElements();
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

		MSAACoverageMat* mat = MSAACoverageMat::GetVariation(viewProps.Target.NumSamples);
		commandBuffer.BeginRenderPass(msaaCoverageNode->Output->RenderTexture);
		mat->Execute(commandBuffer, inputs.View, gbuffer);
		commandBuffer.EndRenderPass();

		MSAACoverageStencilMat* stencilMat = MSAACoverageStencilMat::Get();
		commandBuffer.BeginRenderPass(sceneDepthNode->DepthTex->RenderTexture);
		stencilMat->Execute(commandBuffer, inputs.View, msaaCoverageNode->Output->Texture);
		commandBuffer.EndRenderPass();
	}

	// Ensure depth stencil can be read
	commandBuffer.IssueBarriers({{
		GpuTextureBarrier(sceneDepthNode->DepthTex->Texture, GpuResourceUseFlag::DepthStencilAttachment, GpuAccessFlag::Write, GpuResourceUseFlag::DepthStencilAttachment, GpuAccessFlag::Read)
	}});

	// Render decals after all normal objects, using a read-only depth buffer
	commandBuffer.BeginRenderPass(RenderTargetNoMask, RT_DEPTH, RT_ALL);

	const Vector<RenderQueueElement>& decalElements = inputs.View.GetDecalQueue()->GetSortedElements();
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
	commandBuffer.BeginRenderPass(nullptr);  // TODO - RenderPass
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

	u32 usageFlags = TU_RENDERTARGET;

	bool tiledDeferredSupported = inputs.FeatureSet != RenderBeastFeatureSet::DesktopMacOS;
	if(tiledDeferredSupported && numSamples == 1)
		usageFlags |= TU_LOADSTORE;

	// Note: Consider customizable HDR format via options? e.g. smaller PF_FLOAT_R11G11B10 or larger 32-bit format
	SceneColorTex = resPool.Get(PooledRenderTextureCreateInformation::Create2D(PF_RGBA16F, width, height, usageFlags, numSamples, false));

	auto dependencies = GetDependencyDefinition().ResolveDependencies(inputs);
	RCNodeSceneDepth* sceneDepthNode = dependencies.Get<RCNodeSceneDepth>();
	SPtr<PooledRenderTexture> sceneDepthTex = sceneDepthNode->DepthTex;

	if(tiledDeferredSupported && viewProps.Target.NumSamples > 1)
	{
		SceneColorTexArray = resPool.Get(PooledRenderTextureCreateInformation::Create2D(PF_RGBA16F, width, height, TU_LOADSTORE, 1, false, viewProps.Target.NumSamples));
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
	commandBuffer.BeginRenderPass(RenderTarget, RT_DEPTH_STENCIL, RT_DEPTH_STENCIL);

	Area2 area(0.0f, 0.0f, 1.0f, 1.0f);
	commandBuffer.SetViewport(area);

	TextureArrayToMSAATexture* material = TextureArrayToMSAATexture::Get();
	material->Execute(commandBuffer, SceneColorTexArray->Texture, SceneColorTex->Texture);

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
	Output = GetGpuResourcePool().Get(PooledRenderTextureCreateInformation::Create2D(PF_R8, width, height, TU_RENDERTARGET));
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

	u32 usage = TU_RENDERTARGET;
	if(numSamples > 1)
	{
		resPool.Get(LightAccumulationTexArray, PooledRenderTextureCreateInformation::Create2D(PF_RGBA16F, width, height, TU_LOADSTORE, 1, false, numSamples));

		ClearLoadStoreMat* clearMat = ClearLoadStoreMat::GetVariation(ClearLoadStoreType::TextureArray, ClearLoadStoreDataType::Float, 4);

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
		usage |= TU_LOADSTORE;
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
	commandBuffer.BeginRenderPass(RenderTarget, RT_DEPTH_STENCIL, RT_DEPTH_STENCIL);

	TextureArrayToMSAATexture* material = TextureArrayToMSAATexture::Get();
	material->Execute(commandBuffer, LightAccumulationTexArray->Texture, LightAccumulationTex->Texture);

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

		TiledDeferredLightingMat* tiledDeferredMat =
			TiledDeferredLightingMat::GetVariation(viewProps.Target.NumSamples);

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

		commandBuffer.BeginRenderPass(Output->RenderTarget, RT_DEPTH_STENCIL, RT_DEPTH_STENCIL);

		for(u32 i = 0; i < (u32)LightType::Count; i++)
		{
			LightType lightType = (LightType)i;

			auto& lights = lightData.GetLights(lightType);
			u32 count = lightData.GetNumUnshadowedLights(lightType);

			for(u32 j = 0; j < count; j++)
			{
				u32 lightIdx = j;
				const RendererLight& light = *lights[lightIdx];

				StandardDeferred::Instance().RenderLight(commandBuffer, lightType, light, inputs.View, gbuffer, Texture::kBlack);
			}
		}

		commandBuffer.EndRenderPass();
	}

	// Allocate light occlusion
	SPtr<PooledRenderTexture> lightOcclusionTex = GetGpuResourcePool().Get(
		PooledRenderTextureCreateInformation::Create2D(PF_R8, width, height, TU_RENDERTARGET, numSamples, false));

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
		for(u32 i = 0; i < (u32)LightType::Count; i++)
		{
			LightType lightType = (LightType)i;

			auto& lights = lightData.GetLights(lightType);
			u32 count = lightData.GetNumShadowedLights(lightType);
			u32 offset = lightData.GetNumUnshadowedLights(lightType);

			for(u32 j = 0; j < count; j++)
			{
				commandBuffer.BeginRenderPass(mLightOcclusionRT, RT_DEPTH, RT_DEPTH_STENCIL);

				Area2 area(0.0f, 0.0f, 1.0f, 1.0f);
				commandBuffer.SetViewport(area);

				commandBuffer.ClearViewport(FBT_COLOR, Color::kZero);

				u32 lightIdx = offset + j;
				const RendererLight& light = *lights[lightIdx];
				shadowRenderer.RenderShadowOcclusion(commandBuffer, inputs.View, light, gbuffer);
				commandBuffer.EndRenderPass();

				commandBuffer.BeginRenderPass(Output->RenderTarget, RT_DEPTH_STENCIL, RT_COLOR0 | RT_DEPTH_STENCIL);
				StandardDeferred::Instance().RenderLight(commandBuffer, lightType, light, inputs.View, gbuffer, lightOcclusionTex->Texture);
				commandBuffer.EndRenderPass();
			}
		}
	}

	// Makes sure light accumulation can be read by following passes
	commandBuffer.BeginRenderPass(nullptr); // TODO - RenderPass
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

	IrradianceEvaluateMat* evaluateMat;
	SPtr<PooledRenderTexture> volumeIndices;
	if(lightProbes.HasAnyProbes())
	{
		PooledRenderTextureCreateInformation volumeIndicesDesc;
		PooledRenderTextureCreateInformation depthDesc;
		TetrahedraRenderMat::GetOutputDesc(inputs.View, volumeIndicesDesc, depthDesc);

		volumeIndices = resPool.Get(volumeIndicesDesc);
		SPtr<PooledRenderTexture> depthTex = resPool.Get(depthDesc);

		RenderTextureCreateInformation rtDesc;
		rtDesc.ColorSurfaces[0].Texture = volumeIndices->Texture;
		rtDesc.DepthStencilSurface.Texture = depthTex->Texture;

		SPtr<RenderTexture> rt = RenderTexture::Create(rtDesc);

		commandBuffer.BeginRenderPass(rt);
		commandBuffer.ClearRenderTarget(FBT_DEPTH);
		GetRendererUtility().Clear(commandBuffer, -1);
		commandBuffer.EndRenderPass();

		TetrahedraRenderMat* renderTetrahedra =
			TetrahedraRenderMat::GetVariation(viewProps.Target.NumSamples > 1, true);
		renderTetrahedra->Execute(commandBuffer, inputs.View, sceneDepthNode->DepthTex->Texture, lpInfo.TetrahedraVolume, rt);

		rt = nullptr;
		depthTex = nullptr;

		evaluateMat = IrradianceEvaluateMat::GetVariation(viewProps.Target.NumSamples > 1, true, false);
	}
	else // Sky only
	{
		evaluateMat = IrradianceEvaluateMat::GetVariation(viewProps.Target.NumSamples > 1, true, true);
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

	// Ensure the light accumulation texture was previously written by shader in tile deferred lighting, ensure it can be used as an attachment now
	commandBuffer.IssueBarriers(GpuTextureBarrier(lightAccumNode->LightAccumulationTex->Texture,
		GpuResourceUseFlag::ShaderAccess, GpuAccessFlag::Write,
		GpuResourceUseFlag::ColorAttachment, GpuAccessFlag::Read | GpuAccessFlag::Write));

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

		TiledDeferredImageBasedLightingMat* material =
			TiledDeferredImageBasedLightingMat::GetVariation(viewProps.Target.NumSamples);

		TiledDeferredImageBasedLightingMat::Inputs iblInputs;
		iblInputs.Gbuffer = gbuffer;
		iblInputs.SceneColorTex = sceneColorNode->SceneColorTex->Texture;
		iblInputs.LightAccumulation = lightAccumNode->LightAccumulationTex->Texture;
		iblInputs.PreIntegratedGf = RendererTextures::preintegratedEnvGF;
		iblInputs.AmbientOcclusion = ssaoNode->Output;
		iblInputs.Ssr = ssrNode->Output;
		iblInputs.MsaaCoverage = msaaCoverage;

		if(sceneColorNode->SceneColorTexArray)
			iblInputs.SceneColorTexArray = sceneColorNode->SceneColorTexArray->Texture;

		// Ensure any reads to the scene color texture finish before attempting the write
		commandBuffer.IssueBarriers(GpuTextureBarrier(sceneColorNode->SceneColorTex->Texture, GpuResourceUseFlag::ShaderAccess | GpuResourceUseFlag::ColorAttachment, GpuAccessFlag::Read, GpuResourceUseFlag::ShaderAccess, GpuAccessFlag::Write));

		material->Execute(*inputs.ActiveCommandBuffer, inputs.View, inputs.Scene, inputs.ViewGroup.GetVisibleReflProbeData(), iblInputs);

		if(viewProps.Target.NumSamples > 1)
			sceneColorNode->MsaaTexArrayToTexture(*inputs.ActiveCommandBuffer);
	}
	else // Standard deferred
	{
		SPtr<RenderTexture> outputRT = lightAccumNode->RenderTarget;

		u32 width = viewProps.Target.ViewRect.Width;
		u32 height = viewProps.Target.ViewRect.Height;
		u32 numSamples = viewProps.Target.NumSamples;

		bool isMSAA = viewProps.Target.NumSamples > 1;

		SPtr<PooledRenderTexture> iblRadianceTex = GetGpuResourcePool().Get(
			PooledRenderTextureCreateInformation::Create2D(PF_RGBA16F, width, height, TU_RENDERTARGET, numSamples, false));

		RenderTextureCreateInformation rtDesc;
		rtDesc.ColorSurfaces[0].Texture = iblRadianceTex->Texture;
		rtDesc.DepthStencilSurface.Texture = sceneDepthNode->DepthTex->Texture;

		SPtr<GpuBuffer> perViewBuffer = inputs.View.GetPerViewBuffer();

		SPtr<RenderTexture> iblRadianceRT = RenderTexture::Create(rtDesc);
		commandBuffer.BeginRenderPass(iblRadianceRT, RT_DEPTH_STENCIL, RT_DEPTH_STENCIL);

		const VisibleReflProbeData& probeData = inputs.ViewGroup.GetVisibleReflProbeData();

		Skybox* skybox = nullptr;
		if(inputs.View.GetRenderSettings().EnableSkybox)
			skybox = inputs.Scene.Skybox;

		ReflProbeParamBuffer reflProbeParams;
		reflProbeParams.Populate(skybox, probeData.GetNumProbes(), inputs.Scene.ReflProbeCubemapsTex, viewProps.CapturingReflections);

		// Prepare the texture for refl. probe and skybox rendering
		{
			DeferredIBLSetupMat* mat = DeferredIBLSetupMat::GetVariation(isMSAA, true);
			mat->Bind(commandBuffer, gbuffer, perViewBuffer, ssrNode->Output, ssaoNode->Output, reflProbeParams.Buffer);

			GetRendererUtility().DrawScreenQuad(commandBuffer);

			// Draw pixels requiring per-sample evaluation
			if(isMSAA)
			{
				DeferredIBLSetupMat* msaaMat = DeferredIBLSetupMat::GetVariation(true, false);
				msaaMat->Bind(commandBuffer, gbuffer, perViewBuffer, ssrNode->Output, ssaoNode->Output, reflProbeParams.Buffer);

				GetRendererUtility().DrawScreenQuad(commandBuffer);
			}
		}

		if(!viewProps.CapturingReflections)
		{
			// Render refl. probes
			u32 numProbes = probeData.GetNumProbes();
			for(u32 i = 0; i < numProbes; i++)
			{
				const ReflProbeData& probe = probeData.GetProbeData(i);

				StandardDeferred::Instance().RenderReflProbe(commandBuffer, probe, inputs.View, gbuffer, inputs.Scene, reflProbeParams.Buffer);
			}

			// Render sky
			SPtr<Texture> skyFilteredRadiance;
			if(skybox)
				skyFilteredRadiance = skybox->GetFilteredRadiance();

			if(skyFilteredRadiance)
			{
				DeferredIBLSkyMat* skymat = DeferredIBLSkyMat::GetVariation(isMSAA, true);
				skymat->Bind(commandBuffer, gbuffer, perViewBuffer, skybox, reflProbeParams.Buffer);

				GetRendererUtility().DrawScreenQuad(commandBuffer);

				// Draw pixels requiring per-sample evaluation
				if(isMSAA)
				{
					DeferredIBLSkyMat* msaaMat = DeferredIBLSkyMat::GetVariation(true, false);
					msaaMat->Bind(commandBuffer, gbuffer, perViewBuffer, skybox, reflProbeParams.Buffer);

					GetRendererUtility().DrawScreenQuad(commandBuffer);
				}
			}
		}

		commandBuffer.EndRenderPass();

		// Finalize rendered reflections and output them to main render target
		{
			commandBuffer.BeginRenderPass(outputRT, RT_DEPTH_STENCIL, RT_COLOR0 | RT_DEPTH_STENCIL);

			DeferredIBLFinalizeMat* mat = DeferredIBLFinalizeMat::GetVariation(isMSAA, true);
			mat->Bind(commandBuffer, gbuffer, perViewBuffer, iblRadianceTex->Texture, RendererTextures::preintegratedEnvGF, reflProbeParams.Buffer);

			GetRendererUtility().DrawScreenQuad(commandBuffer);

			// Draw pixels requiring per-sample evaluation
			if(isMSAA)
			{
				DeferredIBLFinalizeMat* msaaMat = DeferredIBLFinalizeMat::GetVariation(true, false);
				msaaMat->Bind(commandBuffer, gbuffer, perViewBuffer, iblRadianceTex->Texture, RendererTextures::preintegratedEnvGF, reflProbeParams.Buffer);

				GetRendererUtility().DrawScreenQuad(commandBuffer);
			}

			commandBuffer.EndRenderPass();
		}

		// Makes sure light accumulation can be read by following passes
		commandBuffer.BeginRenderPass(nullptr); // TODO - RenderPass
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
	const VisibleReflProbeData& visibleReflProbeData = inputs.ViewGroup.GetVisibleReflProbeData();

	LightGridOutputs lightGridOutputs;

	struct StandardForwardBuffers
	{
		SPtr<GpuBuffer> LightsParamBlock;
		SPtr<GpuBuffer> ReflProbesParamBlock;
		SPtr<GpuBuffer> LightAndReflProbeParamsParamBlock;
	} standardForwardBuffers;

	const bool supportsClusteredForward = GetRenderBeast()->GetFeatureSet() == RenderBeastFeatureSet::Desktop;
	if(supportsClusteredForward)
	{
		const LightGrid& lightGrid = inputs.View.GetLightGrid();
		lightGridOutputs = lightGrid.GetOutputs();
	}
	else
	{
		// Note: Store these instead of creating them every time?
		standardForwardBuffers.LightsParamBlock = gLightsParamDef.CreateBuffer();
		standardForwardBuffers.ReflProbesParamBlock = gReflProbesParamDef.CreateBuffer();
		standardForwardBuffers.LightAndReflProbeParamsParamBlock = gLightAndReflProbeParamsParamDef.CreateBuffer();
	}

	Skybox* skybox = nullptr;
	if(inputs.View.GetRenderSettings().EnableSkybox)
		skybox = sceneInfo.Skybox;

	// Prepare refl. probe param buffer
	ReflProbeParamBuffer reflProbeParamBuffer;
	reflProbeParamBuffer.Populate(skybox, visibleReflProbeData.GetNumProbes(), sceneInfo.ReflProbeCubemapsTex, viewProps.CapturingReflections);

	SPtr<Texture> skyFilteredRadiance;
	if(skybox)
		skyFilteredRadiance = skybox->GetFilteredRadiance();

	const auto bindParamsForClustered = [&lightGridOutputs, &visibleLightData, &visibleReflProbeData](GpuParameters& gpuParams, const ForwardLightingParams& fwdParams, const ImageBasedLightingParams& iblParams)
	{
		const GpuParameterBinding& binding = fwdParams.GridParamsBinding;
		if(binding.Slot != (u32)-1)
			gpuParams.SetUniformBuffer(binding.Set, binding.Slot, lightGridOutputs.GridParams);

		fwdParams.GridLightOffsetsAndSizeParam.Set(lightGridOutputs.GridLightOffsetsAndSize);
		fwdParams.GridProbeOffsetsAndSizeParam.Set(lightGridOutputs.GridProbeOffsetsAndSize);

		fwdParams.GridLightIndicesParam.Set(lightGridOutputs.GridLightIndices);
		iblParams.ReflectionProbeIndicesParam.Set(lightGridOutputs.GridProbeIndices);

		fwdParams.LightsBufferParam.Set(visibleLightData.GetLightBuffer());
		iblParams.ReflectionProbesParam.Set(visibleReflProbeData.GetProbeBuffer());
	};

	const auto bindParamsForStandardForward = [&standardForwardBuffers, &visibleLightData, &visibleReflProbeData](GpuParameters& gpuParams, const Bounds& bounds, const ForwardLightingParams& fwdParams, const ImageBasedLightingParams& iblParams)
	{
		// Populate light & probe buffers
		Vector3I lightCounts;
		const LightData* lights[kStandardForwardMaxNumLights];
		visibleLightData.GatherInfluencingLights(bounds, lights, lightCounts);

		Vector4I lightOffsets;
		lightOffsets.X = lightCounts.X;
		lightOffsets.Y = lightCounts.X;
		lightOffsets.Z = lightOffsets.Y + lightCounts.Y;
		lightOffsets.W = lightOffsets.Z + lightCounts.Z;

		for(i32 j = 0; j < lightOffsets.W; j++)
			gLightsParamDef.gLights.Set(standardForwardBuffers.LightsParamBlock, *lights[j], j);

		i32 numReflProbes = std::min(visibleReflProbeData.GetNumProbes(), kStandardForwardMaxNumProbes);
		for(i32 j = 0; j < numReflProbes; j++)
		{
			gReflProbesParamDef.gReflectionProbes.Set(standardForwardBuffers.ReflProbesParamBlock, visibleReflProbeData.GetProbeData(j), j);
		}

		gLightAndReflProbeParamsParamDef.gLightOffsets.Set(standardForwardBuffers.LightAndReflProbeParamsParamBlock, lightOffsets);
		gLightAndReflProbeParamsParamDef.gReflProbeCount.Set(standardForwardBuffers.LightAndReflProbeParamsParamBlock, numReflProbes);

		if(iblParams.ReflProbesBinding.Set != (u32)-1)
		{
			gpuParams.SetUniformBuffer(
				iblParams.ReflProbesBinding.Set,
				iblParams.ReflProbesBinding.Slot,
				standardForwardBuffers.ReflProbesParamBlock);
		}

		if(fwdParams.LightsParamBlockBinding.Set != (u32)-1)
		{
			gpuParams.SetUniformBuffer(
				fwdParams.LightsParamBlockBinding.Set,
				fwdParams.LightsParamBlockBinding.Slot,
				standardForwardBuffers.LightsParamBlock);
		}

		if(fwdParams.LightAndReflProbeParamsParamBlockBinding.Set != (u32)-1)
		{
			gpuParams.SetUniformBuffer(
				fwdParams.LightAndReflProbeParamsParamBlockBinding.Set,
				fwdParams.LightAndReflProbeParamsParamBlockBinding.Slot,
				standardForwardBuffers.LightAndReflProbeParamsParamBlock);
		}
	};

	const auto bindCommonIBLParams = [&reflProbeParamBuffer, &skyFilteredRadiance, &sceneInfo](GpuParameters& gpuParams, ImageBasedLightingParams& iblParams)
	{
		// Note: Ideally these should be bound once (they are the same for all renderables)
		if(iblParams.ReflProbeParamBindings.Set != (u32)-1)
		{
			gpuParams.SetUniformBuffer(
				iblParams.ReflProbeParamBindings.Set,
				iblParams.ReflProbeParamBindings.Slot,
				reflProbeParamBuffer.Buffer);
		}

		iblParams.SkyReflectionsTexParam.Set(skyFilteredRadiance);
		iblParams.AmbientOcclusionTexParam.Set(Texture::kWhite); // Note: Add SSAO here?
		iblParams.SsrTexParam.Set(Texture::kBlack); // Note: Add SSR here?

		iblParams.ReflectionProbeCubemapsTexParam.Set(sceneInfo.ReflProbeCubemapsTex);
		iblParams.PreintegratedEnvBrdfParam.Set(RendererTextures::preintegratedEnvGF);
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
			const SPtr<GpuParameters> gpuParams = element.Params->GetGpuParams();
			if(supportsClusteredForward)
				bindParamsForClustered(*gpuParams, element.ForwardLightingParams, element.ImageBasedParams);
			else
			{
				// Populate light & probe buffers
				const Bounds& bounds = sceneInfo.RenderableCullInfos[i].Bounds;
				bindParamsForStandardForward(*gpuParams, bounds, element.ForwardLightingParams, element.ImageBasedParams);
			}

			bindCommonIBLParams(*gpuParams, element.ImageBasedParams);
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

			const SPtr<GpuParameters> gpuParams = renderElement.Params->GetGpuParams();

			// Note: It would be nice to be able to set this once and keep it, only updating if the buffers actually
			// change (e.g. when growing).
			if(supportsClusteredForward)
				bindParamsForClustered(*gpuParams, renderElement.ForwardLightingParams, renderElement.ImageBasedParams);
			else
			{
				// Populate light & probe buffers
				const Bounds& bounds = sceneInfo.ParticleSystemCullInfos[i].Bounds;
				bindParamsForStandardForward(*gpuParams, bounds, renderElement.ForwardLightingParams, renderElement.ImageBasedParams);
			}

			bindCommonIBLParams(*gpuParams, renderElement.ImageBasedParams);
		}
	}

	// TODO: Forward pipeline rendering doesn't support shadows. In order to support this I'd have to render the light
	// occlusion for all lights affecting this object into a single (or a few) textures. I can likely use texture
	// arrays for this, or to avoid sampling many textures, perhaps just jam it all in one or few texture channels.

	// Render everything
	GpuCommandBuffer& commandBuffer = *inputs.ActiveCommandBuffer;

	RenderQueue* opaqueQueue = inputs.View.GetOpaqueQueue(true).get();
	RenderQueue* transparentQueue = inputs.View.GetTransparentQueue().get();

	commandBuffer.BeginRenderPass(renderTarget, RT_NONE, RT_ALL);
	RenderQueueElements(commandBuffer, opaqueQueue->GetSortedElements());
	commandBuffer.EndRenderPass();

	// Allow depth-testing against the depth buffer after the writes above
	commandBuffer.IssueBarriers(GpuTextureBarrier(sceneDepthNode->DepthTex->Texture, GpuResourceUseFlag::DepthStencilAttachment, GpuAccessFlag::Write, GpuResourceUseFlag::DepthStencilAttachment, GpuAccessFlag::Read));

	commandBuffer.BeginRenderPass(renderTarget, RT_DEPTH, RT_ALL);
	RenderQueueElements(commandBuffer, transparentQueue->GetSortedElements());
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

	if(radiance != nullptr)
	{
		SkyboxMat* material = SkyboxMat::GetVariation(false);
		material->Bind(commandBuffer, inputs.View.GetPerViewBuffer(), radiance, Color::kWhite);
	}
	else
	{
		// Cancel out the linear->SRGB conversion
		Color clearColor = inputs.View.GetProperties().Target.ClearColor.GetLinear();

		SkyboxMat* material = SkyboxMat::GetVariation(true);
		material->Bind(commandBuffer, inputs.View.GetPerViewBuffer(), nullptr, clearColor);
	}

	auto dependencies = GetDependencyDefinition().ResolveDependencies(inputs);
	RCNodeSceneColor* sceneColorNode = dependencies.Get<RCNodeSceneColor>();

	// Ensure the scene color texture can be used as a color attachment, as previously it was written by a shader
	commandBuffer.IssueBarriers(GpuTextureBarrier(sceneColorNode->SceneColorTex->Texture, GpuResourceUseFlag::ShaderAccess, GpuAccessFlag::Write, GpuResourceUseFlag::ColorAttachment, GpuAccessFlag::Read | GpuAccessFlag::Write));

	commandBuffer.BeginRenderPass(sceneColorNode->RenderTarget, RT_DEPTH_STENCIL, RT_COLOR0 | RT_DEPTH_STENCIL);

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
	const RendererViewProperties& viewProps = inputs.View.GetProperties();

	SPtr<Texture> input;
	if(viewProps.RunPostProcessing)
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

	SPtr<RenderTarget> target = viewProps.Target.Target;

	GpuCommandBuffer& commandBuffer = *inputs.ActiveCommandBuffer;
	commandBuffer.BeginRenderPass(target);
	commandBuffer.SetViewport(viewProps.Target.NrmViewRect);

	GetRendererUtility().Blit(commandBuffer, input, Area2I::kEmpty, viewProps.FlipView);

	if(viewProps.EncodeDepth)
	{
		RCNodeResolvedSceneDepth* resolvedSceneDepthNode = dependencies.Get<RCNodeResolvedSceneDepth>();

		EncodeDepthMat* encodeDepthMat = EncodeDepthMat::Get();
		encodeDepthMat->Execute(commandBuffer, resolvedSceneDepthNode->Output->Texture, viewProps.DepthEncodeNear, viewProps.DepthEncodeFar, target);
	}

	commandBuffer.EndRenderPass();

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
			PooledRenderTextureCreateInformation::Create2D(PF_RGBA8, width, height, TU_RENDERTARGET, 1, false));
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
				resPool.Get(EyeAdaptHistogramMat::GetOutputDesc(downsampledScene->Texture));
			EyeAdaptHistogramMat* eyeAdaptHistogramMat = EyeAdaptHistogramMat::Get();
			eyeAdaptHistogramMat->Execute(commandBuffer, downsampledScene->Texture, eyeAdaptHistogram->Texture, settings.AutoExposure);

			// Ensure eye adaptation histogram texture can be read after the write above
			commandBuffer.IssueBarriers(GpuTextureBarrier(eyeAdaptHistogram->Texture, GpuResourceUseFlag::ShaderAccess, GpuAccessFlag::Write, GpuResourceUseFlag::ShaderAccess, GpuAccessFlag::Read));

			// Reduce histogram
			SPtr<PooledRenderTexture> reducedHistogram = resPool.Get(EyeAdaptHistogramReduceMat::GetOutputDesc());

			SPtr<Texture> prevFrameEyeAdaptation;
			if(previous != nullptr)
				prevFrameEyeAdaptation = previous->Texture;

			EyeAdaptHistogramReduceMat* eyeAdaptHistogramReduce = EyeAdaptHistogramReduceMat::Get();
			eyeAdaptHistogramReduce->Execute(commandBuffer, 
				downsampledScene->Texture,
				eyeAdaptHistogram->Texture,
				prevFrameEyeAdaptation,
				reducedHistogram->RenderTexture);

			eyeAdaptHistogram = nullptr;

			// Generate eye adaptation value
			Output = resPool.Get(EyeAdaptationMat::GetOutputDesc());
			EyeAdaptationMat* eyeAdaptationMat = EyeAdaptationMat::Get();
			eyeAdaptationMat->Execute(commandBuffer,
				reducedHistogram->Texture,
				Output->RenderTexture,
				inputs.FrameInfo.Timings.TimeDelta,
				settings.AutoExposure,
				settings.ExposureScale);
		}
		else
		{
			// Populate alpha values of the downsampled texture with luminance
			SPtr<PooledRenderTexture> luminanceTex =
				resPool.Get(EyeAdaptationBasicSetupMat::GetOutputDesc(downsampledScene->Texture));

			EyeAdaptationBasicSetupMat* setupMat = EyeAdaptationBasicSetupMat::Get();
			setupMat->Execute(commandBuffer,
				downsampledScene->Texture,
				luminanceTex->RenderTexture,
				inputs.FrameInfo.Timings.TimeDelta,
				settings.AutoExposure,
				settings.ExposureScale);

			SPtr<Texture> downsampleInput = luminanceTex->Texture;
			luminanceTex = nullptr;

			// Downsample some more
			for(u32 i = 0; i < 5; i++)
			{
				DownsampleMat* downsampleMat = DownsampleMat::GetVariation(1, false);
				SPtr<PooledRenderTexture> downsampledLuminance =
					resPool.Get(DownsampleMat::GetOutputDesc(downsampleInput));

				downsampleMat->Execute(commandBuffer, downsampleInput, downsampledLuminance->RenderTexture);
				downsampleInput = downsampledLuminance->Texture;
			}

			// Generate eye adaptation value
			EyeAdaptationBasicMat* eyeAdaptationMat = EyeAdaptationBasicMat::Get();

			SPtr<Texture> prevFrameEyeAdaptation;
			if(previous != nullptr)
				prevFrameEyeAdaptation = previous->Texture;

			Output = resPool.Get(EyeAdaptationBasicMat::GetOutputDesc());
			eyeAdaptationMat->Execute(commandBuffer,
				downsampleInput,
				prevFrameEyeAdaptation,
				Output->RenderTexture,
				inputs.FrameInfo.Timings.TimeDelta,
				settings.AutoExposure,
				settings.ExposureScale);
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
					CreateTonemap3DLUTMat* createLUT = CreateTonemap3DLUTMat::Get();
					if(mTonemapLUT == nullptr)
						mTonemapLUT = GetGpuResourcePool().Get(createLUT->GetOutputDesc());

					createLUT->Execute(commandBuffer, mTonemapLUT->Texture, settings);
				}
				else
				{
					CreateTonemap2DLUTMat* createLUT = CreateTonemap2DLUTMat::Get();
					if(mTonemapLUT == nullptr)
						mTonemapLUT = GetGpuResourcePool().Get(createLUT->GetOutputDesc());

					createLUT->Execute(commandBuffer, mTonemapLUT->RenderTexture, settings);
				}

				// Ensure tonemap LUT can be read by the tonemapping shader after the write above
				commandBuffer.IssueBarriers(GpuTextureBarrier(mTonemapLUT->Texture, GpuResourceUseFlag::ShaderAccess, GpuAccessFlag::Write, GpuResourceUseFlag::ShaderAccess, GpuAccessFlag::Read));

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

	TonemappingMat* tonemapping = TonemappingMat::GetVariation(volumeLUT, gammaOnly, autoExposure, msaa);

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

	tonemapping->Execute(commandBuffer, sceneColor, eyeAdaptationTex, bloomTex, tonemapLUTTex, ppOutput, settings);
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

	BokehDOFPrepareMat* prepareMat = BokehDOFPrepareMat::GetVariation(msaa);
	BokehDOFMat* renderMat = BokehDOFMat::GetVariation(settings.BokehOcclusion);
	BokehDOFCombineMat* combineMat = BokehDOFCombineMat::GetVariation(msaa ? MSAAMode::Full : MSAAMode::None);

	SPtr<Texture> depth = sceneDepthNode->DepthTex->Texture;

	// Downsample scene and store depth in .w
	SPtr<PooledRenderTexture> halfResSceneAndDepth =
		GetGpuResourcePool().Get(BokehDOFPrepareMat::GetOutputDesc(sceneColorNode->SceneColorTex->Texture));

	prepareMat->Execute(commandBuffer, sceneColorNode->SceneColorTex->Texture, depth, inputs.View, settings, halfResSceneAndDepth->RenderTexture);

	SPtr<PooledRenderTexture> unfocusedTex =
		GetGpuResourcePool().Get(BokehDOFMat::GetOutputDesc(halfResSceneAndDepth->Texture));

	renderMat->Execute(commandBuffer, halfResSceneAndDepth->Texture, inputs.View, settings, unfocusedTex->RenderTexture);
	halfResSceneAndDepth = nullptr;

	// Combine the unfocused and focused textures to form the final image
	combineMat->Execute(commandBuffer, unfocusedTex->Texture, sceneColorNode->SceneColorTex->Texture, depth, inputs.View, settings, lightAccumNode->LightAccumulationTex->RenderTexture);

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
		resolvedSceneColor = resPool.Get(PooledRenderTextureCreateInformation::Create2D(PF_RGBA16F, width, height, TU_RENDERTARGET));

		commandBuffer.BeginRenderPass(resolvedSceneColor->RenderTexture);
		GetRendererUtility().Blit(commandBuffer, sceneColor->Texture);
		commandBuffer.EndRenderPass();

		sceneColor = resolvedSceneColor;
	}

	if(mPrevFrame)
	{
		mPooledOutput = resPool.Get(PooledRenderTextureCreateInformation::Create2D(PF_RGBA16F, width, height, TU_RENDERTARGET));

		SPtr<Texture> velocityTex;
		if(basePassNode->VelocityTex)
			velocityTex = basePassNode->VelocityTex->Texture;

		float exposure = inputs.View.GetCurrentExposure();

		TemporalFilteringMat* temporalFilteringMat =
			TemporalFilteringMat::GetVariation(TemporalFilteringType::FullScreenAA, true, viewProps.Target.NumSamples > 1);
		temporalFilteringMat->Execute(commandBuffer, inputs.View, mPrevFrame->Texture, sceneColor->Texture, velocityTex, sceneDepthNode->DepthTex->Texture, viewProps.TemporalJitter, exposure, mPooledOutput->RenderTexture);

		sceneColorNode->SetExternalTexture(mPooledOutput);
	}
	else
		mPooledOutput = sceneColor;

	commandBuffer.BeginRenderPass(nullptr); // TODO - RenderPass
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

	// MotionBlurMat* motionBlurMat = MotionBlurMat::Get();

	// SPtr<Texture> depth = sceneDepthNode->depthTex->texture;
	// motionBlurMat->Execute(sceneColorNode->sceneColorTex->texture, depth, inputs.view, settings,
	//	lightAccumNode->lightAccumulationTex->renderTexture);

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

	GaussianDOFSeparateMat* separateMat = GaussianDOFSeparateMat::GetVariation(near, far);
	GaussianDOFCombineMat* combineMat = GaussianDOFCombineMat::GetVariation(near, far);
	GaussianBlurMat* blurMat = GaussianBlurMat::Get();

	SPtr<RenderTexture> ppOutput;
	SPtr<Texture> ppLastFrame;
	postProcessNode->GetAndSwitch(inputs.View, ppOutput, ppLastFrame);

	separateMat->Execute(commandBuffer, ppLastFrame, sceneDepthNode->DepthTex->Texture, inputs.View, settings);

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
	PooledRenderTextureCreateInformation tempTexDesc = PooledRenderTextureCreateInformation::Create2D(texProps.Format, texProps.Width, texProps.Height, TU_RENDERTARGET);
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

	combineMat->Execute(commandBuffer, ppLastFrame, blurredNearTex, blurredFarTex, sceneDepthNode->DepthTex->Texture, ppOutput, inputs.View, settings);

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
	FXAAMat* fxaa = FXAAMat::Get();
	fxaa->Execute(*inputs.ActiveCommandBuffer, ppLastFrame, ppOutput);
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

	ChromaticAberrationMat* chromaticAberration = ChromaticAberrationMat::GetVariation(settings.ChromaticAberration.Type);
	chromaticAberration->Execute(*inputs.ActiveCommandBuffer, ppLastFrame, settings.ChromaticAberration, ppOutput);
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

	FilmGrainMat* filmGrain = FilmGrainMat::Get();
	filmGrain->Execute(*inputs.ActiveCommandBuffer, ppLastFrame, inputs.FrameInfo.Timings.Time, settings.FilmGrain, ppOutput);
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
	DownsampleMat* downsampleMat = DownsampleMat::GetVariation(1, msaa);

	Output = GetGpuResourcePool().Get(DownsampleMat::GetOutputDesc(input));

	// Ensure the scene color texture can be read by a shader
	inputs.ActiveCommandBuffer->IssueBarriers(GpuTextureBarrier(sceneColorNode->SceneColorTex->Texture, GpuResourceUseFlag::ShaderAccess | GpuResourceUseFlag::ColorAttachment, GpuAccessFlag::Write, GpuResourceUseFlag::ShaderAccess, GpuAccessFlag::Read));

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

		DownsampleMat* downsampleMat = DownsampleMat::GetVariation(1, false);
		for(u32 i = 1; i < AvailableDownsamples; i++)
		{
			Output[i] = resPool.Get(DownsampleMat::GetOutputDesc(Output[i - 1]->Texture));
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
			PooledRenderTextureCreateInformation::Create2D(PF_D32_S8X24, width, height, TU_DEPTHSTENCIL, 1, false));

		commandBuffer.BeginRenderPass(Output->RenderTexture);
		commandBuffer.ClearRenderTarget(FBT_STENCIL);
		GetRendererUtility().Blit(*inputs.ActiveCommandBuffer, sceneDepthNode->DepthTex->Texture, Area2I::kEmpty, false, true);
		commandBuffer.EndRenderPass();
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
		PooledRenderTextureCreateInformation::Create2D(PF_R32F, size, size, TU_RENDERTARGET, 1, false, 1, numMips));

	Area2 srcRect = viewProps.Target.NrmViewRect;

	// If viewport size is odd, adjust UV
	srcRect.Width += (viewProps.Target.ViewRect.Width % 2) * (1.0f / viewProps.Target.ViewRect.Width);
	srcRect.Height += (viewProps.Target.ViewRect.Height % 2) * (1.0f / viewProps.Target.ViewRect.Height);

	bool noTextureViews = !commandBuffer.GetGpuDevice().GetCapabilities().HasCapability(RSC_TEXTURE_VIEWS);

	BuildHiZMat* material = BuildHiZMat::GetVariation(noTextureViews);

	// Generate first mip
	RenderTextureCreateInformation rtDesc;
	rtDesc.ColorSurfaces[0].Texture = Output->Texture;
	rtDesc.ColorSurfaces[0].MipLevel = 0;

	SPtr<RenderTexture> rt = RenderTexture::Create(rtDesc);

	Area2 destRect;
	bool downsampledFirstMip = false; // Not used currently
	if(downsampledFirstMip)
	{
		// Make sure that 1 pixel in HiZ maps to a 2x2 block in source
		destRect = Area2(0, 0, Math::CeilToInt(viewProps.Target.ViewRect.Width / 2.0f) / (float)size, Math::CeilToInt(viewProps.Target.ViewRect.Height / 2.0f) / (float)size);

		material->Execute(commandBuffer, resolvedSceneDepth->Output->Texture, 0, srcRect, destRect, rt);
	}
	else // First level is just a copy of the depth buffer
	{
		destRect = Area2(0, 0, viewProps.Target.ViewRect.Width / (float)size, viewProps.Target.ViewRect.Height / (float)size);

		commandBuffer.BeginRenderPass(rt);
		commandBuffer.SetViewport(destRect);

		Area2I srcAreaInt;
		srcAreaInt.X = (i32)(srcRect.X * viewProps.Target.ViewRect.Width);
		srcAreaInt.Y = (i32)(srcRect.Y * viewProps.Target.ViewRect.Height);
		srcAreaInt.Width = (u32)(srcRect.Width * viewProps.Target.ViewRect.Width);
		srcAreaInt.Height = (u32)(srcRect.Height * viewProps.Target.ViewRect.Height);

		GetRendererUtility().Blit(commandBuffer, resolvedSceneDepth->Output->Texture, srcAreaInt);
		commandBuffer.EndRenderPass();
		commandBuffer.SetViewport(Area2(0, 0, 1, 1));
	}

	// Generate remaining mip levels
	for(u32 i = 1; i <= numMips; i++)
	{
		rtDesc.ColorSurfaces[0].MipLevel = i;
		rt = RenderTexture::Create(rtDesc);

		material->Execute(commandBuffer, Output->Texture, i - 1, destRect, destRect, rt);
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
		PooledRenderTextureCreateInformation desc = PooledRenderTextureCreateInformation::Create2D(normalsProps.Format, normalsProps.Width, normalsProps.Height, TU_RENDERTARGET);
		resolvedNormals = resPool.Get(desc);

		commandBuffer.BeginRenderPass(resolvedNormals->RenderTexture);
		GetRendererUtility().Blit(commandBuffer, sceneNormals);
		commandBuffer.EndRenderPass();

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

	SSAODownsampleMat* downsample = SSAODownsampleMat::Get();

	SPtr<PooledRenderTexture> setupTex0;
	if(numDownsampleLevels > 0)
	{
		Vector2I downsampledSize(
			std::max(1, Math::DivideAndRoundUp((i32)viewProps.Target.ViewRect.Width, 2)),
			std::max(1, Math::DivideAndRoundUp((i32)viewProps.Target.ViewRect.Height, 2)));

		PooledRenderTextureCreateInformation desc = PooledRenderTextureCreateInformation::Create2D(PF_RGBA16F, downsampledSize.X, downsampledSize.Y, TU_RENDERTARGET);
		setupTex0 = resPool.Get(desc);

		downsample->Execute(commandBuffer, inputs.View, sceneDepth, sceneNormals, setupTex0->RenderTexture, kDepthRange);
	}

	SPtr<PooledRenderTexture> setupTex1;
	if(numDownsampleLevels > 1)
	{
		Vector2I downsampledSize(
			std::max(1, Math::DivideAndRoundUp((i32)viewProps.Target.ViewRect.Width, 4)),
			std::max(1, Math::DivideAndRoundUp((i32)viewProps.Target.ViewRect.Height, 4)));

		PooledRenderTextureCreateInformation desc = PooledRenderTextureCreateInformation::Create2D(PF_RGBA16F, downsampledSize.X, downsampledSize.Y, TU_RENDERTARGET);
		setupTex1 = resPool.Get(desc);

		downsample->Execute(commandBuffer, inputs.View, sceneDepth, sceneNormals, setupTex1->RenderTexture, kDepthRange);
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

		PooledRenderTextureCreateInformation desc = PooledRenderTextureCreateInformation::Create2D(PF_R8, downsampledSize.X, downsampledSize.Y, TU_RENDERTARGET);
		downAOTex1 = resPool.Get(desc);

		SSAOMat* ssaoMat = SSAOMat::GetVariation(false, false, quality);
		ssaoMat->Execute(commandBuffer, inputs.View, textures, downAOTex1->RenderTexture, settings);

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

		PooledRenderTextureCreateInformation desc = PooledRenderTextureCreateInformation::Create2D(PF_R8, downsampledSize.X, downsampledSize.Y, TU_RENDERTARGET);
		downAOTex0 = resPool.Get(desc);

		bool upsample = numDownsampleLevels > 1;
		SSAOMat* ssaoMat = SSAOMat::GetVariation(upsample, false, quality);
		ssaoMat->Execute(commandBuffer, inputs.View, textures, downAOTex0->RenderTexture, settings);

		if(upsample)
			downAOTex1 = nullptr;
	}

	u32 width = viewProps.Target.ViewRect.Width;
	u32 height = viewProps.Target.ViewRect.Height;
	mPooledOutput = resPool.Get(PooledRenderTextureCreateInformation::Create2D(PF_R8, width, height, TU_RENDERTARGET));

	{
		if(setupTex0)
			textures.AoSetup = setupTex0->Texture;

		if(downAOTex0)
			textures.AoDownsampled = downAOTex0->Texture;

		bool upsample = numDownsampleLevels > 0;
		SSAOMat* ssaoMat = SSAOMat::GetVariation(upsample, true, quality);
		ssaoMat->Execute(commandBuffer, inputs.View, textures, mPooledOutput->RenderTexture, settings);
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

		PooledRenderTextureCreateInformation desc = PooledRenderTextureCreateInformation::Create2D(PF_R8, rtProps.Width, rtProps.Height, TU_RENDERTARGET);
		SPtr<PooledRenderTexture> blurIntermediateTex = resPool.Get(desc);

		SSAOBlurMat* blurHorz = SSAOBlurMat::GetVariation(true);
		SSAOBlurMat* blurVert = SSAOBlurMat::GetVariation(false);

		blurHorz->Execute(commandBuffer, inputs.View, mPooledOutput->Texture, sceneDepth, blurIntermediateTex->RenderTexture, kDepthRange);
		blurVert->Execute(commandBuffer, inputs.View, blurIntermediateTex->Texture, sceneDepth, mPooledOutput->RenderTexture, kDepthRange);
	}

	commandBuffer.BeginRenderPass(nullptr); // TODO - RenderPass
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
		resolvedSceneColor = resPool.Get(PooledRenderTextureCreateInformation::Create2D(PF_RGBA16F, width, height, TU_RENDERTARGET));

		commandBuffer.BeginRenderPass(resolvedSceneColor->RenderTexture);
		GetRendererUtility().Blit(commandBuffer, sceneColor);
		commandBuffer.EndRenderPass();

		sceneColor = resolvedSceneColor->Texture;
	}
	else
	{
		sceneColor = lightAccumNode->LightAccumulationTex->Texture;

		// Light accumulation was written by a shader and used as a color attachment, issue barrier so its safe to read
		commandBuffer.IssueBarriers(GpuTextureBarrier(sceneColor, GpuResourceUseFlag::ShaderAccess, GpuAccessFlag::Write, GpuResourceUseFlag::ShaderAccess, GpuAccessFlag::Read));
	}

	GBufferTextures gbuffer;
	gbuffer.Albedo = gbufferNode->AlbedoTex->Texture;
	gbuffer.Normals = gbufferNode->NormalTex->Texture;
	gbuffer.RoughMetal = gbufferNode->RoughMetalTex->Texture;
	gbuffer.Depth = sceneDepthNode->DepthTex->Texture;

	SSRStencilMat* stencilMat = SSRStencilMat::GetVariation(viewProps.Target.NumSamples > 1, true);

	// Note: Making the assumption that the stencil buffer is clear at this point
	commandBuffer.BeginRenderPass(resolvedSceneDepthNode->Output->RenderTexture, RT_DEPTH, RT_DEPTH_STENCIL);
	stencilMat->Execute(commandBuffer, inputs.View, gbuffer, settings);
	commandBuffer.EndRenderPass();

	SPtr<PooledRenderTexture> traceOutput = resPool.Get(PooledRenderTextureCreateInformation::Create2D(PF_RGBA16F, width, height, TU_RENDERTARGET));

	RenderTextureCreateInformation traceRtDesc;
	traceRtDesc.ColorSurfaces[0].Texture = traceOutput->Texture;
	traceRtDesc.DepthStencilSurface.Texture = resolvedSceneDepthNode->Output->Texture;

	SPtr<RenderTexture> traceRt = RenderTexture::Create(traceRtDesc);

	SSRTraceMat* traceMat = SSRTraceMat::GetVariation(settings.Quality, viewProps.Target.NumSamples > 1, true);
	traceMat->Execute(commandBuffer, inputs.View, gbuffer, sceneColor, hiZ, settings, traceRt);

	resolvedSceneColor = nullptr;

	mUsingTemporalAA = inputs.View.GetRenderSettings().TemporalAa.Enabled;
	if(mPrevFrame && !mUsingTemporalAA)
	{
		mPooledOutput = resPool.Get(PooledRenderTextureCreateInformation::Create2D(PF_RGBA16F, width, height, TU_RENDERTARGET));

		TemporalFilteringMat* temporalFilteringMat =
			TemporalFilteringMat::GetVariation(TemporalFilteringType::SSR, false, viewProps.Target.NumSamples > 1);
		temporalFilteringMat->Execute(commandBuffer, inputs.View, mPrevFrame->Texture, traceOutput->Texture, nullptr, sceneDepthNode->DepthTex->Texture, Vector2::kZero, 1.0f, mPooledOutput->RenderTexture);

		traceOutput = nullptr;
	}
	else
		mPooledOutput = traceOutput;

	commandBuffer.BeginRenderPass(nullptr); // TODO - RenderPass
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

	GaussianBlurMat* filterMat = GaussianBlurMat::GetVariation(true);

	const bool autoExposure = settings.EnableHdr && settings.EnableAutoExposure;
	BloomClipMat* clipMat = BloomClipMat::GetVariation(autoExposure);

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
				TU_RENDERTARGET));

		SPtr<PooledRenderTexture> blurInput = downsampledTex;
		SPtr<PooledRenderTexture> blurOutput = filterOutput;
		if(settings.Bloom.Threshold > 0.0f)
		{
			clipMat->Execute(*inputs.ActiveCommandBuffer, downsampledTex->Texture, settings.Bloom.Threshold, eyeAdaptationTex, settings, filterOutput->RenderTexture);

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
			TU_RENDERTARGET));

	bool haloAspect = lensFlareSettings.HaloAspectRatio != 1.0f;
	ScreenSpaceLensFlareMat* lensFlareMat = ScreenSpaceLensFlareMat::GetVariation(
		lensFlareSettings.Halo,
		haloAspect,
		lensFlareSettings.ChromaticAberration);
	lensFlareMat->Execute(commandBuffer, downsampledTex->Texture, lensFlareSettings, featureTex->RenderTexture);

	// Blur
	GaussianBlurMat* filterMat = GaussianBlurMat::Get();
	filterMat->Execute(commandBuffer, featureTex->Texture, lensFlareSettings.FilterSize, downsampledTex->RenderTexture, Color::kWhite);

	auto* sceneColorNode = static_cast<RCNodeSceneColor*>(inputs.InputNodes[1]);

	if(lensFlareSettings.BicubicUpsampling)
	{
		BicubicUpsampleMat* upsampleMat = BicubicUpsampleMat::GetVariation(false);
		upsampleMat->Execute(commandBuffer, 
			downsampledTex->Texture,
			sceneColorNode->RenderTarget,
			Color::kWhite * lensFlareSettings.Brightness);
	}
	else
	{
		CompositeMat* upsampleMat = CompositeMat::Get();
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
