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

	/**
	 * Adds a raster pass that draws @p material into @p output, with the parameter set from @p material's Prepare(pass, args...). The
	 * material's Execute() takes only the set.
	 */
	template<class Material, class... Args>
	void AddFullScreenPass(FrameGraph& graph, const StringView& name, Material* material, const TShared<Texture>& output, FrameGraphAttachmentLoad load, Args&&... args)
	{
		FrameGraphRasterPass pass;
		pass.SetColorAttachment(0, output, load);

		const TShared<GpuParameterSet> set = material->Prepare(pass, std::forward<Args>(args)...);
		graph.AddRasterPass(name, std::move(pass), [material, set](FrameGraphPassContext& context)
		{
			context.GetCommandBuffer().SetViewport(Area2(0.0f, 0.0f, 1.0f, 1.0f));
			material->Execute(context, set);
		});
	}

	/**
	 * Returns the description of a texture array the size of @p view, with one slice per sample of the view. Compute shaders write the
	 * samples of a multisampled texture into it, as not every backend can write multisampled textures from compute shaders.
	 */
	TextureCreateInformation CreateViewSampleArrayInformation(const RendererView& view, PixelFormat format)
	{
		const RendererViewProperties& viewProperties = view.GetProperties();

		TextureCreateInformation information = TextureCreateInformation::Create2D(format, viewProperties.Target.ViewRect.Width, viewProperties.Target.ViewRect.Height,
			TextureUsageFlag::AllowUnorderedAccessOnTheGPU);
		information.ArraySliceCount = viewProperties.Target.NumSamples;

		return information;
	}

	/** Adds a raster pass that copies @p inputArray, a texture array with one slice per sample, into the multisampled @p output. */
	void AddTextureArrayToMSAAPass(FrameGraph& graph, const StringView& name, const TShared<Texture>& inputArray, const TShared<Texture>& output)
	{
		TextureArrayToMSAATexture* const material = TextureArrayToMSAATexture::Get();

		// The material writes every sample
		FrameGraphRasterPass pass;
		pass.SetColorAttachment(0, output, FrameGraphAttachmentLoad::DontCare);

		const TShared<GpuParameterSet> set = material->Prepare(pass, inputArray, output);
		graph.AddRasterPass(name, std::move(pass), [material, set, output](FrameGraphPassContext& context)
		{
			context.GetCommandBuffer().SetViewport(Area2(0.0f, 0.0f, 1.0f, 1.0f));
			material->Execute(context, set, output);
		});
	}

	/** Adds a raster pass that resolves the multisampled @p input into @p output, averaging the samples of each pixel. */
	void AddResolvePass(FrameGraph& graph, const StringView& name, const TShared<Texture>& input, const TShared<Texture>& output)
	{
		const TextureProperties& inputProperties = input->GetProperties();
		BlitMat* const material = BlitMat::GetVariation(inputProperties.SampleCount, true, false);

		FrameGraphRasterPass pass;
		pass.SetColorAttachment(0, output, FrameGraphAttachmentLoad::DontCare);

		const TShared<GpuParameterSet> set = material->Prepare(pass, input);
		const Area2 inputArea(0.0f, 0.0f, (float)inputProperties.Width, (float)inputProperties.Height);
		graph.AddRasterPass(name, std::move(pass), [material, set, inputArea](FrameGraphPassContext& context)
		{
			context.GetCommandBuffer().SetViewport(Area2(0.0f, 0.0f, 1.0f, 1.0f));
			material->Execute(context, set, inputArea, false);
		});
	}

	/** Adds a raster pass that downsamples @p input into @p output, created from DownsampleMaterial::GetOutputCreateInformation(). */
	void AddDownsamplePass(FrameGraph& graph, const StringView& name, DownsampleMaterial* material, const TShared<Texture>& input, const TShared<Texture>& output)
	{
		FrameGraphRasterPass pass;
		pass.SetColorAttachment(0, output, FrameGraphAttachmentLoad::DontCare);

		const TShared<GpuParameterSet> set = material->Prepare(pass, input);
		graph.AddRasterPass(name, std::move(pass), [material, set, input](FrameGraphPassContext& context)
		{
			context.GetCommandBuffer().SetViewport(Area2(0.0f, 0.0f, 1.0f, 1.0f));
			material->Execute(context, set, input);
		});
	}
} // namespace

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

	for(const NodeInfo& entry : mRegisteredNodes)
	{
		inputs.InputNodes = entry.Inputs;
		entry.Node->Render(inputs);
	}

	for(const NodeInfo& entry : mRegisteredNodes)
		entry.Node->Clear();
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

	const bool needsVelocity = inputs.View.RequiresVelocityWrites();
	if(needsVelocity)
		VelocityTex = inputs.Graph.CreateTexture("GBufferVelocity", CreateViewTextureInformation(inputs.View, PF_RG16S, TextureUsageFlag::RenderTarget));

	auto dependencies = GetDependencyDefinition().ResolveDependencies(inputs);
	auto sceneDepthNode = dependencies.Get<RCNodeSceneDepth>();
	auto sceneColorNode = dependencies.Get<RCNodeSceneColor>();

	// Prepare all visible objects. Note that this also prepares non-opaque objects.
	const VisibilityInfo& visibility = inputs.View.GetVisibilityMasks();

	mSurfaceShaderParameterSets.Prepare(inputs.View, inputs.Scene);
	mSurfaceShaderParameterSets.SetDecalInputs(sceneDepthNode->DepthTex, IdTex);

	//// Prepare particle systems
	// CPU simulated systems bound their inputs for the frame (RenderBeastScene::PrepareVisibleParticleSystems), and draw in the order of
	// the view's particle index buffer. GPU simulated systems bind the sort offsets that the simulation computes when the first view
	// builds its graph, which are the same in every view.
	const EvaluatedParticleData* particleData = inputs.FrameInfo.PerSceneFrameData.Particles;
	if(particleData)
	{
		const auto particleSystemCount = inputs.Scene.GetParticleSystemCount();

		for(u32 i = 0; i < particleSystemCount; i++)
		{
			if(!visibility.ParticleSystems[i])
				continue;

			const ParticleRenderState& renderState = inputs.Scene.GetParticleRenderState(i);
			if(renderState.GpuParticleSystem != nullptr && renderState.DrawCommand.IsValid())
				renderState.BindGpuSimulatedInputs(inputs.Scene.GetParticleSystemProxy(i));
		}
	}

	mSurfaceShaderParameterSets.SetCpuParticleIndices(dependencies.Get<RCNodeParticleSort>()->ParticleIndices);

	// Draws of GPU simulated particles read the outputs of the last simulation, which ParticleSimulate writes later in the graph
	GpuParticleInputs = GpuParticleSimulation::Instance().GetResources().GetDrawInputs();
	GpuParticleInputs.Import(inputs.Graph);
	mSurfaceShaderParameterSets.SetGpuParticleInputs(GpuParticleInputs);

	Camera* sceneCamera = inputs.View.GetSceneCamera();

	// Trigger prepare callbacks
	if(sceneCamera != nullptr)
	{
		const RendererViewContext context(inputs.Graph);
		for(RendererExtension* extension : inputs.ExtPrepare)
			extension->Render(*sceneCamera, context);
	}

	// Velocity follows the Id, so draws whose shader cannot write velocity still write the Id into its own target
	TInlineArray<TShared<Texture>, 8> gbufferAttachments = { sceneColorNode->SceneColorTex, AlbedoTex, NormalTex, RoughMetalTex, IdTex };
	if(needsVelocity)
		gbufferAttachments.Add(VelocityTex);

	// Without pre-base-pass callbacks the base pass clears the G-buffer itself. Otherwise a pass of its own clears it before the callbacks.
	FrameGraphAttachmentLoad basePassLoad = FrameGraphAttachmentLoad::Clear;
	if(sceneCamera != nullptr && !inputs.ExtPreBasePass.Empty())
	{
		FrameGraphRasterPass clearPass;
		for(u32 colorIndex = 0; colorIndex < gbufferAttachments.Size(); colorIndex++)
			clearPass.SetColorAttachment(colorIndex, gbufferAttachments[colorIndex], FrameGraphAttachmentLoad::Clear);

		clearPass.SetDepthStencilAttachment(sceneDepthNode->DepthTex, FrameGraphDepthStencilMode::Write, FrameGraphAttachmentLoad::Clear);

		inputs.Graph.AddRasterPass("BasePassClear", std::move(clearPass), [](FrameGraphPassContext&) {});

		RendererViewContext context(inputs.Graph);
		context.ColorAttachments = gbufferAttachments;
		context.DepthStencilAttachment = sceneDepthNode->DepthTex;

		for(RendererExtension* extension : inputs.ExtPreBasePass)
			extension->Render(*sceneCamera, context);

		basePassLoad = FrameGraphAttachmentLoad::Load;
	}

	// Render all visible opaque elements that use the deferred pipeline, into the G-buffer
	FrameGraphRasterPass basePass;
	for(u32 colorIndex = 0; colorIndex < gbufferAttachments.Size(); colorIndex++)
		basePass.SetColorAttachment(colorIndex, gbufferAttachments[colorIndex], basePassLoad);

	basePass.SetDepthStencilAttachment(sceneDepthNode->DepthTex, FrameGraphDepthStencilMode::Write, basePassLoad);

	// The render queue lives until the view's frame ends, after the graph executes
	const Vector<RenderQueueEntry>& opaqueEntries = inputs.View.GetOpaqueQueue(false)->GetSortedEntries();
	mSurfaceShaderParameterSets.DeclareSets(basePass, opaqueEntries);

	inputs.Graph.AddRasterPass("BasePass", std::move(basePass), [this, &opaqueEntries](FrameGraphPassContext& context)
	{
		GpuCommandBuffer& commandBuffer = context.GetCommandBuffer();
		commandBuffer.SetViewport(Area2(0.0f, 0.0f, 1.0f, 1.0f));

		RenderQueueEntries(commandBuffer, opaqueEntries, mSurfaceShaderParameterSets);
	});

	// Determine MSAA coverage if required: the pixels whose samples differ, in the coverage texture for compute shaders and in the
	// stencil for draws
	const u32 sampleCount = inputs.View.GetProperties().Target.NumSamples;
	if(sampleCount > 1)
	{
		RCNodeMSAACoverage* msaaCoverageNode = dependencies.Get<RCNodeMSAACoverage>();

		GBufferTextures gbuffer;
		gbuffer.Albedo = AlbedoTex;
		gbuffer.Normals = NormalTex;
		gbuffer.RoughMetal = RoughMetalTex;
		gbuffer.Depth = sceneDepthNode->DepthTex;

		MSAACoverageMaterial* const msaaCoverageMaterial = MSAACoverageMaterial::GetVariation(sampleCount);

		FrameGraphRasterPass msaaCoveragePass;
		msaaCoveragePass.SetColorAttachment(0, msaaCoverageNode->Output, FrameGraphAttachmentLoad::DontCare);

		const TShared<GpuParameterSet> msaaCoverageSet = msaaCoverageMaterial->Prepare(msaaCoveragePass, inputs.View, gbuffer);
		inputs.Graph.AddRasterPass("MSAACoverage", std::move(msaaCoveragePass), [msaaCoverageMaterial, msaaCoverageSet, &view = inputs.View](FrameGraphPassContext& context)
		{
			context.GetCommandBuffer().SetViewport(Area2(0.0f, 0.0f, 1.0f, 1.0f));
			msaaCoverageMaterial->Execute(context, msaaCoverageSet, view);
		});

		// Marks samples in the stencil, keeping the depth the base pass wrote
		MSAACoverageStencilMaterial* const msaaCoverageStencilMaterial = MSAACoverageStencilMaterial::Get();

		FrameGraphRasterPass msaaCoverageStencilPass;
		msaaCoverageStencilPass.SetDepthStencilAttachment(sceneDepthNode->DepthTex, FrameGraphDepthStencilMode::DepthReadOnly, FrameGraphAttachmentLoad::Load);

		const TShared<GpuParameterSet> msaaCoverageStencilSet = msaaCoverageStencilMaterial->Prepare(msaaCoverageStencilPass, msaaCoverageNode->Output);
		inputs.Graph.AddRasterPass("MSAACoverageStencil", std::move(msaaCoverageStencilPass), [msaaCoverageStencilMaterial, msaaCoverageStencilSet, &view = inputs.View](FrameGraphPassContext& context)
		{
			context.GetCommandBuffer().SetViewport(Area2(0.0f, 0.0f, 1.0f, 1.0f));
			msaaCoverageStencilMaterial->Execute(context, msaaCoverageStencilSet, view);
		});
	}

	// Render decals after all normal objects, using a read-only depth buffer. They sample depth and the Id texture.
	const Vector<RenderQueueEntry>& decalEntries = inputs.View.GetDecalQueue()->GetSortedEntries();
	if(!decalEntries.empty())
	{
		FrameGraphRasterPass decalPass;

		u32 decalColorIndex = 0;
		decalPass.SetColorAttachment(decalColorIndex++, sceneColorNode->SceneColorTex, FrameGraphAttachmentLoad::Load);
		decalPass.SetColorAttachment(decalColorIndex++, AlbedoTex, FrameGraphAttachmentLoad::Load);
		decalPass.SetColorAttachment(decalColorIndex++, NormalTex, FrameGraphAttachmentLoad::Load);
		decalPass.SetColorAttachment(decalColorIndex++, RoughMetalTex, FrameGraphAttachmentLoad::Load);

		decalPass.SetDepthStencilAttachment(sceneDepthNode->DepthTex, FrameGraphDepthStencilMode::DepthReadOnly, FrameGraphAttachmentLoad::Load);
		mSurfaceShaderParameterSets.DeclareSets(decalPass, decalEntries);

		inputs.Graph.AddRasterPass("Decals", std::move(decalPass), [this, &decalEntries](FrameGraphPassContext& context)
		{
			GpuCommandBuffer& commandBuffer = context.GetCommandBuffer();
			commandBuffer.SetViewport(Area2(0.0f, 0.0f, 1.0f, 1.0f));

			RenderQueueEntries(commandBuffer, decalEntries, mSurfaceShaderParameterSets);
		});
	}

	// Trigger post-base-pass callbacks. They render into the G-buffer without the Id, with read-only depth, as decals do.
	if(sceneCamera != nullptr && !inputs.ExtPostBasePass.Empty())
	{
		RendererViewContext context(inputs.Graph);
		context.ColorAttachments = { sceneColorNode->SceneColorTex, AlbedoTex, NormalTex, RoughMetalTex };
		if(needsVelocity)
			context.ColorAttachments.Add(VelocityTex);

		context.DepthStencilAttachment = sceneDepthNode->DepthTex;
		context.DepthStencilMode = FrameGraphDepthStencilMode::DepthReadOnly;

		for(RendererExtension* extension : inputs.ExtPostBasePass)
			extension->Render(*sceneCamera, context);
	}
}

