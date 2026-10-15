//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
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
#include "GpuBackend/B3DGpuCommandBuffer.h"
#include "GpuBackend/B3DRenderTexture.h"
#include "FrameGraph/B3DFrameGraph.h"
#include "Image/B3DPixelUtility.h"

namespace b3d { namespace render {

UnorderedMap<StringID, RenderCompositor::NodeDescriptor*> RenderCompositor::mNodeDescriptors;

namespace
{
	/** Returns the description of a two-dimensional texture the size of @p view, with one sample per pixel unless the view has more. */
	TextureCreateInformation CreateViewTextureInformation(const RendererView& view, PixelFormat format, TextureUsageFlags usage, bool hardwareSrgb = false)
	{
		const RendererViewProperties& viewProperties = view.GetProperties();

		TextureCreateInformation information;
		information.Format = format;
		information.Width = viewProperties.Target.ViewRect.Width;
		information.Height = viewProperties.Target.ViewRect.Height;
		information.Usage = usage;
		information.UseHardwareSRGB = hardwareSrgb;
		information.SampleCount = viewProperties.Target.NumSamples;

		return information;
	}
} // namespace

/**
 * Lets the draws of @p pass bind the parameter sets of the render queue elements. The per-pass renderer inputs are added as pass
 * sets, the material and per-object sets as render pass sets, which hold only resources at rest.
 */
void PrepareRenderQueuePass(FrameGraphRasterPass& pass, const Vector<RenderQueueEntry>& entries, const SurfaceShaderParameterSets& surfaceShaderParameterSets)
{
	surfaceShaderParameterSets.DeclareSets(pass, entries);

	TInlineArray<TShared<GpuParameterSet>, 4> perObjectParameterSets;

	for(const auto& entry : entries)
	{
		TShared<GpuParameterSet> parameterSet0 = entry.DrawCommand->ParameterAdapter->GetGpuParameterSet(entry.PassIndex);
		if(parameterSet0 != nullptr)
			pass.AddRenderPassParameterSet(parameterSet0);

		const TShared<GpuParameterSet>& parameterSet1 = entry.DrawCommand->SharedPerObjectParameterSet;
		if(parameterSet1 == nullptr)
			continue;

		auto found = std::find(perObjectParameterSets.begin(), perObjectParameterSets.end(), parameterSet1);
		if(found == perObjectParameterSets.end())
		{
			pass.AddRenderPassParameterSet(parameterSet1);
			perObjectParameterSets.Add(parameterSet1);
		}
	}
}

/** Renders all elements in a render queue. */
void RenderQueueEntries(GpuCommandBuffer& commandBuffer, const Vector<RenderQueueEntry>& entries, const SurfaceShaderParameterSets& surfaceShaderParameterSets)
{
	// TODO: Consider sorting elements by SharedPerObjectParameterSet to minimize parameter set rebinds.
	// Currently respecting existing RenderQueue sorting rules (material, distance, etc.).

	const u32 perObjectDynamicOffsetIndex = GetRenderBeast()->GetRenderableParameterSetInfo().PerObjectDynamicOffsetIndex;
	TShared<GpuParameterSet> lastBoundPerObjectSet;
	u32 boundShaderType = ~0u;

	for(auto& entry : entries)
	{
		if(entry.ApplyPass)
			GetRendererUtility().SetPass(commandBuffer, entry.DrawCommand->Material, entry.PassIndex, entry.VariationIndex);

		// Bind shared per-object parameter set if changed (using SetGpuParameterSet directly, not SetPassParams)
		if(entry.DrawCommand->SharedPerObjectParameterSet != lastBoundPerObjectSet)
		{
			if(entry.DrawCommand->SharedPerObjectParameterSet)
			{
				commandBuffer.SetGpuParameterSet(entry.DrawCommand->SharedPerObjectParameterSet);
				lastBoundPerObjectSet = entry.DrawCommand->SharedPerObjectParameterSet;
			}
		}

		// Set the dynamic buffer offset for this element's per-object data
		commandBuffer.SetDynamicBufferOffset(GpuPipelineSet::kPerObjectSetIndex, perObjectDynamicOffsetIndex, entry.DrawCommand->PerObjectBufferOffset);

		surfaceShaderParameterSets.Bind(commandBuffer, entry, boundShaderType);

		GetRendererUtility().SetPassParams(commandBuffer, entry.DrawCommand->ParameterAdapter, entry.PassIndex);

		entry.DrawCommand->Draw(commandBuffer);
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

	FrameVector<const NodeInfo*> activeNodes;

	u32 idx = 0;
	for(auto& entry : mRegisteredNodes)
	{
		inputs.InputNodes = entry.Inputs;
		entry.Node->Render(inputs);

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
	DepthTex = inputs.Graph.CreateTexture("SceneDepth", CreateViewTextureInformation(inputs.View, PF_D32_S8X24, TextureUsageFlag::DepthStencil));
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
	// Note: Consider customizable formats. e.g. for testing if quality can be improved with higher precision normals.
	AlbedoTex = inputs.Graph.CreateTexture("GBufferAlbedo", CreateViewTextureInformation(inputs.View, PF_RGBA8, TextureUsageFlag::RenderTarget, true));
	NormalTex = inputs.Graph.CreateTexture("GBufferNormals", CreateViewTextureInformation(inputs.View, PF_RGB10A2, TextureUsageFlag::RenderTarget));
	RoughMetalTex = inputs.Graph.CreateTexture("GBufferRoughMetal", CreateViewTextureInformation(inputs.View, PF_RG16F, TextureUsageFlag::RenderTarget)); // Note: Metal doesn't need 16-bit float
	IdTex = inputs.Graph.CreateTexture("GBufferId", CreateViewTextureInformation(inputs.View, PF_R8, TextureUsageFlag::RenderTarget));

#if B3D_FRAME_GRAPH_PORT_PENDING(TemporalAA)
	bool needsVelocity = inputs.View.RequiresVelocityWrites();
	if(needsVelocity)
	{
		VelocityTex = resPool.Get(PooledRenderTextureCreateInformation::Create2D(PF_RG16S, width, height, TextureUsageFlag::RenderTarget, numSamples, false));
	}
#endif

	auto dependencies = GetDependencyDefinition().ResolveDependencies(inputs);
	auto sceneDepthNode = dependencies.Get<RCNodeSceneDepth>();
	auto sceneColorNode = dependencies.Get<RCNodeSceneColor>();

	// Prepare all visible objects. Note that this also prepares non-opaque objects.
	const VisibilityInfo& visibility = inputs.View.GetVisibilityMasks();

	mSurfaceShaderParameterSets.Prepare(inputs.View, inputs.Scene);
#if B3D_FRAME_GRAPH_PORT_PENDING(Decals)
	mSurfaceShaderParameterSets.SetDecalInputs(sceneDepthTex->Texture, IdTex->Texture);
#endif

	//// Prepare particle systems
	// The views of a group share the draw command of a particle system, so the passes of every view draw with the inputs that the
	// last view binds. The inputs only differ in the order that distance sorting gives the particles of each view.
	const EvaluatedParticleData* particleData = inputs.FrameInfo.PerSceneFrameData.Particles;
	if(particleData)
	{
		const auto particleSystemCount = inputs.Scene.GetParticleSystemCount();

		for(u32 i = 0; i < particleSystemCount; i++)
		{
			if(!visibility.ParticleSystems[i])
				continue;

			const ParticleRenderState& renderState = inputs.Scene.GetParticleRenderState(i);
			ParticlesDrawCommand& drawCommand = renderState.DrawCommand;

			if(!drawCommand.IsValid())
				continue;

			const render::ParticleSystemProxy& proxy = inputs.Scene.GetParticleSystemProxy(i);

			// Bind textures/buffers from CPU simulation
			const auto iterFind = particleData->CpuData.find(proxy.GetId());
			if(iterFind != particleData->CpuData.end())
			{
				ParticleRenderData* renderData = iterFind->second;
				renderState.BindCpuSimulatedInputs(proxy, renderData);
			}
#if B3D_FRAME_GRAPH_PORT_PENDING(Particles)
			// Bind textures/buffers from GPU simulation
			else if(renderState.GpuParticleSystem)
			{
				renderState.BindGpuSimulatedInputs(proxy, GpuParticleSimulation::Instance().GetResources());
			}
#endif
		}
	}

#if B3D_FRAME_GRAPH_PORT_PENDING(Extensions)
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

	// Trigger pre-base-pass callbacks
	if(sceneCamera != nullptr)
	{
		inputs.View.NotifyCompositorTargetChanged(RenderTarget);

		for(auto& extension : inputs.ExtPreBasePass)
		{
			RendererViewContext context;
			context.CurrentTarget = inputs.View.GetCompositorRenderTarget();
			context.CommandBuffer = inputs.ActiveCommandBuffer;

			extension->Render(*sceneCamera, context);
		}
	}
#endif

	// Render all visible opaque elements that use the deferred pipeline, into a cleared G-buffer
	FrameGraphRasterPass basePass;

	u32 colorIndex = 0;
	basePass.SetColorAttachment(colorIndex++, sceneColorNode->SceneColorTex, FrameGraphAttachmentLoad::Clear);
	basePass.SetColorAttachment(colorIndex++, AlbedoTex, FrameGraphAttachmentLoad::Clear);
	basePass.SetColorAttachment(colorIndex++, NormalTex, FrameGraphAttachmentLoad::Clear);
	basePass.SetColorAttachment(colorIndex++, RoughMetalTex, FrameGraphAttachmentLoad::Clear);

#if B3D_FRAME_GRAPH_PORT_PENDING(TemporalAA)
	if(needsVelocity)
		basePass.SetColorAttachment(colorIndex++, VelocityTex, FrameGraphAttachmentLoad::Clear);
#endif

	basePass.SetColorAttachment(colorIndex, IdTex, FrameGraphAttachmentLoad::Clear);
	basePass.SetDepthStencilAttachment(sceneDepthNode->DepthTex, FrameGraphDepthStencilMode::Write, FrameGraphAttachmentLoad::Clear);

	// The render queue lives until the view's frame ends, after the graph executes
	const Vector<RenderQueueEntry>& opaqueEntries = inputs.View.GetOpaqueQueue(false)->GetSortedEntries();
	PrepareRenderQueuePass(basePass, opaqueEntries, mSurfaceShaderParameterSets);

	inputs.Graph.AddRasterPass("BasePass", std::move(basePass), [this, &opaqueEntries](FrameGraphPassContext& context)
	{
		GpuCommandBuffer& commandBuffer = context.GetCommandBuffer();
		commandBuffer.SetViewport(Area2(0.0f, 0.0f, 1.0f, 1.0f));

		RenderQueueEntries(commandBuffer, opaqueEntries, mSurfaceShaderParameterSets);
	});

#if B3D_FRAME_GRAPH_PORT_PENDING(Msaa)
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

		// Marks samples in the stencil, keeping the depth the base pass wrote
		RenderPassCreateInformation msaaStencilInfo(sceneDepthNode->DepthTex->RenderTexture, msaaCoverageStencilMaterial->GetGpuParameterSet(), RT_NONE, RT_DEPTH_STENCIL);
		commandBuffer.BeginRenderPass(msaaStencilInfo);
		msaaCoverageStencilMaterial->Execute(commandBuffer, inputs.View);
		commandBuffer.EndRenderPass();
	}
#endif

#if B3D_FRAME_GRAPH_PORT_PENDING(Decals)
	// Render decals after all normal objects, using a read-only depth buffer
	const Vector<RenderQueueEntry>& decalEntries = inputs.View.GetDecalQueue()->GetSortedEntries();
	RenderPassCreateInformation decalPassInfo(RenderTargetNoMask, RT_DEPTH, RT_ALL);
	PrepareRenderQueuePass(decalPassInfo, decalEntries, mSurfaceShaderParameterSets);

	commandBuffer.BeginRenderPass(decalPassInfo);

	RenderQueueEntries(commandBuffer, decalEntries, mSurfaceShaderParameterSets);

	// Trigger post-base-pass callbacks
	if(sceneCamera != nullptr)
	{
		inputs.View.NotifyCompositorTargetChanged(RenderTargetNoMask);

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
#endif
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
	const RendererViewProperties& viewProps = inputs.View.GetProperties();

	TextureUsageFlags usageFlags = TextureUsageFlag::RenderTarget;
	if(viewProps.Target.NumSamples == 1)
		usageFlags |= TextureUsageFlag::AllowUnorderedAccessOnTheGPU;

	// Note: Consider customizable HDR format via options? e.g. smaller PF_FLOAT_R11G11B10 or larger 32-bit format
	SceneColorTex = inputs.Graph.CreateTexture("SceneColor", CreateViewTextureInformation(inputs.View, PF_RGBA16F, usageFlags));

#if B3D_FRAME_GRAPH_PORT_PENDING(Msaa)
	if(viewProps.Target.NumSamples > 1)
	{
		SceneColorTexArray = resPool.Get(PooledRenderTextureCreateInformation::Create2D(PF_RGBA16F, width, height, TextureUsageFlag::AllowUnorderedAccessOnTheGPU, 1, false, viewProps.Target.NumSamples));
	}
	else
		SceneColorTexArray = nullptr;
#endif
}

void RCNodeSceneColor::Clear()
{
	SceneColorTex = nullptr;
	SceneColorTexArray = nullptr;
}

#if B3D_FRAME_GRAPH_PORT_PENDING(Msaa)
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
#endif

void RCNodeSceneColor::Swap(RCNodeLightAccumulation* lightAccumNode)
{
	lightAccumNode->LightAccumulationTex.swap(SceneColorTex);
	lightAccumNode->LightAccumulationTexArray.swap(SceneColorTexArray);
}

void RCNodeSceneColor::SetExternalTexture(const TShared<Texture>& texture)
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

#if B3D_FRAME_GRAPH_PORT_PENDING(Msaa)
	u32 width = viewProps.Target.ViewRect.Width;
	u32 height = viewProps.Target.ViewRect.Height;

	// We just allocate the texture, while the base pass is responsible for filling it out
	Output = GetGpuResourcePool().Get(PooledRenderTextureCreateInformation::Create2D(PF_R8, width, height, TextureUsageFlag::RenderTarget));
#endif
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
#if B3D_FRAME_GRAPH_PORT_PENDING(Particles)
	// Only simulate particles for the first view in the main render pass
	if(inputs.ViewGroup.IsMainPass() && inputs.View.GetViewIndex() == 0)
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

	GpuParticleSimulation::Instance().Sort(*inputs.ActiveCommandBuffer, inputs.Scene, inputs.View);
#endif
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
	const auto particleSystemCount = inputs.Scene.GetParticleSystemCount();

	// Sort particles
	B3DMarkAllocatorFrame();
	{
		struct SortData
		{
			PackedRendererId ProxyId;
			ParticleRenderData* RenderData;
		};

		FrameVector<SortData> systemsToSort;
		for(u32 particleSystemIndex = 0; particleSystemIndex < particleSystemCount; particleSystemIndex++)
		{
			if(!visibility.ParticleSystems[particleSystemIndex])
				continue;

			const render::ParticleSystemProxy& proxy = inputs.Scene.GetParticleSystemProxy(particleSystemIndex);
			const auto iterFind = particleData->CpuData.find(proxy.GetId());
			if(iterFind == particleData->CpuData.end())
				continue;

			ParticleRenderData* simulationData = iterFind->second;
			if(proxy.GetSettings().SortMode == ParticleSortMode::Distance)
				systemsToSort.push_back({ (PackedRendererId)particleSystemIndex, simulationData });
		}

		WaitGroup waitGroup((u32)systemsToSort.size());
		const auto fnSortWorker = [&waitGroup, &scene = inputs.Scene, viewOrigin = viewProps.ViewOrigin](const SortData& data)
		{
			Vector3 refPoint = viewOrigin;

			// Transform the view point into particle system's local space
			const render::ParticleSystemProxy& proxy = scene.GetParticleSystemProxy(data.ProxyId);
			const render::ParticleSystemSettings& settings = proxy.GetSettings();
			if(settings.SimulationSpace == ParticleSimulationSpace::Local)
				refPoint = proxy.GetWorldTransform().GetInvMatrix().MultiplyAffine(refPoint);

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
	const RendererViewProperties& viewProps = inputs.View.GetProperties();

	TextureUsageFlags usage = TextureUsageFlag::RenderTarget;
	if(viewProps.Target.NumSamples > 1)
	{
#if B3D_FRAME_GRAPH_PORT_PENDING(Msaa)
		resPool.Get(LightAccumulationTexArray, PooledRenderTextureCreateInformation::Create2D(PF_RGBA16F, width, height, TextureUsageFlag::AllowUnorderedAccessOnTheGPU, 1, false, numSamples));

		ClearLoadStoreMaterial* clearMat = ClearLoadStoreMaterial::GetVariation(ClearLoadStoreType::TextureArray, ClearLoadStoreDataType::Float, 4);

		for(u32 i = 0; i < numSamples; i++)
		{
			TextureSurface surface;
			surface.Face = i;
			surface.FaceCount = 1;
			surface.MipLevel = 0;
			surface.MipLevelCount = 1;
			surface.IsBoundAs2DArray = true;

			clearMat->Execute(*inputs.ActiveCommandBuffer, LightAccumulationTexArray->Texture, Color::kZero, surface);
		}
#endif
	}
	else
	{
		usage |= TextureUsageFlag::AllowUnorderedAccessOnTheGPU;
		LightAccumulationTexArray = nullptr;
	}

	LightAccumulationTex = inputs.Graph.CreateTexture("LightAccumulation", CreateViewTextureInformation(inputs.View, PF_RGBA16F, usage));
}

#if B3D_FRAME_GRAPH_PORT_PENDING(Msaa)
void RCNodeLightAccumulation::MsaaTexArrayToTexture(GpuCommandBuffer& commandBuffer)
{
	TextureArrayToMSAATexture* material = TextureArrayToMSAATexture::Get();
	material->Prepare(LightAccumulationTexArray->Texture, LightAccumulationTex->Texture);
	
	commandBuffer.BeginRenderPass(RenderPassCreateInformation(RenderTarget, material->GetGpuParameterSet(), RT_DEPTH_STENCIL, RT_DEPTH_STENCIL));

	material->Execute(commandBuffer, LightAccumulationTex->Texture);

	commandBuffer.EndRenderPass();
}
#endif

void RCNodeLightAccumulation::Clear()
{
	LightAccumulationTex = nullptr;
	LightAccumulationTexArray = nullptr;
}

RCNodeLightAccumulation::DependencyDefinition RCNodeLightAccumulation::GetDependencyDefinition()
{
	static const DependencyDefinition kDependencyDefinition;
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
	gbuffer.Albedo = gbufferNode->AlbedoTex;
	gbuffer.Normals = gbufferNode->NormalTex;
	gbuffer.RoughMetal = gbufferNode->RoughMetalTex;
	gbuffer.Depth = sceneDepthNode->DepthTex;

	const RendererViewProperties& viewProps = inputs.View.GetProperties();

	TShared<Texture> msaaCoverage;
	if(viewProps.Target.NumSamples > 1)
	{
		RCNodeMSAACoverage* coverageNode = dependencies.Get<RCNodeMSAACoverage>();
		msaaCoverage = coverageNode->Output;
	}

	TiledDeferredLightingMaterial* tiledDeferredMat = TiledDeferredLightingMaterial::GetVariation(viewProps.Target.NumSamples);
	const VisibleLightData& lightData = inputs.ViewGroup.GetVisibleLightData();

	FrameGraphComputePass tiledDeferredPass;
	const TShared<GpuParameterSet> tiledDeferredSet = tiledDeferredMat->Prepare(tiledDeferredPass, inputs.View, lightData, gbuffer,
		sceneColorNode->SceneColorTex, Output->LightAccumulationTex, Output->LightAccumulationTexArray, msaaCoverage);

	const u32 width = viewProps.Target.ViewRect.Width;
	const u32 height = viewProps.Target.ViewRect.Height;
	inputs.Graph.AddComputePass("TiledDeferredLighting", std::move(tiledDeferredPass), [tiledDeferredMat, tiledDeferredSet, width, height](FrameGraphPassContext& context)
	{
		tiledDeferredMat->Execute(context, tiledDeferredSet, width, height);
	});

#if B3D_FRAME_GRAPH_PORT_PENDING(Msaa)
	if(viewProps.Target.NumSamples > 1)
		Output->MsaaTexArrayToTexture(commandBuffer);
#endif

	// With shadows disabled, tiled deferred handles all lights
	if(inputs.View.GetRenderSettings().EnableShadows)
	{
		// Standard deferred used for shadowed lights. Each light renders its shadow occlusion, then its lighting.
		const TShared<Texture> lightOcclusionTex = inputs.Graph.CreateTexture("LightOcclusion", CreateViewTextureInformation(inputs.View, PF_R8, TextureUsageFlag::RenderTarget));

		GpuDevice& gpuDevice = inputs.Graph.GetWorkContext().GetDevice();
		const ShadowRendering& shadowRenderer = inputs.ViewGroup.GetShadowRenderer();
		const RendererView& view = inputs.View;
		const RenderBeastScene& scene = inputs.Scene;

		for(u32 lightTypeIndex = 0; lightTypeIndex < (u32)LightType::Count; lightTypeIndex++)
		{
			const LightType lightType = (LightType)lightTypeIndex;

			auto& lights = lightData.GetLights(lightType);
			u32 lightCount = lightData.GetShadowedLightCount(lightType);
			u32 offset = lightData.GetUnshadowedLightCount(lightType);

			for(u32 shadowedLightIndex = 0; shadowedLightIndex < lightCount; shadowedLightIndex++)
			{
				u32 visibleLightIndex = offset + shadowedLightIndex;
				PackedRendererId packedLightId = lights[visibleLightIndex];

				// Shadow projection samples scene depth, and writes the stencil it tests
				FrameGraphRasterPass shadowOcclusionPass;
				shadowOcclusionPass.SetColorAttachment(0, lightOcclusionTex, FrameGraphAttachmentLoad::Clear);
				shadowOcclusionPass.SetDepthStencilAttachment(sceneDepthNode->DepthTex, FrameGraphDepthStencilMode::DepthReadOnly, FrameGraphAttachmentLoad::Load);

				ShadowRendering::ProjectedShadowRenderingBatchInformation shadowProjectionBatch =
					shadowRenderer.PrepareParametersForRenderShadowProjection(shadowOcclusionPass, gpuDevice, view, packedLightId, scene, gbuffer);

				inputs.Graph.AddRasterPass("ShadowOcclusion", std::move(shadowOcclusionPass), [&shadowRenderer, &view, &scene, packedLightId, shadowProjectionBatch = std::move(shadowProjectionBatch)](FrameGraphPassContext& context)
				{
					context.GetCommandBuffer().SetViewport(Area2(0.0f, 0.0f, 1.0f, 1.0f));
					shadowRenderer.RenderShadowProjectionBatch(context, view, packedLightId, scene, shadowProjectionBatch);
				});

				FrameGraphRasterPass lightingPass;
				lightingPass.SetColorAttachment(0, Output->LightAccumulationTex, FrameGraphAttachmentLoad::Load);
				lightingPass.SetDepthStencilAttachment(sceneDepthNode->DepthTex, FrameGraphDepthStencilMode::ReadOnly, FrameGraphAttachmentLoad::Load);

				StandardDeferred::LightBatches batches = StandardDeferred::Instance().PrepareLightBatches(lightingPass, { &packedLightId, 1 }, scene, view, gbuffer, lightOcclusionTex);
				inputs.Graph.AddRasterPass("StandardDeferredLighting", std::move(lightingPass), [batches = std::move(batches)](FrameGraphPassContext& context)
				{
					context.GetCommandBuffer().SetViewport(Area2(0.0f, 0.0f, 1.0f, 1.0f));
					StandardDeferred::Instance().RenderLightBatches(context, batches);
				});
			}
		}
	}

#if !B3D_FRAME_GRAPH_PORT_PENDING(IndirectLighting)
	// Indirect specular lighting writes the lit result into scene color. Until its port, the lit result moves there directly.
	sceneColorNode->Swap(Output);
#endif
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
#if B3D_FRAME_GRAPH_PORT_PENDING(IndirectLighting)
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

	const LightProbes& lightProbes = inputs.Scene.GetLightProbes();
	LightProbesInfo lpInfo = lightProbes.GetInfo();

	IrradianceEvaluateMaterial* evaluateMat;
	TShared<PooledRenderTexture> volumeIndices;
	if(lightProbes.HasAnyProbes())
	{
		PooledRenderTextureCreateInformation volumeIndicesDesc;
		PooledRenderTextureCreateInformation depthDesc;
		TetrahedraRenderMaterial::GetOutputDesc(inputs.View, volumeIndicesDesc, depthDesc);

		volumeIndices = resPool.Get(volumeIndicesDesc);
		TShared<PooledRenderTexture> depthTex = resPool.Get(depthDesc);

		RenderTextureCreateInformation rtDesc;
		rtDesc.ColorSurfaces[0].Texture = volumeIndices->Texture;
		rtDesc.DepthStencilSurface.Texture = depthTex->Texture;

		TShared<RenderTexture> rt = RenderTexture::Create(rtDesc);

		ClearMaterial* clearMaterial = ClearMaterial::GetVariation(false);
		clearMaterial->Prepare(~0u);

		RenderPassCreateInformation clearRenderPassCreateInformation(rt, clearMaterial->GetGpuParameterSet());
		clearRenderPassCreateInformation.ClearMask = RT_DEPTH;

		commandBuffer.BeginRenderPass(clearRenderPassCreateInformation);
		clearMaterial->Execute(commandBuffer);
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

	TShared<Texture> volumeIndicesTex;
	if(volumeIndices)
		volumeIndicesTex = volumeIndices->Texture;

	Skybox* skybox = nullptr;
	if(inputs.View.GetRenderSettings().EnableSkybox)
		skybox = inputs.Scene.GetSkybox();

	evaluateMat->Execute(commandBuffer, inputs.View, gbuffer, volumeIndicesTex, lpInfo, skybox, ssaoNode->Output, lightAccumNode->RenderTarget);

	volumeIndices = nullptr;
#endif
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
#if B3D_FRAME_GRAPH_PORT_PENDING(IndirectLighting)
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

	TShared<Texture> msaaCoverage;
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

	material->Execute(commandBuffer, inputs.View, inputs.Scene, inputs.ViewGroup.GetVisibleReflProbeData(), iblInputs);

	if(viewProps.Target.NumSamples > 1)
		sceneColorNode->MsaaTexArrayToTexture(commandBuffer);
#endif
}

void RCNodeDeferredIndirectSpecularLighting::Clear()
{
#if B3D_FRAME_GRAPH_PORT_PENDING(IndirectLighting)
	Output = nullptr;
#endif
}

RCNodeDeferredIndirectSpecularLighting::DependencyDefinition RCNodeDeferredIndirectSpecularLighting::GetDependencyDefinition()
{
	static const DependencyDefinition kDependencyDefinition;
	return kDependencyDefinition;
}

void RCNodeClusteredForward::Render(const RenderCompositorNodeInputs& inputs)
{
#if B3D_FRAME_GRAPH_PORT_PENDING(Forward)
	const RenderBeastScene& scene = inputs.Scene;

	const VisibleLightData& visibleLightData = inputs.ViewGroup.GetVisibleLightData();
	const VisibleReflectionProbeData& visibleReflProbeData = inputs.ViewGroup.GetVisibleReflProbeData();

	Skybox* skybox = nullptr;
	if(inputs.View.GetRenderSettings().EnableSkybox)
		skybox = scene.GetSkybox();

	// Prepare render target
	auto dependencies = GetDependencyDefinition().ResolveDependencies(inputs);
	auto sceneColorNode = dependencies.Get<RCNodeSceneColor>();
	auto sceneDepthNode = dependencies.Get<RCNodeSceneDepth>();
	auto resolvedSceneDepthNode = dependencies.Get<RCNodeResolvedSceneDepth>();

	bool rebuildRT;
	if(renderTarget != nullptr)
	{
		rebuildRT = renderTarget->GetColorSurfaceInformation(0).Texture != sceneColorNode->SceneColorTex->Texture;
		rebuildRT |= renderTarget->GetDepthStencilSurfaceInformation().Texture != sceneDepthNode->DepthTex->Texture;
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

	// Prepare the renderer inputs of forward draws
	mSurfaceShaderParameterSets.Prepare(inputs.View, inputs.Scene);
	mSurfaceShaderParameterSets.SetSceneDepth(resolvedSceneDepthNode->Output->Texture);
	mSurfaceShaderParameterSets.SetForwardLightingInputs(inputs.View, visibleLightData, visibleReflProbeData, scene, skybox);

	// TODO: Forward pipeline rendering doesn't support shadows. In order to support this I'd have to render the light
	// occlusion for all lights affecting this object into a single (or a few) textures. I can likely use texture
	// arrays for this, or to avoid sampling many textures, perhaps just jam it all in one or few texture channels.

	// Render everything
	GpuCommandBuffer& commandBuffer = *inputs.ActiveCommandBuffer;

	RenderQueue* opaqueQueue = inputs.View.GetOpaqueQueue(true).get();
	RenderQueue* transparentQueue = inputs.View.GetTransparentQueue().get();

	// Collect all GpuParameters that will be used in the opaque pass
	const Vector<RenderQueueEntry>& opaqueElements = opaqueQueue->GetSortedEntries();
	RenderPassCreateInformation opaquePassInfo(renderTarget, RT_NONE, RT_ALL);
	PrepareRenderQueuePass(opaquePassInfo, opaqueElements, mSurfaceShaderParameterSets);

	commandBuffer.BeginRenderPass(opaquePassInfo);
	RenderQueueEntries(commandBuffer, opaqueElements, mSurfaceShaderParameterSets);
	commandBuffer.EndRenderPass();

	// Collect all GpuParameters that will be used in the transparent pass
	const Vector<RenderQueueEntry>& transparentElements = transparentQueue->GetSortedEntries();
	RenderPassCreateInformation transparentPassInfo(renderTarget, RT_DEPTH, RT_ALL);
	PrepareRenderQueuePass(transparentPassInfo, transparentElements, mSurfaceShaderParameterSets);

	commandBuffer.BeginRenderPass(transparentPassInfo);
	RenderQueueEntries(commandBuffer, transparentElements, mSurfaceShaderParameterSets);
	commandBuffer.EndRenderPass();

	// Note: Perhaps delay clearing this one frame, so previous frame textures have a better chance of being done
	ParticleRenderer::Instance().GetTexturePool().Clear();

	// Trigger post-lighting callbacks
	Camera* sceneCamera = inputs.View.GetSceneCamera();
	if(sceneCamera != nullptr)
	{
		inputs.View.NotifyCompositorTargetChanged(renderTarget);

		for(auto& extension : inputs.ExtPostLighting)
		{
			RendererViewContext context;
			context.CurrentTarget = inputs.View.GetCompositorRenderTarget();
			context.CommandBuffer = inputs.ActiveCommandBuffer;

			// TODO - RenderPass
			extension->Render(*sceneCamera, context);
		}
	}
#endif
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
		skybox = inputs.Scene.GetSkybox();

	TShared<Texture> radiance = skybox ? skybox->GetTexture() : nullptr;

	auto dependencies = GetDependencyDefinition().ResolveDependencies(inputs);
	RCNodeSceneColor* sceneColorNode = dependencies.Get<RCNodeSceneColor>();
	RCNodeSceneDepth* sceneDepthNode = dependencies.Get<RCNodeSceneDepth>();

	FrameGraphRasterPass skyboxPass;
	skyboxPass.SetColorAttachment(0, sceneColorNode->SceneColorTex, FrameGraphAttachmentLoad::Load);
	skyboxPass.SetDepthStencilAttachment(sceneDepthNode->DepthTex, FrameGraphDepthStencilMode::ReadOnly, FrameGraphAttachmentLoad::Load);

	SkyboxMaterial* material;
	TShared<GpuParameterSet> skyboxSet;
	if(radiance != nullptr)
	{
		material = SkyboxMaterial::GetVariation(false);
		skyboxSet = material->Prepare(skyboxPass, inputs.View.GetPerViewBuffer(), radiance, Color::kWhite);
	}
	else
	{
		// Cancel out the linear->SRGB conversion
		Color backgroundColor = inputs.View.GetProperties().Target.BackgroundColor.GetLinear();

		material = SkyboxMaterial::GetVariation(true);
		skyboxSet = material->Prepare(skyboxPass, inputs.View.GetPerViewBuffer(), nullptr, backgroundColor);
	}

	inputs.Graph.AddRasterPass("Skybox", std::move(skyboxPass), [material, skyboxSet](FrameGraphPassContext& context)
	{
		context.GetCommandBuffer().SetViewport(Area2(0.0f, 0.0f, 1.0f, 1.0f));
		material->Execute(context, skyboxSet);
	});
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

	TShared<Texture> input;
	if(viewProperties.RunPostProcessing)
	{
		RCNodePostProcess* postProcessNode = dependencies.Get<RCNodePostProcess>();

		// Note: Ideally the last PP effect could write directly to the final target and we could avoid this copy
		input = postProcessNode->GetLastOutput();
	}
	else
	{
		RCNodeSceneColor* sceneColorNode = dependencies.Get<RCNodeSceneColor>();
		input = sceneColorNode->SceneColorTex;
	}

	const TShared<RenderTarget>& target = viewProperties.Target.Target;

	// Float render textures need the variation that outputs 32-bit values, as RendererUtility::Blit() picks it
	bool output32Bit = false;
	if(!target->GetProperties().IsWindow)
	{
		const RenderSurfaceInformation& colorSurface = static_cast<RenderTexture&>(*target).GetColorSurfaceInformation(0);
		if(colorSurface.Texture != nullptr)
			output32Bit = PixelUtility::GetElementType(colorSurface.Texture->GetProperties().Format) == PCT_FLOAT32;
	}

	const TextureProperties& inputProperties = input->GetProperties();
	BlitMat* const blitMaterial = BlitMat::GetVariation(inputProperties.SampleCount, true, false, false, false, false, false, output32Bit);

	// Other views and overlays may have drawn outside of this view's area
	const Area2& viewArea = viewProperties.Target.NrmViewRect;
	const bool coversTarget = viewArea == Area2(0.0f, 0.0f, 1.0f, 1.0f);

	FrameGraphRasterPass resolvePass;
	resolvePass.SetRenderTarget(target, coversTarget ? FrameGraphAttachmentLoad::DontCare : FrameGraphAttachmentLoad::Load);

	const TShared<GpuParameterSet> blitSet = blitMaterial->Prepare(resolvePass, input);
	const Area2 inputArea(0.0f, 0.0f, (float)inputProperties.Width, (float)inputProperties.Height);
	const bool flipUV = viewProperties.FlipView;

	inputs.Graph.AddRasterPass("FinalResolve", std::move(resolvePass), [blitMaterial, blitSet, viewArea, inputArea, flipUV](FrameGraphPassContext& context)
	{
		context.GetCommandBuffer().SetViewport(viewArea);
		blitMaterial->Execute(context, blitSet, inputArea, flipUV);
	});

#if B3D_FRAME_GRAPH_PORT_PENDING(RendererTasks)
	GpuCommandBuffer& commandBuffer = *inputs.ActiveCommandBuffer;
	if(viewProperties.EncodeDepth)
	{
		RCNodeResolvedSceneDepth* resolvedSceneDepthNode = dependencies.Get<RCNodeResolvedSceneDepth>();

		EncodeDepthMaterial* const encodeDepthMat = EncodeDepthMaterial::Get();
		encodeDepthMat->Prepare(resolvedSceneDepthNode->Output->Texture, viewProperties.DepthEncodeNear, viewProperties.DepthEncodeFar);
		encodeDepthMat->Execute(commandBuffer, target);
	}
#endif

#if B3D_FRAME_GRAPH_PORT_PENDING(Extensions)
	// Trigger overlay callbacks
	Camera* sceneCamera = inputs.View.GetSceneCamera();
	if(sceneCamera != nullptr)
	{
		inputs.View.NotifyCompositorTargetChanged(target);

		for(auto& extension : inputs.ExtOverlay)
		{
			RendererViewContext context;
			context.CurrentTarget = inputs.View.GetCompositorRenderTarget();
			context.CommandBuffer = inputs.ActiveCommandBuffer;

			// TODO - RenderPass
			extension->Render(*sceneCamera, context);
		}
	}
#endif

	// Resolve pending view captures after overlays and before clearing the target.
	inputs.View.ResolveCaptures(inputs.Graph, target);

	inputs.View.NotifyCompositorTargetChanged(nullptr);
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
#if B3D_FRAME_GRAPH_PORT_PENDING(Forward)
		.ConditionalEnable<RCNodeClusteredForward>([fnRunPostProcessing](const RendererView& view) { return !fnRunPostProcessing(view); })
#else
		.ConditionalEnable<RCNodeClusteredForward>([](const RendererView&) { return false; })
#endif
		.ConditionalEnable<RCNodeResolvedSceneDepth>(fnEncodeDepth);

	return kDependencyDefinition;
}

void RCNodePostProcess::GetAndSwitch(const RendererView& view, TShared<RenderTexture>& output, TShared<Texture>& lastFrame) const
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

TShared<Texture> RCNodePostProcess::GetLastOutput() const
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
#if B3D_FRAME_GRAPH_PORT_PENDING(PostProcessing)
	mOutput[0] = nullptr;
	mOutput[1] = nullptr;
	mCurrentIdx = 0;
#endif
}

RCNodePostProcess::DependencyDefinition RCNodePostProcess::GetDependencyDefinition()
{
	static const DependencyDefinition kDependencyDefinition;
	return kDependencyDefinition;
}

void RCNodeEyeAdaptation::Render(const RenderCompositorNodeInputs& inputs)
{
#if B3D_FRAME_GRAPH_PORT_PENDING(PostProcessing)
	auto dependencies = GetDependencyDefinition().ResolveDependencies(inputs);
	GpuResourcePool& resPool = GetGpuResourcePool();

	GpuCommandBuffer& commandBuffer = *inputs.ActiveCommandBuffer;
	const RenderSettings& settings = inputs.View.GetRenderSettings();

	const bool hdr = settings.EnableHdr;

	if(hdr && settings.EnableAutoExposure)
	{
		// Get downsample scene
		auto* halfSceneColorNode = dependencies.Get<RCNodeHalfSceneColor>();
		const TShared<PooledRenderTexture>& downsampledScene = halfSceneColorNode->Output;

		// Generate histogram
		TShared<PooledRenderTexture> eyeAdaptHistogram =
			resPool.Get(EyeAdaptHistogramMaterial::GetOutputDesc(downsampledScene->Texture));
		EyeAdaptHistogramMaterial* eyeAdaptHistogramMat = EyeAdaptHistogramMaterial::Get();
		eyeAdaptHistogramMat->Execute(commandBuffer, downsampledScene->Texture, eyeAdaptHistogram->Texture, settings.AutoExposure);

		// Reduce histogram
		TShared<PooledRenderTexture> reducedHistogram = resPool.Get(EyeAdaptHistogramReduceMaterial::GetOutputDesc());

		TShared<Texture> prevFrameEyeAdaptation;
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

		const RendererView& view = inputs.View;

		// Notify the view eye adaptation value will change
		view.NotifyLuminanceUpdated(inputs.FrameInfo.Timings.FrameIndex, inputs.ActiveCommandBuffer, Output);
	}
	else
	{
		previous = nullptr;
		Output = nullptr;
	}
#endif
}

void RCNodeEyeAdaptation::Clear()
{
#if B3D_FRAME_GRAPH_PORT_PENDING(PostProcessing)
	std::swap(Output, previous);
	Output = nullptr;
#endif
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
#if B3D_FRAME_GRAPH_PORT_PENDING(PostProcessing)
	auto dependencies = GetDependencyDefinition().ResolveDependencies(inputs);

	GpuCommandBuffer& commandBuffer = *inputs.ActiveCommandBuffer;
	const RenderSettings& settings = inputs.View.GetRenderSettings();

	auto* eyeAdaptationNode = dependencies.Get<RCNodeEyeAdaptation>();
	auto* sceneColorNode = dependencies.Get<RCNodeSceneColor>();
	auto* postProcessNode = dependencies.Get<RCNodePostProcess>();
	const TShared<Texture>& sceneColor = sceneColorNode->SceneColorTex->Texture;

	const bool hdr = settings.EnableHdr;
	const bool msaa = sceneColor->GetProperties().SampleCount > 1;

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
				CreateTonemap3DLUTMaterial* createLUT = CreateTonemap3DLUTMaterial::Get();
				if(mTonemapLUT == nullptr)
					mTonemapLUT = GetGpuResourcePool().Get(createLUT->GetOutputDesc());

				createLUT->Execute(commandBuffer, mTonemapLUT->Texture, settings);

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

	TonemappingMaterial* tonemapping = TonemappingMaterial::GetVariation(gammaOnly, autoExposure, msaa);

	TShared<RenderTexture> ppOutput;
	TShared<Texture> ppLastFrame;
	postProcessNode->GetAndSwitch(inputs.View, ppOutput, ppLastFrame);

	TShared<Texture> eyeAdaptationTex;
	if(eyeAdaptationNode->Output)
		eyeAdaptationTex = eyeAdaptationNode->Output->Texture;

	TShared<Texture> tonemapLUTTex;
	if(mTonemapLUT)
		tonemapLUTTex = mTonemapLUT->Texture;

	TShared<Texture> bloomTex;
	if(settings.Bloom.Enabled)
	{
		auto* bloomNode = dependencies.Get<RCNodeBloom>();
		bloomTex = bloomNode->Output;
	}

	tonemapping->Prepare(sceneColor, eyeAdaptationTex, bloomTex, tonemapLUTTex, settings);
	tonemapping->Execute(commandBuffer, ppOutput);
#endif
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
#if B3D_FRAME_GRAPH_PORT_PENDING(PostProcessing)
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

	TShared<Texture> depth = sceneDepthNode->DepthTex->Texture;

	// Downsample scene and store depth in .w
	TShared<PooledRenderTexture> halfResSceneAndDepth =
		GetGpuResourcePool().Get(BokehDOFPrepareMaterial::GetOutputDesc(sceneColorNode->SceneColorTex->Texture));

	prepareMat->Prepare(sceneColorNode->SceneColorTex->Texture, depth, inputs.View, settings);
	prepareMat->Execute(commandBuffer, halfResSceneAndDepth->RenderTexture);

	TShared<PooledRenderTexture> unfocusedTex =
		GetGpuResourcePool().Get(BokehDOFMaterial::GetOutputDesc(halfResSceneAndDepth->Texture));

	renderMat->Prepare(halfResSceneAndDepth->Texture, inputs.View, settings, unfocusedTex->RenderTexture);
	renderMat->Execute(commandBuffer, halfResSceneAndDepth->Texture, unfocusedTex->RenderTexture);
	halfResSceneAndDepth = nullptr;

	// Combine the unfocused and focused textures to form the final image
	combineMat->Prepare(unfocusedTex->Texture, sceneColorNode->SceneColorTex->Texture, depth, inputs.View, settings);
	combineMat->Execute(commandBuffer, lightAccumNode->LightAccumulationTex->RenderTexture);

	// TODO - This might be incorrect when not supporting tiled deferred? As light accum is the same as scene color
	sceneColorNode->Swap(lightAccumNode);
#endif
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
#if B3D_FRAME_GRAPH_PORT_PENDING(PostProcessing)
	auto dependencies = GetDependencyDefinition().ResolveDependencies(inputs);
	auto* sceneColorNode = dependencies.Get<RCNodeSceneColor>();
	TShared<PooledRenderTexture> sceneColor = sceneColorNode->SceneColorTex;

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
	TShared<PooledRenderTexture> resolvedSceneColor;
	if(viewProps.Target.NumSamples > 1)
	{
		resolvedSceneColor = resPool.Get(PooledRenderTextureCreateInformation::Create2D(PF_RGBA16F, width, height, TextureUsageFlag::RenderTarget));

		GetRendererUtility().Blit(commandBuffer, BlitInformation::BlitColor(sceneColor->Texture, resolvedSceneColor->RenderTexture));
		sceneColor = resolvedSceneColor;
	}

	if(mPrevFrame)
	{
		mPooledOutput = resPool.Get(PooledRenderTextureCreateInformation::Create2D(PF_RGBA16F, width, height, TextureUsageFlag::RenderTarget));

		TShared<Texture> velocityTex;
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
#endif
}

void RCNodeTemporalAA::Clear()
{
#if B3D_FRAME_GRAPH_PORT_PENDING(PostProcessing)
	mPrevFrame = mPooledOutput;
	mPooledOutput = nullptr;
	Output = nullptr;
#endif
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
#if B3D_FRAME_GRAPH_PORT_PENDING(PostProcessing)
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

	// TShared<Texture> depth = sceneDepthNode->depthTex->texture;
	// motionBlurMat->Prepare(sceneColorNode->sceneColorTex->texture, depth, inputs.view, settings);
	// motionBlurMat->Execute(commandBuffer, lightAccumNode->lightAccumulationTex->renderTexture);

	// sceneColorNode->swap(lightAccumNode);
#endif
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
#if B3D_FRAME_GRAPH_PORT_PENDING(PostProcessing)
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

	TShared<RenderTexture> ppOutput;
	TShared<Texture> ppLastFrame;
	postProcessNode->GetAndSwitch(inputs.View, ppOutput, ppLastFrame);

	separateMat->Prepare(ppLastFrame, sceneDepthNode->DepthTex->Texture, inputs.View, settings);
	separateMat->Execute(commandBuffer, ppLastFrame);

	TShared<PooledRenderTexture> nearTex, farTex;
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
	TShared<PooledRenderTexture> tempTexture = GetGpuResourcePool().Get(tempTexDesc);

	TShared<Texture> blurredNearTex;
	if(nearTex)
	{
		blurMat->Execute(commandBuffer, nearTex->Texture, settings.NearBlurAmount, tempTexture->RenderTexture);
		blurredNearTex = tempTexture->Texture;
	}

	TShared<Texture> blurredFarTex;
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
#endif
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
#if B3D_FRAME_GRAPH_PORT_PENDING(PostProcessing)
	auto dependencies = GetDependencyDefinition().ResolveDependencies(inputs);
	const RenderSettings& settings = inputs.View.GetRenderSettings();
	if(!settings.EnableFxaa)
		return;

	RCNodePostProcess* postProcessNode = dependencies.Get<RCNodePostProcess>();

	TShared<RenderTexture> ppOutput;
	TShared<Texture> ppLastFrame;
	postProcessNode->GetAndSwitch(inputs.View, ppOutput, ppLastFrame);

	// Note: I could skip executing FXAA over DOF and motion blurred pixels
	FXAAMaterial* fxaa = FXAAMaterial::Get();
	fxaa->Prepare(ppLastFrame);
	fxaa->Execute(*inputs.ActiveCommandBuffer, ppOutput);
#endif
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
#if B3D_FRAME_GRAPH_PORT_PENDING(PostProcessing)
	auto dependencies = GetDependencyDefinition().ResolveDependencies(inputs);
	const RenderSettings& settings = inputs.View.GetRenderSettings();
	if(!settings.ChromaticAberration.Enabled)
		return;

	auto* postProcessNode = dependencies.Get<RCNodePostProcess>();

	TShared<RenderTexture> ppOutput;
	TShared<Texture> ppLastFrame;
	postProcessNode->GetAndSwitch(inputs.View, ppOutput, ppLastFrame);

	ChromaticAberrationMaterial* chromaticAberration = ChromaticAberrationMaterial::GetVariation(settings.ChromaticAberration.Type);
	chromaticAberration->Prepare(ppLastFrame, settings.ChromaticAberration);
	chromaticAberration->Execute(*inputs.ActiveCommandBuffer, ppOutput);
#endif
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
#if B3D_FRAME_GRAPH_PORT_PENDING(PostProcessing)
	auto dependencies = GetDependencyDefinition().ResolveDependencies(inputs);
	const RenderSettings& settings = inputs.View.GetRenderSettings();
	if(!settings.FilmGrain.Enabled)
		return;

	auto* postProcessNode = dependencies.Get<RCNodePostProcess>();

	TShared<RenderTexture> ppOutput;
	TShared<Texture> ppLastFrame;
	postProcessNode->GetAndSwitch(inputs.View, ppOutput, ppLastFrame);

	FilmGrainMaterial* filmGrain = FilmGrainMaterial::Get();
	filmGrain->Prepare(ppLastFrame, inputs.FrameInfo.Timings.Time, settings.FilmGrain);
	filmGrain->Execute(*inputs.ActiveCommandBuffer, ppOutput);
#endif
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
#if B3D_FRAME_GRAPH_PORT_PENDING(PostProcessing)
	auto dependencies = GetDependencyDefinition().ResolveDependencies(inputs);
	const RendererViewProperties& viewProps = inputs.View.GetProperties();

	auto* sceneColorNode = dependencies.Get<RCNodeSceneColor>();
	const TShared<Texture>& input = sceneColorNode->SceneColorTex->Texture;

	// Downsample scene
	const bool msaa = viewProps.Target.NumSamples > 1;
	DownsampleMaterial* downsampleMat = DownsampleMaterial::GetVariation(1, msaa);

	Output = GetGpuResourcePool().Get(DownsampleMaterial::GetOutputDesc(input));

	downsampleMat->Execute(*inputs.ActiveCommandBuffer, input, Output->RenderTexture);
#endif
}

void RCNodeHalfSceneColor::Clear()
{
#if B3D_FRAME_GRAPH_PORT_PENDING(PostProcessing)
	Output = nullptr;
#endif
}

RCNodeHalfSceneColor::DependencyDefinition RCNodeHalfSceneColor::GetDependencyDefinition()
{
	static const DependencyDefinition kDependencyDefinition;
	return kDependencyDefinition;
}

constexpr u32 RCNodeSceneColorDownsamples::kMaxNumDownsamples;

void RCNodeSceneColorDownsamples::Render(const RenderCompositorNodeInputs& inputs)
{
#if B3D_FRAME_GRAPH_PORT_PENDING(PostProcessing)
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
#endif
}

void RCNodeSceneColorDownsamples::Clear()
{
#if B3D_FRAME_GRAPH_PORT_PENDING(PostProcessing)
	for(u32 i = 0; i < kMaxNumDownsamples; i++)
		Output[i] = nullptr;
#endif
}

RCNodeSceneColorDownsamples::DependencyDefinition RCNodeSceneColorDownsamples::GetDependencyDefinition()
{
	static const DependencyDefinition kDependencyDefinition;
	return kDependencyDefinition;
}

void RCNodeResolvedSceneDepth::Render(const RenderCompositorNodeInputs& inputs)
{
#if B3D_FRAME_GRAPH_PORT_PENDING(IndirectLighting)
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

		GetRendererUtility().Blit(commandBuffer, blitInformation);
	}
	else
		Output = sceneDepthNode->DepthTex;
#endif
}

void RCNodeResolvedSceneDepth::Clear()
{
#if B3D_FRAME_GRAPH_PORT_PENDING(IndirectLighting)
	Output = nullptr;
#endif
}

RCNodeResolvedSceneDepth::DependencyDefinition RCNodeResolvedSceneDepth::GetDependencyDefinition()
{
	static const DependencyDefinition kDependencyDefinition;
	return kDependencyDefinition;
}

void RCNodeHiZ::Render(const RenderCompositorNodeInputs& inputs)
{
#if B3D_FRAME_GRAPH_PORT_PENDING(ScreenSpace)
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

	BuildHiZMaterial* material = BuildHiZMaterial::Get();

	// Generate first mip
	RenderTextureCreateInformation rtDesc;
	rtDesc.ColorSurfaces[0].Texture = Output->Texture;
	rtDesc.ColorSurfaces[0].MipLevel = 0;

	TShared<RenderTexture> renderTexture = RenderTexture::Create(rtDesc);

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
#endif
}

void RCNodeHiZ::Clear()
{
#if B3D_FRAME_GRAPH_PORT_PENDING(ScreenSpace)
	Output = nullptr;
#endif
}

RCNodeHiZ::DependencyDefinition RCNodeHiZ::GetDependencyDefinition()
{
	static const DependencyDefinition kDependencyDefinition;
	return kDependencyDefinition;
}

void RCNodeSSAO::Render(const RenderCompositorNodeInputs& inputs)
{
#if B3D_FRAME_GRAPH_PORT_PENDING(IndirectLighting)
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

	TShared<Texture> sceneDepth = resolvedDepthNode->Output->Texture;
	TShared<Texture> sceneNormals = gbufferNode->NormalTex->Texture;

	const TextureProperties& normalsProps = sceneNormals->GetProperties();
	TShared<PooledRenderTexture> resolvedNormals;

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

	TShared<PooledRenderTexture> setupTex0;
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

	TShared<PooledRenderTexture> setupTex1;
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

	TShared<PooledRenderTexture> downAOTex1;
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

	TShared<PooledRenderTexture> downAOTex0;
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
		TShared<PooledRenderTexture> blurIntermediateTex = resPool.Get(desc);

		SSAOBlurMaterial* blurHorz = SSAOBlurMaterial::GetVariation(true);
		SSAOBlurMaterial* blurVert = SSAOBlurMaterial::GetVariation(false);

		blurHorz->Prepare(inputs.View, mPooledOutput->Texture, sceneDepth, kDepthRange);
		blurHorz->Execute(commandBuffer, blurIntermediateTex->RenderTexture);

		blurVert->Prepare(inputs.View, blurIntermediateTex->Texture, sceneDepth, kDepthRange);
		blurVert->Execute(commandBuffer, mPooledOutput->RenderTexture);
	}

	Output = mPooledOutput->Texture;
#endif
}

void RCNodeSSAO::Clear()
{
#if B3D_FRAME_GRAPH_PORT_PENDING(IndirectLighting)
	mPooledOutput = nullptr;
	Output = nullptr;
#endif
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
#if B3D_FRAME_GRAPH_PORT_PENDING(IndirectLighting)
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

	TShared<Texture> hiZ = hiZNode->Output->Texture;

	// This will be executing before scene color is resolved, so get the light accum buffer instead
	TShared<Texture> sceneColor = lightAccumNode->LightAccumulationTex->Texture;

	// Resolve multiple samples if MSAA is used
	TShared<PooledRenderTexture> resolvedSceneColor;
	if(viewProps.Target.NumSamples > 1)
	{
		resolvedSceneColor = resPool.Get(PooledRenderTextureCreateInformation::Create2D(PF_RGBA16F, width, height, TextureUsageFlag::RenderTarget));

		GetRendererUtility().Blit(commandBuffer, BlitInformation::BlitColor(sceneColor, resolvedSceneColor->RenderTexture));
		sceneColor = resolvedSceneColor->Texture;
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

	TShared<PooledRenderTexture> traceOutput = resPool.Get(PooledRenderTextureCreateInformation::Create2D(PF_RGBA16F, width, height, TextureUsageFlag::RenderTarget));

	RenderTextureCreateInformation traceRtDesc;
	traceRtDesc.ColorSurfaces[0].Texture = traceOutput->Texture;
	traceRtDesc.DepthStencilSurface.Texture = resolvedSceneDepthNode->Output->Texture;

	TShared<RenderTexture> traceRt = RenderTexture::Create(traceRtDesc);

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
#endif
}

void RCNodeSSR::Clear()
{
#if B3D_FRAME_GRAPH_PORT_PENDING(IndirectLighting)
	if(!mUsingTemporalAA)
		mPrevFrame = mPooledOutput;

	mPooledOutput = nullptr;
	Output = nullptr;
#endif
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
#if B3D_FRAME_GRAPH_PORT_PENDING(PostProcessing)
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

	TShared<Texture> eyeAdaptationTex = nullptr;
	if(autoExposure)
	{
		auto* eyeAdapatationNode = dependencies.Get<RCNodeEyeAdaptation>();

		if(eyeAdapatationNode->Output)
			eyeAdaptationTex = eyeAdapatationNode->Output->Texture;
	}

	const u32 numSteps = NUM_STEPS_PER_QUALITY[quality];
	TShared<PooledRenderTexture> prevOutput;
	for(u32 i = 0; i < numSteps; i++)
	{
		const u32 srcIdx = numDownsamples - i - 1;
		const TShared<PooledRenderTexture> downsampledTex = sceneDownsamplesNode->Output[srcIdx];

		const TextureProperties& inputProps = downsampledTex->Texture->GetProperties();

		TShared<PooledRenderTexture> filterOutput = GetGpuResourcePool().Get(
			PooledRenderTextureCreateInformation::Create2D(
				inputProps.Format,
				inputProps.Width,
				inputProps.Height,
				TextureUsageFlag::RenderTarget));

		TShared<PooledRenderTexture> blurInput = downsampledTex;
		TShared<PooledRenderTexture> blurOutput = filterOutput;
		if(settings.Bloom.Threshold > 0.0f)
		{
			clipMat->Prepare(downsampledTex->Texture, settings.Bloom.Threshold, eyeAdaptationTex, settings);
			clipMat->Execute(*inputs.ActiveCommandBuffer, filterOutput->RenderTexture);

			blurOutput = blurInput;
			blurInput = filterOutput;
		}

		TShared<Texture> additiveInput;
		if(prevOutput)
			additiveInput = prevOutput->Texture;

		const Color tint = Color::kWhite * (settings.Bloom.Intensity / (float)numSteps);
		filterMat->Execute(*inputs.ActiveCommandBuffer, blurInput->Texture, settings.Bloom.FilterSize, blurOutput->RenderTexture, tint, additiveInput);
		prevOutput = blurOutput;
	}

	mPooledOutput = prevOutput;
	Output = mPooledOutput->Texture;
#endif
}

void RCNodeBloom::Clear()
{
#if B3D_FRAME_GRAPH_PORT_PENDING(PostProcessing)
	mPooledOutput = nullptr;
	Output = nullptr;
#endif
}

RCNodeBloom::DependencyDefinition RCNodeBloom::GetDependencyDefinition()
{
	static const DependencyDefinition kDependencyDefinition;
	return kDependencyDefinition;
}

void RCNodeScreenSpaceLensFlare::Render(const RenderCompositorNodeInputs& inputs)
{
#if B3D_FRAME_GRAPH_PORT_PENDING(PostProcessing)
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

	TShared<PooledRenderTexture> downsampledTex = sceneDownsamplesNode->Output[numDownsamples - 1];
	const TextureProperties& sceneTexProps = downsampledTex->Texture->GetProperties();

	// Ghost features
	TShared<PooledRenderTexture> featureTex = resPool.Get(
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
#endif
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