void RCNodeBasePass::Clear()
{
	AlbedoTex = nullptr;
	NormalTex = nullptr;
	RoughMetalTex = nullptr;
	IdTex = nullptr;
	VelocityTex = nullptr;
	GpuParticleInputs = GpuParticleDrawInputs();
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

	if(viewProps.Target.NumSamples > 1)
		SceneColorTexArray = inputs.Graph.CreateTexture("SceneColorArray", CreateViewSampleArrayInformation(inputs.View, PF_RGBA16F));
	else
		SceneColorTexArray = nullptr;
}

void RCNodeSceneColor::Clear()
{
	SceneColorTex = nullptr;
	SceneColorTexArray = nullptr;
}

void RCNodeSceneColor::AddArrayToMSAAPass(FrameGraph& graph)
{
	AddTextureArrayToMSAAPass(graph, "SceneColorArrayToMSAA", SceneColorTexArray, SceneColorTex);
	SceneColorTexArray = nullptr;
}

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

	// We just allocate the texture, while the base pass is responsible for filling it out
	Output = inputs.Graph.CreateTexture("MSAACoverage", TextureCreateInformation::Create2D(PF_R8, viewProps.Target.ViewRect.Width, viewProps.Target.ViewRect.Height,
		TextureUsageFlag::RenderTarget));
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
	GpuParticleSimulation& simulation = GpuParticleSimulation::Instance();

	// The first view of the main pass that renders in a frame simulates
	if(inputs.ViewGroup.IsMainPass())
	{
		auto dependencies = GetDependencyDefinition().ResolveDependencies(inputs);
		RCNodeBasePass* gbufferNode = dependencies.Get<RCNodeBasePass>();
		RCNodeSceneDepth* sceneDepthNode = dependencies.Get<RCNodeSceneDepth>();

		GBufferTextures gbuffer;
		gbuffer.Albedo = gbufferNode->AlbedoTex;
		gbuffer.Normals = gbufferNode->NormalTex;
		gbuffer.RoughMetal = gbufferNode->RoughMetalTex;
		gbuffer.Depth = sceneDepthNode->DepthTex;

		// Depth collisions sample depth and normals, so with MSAA they read resolved ones. The graph culls the normals resolve if no
		// particle system collides with depth.
		const TextureProperties& normalsProperties = gbuffer.Normals->GetProperties();
		if(normalsProperties.SampleCount > 1)
		{
			gbuffer.Depth = dependencies.Get<RCNodeResolvedSceneDepth>()->Output;

			const TShared<Texture> resolvedNormals = inputs.Graph.CreateTexture("ParticleResolvedNormals", TextureCreateInformation::Create2D(normalsProperties.Format,
				normalsProperties.Width, normalsProperties.Height, TextureUsageFlag::RenderTarget));
			AddResolvePass(inputs.Graph, "ParticleNormalsResolve", gbuffer.Normals, resolvedNormals);

			gbuffer.Normals = resolvedNormals;
		}

		simulation.Simulate(inputs.Graph, inputs.Scene, inputs.FrameInfo.PerSceneFrameData.Particles, inputs.View.GetPerViewBuffer(), gbuffer, inputs.FrameInfo.Timings);
	}

	simulation.Sort(inputs.Graph, inputs.Scene, inputs.View);
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
	// Sets of views without CPU simulated particles still bind a buffer
	const u32 indexCount = inputs.Scene.GetParticleIndexCount();
	const u32 requiredSize = std::max(indexCount, 1u) * (u32)sizeof(u32);
	if(ParticleIndices == nullptr || ParticleIndices->GetTotalSize() < requiredSize)
	{
		GpuBufferCreateInformation bufferCreateInformation;
		bufferCreateInformation.Type = GpuBufferType::SimpleStorage;
		bufferCreateInformation.SimpleStorage.Count = Bitwise::NextPow2(std::max(indexCount, 1u));
		bufferCreateInformation.SimpleStorage.Format = BF_16X2U;

		ParticleIndices = GetRenderBeast()->GetGpuDevice()->CreateGpuBuffer(bufferCreateInformation);
	}

	const EvaluatedParticleData* particleData = inputs.FrameInfo.PerSceneFrameData.Particles;
	if(!particleData || indexCount == 0)
		return;

	const RendererViewProperties& viewProps = inputs.View.GetProperties();
	const VisibilityInfo& visibility = inputs.View.GetVisibilityMasks();
	const auto particleSystemCount = inputs.Scene.GetParticleSystemCount();

	// Write the particle indices of the visible CPU simulated systems, distance sorted for the view where the system sorts by distance.
	// The simulation's render data is shared by every view, so it is not changed.
	B3DMarkAllocatorFrame();
	{
		struct SystemIndices
		{
			PackedRendererId ProxyId;
			const ParticleRenderData* RenderData;
			u32 Offset;
		};

		FrameVector<SystemIndices> systems;
		for(u32 particleSystemIndex = 0; particleSystemIndex < particleSystemCount; particleSystemIndex++)
		{
			if(!visibility.ParticleSystems[particleSystemIndex])
				continue;

			const render::ParticleSystemProxy& proxy = inputs.Scene.GetParticleSystemProxy(particleSystemIndex);
			const auto iterFind = particleData->CpuData.find(proxy.GetId());
			if(iterFind == particleData->CpuData.end())
				continue;

			const u32 offset = inputs.Scene.GetParticleRenderState(particleSystemIndex).ParticleIndicesOffset;
			systems.push_back({ (PackedRendererId)particleSystemIndex, iterFind->second, offset });
		}

		FrameVector<u32> indices(indexCount);
		u32* const indicesData = indices.data();

		WaitGroup waitGroup((u32)systems.size());
		const auto fnWorker = [&waitGroup, &scene = inputs.Scene, viewOrigin = viewProps.ViewOrigin, indicesData](const SystemIndices& system)
		{
			const render::ParticleSystemProxy& proxy = scene.GetParticleSystemProxy(system.ProxyId);
			const render::ParticleSystemSettings& settings = proxy.GetSettings();

			const bool isBillboard = settings.RenderMode == ParticleRenderMode::Billboard;
			const PixelData& positions = isBillboard ? static_cast<const ParticleBillboardRenderData*>(system.RenderData)->PositionAndRotation
				: static_cast<const ParticleMeshRenderData*>(system.RenderData)->Position;

			const u32 particleCount = system.RenderData->NumParticles;
			u32* const systemIndices = indicesData + system.Offset;
			if(settings.SortMode == ParticleSortMode::Distance)
			{
				// Transform the view point into particle system's local space
				Vector3 refPoint = viewOrigin;
				if(settings.SimulationSpace == ParticleSimulationSpace::Local)
					refPoint = proxy.GetWorldTransform().GetInvMatrix().MultiplyAffine(refPoint);

				ParticleRenderer::SortByDistance(refPoint, positions, particleCount, isBillboard ? 4 : 3, systemIndices);
			}
			else if(particleCount > 0)
				memcpy(systemIndices, system.RenderData->Indices.data(), particleCount * sizeof(u32));

			// The shader reads the particles at texel coordinates of the system's textures
			const u32 textureSize = positions.GetWidth();
			for(u32 i = 0; i < particleCount; i++)
			{
				const u32 particleIndex = systemIndices[i];
				systemIndices[i] = ((particleIndex % textureSize) & 0xFFFF) | ((particleIndex / textureSize) << 16);
			}

			waitGroup.NotifyDone();
		};

		Scheduler& taskScheduler = GetApplication().GetTaskScheduler();
		for (const auto& system : systems)
		{
			taskScheduler.Post(SchedulerTask([&fnWorker, &system] { fnWorker(system); }, "ParticleSort"));
		}

		waitGroup.Wait();

		GpuBufferUtility::Write(GetRenderer()->GetGpuContext(), ParticleIndices, 0, indexCount * (u32)sizeof(u32), indicesData, GpuBufferWriteFlag::Discard);
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
		// Tiled lighting offsets the texels it writes by the view's position in its target, so a view that doesn't start at the origin
		// leaves some unwritten
		LightAccumulationTexArray = inputs.Graph.CreateTexture("LightAccumulationArray", CreateViewSampleArrayInformation(inputs.View, PF_RGBA16F));

		ClearLoadStoreMaterial* const clearMaterial = ClearLoadStoreMaterial::GetVariation(ClearLoadStoreType::TextureArray, ClearLoadStoreDataType::Float, 4);

		// One slice per dispatch
		FrameGraphComputePass clearPass;
		TInlineArray<TShared<GpuParameterSet>, 8> clearSets;
		for(u32 i = 0; i < viewProps.Target.NumSamples; i++)
		{
			TextureSurface surface;
			surface.Face = i;
			surface.FaceCount = 1;
			surface.MipLevel = 0;
			surface.MipLevelCount = 1;
			surface.IsBoundAs2DArray = true;

			clearSets.Add(clearMaterial->Prepare(clearPass, LightAccumulationTexArray, Color::kZero, surface));
		}

		const u32 width = viewProps.Target.ViewRect.Width;
		const u32 height = viewProps.Target.ViewRect.Height;
		inputs.Graph.AddComputePass("LightAccumulationArrayClear", std::move(clearPass), [clearMaterial, clearSets = std::move(clearSets), width, height](FrameGraphPassContext& context)
		{
			for(const TShared<GpuParameterSet>& set : clearSets)
				clearMaterial->Execute(context, set, width, height);
		});
	}
	else
	{
		usage |= TextureUsageFlag::AllowUnorderedAccessOnTheGPU;
		LightAccumulationTexArray = nullptr;
	}

	LightAccumulationTex = inputs.Graph.CreateTexture("LightAccumulation", CreateViewTextureInformation(inputs.View, PF_RGBA16F, usage));
}

void RCNodeLightAccumulation::AddArrayToMSAAPass(FrameGraph& graph)
{
	AddTextureArrayToMSAAPass(graph, "LightAccumulationArrayToMSAA", LightAccumulationTexArray, LightAccumulationTex);
	LightAccumulationTexArray = nullptr;
}

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

	if(viewProps.Target.NumSamples > 1)
		Output->AddArrayToMSAAPass(inputs.Graph);

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

	const RendererView& view = inputs.View;
	const bool msaa = view.GetProperties().Target.NumSamples > 1;

	const LightProbes& lightProbes = inputs.Scene.GetLightProbes();
	const LightProbesInfo lpInfo = lightProbes.GetInfo();

	IrradianceEvaluateMaterial* evaluateMat;
	TShared<Texture> volumeIndices;
	if(lightProbes.HasAnyProbes())
	{
		// Pixels that no tetrahedron covers keep the cleared index
		volumeIndices = inputs.Graph.CreateTexture("LightProbeVolumeIndices", CreateViewTextureInformation(view, PF_R16U, TextureUsageFlag::RenderTarget));
		const TShared<Texture> volumeDepth = inputs.Graph.CreateTexture("LightProbeVolumeDepth", CreateViewTextureInformation(view, PF_D32, TextureUsageFlag::DepthStencil));

		FrameGraphRasterPass tetrahedraPass;
		tetrahedraPass.SetColorAttachment(0, volumeIndices, FrameGraphAttachmentLoad::DontCare);
		tetrahedraPass.SetDepthStencilAttachment(volumeDepth, FrameGraphDepthStencilMode::Write, FrameGraphAttachmentLoad::Clear);

		ClearMaterial* clearMat = ClearMaterial::GetVariation(false);
		const TShared<GpuParameterSet> clearSet = clearMat->Prepare(tetrahedraPass, ~0u);

		TetrahedraRenderMaterial* tetrahedraMat = TetrahedraRenderMaterial::GetVariation(msaa, true);
		const TShared<GpuParameterSet> tetrahedraSet = tetrahedraMat->Prepare(tetrahedraPass, view, sceneDepthNode->DepthTex);

		inputs.Graph.AddRasterPass("LightProbeTetrahedra", std::move(tetrahedraPass), [clearMat, clearSet, tetrahedraMat, tetrahedraSet, volumeMesh = lpInfo.TetrahedraVolume](FrameGraphPassContext& context)
		{
			context.GetCommandBuffer().SetViewport(Area2(0.0f, 0.0f, 1.0f, 1.0f));
			clearMat->Execute(context, clearSet);
			tetrahedraMat->Execute(context, tetrahedraSet, volumeMesh);
		});

		evaluateMat = IrradianceEvaluateMaterial::GetVariation(msaa, true, false);
	}
	else // Sky only
	{
		evaluateMat = IrradianceEvaluateMaterial::GetVariation(msaa, true, true);
	}

	GBufferTextures gbuffer;
	gbuffer.Albedo = gbufferNode->AlbedoTex;
	gbuffer.Normals = gbufferNode->NormalTex;
	gbuffer.RoughMetal = gbufferNode->RoughMetalTex;
	gbuffer.Depth = sceneDepthNode->DepthTex;

	Skybox* skybox = nullptr;
	if(view.GetRenderSettings().EnableSkybox)
		skybox = inputs.Scene.GetSkybox();

	// Adds onto the light accumulation. The pass samples the scene depth as the G-buffer depth.
	FrameGraphRasterPass evaluatePass;
	evaluatePass.SetColorAttachment(0, lightAccumNode->LightAccumulationTex, FrameGraphAttachmentLoad::Load);
	evaluatePass.SetDepthStencilAttachment(sceneDepthNode->DepthTex, FrameGraphDepthStencilMode::ReadOnly, FrameGraphAttachmentLoad::Load);

	const TShared<GpuParameterSet> evaluateSet = evaluateMat->Prepare(evaluatePass, view, gbuffer, volumeIndices, lpInfo, skybox, ssaoNode->Output);
	inputs.Graph.AddRasterPass("IrradianceEvaluate", std::move(evaluatePass), [evaluateMat, evaluateSet, &view](FrameGraphPassContext& context)
	{
		context.GetCommandBuffer().SetViewport(Area2(0.0f, 0.0f, 1.0f, 1.0f));
		evaluateMat->Execute(context, evaluateSet, view);
	});
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

	TiledDeferredImageBasedLightingMaterial* material =
		TiledDeferredImageBasedLightingMaterial::GetVariation(viewProps.Target.NumSamples);

	TiledDeferredImageBasedLightingMaterial::Inputs iblInputs;
	iblInputs.Gbuffer = gbuffer;
	iblInputs.SceneColorTex = sceneColorNode->SceneColorTex;
	iblInputs.SceneColorTexArray = sceneColorNode->SceneColorTexArray;
	iblInputs.LightAccumulation = lightAccumNode->LightAccumulationTex;
	iblInputs.PreIntegratedGf = RendererTextures::preintegratedEnvGF;
	iblInputs.AmbientOcclusion = ssaoNode->Output;
	iblInputs.Ssr = ssrNode->Output;
	iblInputs.MsaaCoverage = msaaCoverage;

	FrameGraphComputePass iblPass;
	const TShared<GpuParameterSet> iblSet = material->Prepare(iblPass, inputs.View, inputs.Scene, inputs.ViewGroup.GetVisibleReflProbeData(), iblInputs);

	const u32 width = viewProps.Target.ViewRect.Width;
	const u32 height = viewProps.Target.ViewRect.Height;
	inputs.Graph.AddComputePass("TiledImageBasedLighting", std::move(iblPass), [material, iblSet, width, height](FrameGraphPassContext& context)
	{
		material->Execute(context, iblSet, width, height);
	});

	if(viewProps.Target.NumSamples > 1)
		sceneColorNode->AddArrayToMSAAPass(inputs.Graph);
}

void RCNodeDeferredIndirectSpecularLighting::Clear()
{
	// Do nothing
}

RCNodeDeferredIndirectSpecularLighting::DependencyDefinition RCNodeDeferredIndirectSpecularLighting::GetDependencyDefinition()
{
	static const DependencyDefinition kDependencyDefinition;
	return kDependencyDefinition;
}

void RCNodeClusteredForward::Render(const RenderCompositorNodeInputs& inputs)
{
	const RenderBeastScene& scene = inputs.Scene;

	const VisibleLightData& visibleLightData = inputs.ViewGroup.GetVisibleLightData();
	const VisibleReflectionProbeData& visibleReflProbeData = inputs.ViewGroup.GetVisibleReflProbeData();

	Skybox* skybox = nullptr;
	if(inputs.View.GetRenderSettings().EnableSkybox)
		skybox = scene.GetSkybox();

	auto dependencies = GetDependencyDefinition().ResolveDependencies(inputs);
	auto sceneColorNode = dependencies.Get<RCNodeSceneColor>();
	auto sceneDepthNode = dependencies.Get<RCNodeSceneDepth>();
	auto resolvedSceneDepthNode = dependencies.Get<RCNodeResolvedSceneDepth>();

	// The graph culls the grid's passes if no draw reads it
	const LightGridOutputs lightGrid = AddLightGridPasses(inputs.Graph, inputs.View, visibleLightData, visibleReflProbeData, !inputs.View.GetRenderSettings().EnableLighting);

	// Prepare the renderer inputs of forward draws
	mSurfaceShaderParameterSets.Prepare(inputs.View, inputs.Scene);
	mSurfaceShaderParameterSets.SetSceneDepth(resolvedSceneDepthNode->Output);
	mSurfaceShaderParameterSets.SetForwardLightingInputs(inputs.View, lightGrid, visibleLightData, visibleReflProbeData, scene, skybox);

	// Forward draws of GPU simulated particles read what the base pass draws read, from before the simulation swapped its state
	mSurfaceShaderParameterSets.SetGpuParticleInputs(dependencies.Get<RCNodeBasePass>()->GpuParticleInputs);
	mSurfaceShaderParameterSets.SetCpuParticleIndices(dependencies.Get<RCNodeParticleSort>()->ParticleIndices);

	// TODO: Forward pipeline rendering doesn't support shadows. In order to support this I'd have to render the light
	// occlusion for all lights affecting this object into a single (or a few) textures. I can likely use texture
	// arrays for this, or to avoid sampling many textures, perhaps just jam it all in one or few texture channels.

	// The render queues live until the view's frame ends, after the graph executes
	const Vector<RenderQueueEntry>& opaqueEntries = inputs.View.GetOpaqueQueue(true)->GetSortedEntries();
	if(!opaqueEntries.empty())
	{
		FrameGraphRasterPass opaquePass;
		opaquePass.SetColorAttachment(0, sceneColorNode->SceneColorTex, FrameGraphAttachmentLoad::Load);
		opaquePass.SetDepthStencilAttachment(sceneDepthNode->DepthTex, FrameGraphDepthStencilMode::Write, FrameGraphAttachmentLoad::Load);
		mSurfaceShaderParameterSets.DeclareSets(opaquePass, opaqueEntries);

		inputs.Graph.AddRasterPass("ForwardOpaque", std::move(opaquePass), [this, &opaqueEntries](FrameGraphPassContext& context)
		{
			GpuCommandBuffer& commandBuffer = context.GetCommandBuffer();
			commandBuffer.SetViewport(Area2(0.0f, 0.0f, 1.0f, 1.0f));

			RenderQueueEntries(commandBuffer, opaqueEntries, mSurfaceShaderParameterSets);
		});
	}

	// Transparent draws test against depth without writing it, and soft particles sample it
	const Vector<RenderQueueEntry>& transparentEntries = inputs.View.GetTransparentQueue()->GetSortedEntries();
	if(!transparentEntries.empty())
	{
		FrameGraphRasterPass transparentPass;
		transparentPass.SetColorAttachment(0, sceneColorNode->SceneColorTex, FrameGraphAttachmentLoad::Load);
		transparentPass.SetDepthStencilAttachment(sceneDepthNode->DepthTex, FrameGraphDepthStencilMode::DepthReadOnly, FrameGraphAttachmentLoad::Load);
		mSurfaceShaderParameterSets.DeclareSets(transparentPass, transparentEntries);

		inputs.Graph.AddRasterPass("ForwardTransparent", std::move(transparentPass), [this, &transparentEntries](FrameGraphPassContext& context)
		{
			GpuCommandBuffer& commandBuffer = context.GetCommandBuffer();
			commandBuffer.SetViewport(Area2(0.0f, 0.0f, 1.0f, 1.0f));

			RenderQueueEntries(commandBuffer, transparentEntries, mSurfaceShaderParameterSets);
		});
	}

	// Trigger post-lighting callbacks
	Camera* sceneCamera = inputs.View.GetSceneCamera();
	if(sceneCamera != nullptr && !inputs.ExtPostLighting.Empty())
	{
		RendererViewContext context(inputs.Graph);
		context.ColorAttachments.Add(sceneColorNode->SceneColorTex);
		context.DepthStencilAttachment = sceneDepthNode->DepthTex;

		for(RendererExtension* extension : inputs.ExtPostLighting)
			extension->Render(*sceneCamera, context);
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

	// Writes only the alpha of the target, so it keeps what the resolve wrote
	if(viewProperties.EncodeDepth)
	{
		RCNodeResolvedSceneDepth* resolvedSceneDepthNode = dependencies.Get<RCNodeResolvedSceneDepth>();

		FrameGraphRasterPass encodeDepthPass;
		encodeDepthPass.SetRenderTarget(target, FrameGraphAttachmentLoad::Load);

		EncodeDepthMaterial* const encodeDepthMaterial = EncodeDepthMaterial::Get();
		const TShared<GpuParameterSet> encodeDepthSet = encodeDepthMaterial->Prepare(encodeDepthPass, inputs.View, resolvedSceneDepthNode->Output,
			viewProperties.DepthEncodeNear, viewProperties.DepthEncodeFar);

		inputs.Graph.AddRasterPass("EncodeDepth", std::move(encodeDepthPass), [encodeDepthMaterial, encodeDepthSet, viewArea](FrameGraphPassContext& context)
		{
			context.GetCommandBuffer().SetViewport(viewArea);
			encodeDepthMaterial->Execute(context, encodeDepthSet);
		});
	}

	// Trigger overlay callbacks
	Camera* sceneCamera = inputs.View.GetSceneCamera();
	if(sceneCamera != nullptr && !inputs.ExtOverlay.Empty())
	{
		RendererViewContext context(inputs.Graph);
		context.Target = target;
		context.Viewport = viewArea;

		for(RendererExtension* extension : inputs.ExtOverlay)
			extension->Render(*sceneCamera, context);
	}

	// Resolve pending view captures after overlays and before clearing the target.
	inputs.View.ResolveCaptures(inputs.Graph, target);
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

void RCNodePostProcess::Render(const RenderCompositorNodeInputs& inputs)
{
	// Do nothing, this is just a helper node
}

TShared<Texture> RCNodePostProcess::CreateOutput(FrameGraph& graph, const RendererView& view)
{
	const RendererViewProperties& viewProps = view.GetProperties();
	u32 width = viewProps.Target.ViewRect.Width;
	u32 height = viewProps.Target.ViewRect.Height;

	mOutput = graph.CreateTexture("PostProcessOutput", TextureCreateInformation::Create2D(PF_RGBA8, width, height, TextureUsageFlag::RenderTarget));
	return mOutput;
}

void RCNodePostProcess::Clear()
{
	mOutput = nullptr;
}

RCNodePostProcess::DependencyDefinition RCNodePostProcess::GetDependencyDefinition()
{
	static const DependencyDefinition kDependencyDefinition;
	return kDependencyDefinition;
}

void RCNodeEyeAdaptation::Render(const RenderCompositorNodeInputs& inputs)
{
	auto dependencies = GetDependencyDefinition().ResolveDependencies(inputs);
	const RenderSettings& settings = inputs.View.GetRenderSettings();

	const bool hdr = settings.EnableHdr;

	if(hdr && settings.EnableAutoExposure)
	{
		// Get downsample scene
		auto* halfSceneColorNode = dependencies.Get<RCNodeHalfSceneColor>();
		const TShared<Texture>& downsampledScene = halfSceneColorNode->Output;

		// Generate histogram
		const TShared<Texture> eyeAdaptHistogram = inputs.Graph.CreateTexture("EyeAdaptationHistogram", EyeAdaptHistogramMaterial::GetOutputCreateInformation(downsampledScene));
		EyeAdaptHistogramMaterial* eyeAdaptHistogramMat = EyeAdaptHistogramMaterial::Get();

		FrameGraphComputePass histogramPass;
		const TShared<GpuParameterSet> histogramSet = eyeAdaptHistogramMat->Prepare(histogramPass, downsampledScene, eyeAdaptHistogram, settings.AutoExposure);
		inputs.Graph.AddComputePass("EyeAdaptationHistogram", std::move(histogramPass), [eyeAdaptHistogramMat, histogramSet, downsampledScene](FrameGraphPassContext& context)
		{
			eyeAdaptHistogramMat->Execute(context, histogramSet, downsampledScene);
		});

		// Reduce histogram, together with the previous frame's eye adaptation value
		const TShared<Texture> reducedHistogram = inputs.Graph.CreateTexture("EyeAdaptationReducedHistogram", EyeAdaptHistogramReduceMaterial::GetOutputCreateInformation());
		AddFullScreenPass(inputs.Graph, "EyeAdaptationHistogramReduce", EyeAdaptHistogramReduceMaterial::Get(), reducedHistogram, FrameGraphAttachmentLoad::DontCare,
			downsampledScene, eyeAdaptHistogram, previous);

		// Generate eye adaptation value. Kept for the next frame, which reads it at rest.
		Output = inputs.Graph.CreateTexture("EyeAdaptation", EyeAdaptationMaterial::GetOutputCreateInformation());
		inputs.Graph.ExtractTexture(Output);

		AddFullScreenPass(inputs.Graph, "EyeAdaptation", EyeAdaptationMaterial::Get(), Output, FrameGraphAttachmentLoad::DontCare,
			reducedHistogram, inputs.FrameInfo.Timings.TimeDelta, settings.AutoExposure, settings.ExposureScale);

		// Notify the view eye adaptation value will change
		inputs.View.NotifyLuminanceUpdated(inputs.FrameInfo.Timings.FrameIndex, inputs.Graph, Output);
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

	const RenderSettings& settings = inputs.View.GetRenderSettings();

	auto* eyeAdaptationNode = dependencies.Get<RCNodeEyeAdaptation>();
	auto* sceneColorNode = dependencies.Get<RCNodeSceneColor>();
	auto* postProcessNode = dependencies.Get<RCNodePostProcess>();
	const TShared<Texture>& sceneColor = sceneColorNode->SceneColorTex;

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
				{
					TextureCreateInformation lutInformation = CreateTonemap3DLUTMaterial::GetOutputCreateInformation();
					lutInformation.Name = "TonemapLUT";

					mTonemapLUT = inputs.Graph.GetWorkContext().GetDevice().CreateTexture(lutInformation);
				}

				inputs.Graph.ImportTexture(mTonemapLUT);

				FrameGraphComputePass createLUTPass;
				const TShared<GpuParameterSet> createLUTSet = createLUT->Prepare(createLUTPass, mTonemapLUT, settings);
				inputs.Graph.AddComputePass("CreateTonemapLUT", std::move(createLUTPass), [createLUT, createLUTSet](FrameGraphPassContext& context)
				{
					createLUT->Execute(context, createLUTSet);
				});

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

	TShared<Texture> bloomTex;
	if(settings.Bloom.Enabled)
	{
		auto* bloomNode = dependencies.Get<RCNodeBloom>();
		bloomTex = bloomNode->Output;
	}

	const TShared<Texture> output = postProcessNode->CreateOutput(inputs.Graph, inputs.View);
	AddFullScreenPass(inputs.Graph, "Tonemapping", tonemapping, output, FrameGraphAttachmentLoad::DontCare, sceneColor, eyeAdaptationNode->Output, bloomTex, mTonemapLUT, settings);
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

	RCNodeSceneColor* sceneColorNode = dependencies.Get<RCNodeSceneColor>();
	RCNodeSceneDepth* sceneDepthNode = dependencies.Get<RCNodeSceneDepth>();
	RCNodeLightAccumulation* lightAccumNode = dependencies.Get<RCNodeLightAccumulation>();

	// After TAA, scene color has a single sample even with MSAA, and is read with the resolved depth
	const TShared<Texture>& sceneColor = sceneColorNode->SceneColorTex;
	const bool msaa = sceneColor->GetProperties().SampleCount > 1;
	const TShared<Texture>& depth = msaa ? sceneDepthNode->DepthTex : dependencies.Get<RCNodeResolvedSceneDepth>()->Output;

	BokehDOFPrepareMaterial* prepareMat = BokehDOFPrepareMaterial::GetVariation(msaa);
	BokehDOFMaterial* renderMat = BokehDOFMaterial::GetVariation(settings.BokehOcclusion);
	BokehDOFCombineMaterial* combineMat = BokehDOFCombineMaterial::GetVariation(msaa ? MSAAMode::Full : MSAAMode::None);

	// Downsample scene and store depth in .w
	const TShared<Texture> halfResSceneAndDepth = inputs.Graph.CreateTexture("BokehDOFHalfSceneAndDepth", BokehDOFPrepareMaterial::GetOutputCreateInformation(sceneColor));

	FrameGraphRasterPass preparePass;
	preparePass.SetColorAttachment(0, halfResSceneAndDepth, FrameGraphAttachmentLoad::DontCare);

	const TShared<GpuParameterSet> prepareSet = prepareMat->Prepare(preparePass, sceneColor, depth, inputs.View, settings);
	inputs.Graph.AddRasterPass("BokehDOFPrepare", std::move(preparePass), [prepareMat, prepareSet, sceneColor](FrameGraphPassContext& context)
	{
		context.GetCommandBuffer().SetViewport(Area2(0.0f, 0.0f, 1.0f, 1.0f));
		prepareMat->Execute(context, prepareSet, sceneColor);
	});

	// Render the sprites, blending them into the cleared unfocused texture
	const TShared<Texture> unfocusedTex = inputs.Graph.CreateTexture("BokehDOFUnfocused", BokehDOFMaterial::GetOutputCreateInformation(halfResSceneAndDepth));

	FrameGraphRasterPass renderPass;
	renderPass.SetColorAttachment(0, unfocusedTex, FrameGraphAttachmentLoad::Clear);

	const TShared<GpuParameterSet> renderSet = renderMat->Prepare(renderPass, halfResSceneAndDepth, inputs.View, settings, unfocusedTex);
	inputs.Graph.AddRasterPass("BokehDOF", std::move(renderPass), [renderMat, renderSet, halfResSceneAndDepth](FrameGraphPassContext& context)
	{
		context.GetCommandBuffer().SetViewport(Area2(0.0f, 0.0f, 1.0f, 1.0f));
		renderMat->Execute(context, renderSet, halfResSceneAndDepth);
	});

	// Combine the unfocused and focused textures to form the final image
	AddFullScreenPass(inputs.Graph, "BokehDOFCombine", combineMat, lightAccumNode->LightAccumulationTex, FrameGraphAttachmentLoad::DontCare,
		unfocusedTex, sceneColor, depth, inputs.View, settings);

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

	const TemporalAASettings& settings = inputs.View.GetRenderSettings().TemporalAa;
	if(!settings.Enabled)
	{
		DeallocOutputs();

		Output = sceneColorNode->SceneColorTex;
		return;
	}

	TShared<Texture> sceneColor = sceneColorNode->SceneColorTex;

	// TODO - Resolve scene color MSAA (in a way that can be shared by multiple effects)

	auto* sceneDepthNode = dependencies.Get<RCNodeSceneDepth>();
	auto* basePassNode = dependencies.Get<RCNodeBasePass>();

	const RendererViewProperties& viewProps = inputs.View.GetProperties();

	u32 width = viewProps.Target.ViewRect.Width;
	u32 height = viewProps.Target.ViewRect.Height;

	// Resolve multiple samples if MSAA is used
	if(viewProps.Target.NumSamples > 1)
	{
		const TShared<Texture> resolvedSceneColor = inputs.Graph.CreateTexture("TemporalAAResolvedSceneColor", TextureCreateInformation::Create2D(PF_RGBA16F, width, height, TextureUsageFlag::RenderTarget));
		AddResolvePass(inputs.Graph, "TemporalAAResolve", sceneColor, resolvedSceneColor);

		sceneColor = resolvedSceneColor;
	}

	TShared<Texture> output;
	if(mPreviousFrame)
	{
		output = inputs.Graph.CreateTexture("TemporalAAOutput", TextureCreateInformation::Create2D(PF_RGBA16F, width, height, TextureUsageFlag::RenderTarget));

		float exposure = inputs.View.GetCurrentExposure();

		TemporalFilteringMaterial* temporalFilteringMat =
			TemporalFilteringMaterial::GetVariation(TemporalFilteringType::FullScreenAA, true, viewProps.Target.NumSamples > 1);

		FrameGraphRasterPass temporalAAPass;
		temporalAAPass.SetColorAttachment(0, output, FrameGraphAttachmentLoad::Clear);

		const TShared<GpuParameterSet> set = temporalFilteringMat->Prepare(temporalAAPass, inputs.View, mPreviousFrame, sceneColor, basePassNode->VelocityTex,
			sceneDepthNode->DepthTex, viewProps.TemporalJitter, exposure);

		inputs.Graph.AddRasterPass("TemporalAA", std::move(temporalAAPass), [temporalFilteringMat, set, &view = inputs.View](FrameGraphPassContext& context)
		{
			context.GetCommandBuffer().SetViewport(Area2(0.0f, 0.0f, 1.0f, 1.0f));
			temporalFilteringMat->Execute(context, set, view);
		});

		sceneColorNode->SetExternalTexture(output);
	}
	else
		output = sceneColor;

	// Kept for the next frame, which reads it at rest
	inputs.Graph.ExtractTexture(output);

	mHistoryOutput = output;
	Output = output;
}

void RCNodeTemporalAA::Clear()
{
	mPreviousFrame = mHistoryOutput;
	mHistoryOutput = nullptr;
	Output = nullptr;
}

void RCNodeTemporalAA::DeallocOutputs()
{
	mPreviousFrame = nullptr;
	mHistoryOutput = nullptr;
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

	// TShared<Texture> depth = sceneDepthNode->depthTex->texture;
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
	RCNodeResolvedSceneDepth* resolvedSceneDepthNode = dependencies.Get<RCNodeResolvedSceneDepth>();
	RCNodePostProcess* postProcessNode = dependencies.Get<RCNodePostProcess>();

	const DepthOfFieldSettings& settings = inputs.View.GetRenderSettings().DepthOfField;
	bool near = settings.NearBlurAmount > 0.0f;
	bool far = settings.FarBlurAmount > 0.0f;

	bool enabled = settings.Enabled && settings.Type == DepthOfFieldType::Gaussian && (near || far);
	if(!enabled)
		return;

	GaussianDOFSeparateMaterial* separateMat = GaussianDOFSeparateMaterial::GetVariation(near, far);
	GaussianDOFCombineMaterial* combineMat = GaussianDOFCombineMaterial::GetVariation(near, far);

	// The materials sample depth, so with MSAA they read the resolved one
	const TShared<Texture> input = postProcessNode->GetLastOutput();
	const TShared<Texture>& depth = resolvedSceneDepthNode->Output;

	// Separate the unfocused pixels, near into the first attachment if both planes are present
	const TextureCreateInformation separateOutputInformation = GaussianDOFSeparateMaterial::GetOutputCreateInformation(input);

	TShared<Texture> nearTex, farTex;
	FrameGraphRasterPass separatePass;
	if(near)
	{
		nearTex = inputs.Graph.CreateTexture("GaussianDOFNear", separateOutputInformation);
		separatePass.SetColorAttachment(0, nearTex, FrameGraphAttachmentLoad::DontCare);
	}

	if(far)
	{
		farTex = inputs.Graph.CreateTexture("GaussianDOFFar", separateOutputInformation);
		separatePass.SetColorAttachment(near ? 1 : 0, farTex, FrameGraphAttachmentLoad::DontCare);
	}

	const TShared<GpuParameterSet> separateSet = separateMat->Prepare(separatePass, input, depth, inputs.View, settings);
	inputs.Graph.AddRasterPass("GaussianDOFSeparate", std::move(separatePass), [separateMat, separateSet](FrameGraphPassContext& context)
	{
		context.GetCommandBuffer().SetViewport(Area2(0.0f, 0.0f, 1.0f, 1.0f));
		separateMat->Execute(context, separateSet);
	});

	// Blur the out of focus pixels
	// Note: Perhaps set up stencil so I can avoid performing blur on unused parts of the textures?
	TShared<Texture> blurredNearTex;
	if(nearTex)
	{
		blurredNearTex = inputs.Graph.CreateTexture("GaussianDOFBlurredNear", separateOutputInformation);
		AddGaussianBlurPasses(inputs.Graph, nearTex, settings.NearBlurAmount, blurredNearTex);
	}

	TShared<Texture> blurredFarTex;
	if(farTex)
	{
		blurredFarTex = inputs.Graph.CreateTexture("GaussianDOFBlurredFar", separateOutputInformation);
		AddGaussianBlurPasses(inputs.Graph, farTex, settings.FarBlurAmount, blurredFarTex);
	}

	const TShared<Texture> output = postProcessNode->CreateOutput(inputs.Graph, inputs.View);
	AddFullScreenPass(inputs.Graph, "GaussianDOFCombine", combineMat, output, FrameGraphAttachmentLoad::DontCare, input, blurredNearTex, blurredFarTex, depth, inputs.View, settings);
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

	const TShared<Texture> input = postProcessNode->GetLastOutput();
	const TShared<Texture> output = postProcessNode->CreateOutput(inputs.Graph, inputs.View);

	// Note: I could skip executing FXAA over DOF and motion blurred pixels
	AddFullScreenPass(inputs.Graph, "FXAA", FXAAMaterial::Get(), output, FrameGraphAttachmentLoad::DontCare, input);
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

	const TShared<Texture> input = postProcessNode->GetLastOutput();
	const TShared<Texture> output = postProcessNode->CreateOutput(inputs.Graph, inputs.View);

	ChromaticAberrationMaterial* chromaticAberration = ChromaticAberrationMaterial::GetVariation(settings.ChromaticAberration.Type);
	AddFullScreenPass(inputs.Graph, "ChromaticAberration", chromaticAberration, output, FrameGraphAttachmentLoad::DontCare, input, settings.ChromaticAberration);
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

	const TShared<Texture> input = postProcessNode->GetLastOutput();
	const TShared<Texture> output = postProcessNode->CreateOutput(inputs.Graph, inputs.View);

	AddFullScreenPass(inputs.Graph, "FilmGrain", FilmGrainMaterial::Get(), output, FrameGraphAttachmentLoad::DontCare, input, inputs.FrameInfo.Timings.Time, settings.FilmGrain);
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

	auto* sceneColorNode = dependencies.Get<RCNodeSceneColor>();
	const TShared<Texture>& input = sceneColorNode->SceneColorTex;

	// Downsample scene. After TAA, scene color has a single sample even with MSAA.
	const bool msaa = input->GetProperties().SampleCount > 1;
	DownsampleMaterial* downsampleMat = DownsampleMaterial::GetVariation(1, msaa);

	Output = inputs.Graph.CreateTexture("HalfSceneColor", DownsampleMaterial::GetOutputCreateInformation(input));
	AddDownsamplePass(inputs.Graph, "HalfSceneColor", downsampleMat, input, Output);
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

	auto* halfSceneColorNode = dependencies.Get<RCNodeHalfSceneColor>();
	const TextureProperties& halfSceneProps = halfSceneColorNode->Output->GetProperties();

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
			Output[i] = inputs.Graph.CreateTexture("SceneColorDownsample", DownsampleMaterial::GetOutputCreateInformation(Output[i - 1]));
			AddDownsamplePass(inputs.Graph, "SceneColorDownsample", downsampleMat, Output[i - 1], Output[i]);
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

	if(viewProps.Target.NumSamples > 1)
	{
		Output = inputs.Graph.CreateTexture("ResolvedSceneDepth", TextureCreateInformation::Create2D(PF_D32_S8X24, viewProps.Target.ViewRect.Width, viewProps.Target.ViewRect.Height,
			TextureUsageFlag::DepthStencil));

		// Keeps the minimum depth of the samples of each pixel. The clear leaves the stencil clear for the passes that mark it.
		const TShared<Texture>& sceneDepth = sceneDepthNode->DepthTex;
		const TextureProperties& sceneDepthProperties = sceneDepth->GetProperties();
		BlitMat* const blitMaterial = BlitMat::GetVariation(sceneDepthProperties.SampleCount, false, false);

		FrameGraphRasterPass resolvePass;
		resolvePass.SetDepthStencilAttachment(Output, FrameGraphDepthStencilMode::Write, FrameGraphAttachmentLoad::Clear);

		const TShared<GpuParameterSet> blitSet = blitMaterial->Prepare(resolvePass, sceneDepth);
		const Area2 inputArea(0.0f, 0.0f, (float)sceneDepthProperties.Width, (float)sceneDepthProperties.Height);
		inputs.Graph.AddRasterPass("ResolvedSceneDepth", std::move(resolvePass), [blitMaterial, blitSet, inputArea](FrameGraphPassContext& context)
		{
			context.GetCommandBuffer().SetViewport(Area2(0.0f, 0.0f, 1.0f, 1.0f));
			blitMaterial->Execute(context, blitSet, inputArea, false);
		});
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
	TextureCreateInformation hiZInformation = TextureCreateInformation::Create2D(PF_R32F, size, size, TextureUsageFlag::RenderTarget);
	hiZInformation.MipMapCount = numMips;

	Output = inputs.Graph.CreateTexture("HiZ", hiZInformation);

	Area2 srcRect = viewProps.Target.NrmViewRect;

	// If viewport size is odd, adjust UV
	srcRect.Width += (viewProps.Target.ViewRect.Width % 2) * (1.0f / viewProps.Target.ViewRect.Width);
	srcRect.Height += (viewProps.Target.ViewRect.Height % 2) * (1.0f / viewProps.Target.ViewRect.Height);

	BuildHiZMaterial* material = BuildHiZMaterial::Get();

	// Generate first mip
	Area2 destinationArea;
	bool downsampledFirstMip = false; // Not used currently
	if(downsampledFirstMip)
	{
		// Make sure that 1 pixel in HiZ maps to a 2x2 block in source
		destinationArea = Area2(0, 0, Math::CeilToInt(viewProps.Target.ViewRect.Width / 2.0f) / (float)size, Math::CeilToInt(viewProps.Target.ViewRect.Height / 2.0f) / (float)size);

		FrameGraphRasterPass firstMipPass;
		firstMipPass.SetColorAttachment(0, Output, FrameGraphAttachmentLoad::DontCare, 0);

		const TShared<GpuParameterSet> set = material->Prepare(firstMipPass, resolvedSceneDepth->Output, 0);
		inputs.Graph.AddRasterPass("HiZMip", std::move(firstMipPass), [material, set, srcRect, destinationArea](FrameGraphPassContext& context)
		{
			context.GetCommandBuffer().SetViewport(destinationArea);
			material->Execute(context, set, srcRect);
		});
	}
	else // First level is just a copy of the depth buffer
	{
		destinationArea = Area2(0, 0, viewProps.Target.ViewRect.Width / (float)size, viewProps.Target.ViewRect.Height / (float)size);

		Area2I sourceArea;
		sourceArea.X = (i32)(srcRect.X * viewProps.Target.ViewRect.Width);
		sourceArea.Y = (i32)(srcRect.Y * viewProps.Target.ViewRect.Height);
		sourceArea.Width = (u32)(srcRect.Width * viewProps.Target.ViewRect.Width);
		sourceArea.Height = (u32)(srcRect.Height * viewProps.Target.ViewRect.Height);

		// The depth is copied as a single channel color, into a 32-bit float target
		const TShared<Texture>& depth = resolvedSceneDepth->Output;
		BlitMat* const blitMaterial = BlitMat::GetVariation(depth->GetProperties().SampleCount, true, false, false, false, false, false, true);

		FrameGraphRasterPass copyPass;
		copyPass.SetColorAttachment(0, Output, FrameGraphAttachmentLoad::DontCare, 0);

		const TShared<GpuParameterSet> blitSet = blitMaterial->Prepare(copyPass, depth);
		const Area2 inputArea((float)sourceArea.X, (float)sourceArea.Y, (float)sourceArea.Width, (float)sourceArea.Height);

		inputs.Graph.AddRasterPass("HiZCopy", std::move(copyPass), [blitMaterial, blitSet, inputArea, destinationArea](FrameGraphPassContext& context)
		{
			context.GetCommandBuffer().SetViewport(destinationArea);
			blitMaterial->Execute(context, blitSet, inputArea, false);
		});
	}

	// Generate remaining mip levels, each from the one above it
	for(u32 i = 1; i <= numMips; i++)
	{
		FrameGraphRasterPass mipPass;
		mipPass.SetColorAttachment(0, Output, FrameGraphAttachmentLoad::DontCare, i);

		const TShared<GpuParameterSet> set = material->Prepare(mipPass, Output, i - 1);
		inputs.Graph.AddRasterPass("HiZMip", std::move(mipPass), [material, set, destinationArea](FrameGraphPassContext& context)
		{
			context.GetCommandBuffer().SetViewport(destinationArea);
			material->Execute(context, set, destinationArea);
		});
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
	const AmbientOcclusionSettings& settings = inputs.View.GetRenderSettings().AmbientOcclusion;
	if(!settings.Enabled)
	{
		Output = Texture::kWhite;
		return;
	}

	auto dependencies = GetDependencyDefinition().ResolveDependencies(inputs);
	/** Maximum valid depth range within samples in a sample set. In meters. */
	static const float kDepthRange = 1.0f;

	const RendererViewProperties& viewProps = inputs.View.GetProperties();

	RCNodeResolvedSceneDepth* resolvedDepthNode = dependencies.Get<RCNodeResolvedSceneDepth>();
	RCNodeBasePass* gbufferNode = dependencies.Get<RCNodeBasePass>();

	TShared<Texture> sceneDepth = resolvedDepthNode->Output;
	TShared<Texture> sceneNormals = gbufferNode->NormalTex;

	const TextureProperties& normalsProps = sceneNormals->GetProperties();
	if(normalsProps.SampleCount > 1)
	{
		const TShared<Texture> resolvedNormals = inputs.Graph.CreateTexture("SSAOResolvedNormals", TextureCreateInformation::Create2D(normalsProps.Format, normalsProps.Width, normalsProps.Height, TextureUsageFlag::RenderTarget));
		AddResolvePass(inputs.Graph, "SSAONormalsResolve", sceneNormals, resolvedNormals);

		sceneNormals = resolvedNormals;
	}

	// Multiple downsampled AO levels are used to minimize cache trashing. Downsampled AO targets use larger radius,
	// whose contents are then blended with the higher level.
	u32 quality = settings.Quality;
	u32 numDownsampleLevels = 0;
	if(quality == 2)
		numDownsampleLevels = 1;
	else if(quality > 2)
		numDownsampleLevels = 2;

	const Vector2I halfSize(
		std::max(1, Math::DivideAndRoundUp((i32)viewProps.Target.ViewRect.Width, 2)),
		std::max(1, Math::DivideAndRoundUp((i32)viewProps.Target.ViewRect.Height, 2)));

	const Vector2I quarterSize(
		std::max(1, Math::DivideAndRoundUp((i32)viewProps.Target.ViewRect.Width, 4)),
		std::max(1, Math::DivideAndRoundUp((i32)viewProps.Target.ViewRect.Height, 4)));

	SSAODownsampleMaterial* downsample = SSAODownsampleMaterial::Get();

	TShared<Texture> setupTex0;
	if(numDownsampleLevels > 0)
	{
		setupTex0 = inputs.Graph.CreateTexture("SSAOSetupHalf", TextureCreateInformation::Create2D(PF_RGBA16F, halfSize.X, halfSize.Y, TextureUsageFlag::RenderTarget));
		AddFullScreenPass(inputs.Graph, "SSAODownsample", downsample, setupTex0, FrameGraphAttachmentLoad::DontCare, inputs.View, sceneDepth, sceneNormals, setupTex0, kDepthRange);
	}

	TShared<Texture> setupTex1;
	if(numDownsampleLevels > 1)
	{
		setupTex1 = inputs.Graph.CreateTexture("SSAOSetupQuarter", TextureCreateInformation::Create2D(PF_RGBA16F, quarterSize.X, quarterSize.Y, TextureUsageFlag::RenderTarget));
		AddFullScreenPass(inputs.Graph, "SSAODownsample", downsample, setupTex1, FrameGraphAttachmentLoad::DontCare, inputs.View, sceneDepth, sceneNormals, setupTex1, kDepthRange);
	}

	SSAOTextureInputs textures;
	textures.SceneDepth = sceneDepth;
	textures.SceneNormals = sceneNormals;
	textures.RandomRotations = RendererTextures::ssaoRandomization4x4;

	TShared<Texture> downAOTex1;
	if(numDownsampleLevels > 1)
	{
		textures.AoSetup = setupTex1;

		downAOTex1 = inputs.Graph.CreateTexture("SSAOQuarter", TextureCreateInformation::Create2D(PF_R8, quarterSize.X, quarterSize.Y, TextureUsageFlag::RenderTarget));

		SSAOMaterial* ssaoMat = SSAOMaterial::GetVariation(false, false, quality);
		AddFullScreenPass(inputs.Graph, "SSAO", ssaoMat, downAOTex1, FrameGraphAttachmentLoad::DontCare, inputs.View, textures, downAOTex1, settings);
	}

	TShared<Texture> downAOTex0;
	if(numDownsampleLevels > 0)
	{
		textures.AoSetup = setupTex0;

		if(downAOTex1)
			textures.AoDownsampled = downAOTex1;

		downAOTex0 = inputs.Graph.CreateTexture("SSAOHalf", TextureCreateInformation::Create2D(PF_R8, halfSize.X, halfSize.Y, TextureUsageFlag::RenderTarget));

		bool upsample = numDownsampleLevels > 1;
		SSAOMaterial* ssaoMat = SSAOMaterial::GetVariation(upsample, false, quality);
		AddFullScreenPass(inputs.Graph, "SSAO", ssaoMat, downAOTex0, FrameGraphAttachmentLoad::DontCare, inputs.View, textures, downAOTex0, settings);
	}

	u32 width = viewProps.Target.ViewRect.Width;
	u32 height = viewProps.Target.ViewRect.Height;
	TShared<Texture> ambientOcclusion = inputs.Graph.CreateTexture("SSAO", TextureCreateInformation::Create2D(PF_R8, width, height, TextureUsageFlag::RenderTarget));

	{
		if(setupTex0)
			textures.AoSetup = setupTex0;

		if(downAOTex0)
			textures.AoDownsampled = downAOTex0;

		bool upsample = numDownsampleLevels > 0;
		SSAOMaterial* ssaoMat = SSAOMaterial::GetVariation(upsample, true, quality);
		AddFullScreenPass(inputs.Graph, "SSAO", ssaoMat, ambientOcclusion, FrameGraphAttachmentLoad::DontCare, inputs.View, textures, ambientOcclusion, settings);
	}

	// Blur the output
	// Note: If I implement temporal AA then this can probably be avoided. I can instead jitter the sample offsets
	// each frame, and averaging them out should yield blurred AO.
	if(quality > 1) // On level 0 we don't blur at all, on level 1 we use the ad-hoc blur in shader
	{
		const TextureCreateInformation blurInformation = TextureCreateInformation::Create2D(PF_R8, width, height, TextureUsageFlag::RenderTarget);
		const TShared<Texture> blurIntermediateTex = inputs.Graph.CreateTexture("SSAOBlurIntermediate", blurInformation);
		const TShared<Texture> blurredTex = inputs.Graph.CreateTexture("SSAOBlurred", blurInformation);

		SSAOBlurMaterial* blurHorz = SSAOBlurMaterial::GetVariation(true);
		SSAOBlurMaterial* blurVert = SSAOBlurMaterial::GetVariation(false);

		AddFullScreenPass(inputs.Graph, "SSAOBlurH", blurHorz, blurIntermediateTex, FrameGraphAttachmentLoad::DontCare, inputs.View, ambientOcclusion, sceneDepth, kDepthRange);
		AddFullScreenPass(inputs.Graph, "SSAOBlurV", blurVert, blurredTex, FrameGraphAttachmentLoad::DontCare, inputs.View, blurIntermediateTex, sceneDepth, kDepthRange);

		ambientOcclusion = blurredTex;
	}

	Output = ambientOcclusion;
}

void RCNodeSSAO::Clear()
{
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

		Output = Texture::kBlack;
		return;
	}

	auto dependencies = GetDependencyDefinition().ResolveDependencies(inputs);
	RCNodeSceneDepth* sceneDepthNode = dependencies.Get<RCNodeSceneDepth>();
	RCNodeLightAccumulation* lightAccumNode = dependencies.Get<RCNodeLightAccumulation>();
	RCNodeBasePass* gbufferNode = dependencies.Get<RCNodeBasePass>();
	RCNodeHiZ* hiZNode = dependencies.Get<RCNodeHiZ>();
	RCNodeResolvedSceneDepth* resolvedSceneDepthNode = dependencies.Get<RCNodeResolvedSceneDepth>();

	const RendererViewProperties& viewProps = inputs.View.GetProperties();
	const bool msaa = viewProps.Target.NumSamples > 1;

	u32 width = viewProps.Target.ViewRect.Width;
	u32 height = viewProps.Target.ViewRect.Height;

	const TShared<Texture>& hiZ = hiZNode->Output;

	// This will be executing before scene color is resolved, so get the light accum buffer instead
	TShared<Texture> sceneColor = lightAccumNode->LightAccumulationTex;

	// Resolve multiple samples if MSAA is used
	if(msaa)
	{
		const TShared<Texture> resolvedSceneColor = inputs.Graph.CreateTexture("SSRResolvedSceneColor", TextureCreateInformation::Create2D(PF_RGBA16F, width, height, TextureUsageFlag::RenderTarget));
		AddResolvePass(inputs.Graph, "SSRResolve", sceneColor, resolvedSceneColor);

		sceneColor = resolvedSceneColor;
	}

	GBufferTextures gbuffer;
	gbuffer.Albedo = gbufferNode->AlbedoTex;
	gbuffer.Normals = gbufferNode->NormalTex;
	gbuffer.RoughMetal = gbufferNode->RoughMetalTex;
	gbuffer.Depth = sceneDepthNode->DepthTex;

	const TShared<Texture>& resolvedSceneDepth = resolvedSceneDepthNode->Output;

	// Mark the pixels that need reflections in the stencil
	// Note: Making the assumption that the stencil buffer is clear at this point
	SSRStencilMaterial* stencilMat = SSRStencilMaterial::GetVariation(msaa, true);

	FrameGraphRasterPass stencilPass;
	stencilPass.SetDepthStencilAttachment(resolvedSceneDepth, FrameGraphDepthStencilMode::DepthReadOnly, FrameGraphAttachmentLoad::Load);

	const TShared<GpuParameterSet> stencilSet = stencilMat->Prepare(stencilPass, inputs.View, gbuffer, settings);
	inputs.Graph.AddRasterPass("SSRStencil", std::move(stencilPass), [stencilMat, stencilSet, &view = inputs.View](FrameGraphPassContext& context)
	{
		context.GetCommandBuffer().SetViewport(Area2(0.0f, 0.0f, 1.0f, 1.0f));
		stencilMat->Execute(context, stencilSet, view);
	});

	// Trace the marked pixels
	const TShared<Texture> traceOutput = inputs.Graph.CreateTexture("SSRTrace", TextureCreateInformation::Create2D(PF_RGBA16F, width, height, TextureUsageFlag::RenderTarget));
	SSRTraceMaterial* traceMat = SSRTraceMaterial::GetVariation(settings.Quality, msaa, true);

	FrameGraphRasterPass tracePass;
	tracePass.SetColorAttachment(0, traceOutput, FrameGraphAttachmentLoad::Clear);
	tracePass.SetDepthStencilAttachment(resolvedSceneDepth, FrameGraphDepthStencilMode::ReadOnly, FrameGraphAttachmentLoad::Load);

	const TShared<GpuParameterSet> traceSet = traceMat->Prepare(tracePass, inputs.View, gbuffer, sceneColor, hiZ, settings);
	inputs.Graph.AddRasterPass("SSRTrace", std::move(tracePass), [traceMat, traceSet, &view = inputs.View](FrameGraphPassContext& context)
	{
		context.GetCommandBuffer().SetViewport(Area2(0.0f, 0.0f, 1.0f, 1.0f));
		traceMat->Execute(context, traceSet, view);
	});

	// Without temporal AA, the reflections are filtered over frames here
	mUsingTemporalAA = inputs.View.GetRenderSettings().TemporalAa.Enabled;

	TShared<Texture> output;
	if(mPreviousFrame && !mUsingTemporalAA)
	{
		output = inputs.Graph.CreateTexture("SSROutput", TextureCreateInformation::Create2D(PF_RGBA16F, width, height, TextureUsageFlag::RenderTarget));

		TemporalFilteringMaterial* temporalFilteringMat = TemporalFilteringMaterial::GetVariation(TemporalFilteringType::SSR, false, msaa);

		FrameGraphRasterPass filterPass;
		filterPass.SetColorAttachment(0, output, FrameGraphAttachmentLoad::Clear);

		const TShared<GpuParameterSet> filterSet = temporalFilteringMat->Prepare(filterPass, inputs.View, mPreviousFrame, traceOutput, nullptr, sceneDepthNode->DepthTex, Vector2::kZero, 1.0f);
		inputs.Graph.AddRasterPass("SSRTemporalFilter", std::move(filterPass), [temporalFilteringMat, filterSet, &view = inputs.View](FrameGraphPassContext& context)
		{
			context.GetCommandBuffer().SetViewport(Area2(0.0f, 0.0f, 1.0f, 1.0f));
			temporalFilteringMat->Execute(context, filterSet, view);
		});
	}
	else
		output = traceOutput;

	// Kept for the next frame's filter, which reads it at rest
	if(!mUsingTemporalAA)
	{
		inputs.Graph.ExtractTexture(output);
		mHistoryOutput = output;
	}

	Output = output;
}

void RCNodeSSR::Clear()
{
	mPreviousFrame = mHistoryOutput;
	mHistoryOutput = nullptr;
	Output = nullptr;
}

void RCNodeSSR::DeallocOutputs()
{
	mPreviousFrame = nullptr;
	mHistoryOutput = nullptr;
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

	const bool autoExposure = settings.EnableHdr && settings.EnableAutoExposure;
	BloomClipMaterial* clipMat = BloomClipMaterial::GetVariation(autoExposure);

	TShared<Texture> eyeAdaptationTex = nullptr;
	if(autoExposure)
	{
		auto* eyeAdapatationNode = dependencies.Get<RCNodeEyeAdaptation>();
		eyeAdaptationTex = eyeAdapatationNode->Output;
	}

	const u32 numSteps = NUM_STEPS_PER_QUALITY[quality];
	TShared<Texture> prevOutput;
	for(u32 i = 0; i < numSteps; i++)
	{
		const u32 srcIdx = numDownsamples - i - 1;
		const TShared<Texture>& downsampledTex = sceneDownsamplesNode->Output[srcIdx];

		const TextureProperties& inputProps = downsampledTex->GetProperties();

		const TShared<Texture> filterOutput = inputs.Graph.CreateTexture("BloomFilterOutput", TextureCreateInformation::Create2D(inputProps.Format, inputProps.Width, inputProps.Height, TextureUsageFlag::RenderTarget));

		TShared<Texture> blurInput = downsampledTex;
		TShared<Texture> blurOutput = filterOutput;
		if(settings.Bloom.Threshold > 0.0f)
		{
			AddFullScreenPass(inputs.Graph, "BloomClip", clipMat, filterOutput, FrameGraphAttachmentLoad::DontCare, downsampledTex, settings.Bloom.Threshold, eyeAdaptationTex, settings);

			blurOutput = blurInput;
			blurInput = filterOutput;
		}

		const Color tint = Color::kWhite * (settings.Bloom.Intensity / (float)numSteps);
		AddGaussianBlurPasses(inputs.Graph, blurInput, settings.Bloom.FilterSize, blurOutput, tint, prevOutput);
		prevOutput = blurOutput;
	}

	Output = prevOutput;
}

void RCNodeBloom::Clear()
{
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
	const RenderSettings& settings = inputs.View.GetRenderSettings();
	const ScreenSpaceLensFlareSettings& lensFlareSettings = settings.ScreenSpaceLensFlare;

	// Grab downsampled scene color to use as input
	auto* sceneDownsamplesNode = dependencies.Get<RCNodeSceneColorDownsamples>();

	const u32 availableDownsamples = sceneDownsamplesNode->AvailableDownsamples;
	const u32 numDownsamples = Math::Clamp(settings.ScreenSpaceLensFlare.DownsampleCount, 1U, availableDownsamples);
	B3D_ASSERT(numDownsamples >= 1);

	const TShared<Texture>& downsampledTex = sceneDownsamplesNode->Output[numDownsamples - 1];
	const TextureProperties& sceneTexProps = downsampledTex->GetProperties();

	// Ghost features
	const TShared<Texture> featureTex = inputs.Graph.CreateTexture("LensFlareFeatures", TextureCreateInformation::Create2D(sceneTexProps.Format, sceneTexProps.Width, sceneTexProps.Height, TextureUsageFlag::RenderTarget));

	bool haloAspect = lensFlareSettings.HaloAspectRatio != 1.0f;
	ScreenSpaceLensFlareMaterial* lensFlareMat = ScreenSpaceLensFlareMaterial::GetVariation(
		lensFlareSettings.Halo,
		haloAspect,
		lensFlareSettings.ChromaticAberration);
	AddFullScreenPass(inputs.Graph, "ScreenSpaceLensFlare", lensFlareMat, featureTex, FrameGraphAttachmentLoad::DontCare, downsampledTex, lensFlareSettings);

	// Blur
	AddGaussianBlurPasses(inputs.Graph, featureTex, lensFlareSettings.FilterSize, downsampledTex);

	// Add to scene color
	auto* sceneColorNode = dependencies.Get<RCNodeSceneColor>();
	const Color tint = Color::kWhite * lensFlareSettings.Brightness;

	if(lensFlareSettings.BicubicUpsampling)
	{
		BicubicUpsampleMaterial* upsampleMat = BicubicUpsampleMaterial::GetVariation(false);
		AddFullScreenPass(inputs.Graph, "LensFlareUpsample", upsampleMat, sceneColorNode->SceneColorTex, FrameGraphAttachmentLoad::Load, downsampledTex, tint);
	}
	else
	{
		CompositeMaterial* upsampleMat = CompositeMaterial::Get();
		AddFullScreenPass(inputs.Graph, "LensFlareUpsample", upsampleMat, sceneColorNode->SceneColorTex, FrameGraphAttachmentLoad::Load, downsampledTex, tint);
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
