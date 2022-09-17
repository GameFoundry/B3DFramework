//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsRenderCompositor.h"
#include "Renderer/BsRendererExtension.h"
#include "Renderer/BsSkybox.h"
#include "Renderer/BsCamera.h"
#include "Renderer/BsRendererUtility.h"
#include "RenderAPI/BsGpuBuffer.h"
#include "Utility/BsBitwise.h"
#include "Mesh/BsMesh.h"
#include "Material/BsGpuParamsSet.h"
#include "Renderer/BsGpuResourcePool.h"
#include "Utility/BsRendererTextures.h"
#include "Shading/BsStandardDeferred.h"
#include "Shading/BsTiledDeferred.h"
#include "Shading/BsLightProbes.h"
#include "Shading/BsPostProcessing.h"
#include "Shading/BsShadowRendering.h"
#include "Shading/BsLightGrid.h"
#include "BsRendererView.h"
#include "BsRenderBeastOptions.h"
#include "BsRendererScene.h"
#include "BsRenderBeast.h"
#include "Particles/BsParticleManager.h"
#include "Particles/BsParticleSystem.h"
#include "Threading/BsTaskScheduler.h"
#include "Profiling/BsProfilerGPU.h"
#include "Shading/BsGpuParticleSimulation.h"
#include "Profiling/BsProfilerCPU.h"

namespace bs { namespace ct
{
	UnorderedMap<StringID, RenderCompositor::NodeType*> RenderCompositor::mNodeTypes;

	/** Renders all elements in a render queue. */
	void renderQueueElements(const Vector<RenderQueueElement>& elements)
	{
		for(auto& entry : elements)
		{
			if (entry.applyPass)
				gRendererUtility().SetPass(entry.renderElem->material, entry.passIdx, entry.techniqueIdx);

			gRendererUtility().SetPassParams(entry.renderElem->params, entry.passIdx);

			entry.renderElem->Draw();
		}
	}

	RenderCompositor::~RenderCompositor()
	{
		Clear();
	}

	void RenderCompositor::Build(const RendererView& view, const StringID& finalNode)
	{
		Clear();

		bs_frame_mark();
		{
			FrameUnorderedMap<StringID, UINT32> processedNodes;
			mIsValid = true;

			std::function<bool(const StringID&)> registerNode = [&](const StringID& nodeId)
			{
				// Find node type
				auto iterFind = mNodeTypes.find(nodeId);
				if (iterFind == mNodeTypes.end())
				{
					BS_LOG(Error, Renderer, "Cannot find render compositor node of type \"{0}\".", String(nodeId.CStr()));
					return false;
				}

				NodeType* nodeType = iterFind->second;

				// Register current node
				auto iterFind2 = processedNodes.find(nodeId);

				// New node
				if (iterFind2 == processedNodes.end())
				{
					// Mark it as invalid for now
					processedNodes[nodeId] = -1;
				}

				// Register node dependencies
				SmallVector<StringID, 4> depIds = nodeType->GetDependencies(view);
				for (auto& dep : depIds)
				{
					if (!registerNode(dep))
						return false;
				}

				// Register current node
				UINT32 curIdx;

				// New node, properly populate its index
				if (iterFind2 == processedNodes.end())
				{
					iterFind2 = processedNodes.find(nodeId);

					curIdx = (UINT32)mNodeInfos.size();
					mNodeInfos.push_back(NodeInfo());
					processedNodes[nodeId] = curIdx;

					NodeInfo& nodeInfo = mNodeInfos.back();
					nodeInfo.node = nodeType->Create();
					nodeInfo.nodeType = nodeType;
					nodeInfo.lastUseIdx = -1;

					for (auto& depId : depIds)
					{
						iterFind2 = processedNodes.find(depId);

						NodeInfo& depNodeInfo = mNodeInfos[iterFind2->second];
						nodeInfo.inputs.Add(depNodeInfo.node);
					}
				}
				else // Existing node
				{
					curIdx = iterFind2->second;

					// Check if invalid
					if (curIdx == (UINT32)-1)
					{
						BS_LOG(Error, Renderer, "Render compositor nodes recursion detected. Node \"{0}\" "
							"depends on node \"{1}\" which is not available at this stage.",
							String(nodeId.CStr()), String(iterFind->first.CStr()));
						return false;
					}
				}

				// Update dependency last use counters
				for (auto& dep : depIds)
				{
					iterFind2 = processedNodes.find(dep);

					NodeInfo& depNodeInfo = mNodeInfos[iterFind2->second];
					if (depNodeInfo.lastUseIdx == (UINT32)-1)
						depNodeInfo.lastUseIdx = curIdx;
					else
						depNodeInfo.lastUseIdx = std::max(depNodeInfo.lastUseIdx, curIdx);
				}

				return true;
			};

			mIsValid = registerNode(finalNode);

			if (!mIsValid)
				Clear();
		}
		bs_frame_clear();
	}

	void RenderCompositor::Execute(RenderCompositorNodeInputs& inputs) const
	{
		if (!mIsValid)
			return;

		bs_frame_mark();
		{
			FrameVector<const NodeInfo*> activeNodes;

			UINT32 idx = 0;
			for (auto& entry : mNodeInfos)
			{
				inputs.inputNodes = entry.inputs;

#if BS_PROFILING_ENABLED
				const ProfilerString sampleName = ProfilerString("RC: ") + entry.nodeType->id.CStr();
				BS_GPU_PROFILE_BEGIN(sampleName);
				gProfilerCPU().BeginSample(sampleName.c_str());
#endif

				entry.node->Render(inputs);

#if BS_PROFILING_ENABLED
				gProfilerCPU().EndSample(sampleName.c_str());
				BS_GPU_PROFILE_END(sampleName);
#endif

				activeNodes.push_back(&entry);

				for (UINT32 i = 0; i < (UINT32)activeNodes.size(); ++i)
				{
					if (activeNodes[i] == nullptr)
						continue;

					if (activeNodes[i]->lastUseIdx <= idx)
					{
						activeNodes[i]->node->Clear();
						activeNodes[i] = nullptr;
					}
				}

				idx++;
			}
		}
		bs_frame_clear();

		if (!mNodeInfos.empty())
			mNodeInfos.back().node->Clear();
	}

	void RenderCompositor::Clear()
	{
		for (auto& entry : mNodeInfos)
			bs_delete(entry.node);

		mNodeInfos.clear();
		mIsValid = false;
	}

	void RCNodeSceneDepth::Render(const RenderCompositorNodeInputs& inputs)
	{
		const RendererViewProperties& viewProps = inputs.view.GetProperties();

		UINT32 width = viewProps.target.viewRect.width;
		UINT32 height = viewProps.target.viewRect.height;
		UINT32 numSamples = viewProps.target.numSamples;

		depthTex = gGpuResourcePool().Get(POOLED_RENDER_TEXTURE_DESC::Create2D(PF_D32_S8X24, width, height, TU_DEPTHSTENCIL,
			numSamples, false));
	}

	void RCNodeSceneDepth::Clear()
	{
		depthTex = nullptr;
	}

	SmallVector<StringID, 4> RCNodeSceneDepth::GetDependencies(const RendererView& view)
	{
		return {};
	}

	void RCNodeBasePass::Render(const RenderCompositorNodeInputs& inputs)
	{
		// Allocate necessary textures & targets
		GpuResourcePool& resPool = gGpuResourcePool();
		const RendererViewProperties& viewProps = inputs.view.GetProperties();

		const UINT32 width = viewProps.target.viewRect.width;
		const UINT32 height = viewProps.target.viewRect.height;
		const UINT32 numSamples = viewProps.target.numSamples;

		bool needsVelocity = inputs.view.RequiresVelocityWrites();

		// Note: Consider customizable formats. e.g. for testing if quality can be improved with higher precision normals.
		albedoTex = resPool.Get(POOLED_RENDER_TEXTURE_DESC::Create2D(PF_RGBA8, width, height, TU_RENDERTARGET,
			numSamples, true));
		normalTex = resPool.Get(POOLED_RENDER_TEXTURE_DESC::Create2D(PF_RGB10A2, width, height, TU_RENDERTARGET,
			numSamples, false));
		roughMetalTex = resPool.Get(POOLED_RENDER_TEXTURE_DESC::Create2D(PF_RG16F, width, height, TU_RENDERTARGET,
			numSamples, false)); // Note: Metal doesn't need 16-bit float
		idTex = resPool.Get(POOLED_RENDER_TEXTURE_DESC::Create2D(PF_R8, width, height, TU_RENDERTARGET,
			numSamples, false));

		if(needsVelocity)
		{
			velocityTex = resPool.Get(POOLED_RENDER_TEXTURE_DESC::Create2D(PF_RG16S, width, height, TU_RENDERTARGET,
				numSamples, false));
		}

		auto sceneDepthNode = static_cast<RCNodeSceneDepth*>(inputs.inputNodes[0]);
		auto sceneColorNode = static_cast<RCNodeSceneColor*>(inputs.inputNodes[1]);
		SPtr<PooledRenderTexture> sceneDepthTex = sceneDepthNode->depthTex;
		SPtr<PooledRenderTexture> sceneColorTex = sceneColorNode->sceneColorTex;

		bool rebuildRT = false;
		if (renderTarget != nullptr)
		{
			UINT32 targetIdx = 0;
			rebuildRT |= renderTarget->GetColorTexture(targetIdx++) != sceneColorTex->texture;
			rebuildRT |= renderTarget->GetColorTexture(targetIdx++) != albedoTex->texture;
			rebuildRT |= renderTarget->GetColorTexture(targetIdx++) != normalTex->texture;
			rebuildRT |= renderTarget->GetColorTexture(targetIdx++) != roughMetalTex->texture;
			if (needsVelocity) rebuildRT |= renderTarget->GetColorTexture(targetIdx++) != velocityTex->texture;
			rebuildRT |= renderTarget->GetColorTexture(targetIdx++) != idTex->texture;
			rebuildRT |= renderTarget->GetDepthStencilTexture() != sceneDepthTex->texture;
		}
		else
			rebuildRT = true;

		if (renderTarget == nullptr || rebuildRT)
		{
			UINT32 targetIdx = 0;
			
			RENDER_TEXTURE_DESC gbufferDesc;
			gbufferDesc.colorSurfaces[targetIdx].texture = sceneColorTex->texture;
			gbufferDesc.colorSurfaces[targetIdx].face = 0;
			gbufferDesc.colorSurfaces[targetIdx].numFaces = 1;
			gbufferDesc.colorSurfaces[targetIdx].mipLevel = 0;
			targetIdx++;

			gbufferDesc.colorSurfaces[targetIdx].texture = albedoTex->texture;
			gbufferDesc.colorSurfaces[targetIdx].face = 0;
			gbufferDesc.colorSurfaces[targetIdx].numFaces = 1;
			gbufferDesc.colorSurfaces[targetIdx].mipLevel = 0;
			targetIdx++;

			gbufferDesc.colorSurfaces[targetIdx].texture = normalTex->texture;
			gbufferDesc.colorSurfaces[targetIdx].face = 0;
			gbufferDesc.colorSurfaces[targetIdx].numFaces = 1;
			gbufferDesc.colorSurfaces[targetIdx].mipLevel = 0;
			targetIdx++;

			gbufferDesc.colorSurfaces[targetIdx].texture = roughMetalTex->texture;
			gbufferDesc.colorSurfaces[targetIdx].face = 0;
			gbufferDesc.colorSurfaces[targetIdx].numFaces = 1;
			gbufferDesc.colorSurfaces[targetIdx].mipLevel = 0;
			targetIdx++;

			if (needsVelocity)
			{
				gbufferDesc.colorSurfaces[targetIdx].texture = velocityTex->texture;
				gbufferDesc.colorSurfaces[targetIdx].face = 0;
				gbufferDesc.colorSurfaces[targetIdx].numFaces = 1;
				gbufferDesc.colorSurfaces[targetIdx].mipLevel = 0;
				targetIdx++;
			}

			gbufferDesc.depthStencilSurface.texture = sceneDepthTex->texture;
			gbufferDesc.depthStencilSurface.face = 0;
			gbufferDesc.depthStencilSurface.mipLevel = 0;

			renderTargetNoMask = RenderTexture::Create(gbufferDesc);

			gbufferDesc.colorSurfaces[targetIdx].texture = idTex->texture;
			gbufferDesc.colorSurfaces[targetIdx].face = 0;
			gbufferDesc.colorSurfaces[targetIdx].numFaces = 1;
			gbufferDesc.colorSurfaces[targetIdx].mipLevel = 0;
			targetIdx++;

			renderTarget = RenderTexture::Create(gbufferDesc);
		}

		// Prepare all visible objects. Note that this also prepares non-opaque objects.
		//// Prepare normal renderables
		const VisibilityInfo& visibility = inputs.view.GetVisibilityMasks();
		const auto numRenderables = (UINT32)inputs.scene.renderables.size();
		for (UINT32 i = 0; i < numRenderables; i++)
		{
			if (!visibility.renderables[i])
				continue;

			RendererRenderable* rendererRenderable = inputs.scene.renderables[i];
			rendererRenderable->UpdatePerCallBuffer(viewProps.viewProjTransform);

			for (auto& element : inputs.scene.renderables[i]->elements)
			{
				SPtr<GpuParams> gpuParams = element.params->GetGpuParams();
				for(UINT32 j = 0; j < GPT_COUNT; j++)
				{
					const GpuParamBinding& binding = element.perCameraBindings[j];
					if(binding.slot != (UINT32)-1)
						gpuParams->SetParamBlockBuffer(binding.set, binding.slot, inputs.view.GetPerViewBuffer());
				}
			}
		}

		//// Prepare particle systems
		const ParticlePerFrameData* particleData = inputs.frameInfo.perFrameData.particles;
		if(particleData)
		{
			const auto numParticleSystems = (UINT32)inputs.scene.particleSystems.size();

			const GpuParticleResources& gpuSimResources = GpuParticleSimulation::Instance().GetResources();
			for (UINT32 i = 0; i < numParticleSystems; i++)
			{
				if (!visibility.particleSystems[i])
					continue;

				const RendererParticles& rendererParticles = inputs.scene.particleSystems[i];
				ParticlesRenderElement& renderElement = rendererParticles.renderElement;

				if(!renderElement.IsValid())
					continue;

				ParticleSystem* particleSystem = rendererParticles.particleSystem;

				// Bind textures/buffers from CPU simulation
				const auto iterFind = particleData->cpuData.find(particleSystem->GetId());
				if (iterFind != particleData->cpuData.end())
				{
					ParticleRenderData* renderData = iterFind->second;
					rendererParticles.BindCpuSimulatedInputs(renderData, inputs.view);
				}
				// Bind textures/buffers from GPU simulation
				else if(rendererParticles.gpuParticleSystem)
					rendererParticles.BindGpuSimulatedInputs(gpuSimResources, inputs.view);
			}
		}

		//// Prepare decals
		const auto numDecals = (UINT32)inputs.scene.decals.size();
		for (UINT32 i = 0; i < numDecals; i++)
		{
			if (!visibility.decals[i])
				continue;

			const RendererDecal& rendererDecal = inputs.scene.decals[i];
			DecalRenderElement& renderElement = rendererDecal.renderElement;

			rendererDecal.UpdatePerCallBuffer(viewProps.viewProjTransform);

			SPtr<GpuParams> gpuParams = renderElement.params->GetGpuParams();
			for (UINT32 j = 0; j < GPT_COUNT; j++)
			{
				const GpuParamBinding& binding = renderElement.perCameraBindings[j];
				if (binding.slot != (UINT32)-1)
					gpuParams->SetParamBlockBuffer(binding.set, binding.slot, inputs.view.GetPerViewBuffer());
			}

			renderElement.depthInputTexture.Set(sceneDepthTex->texture);
			renderElement.maskInputTexture.Set(idTex->texture);
		}

		Camera* sceneCamera = inputs.view.GetSceneCamera();

		// Trigger prepare callbacks
		if (sceneCamera != nullptr)
		{
			for(auto& extension : inputs.extPrepare)
				extension->Render(*sceneCamera, inputs.view.GetContext());
		}

		// Render base pass
		RenderAPI& rapi = RenderAPI::Instance();
		rapi.SetRenderTarget(renderTarget);

		Rect2 area(0.0f, 0.0f, 1.0f, 1.0f);
		rapi.SetViewport(area);

		// Clear all targets
		rapi.ClearViewport(FBT_COLOR | FBT_DEPTH | FBT_STENCIL, Color::ZERO, 1.0f, 0);

		// Trigger pre-base-pass callbacks
		if (sceneCamera != nullptr)
		{
			inputs.view.NotifyCompositorTargetChangedInternal(renderTarget);

			for(auto& extension : inputs.extPreBasePass)
				extension->Render(*sceneCamera, inputs.view.GetContext());
		}

		// Render all visible opaque elements that use the deferred pipeline
		const Vector<RenderQueueElement>& opaqueElements = inputs.view.GetOpaqueQueue(false)->GetSortedElements();
		renderQueueElements(opaqueElements);

		// Determine MSAA coverage if required
		if (viewProps.target.numSamples > 1)
		{
			auto msaaCoverageNode = static_cast<RCNodeMSAACoverage*>(inputs.inputNodes[3]);

			GBufferTextures gbuffer;
			gbuffer.albedo = albedoTex->texture;
			gbuffer.normals = normalTex->texture;
			gbuffer.roughMetal = roughMetalTex->texture;
			gbuffer.depth = sceneDepthNode->depthTex->texture;

			MSAACoverageMat* mat = MSAACoverageMat::GetVariation(viewProps.target.numSamples);
			rapi.SetRenderTarget(msaaCoverageNode->output->renderTexture);
			mat->Execute(inputs.view, gbuffer);

			MSAACoverageStencilMat* stencilMat = MSAACoverageStencilMat::Get();
			rapi.SetRenderTarget(sceneDepthNode->depthTex->renderTexture);
			stencilMat->Execute(inputs.view, msaaCoverageNode->output->texture);
		}

		// Render decals after all normal objects, using a read-only depth buffer
		rapi.SetRenderTarget(renderTargetNoMask, FBT_DEPTH, RT_ALL);

		const Vector<RenderQueueElement>& decalElements = inputs.view.GetDecalQueue()->GetSortedElements();
		renderQueueElements(decalElements);

		// Trigger post-base-pass callbacks
		if (sceneCamera != nullptr)
		{
			inputs.view.NotifyCompositorTargetChangedInternal(renderTargetNoMask);

			for(auto& extension : inputs.extPostBasePass)
				extension->Render(*sceneCamera, inputs.view.GetContext());
		}

		// Make sure that any compute shaders are able to read g-buffer by unbinding it
		rapi.SetRenderTarget(nullptr);
	}

	void RCNodeBasePass::Clear()
	{
		albedoTex = nullptr;
		normalTex = nullptr;
		roughMetalTex = nullptr;
		idTex = nullptr;
	}

	SmallVector<StringID, 4> RCNodeBasePass::GetDependencies(const RendererView& view)
	{
		return {
			RCNodeSceneDepth::GetNodeId(), RCNodeSceneColor::GetNodeId(), RCNodeParticleSort::GetNodeId(),
			RCNodeMSAACoverage::GetNodeId() };
	}

	void RCNodeSceneColor::Render(const RenderCompositorNodeInputs& inputs)
	{
		GpuResourcePool& resPool = gGpuResourcePool();
		const RendererViewProperties& viewProps = inputs.view.GetProperties();

		UINT32 width = viewProps.target.viewRect.width;
		UINT32 height = viewProps.target.viewRect.height;
		UINT32 numSamples = viewProps.target.numSamples;

		UINT32 usageFlags = TU_RENDERTARGET;

		bool tiledDeferredSupported = inputs.featureSet != RenderBeastFeatureSet::DesktopMacOS;
		if(tiledDeferredSupported && numSamples == 1)
			usageFlags |= TU_LOADSTORE;

		// Note: Consider customizable HDR format via options? e.g. smaller PF_FLOAT_R11G11B10 or larger 32-bit format
		sceneColorTex = resPool.Get(POOLED_RENDER_TEXTURE_DESC::Create2D(PF_RGBA16F, width, height, usageFlags,
			numSamples, false));

		RCNodeSceneDepth* sceneDepthNode = static_cast<RCNodeSceneDepth*>(inputs.inputNodes[0]);
		SPtr<PooledRenderTexture> sceneDepthTex = sceneDepthNode->depthTex;

		if (tiledDeferredSupported && viewProps.target.numSamples > 1)
		{
			sceneColorTexArray = resPool.Get(POOLED_RENDER_TEXTURE_DESC::Create2D(PF_RGBA16F, width, height,
				TU_LOADSTORE, 1, false, viewProps.target.numSamples));
		}
		else
			sceneColorTexArray = nullptr;

		bool rebuildRT = false;
		if (renderTarget != nullptr)
		{
			rebuildRT |= renderTarget->GetColorTexture(0) != sceneColorTex->texture;
			rebuildRT |= renderTarget->GetDepthStencilTexture() != sceneDepthTex->texture;
		}
		else
			rebuildRT = true;

		if (rebuildRT)
		{
			RENDER_TEXTURE_DESC sceneColorDesc;
			sceneColorDesc.colorSurfaces[0].texture = sceneColorTex->texture;
			sceneColorDesc.colorSurfaces[0].face = 0;
			sceneColorDesc.colorSurfaces[0].numFaces = 1;
			sceneColorDesc.colorSurfaces[0].mipLevel = 0;

			sceneColorDesc.depthStencilSurface.texture = sceneDepthTex->texture;
			sceneColorDesc.depthStencilSurface.face = 0;
			sceneColorDesc.depthStencilSurface.numFaces = 1;
			sceneColorDesc.depthStencilSurface.mipLevel = 0;

			renderTarget = RenderTexture::Create(sceneColorDesc);
		}
	}

	void RCNodeSceneColor::Clear()
	{
		sceneColorTex = nullptr;
		sceneColorTexArray = nullptr;
	}

	void RCNodeSceneColor::MsaaTexArrayToTexture()
	{
		RenderAPI& rapi = RenderAPI::Instance();
		rapi.SetRenderTarget(renderTarget, FBT_DEPTH | FBT_STENCIL, RT_DEPTH_STENCIL);

		Rect2 area(0.0f, 0.0f, 1.0f, 1.0f);
		rapi.SetViewport(area);

		TextureArrayToMSAATexture* material = TextureArrayToMSAATexture::Get();
		material->Execute(sceneColorTexArray->texture, sceneColorTex->texture);

		sceneColorTexArray = nullptr;
	}

	void RCNodeSceneColor::Swap(RCNodeLightAccumulation* lightAccumNode)
	{
		lightAccumNode->lightAccumulationTex.swap(sceneColorTex);
		lightAccumNode->lightAccumulationTexArray.swap(sceneColorTexArray);
		lightAccumNode->renderTarget.swap(renderTarget);	
	}

	void RCNodeSceneColor::SetExternalTexture(const SPtr<PooledRenderTexture>& texture)
	{
		assert(sceneColorTexArray == nullptr);
		
		sceneColorTex = texture;
	}

	SmallVector<StringID, 4> RCNodeSceneColor::GetDependencies(const RendererView& view)
	{
		return { RCNodeSceneDepth::GetNodeId() };
	}

	void RCNodeMSAACoverage::Render(const RenderCompositorNodeInputs& inputs)
	{
		const RendererViewProperties& viewProps = inputs.view.GetProperties();
		if(viewProps.target.numSamples <= 1)
		{
			// No need for MSAA coverage
			output = nullptr;
			return;
		}

		UINT32 width = viewProps.target.viewRect.width;
		UINT32 height = viewProps.target.viewRect.height;

		// We just allocate the texture, while the base pass is responsible for filling it out
		output = gGpuResourcePool().Get(POOLED_RENDER_TEXTURE_DESC::Create2D(PF_R8, width, height, TU_RENDERTARGET));
	}

	void RCNodeMSAACoverage::Clear()
	{
		output = nullptr;
	}

	SmallVector<StringID, 4> RCNodeMSAACoverage::GetDependencies(const RendererView& view)
	{
		return { };
	}

	void RCNodeParticleSimulate::Render(const RenderCompositorNodeInputs& inputs)
	{
		// Only simulate particles for the first view in the main render pass
		if(inputs.viewGroup.IsMainPass() && inputs.view.GetViewIdx() == 0)
		{
			RCNodeBasePass* gbufferNode = static_cast<RCNodeBasePass*>(inputs.inputNodes[0]);
			RCNodeSceneDepth* sceneDepthNode = static_cast<RCNodeSceneDepth*>(inputs.inputNodes[1]);

			GBufferTextures gbuffer;
			gbuffer.albedo = gbufferNode->albedoTex->texture;
			gbuffer.normals = gbufferNode->normalTex->texture;
			gbuffer.roughMetal = gbufferNode->roughMetalTex->texture;
			gbuffer.depth = sceneDepthNode->depthTex->texture;

			GpuParticleSimulation::Instance().Simulate(inputs.scene, inputs.frameInfo.perFrameData.particles,
				inputs.view.GetPerViewBuffer(), gbuffer, inputs.frameInfo.timings.timeDelta);
		}

		GpuParticleSimulation::Instance().Sort(inputs.view);
	}

	void RCNodeParticleSimulate::Clear()
	{
		// Do nothing
	}

	SmallVector<StringID, 4> RCNodeParticleSimulate::GetDependencies(const RendererView& view)
	{
		return { RCNodeBasePass::GetNodeId(), RCNodeSceneDepth::GetNodeId() };
	}

	void RCNodeParticleSort::Render(const RenderCompositorNodeInputs& inputs)
	{
		const ParticlePerFrameData* particleData = inputs.frameInfo.perFrameData.particles;
		if(!particleData)
			return;

		const RendererViewProperties& viewProps = inputs.view.GetProperties();
		const VisibilityInfo& visibility = inputs.view.GetVisibilityMasks();
		const auto numParticleSystems = (UINT32)inputs.scene.particleSystems.size();

		// Sort particles
		bs_frame_mark();
		{
			struct SortData
			{
				ParticleSystem* system;
				ParticleRenderData* renderData;
			};

			FrameVector<SortData> systemsToSort;
			for (UINT32 i = 0; i < numParticleSystems; i++)
			{
				if (!visibility.particleSystems[i])
					continue;

				const RendererParticles& rendererParticles = inputs.scene.particleSystems[i];

				ParticleSystem* particleSystem = rendererParticles.particleSystem;
				const auto iterFind = particleData->cpuData.find(particleSystem->GetId());
				if (iterFind == particleData->cpuData.end())
					continue;

				ParticleRenderData* simulationData = iterFind->second;
				if (particleSystem->GetSettings().sortMode == ParticleSortMode::Distance)
					systemsToSort.push_back({ particleSystem, simulationData });
			}

			const auto worker = [&systemsToSort, viewOrigin = viewProps.viewOrigin](UINT32 idx)
			{
				const SortData& data = systemsToSort[idx];

				Vector3 refPoint = viewOrigin;

				// Transform the view point into particle system's local space
				const ParticleSystemSettings& settings = data.system->GetSettings();
				if (settings.simulationSpace == ParticleSimulationSpace::Local)
					refPoint = data.system->GetTransform().GetInvMatrix().MultiplyAffine(refPoint);

				if (settings.renderMode == ParticleRenderMode::Billboard)
				{
					auto renderData = static_cast<ParticleBillboardRenderData*>(data.renderData);
					ParticleRenderer::SortByDistance(refPoint, renderData->positionAndRotation,
						renderData->numParticles, 4, renderData->indices);
				}
				else
				{
					auto renderData = static_cast<ParticleMeshRenderData*>(data.renderData);
					ParticleRenderer::SortByDistance(refPoint, renderData->position, renderData->numParticles,
						3, renderData->indices);
				}
			};

			SPtr<TaskGroup> sortTask = TaskGroup::Create("ParticleSort", worker, (UINT32)systemsToSort.size());

			TaskScheduler::Instance().AddTaskGroup(sortTask);
			sortTask->Wait();
		}
		bs_frame_clear();
	}

	void RCNodeParticleSort::Clear()
	{
		// Do nothing
	}

	SmallVector<StringID, 4> RCNodeParticleSort::GetDependencies(const RendererView& view)
	{
		return { };
	}

	void RCNodeLightAccumulation::Render(const RenderCompositorNodeInputs& inputs)
	{
		bool supportsTiledDeferred = gRenderBeast()->GetFeatureSet() != RenderBeastFeatureSet::DesktopMacOS;
		if(!supportsTiledDeferred)
		{
			// If tiled deferred is not supported, we don't need a separate texture for light accumulation, instead we
			// use scene color directly
			RCNodeSceneColor* sceneColorNode = static_cast<RCNodeSceneColor*>(inputs.inputNodes[0]);
			lightAccumulationTex = sceneColorNode->sceneColorTex;
			renderTarget = sceneColorNode->renderTarget;

			return;
		}

		GpuResourcePool& resPool = gGpuResourcePool();
		const RendererViewProperties& viewProps = inputs.view.GetProperties();

		RCNodeSceneDepth* depthNode = static_cast<RCNodeSceneDepth*>(inputs.inputNodes[0]);

		UINT32 width = viewProps.target.viewRect.width;
		UINT32 height = viewProps.target.viewRect.height;
		UINT32 numSamples = viewProps.target.numSamples;

		UINT32 usage = TU_RENDERTARGET;
		if (numSamples > 1)
		{
			resPool.Get(lightAccumulationTexArray,
				POOLED_RENDER_TEXTURE_DESC::Create2D(PF_RGBA16F, width, height, TU_LOADSTORE, 1, false, numSamples));

			ClearLoadStoreMat* clearMat = ClearLoadStoreMat::GetVariation(ClearLoadStoreType::TextureArray,
					ClearLoadStoreDataType::Float, 4);

			for(UINT32 i = 0; i < numSamples; i++)
			{
				TextureSurface surface;
				surface.face = i;
				surface.numFaces = 1;
				surface.mipLevel = 0;
				surface.numMipLevels = 1;

				clearMat->Execute(lightAccumulationTexArray->texture, Color::ZERO, surface);
			}
		}
		else
		{
			usage |= TU_LOADSTORE;
			lightAccumulationTexArray = nullptr;
		}

		resPool.Get(lightAccumulationTex,
			POOLED_RENDER_TEXTURE_DESC::Create2D(PF_RGBA16F, width, height, usage, numSamples, false));

		bool rebuildRT;
		if (renderTarget != nullptr)
		{
			rebuildRT = renderTarget->GetColorTexture(0) != lightAccumulationTex->texture;
			rebuildRT |= renderTarget->GetDepthStencilTexture() != depthNode->depthTex->texture;
		}
		else
			rebuildRT = true;

		if (rebuildRT)
		{
			RENDER_TEXTURE_DESC lightAccumulationRTDesc;
			lightAccumulationRTDesc.colorSurfaces[0].texture = lightAccumulationTex->texture;
			lightAccumulationRTDesc.colorSurfaces[0].face = 0;
			lightAccumulationRTDesc.colorSurfaces[0].numFaces = 1;
			lightAccumulationRTDesc.colorSurfaces[0].mipLevel = 0;

			lightAccumulationRTDesc.depthStencilSurface.texture = depthNode->depthTex->texture;
			lightAccumulationRTDesc.depthStencilSurface.face = 0;
			lightAccumulationRTDesc.depthStencilSurface.numFaces = 1;
			lightAccumulationRTDesc.depthStencilSurface.mipLevel = 0;

			renderTarget = RenderTexture::Create(lightAccumulationRTDesc);
		}
	}

	void RCNodeLightAccumulation::MsaaTexArrayToTexture()
	{
		RenderAPI& rapi = RenderAPI::Instance();
		rapi.SetRenderTarget(renderTarget, FBT_DEPTH | FBT_STENCIL, RT_DEPTH_STENCIL);

		TextureArrayToMSAATexture* material = TextureArrayToMSAATexture::Get();
		material->Execute(lightAccumulationTexArray->texture, lightAccumulationTex->texture);
	}

	void RCNodeLightAccumulation::Clear()
	{
		renderTarget = nullptr;
		lightAccumulationTex = nullptr;
		lightAccumulationTexArray = nullptr;
	}

	SmallVector<StringID, 4> RCNodeLightAccumulation::GetDependencies(const RendererView& view)
	{
		SmallVector<StringID, 4> deps;

		const bool supportsTiledDeferred = gRenderBeast()->GetFeatureSet() != RenderBeastFeatureSet::DesktopMacOS;
		if(!supportsTiledDeferred)
			deps.Add(RCNodeSceneColor::GetNodeId());
		else
			deps.Add(RCNodeSceneDepth::GetNodeId());

		return deps;
	}

	void RCNodeDeferredDirectLighting::Render(const RenderCompositorNodeInputs& inputs)
	{
		output = static_cast<RCNodeLightAccumulation*>(inputs.inputNodes[0]);

		auto gbufferNode = static_cast<RCNodeBasePass*>(inputs.inputNodes[1]);
		auto sceneDepthNode = static_cast<RCNodeSceneDepth*>(inputs.inputNodes[2]);
		auto sceneColorNode = static_cast<RCNodeSceneColor*>(inputs.inputNodes[3]);

		GBufferTextures gbuffer;
		gbuffer.albedo = gbufferNode->albedoTex->texture;
		gbuffer.normals = gbufferNode->normalTex->texture;
		gbuffer.roughMetal = gbufferNode->roughMetalTex->texture;
		gbuffer.depth = sceneDepthNode->depthTex->texture;

		const RendererViewProperties& viewProps = inputs.view.GetProperties();

		if (!inputs.view.GetRenderSettings().enableShadows)
			mLightOcclusionRT = nullptr;

		bool tiledDeferredSupported = inputs.featureSet != RenderBeastFeatureSet::DesktopMacOS;
		if(tiledDeferredSupported)
		{
			SPtr<Texture> msaaCoverage;
			if(viewProps.target.numSamples > 1)
			{
				RCNodeMSAACoverage* coverageNode = static_cast<RCNodeMSAACoverage*>(inputs.inputNodes[4]);
				msaaCoverage = coverageNode->output->texture;
			}

			TiledDeferredLightingMat* tiledDeferredMat =
				TiledDeferredLightingMat::GetVariation(viewProps.target.numSamples);

			const VisibleLightData& lightData = inputs.viewGroup.GetVisibleLightData();

			SPtr<Texture> lightAccumTexArray;
			if(output->lightAccumulationTexArray)
				lightAccumTexArray = output->lightAccumulationTexArray->texture;

			tiledDeferredMat->Execute(inputs.view, lightData, gbuffer, sceneColorNode->sceneColorTex->texture,
				output->lightAccumulationTex->texture, lightAccumTexArray, msaaCoverage);

			if (viewProps.target.numSamples > 1)
				output->MsaaTexArrayToTexture();

			// If shadows are disabled we handle all lights through tiled deferred so we can exit immediately
			if (!inputs.view.GetRenderSettings().enableShadows)
				return;
		}

		// Standard deferred used for shadowed lights, or when tiled deferred isn't supported
		UINT32 width = viewProps.target.viewRect.width;
		UINT32 height = viewProps.target.viewRect.height;
		UINT32 numSamples = viewProps.target.numSamples;

		const VisibleLightData& lightData = inputs.viewGroup.GetVisibleLightData();

		RenderAPI& rapi = RenderAPI::Instance();

		// Render unshadowed lights
		if(!tiledDeferredSupported)
		{
			ProfileGPUBlock sampleBlock("Standard deferred unshadowed lights");

			rapi.SetRenderTarget(output->renderTarget, FBT_DEPTH | FBT_STENCIL, RT_DEPTH_STENCIL);

			for (UINT32 i = 0; i < (UINT32)LightType::Count; i++)
			{
				LightType lightType = (LightType)i;

				auto& lights = lightData.GetLights(lightType);
				UINT32 count = lightData.GetNumUnshadowedLights(lightType);

				for (UINT32 j = 0; j < count; j++)
				{
					UINT32 lightIdx = j;
					const RendererLight& light = *lights[lightIdx];

					StandardDeferred::Instance().RenderLight(lightType, light, inputs.view, gbuffer, Texture::BLACK);
				}
			}
		}

		// Allocate light occlusion
		SPtr<PooledRenderTexture> lightOcclusionTex = gGpuResourcePool().Get(
			POOLED_RENDER_TEXTURE_DESC::Create2D(PF_R8, width, height, TU_RENDERTARGET, numSamples, false));

		bool rebuildRT = false;
		if (mLightOcclusionRT != nullptr)
		{
			rebuildRT |= mLightOcclusionRT->GetColorTexture(0) != lightOcclusionTex->texture;
			rebuildRT |= mLightOcclusionRT->GetDepthStencilTexture() != sceneDepthNode->depthTex->texture;
		}
		else
			rebuildRT = true;

		if (rebuildRT)
		{
			RENDER_TEXTURE_DESC lightOcclusionRTDesc;
			lightOcclusionRTDesc.colorSurfaces[0].texture = lightOcclusionTex->texture;
			lightOcclusionRTDesc.colorSurfaces[0].face = 0;
			lightOcclusionRTDesc.colorSurfaces[0].numFaces = 1;
			lightOcclusionRTDesc.colorSurfaces[0].mipLevel = 0;

			lightOcclusionRTDesc.depthStencilSurface.texture = sceneDepthNode->depthTex->texture;
			lightOcclusionRTDesc.depthStencilSurface.face = 0;
			lightOcclusionRTDesc.depthStencilSurface.numFaces = 1;
			lightOcclusionRTDesc.depthStencilSurface.mipLevel = 0;

			mLightOcclusionRT = RenderTexture::Create(lightOcclusionRTDesc);
		}

		// Render shadowed lights
		{
			ProfileGPUBlock sampleBlock("Standard deferred shadowed lights");

			const ShadowRendering& shadowRenderer = inputs.viewGroup.GetShadowRenderer();
			for (UINT32 i = 0; i < (UINT32)LightType::Count; i++)
			{
				LightType lightType = (LightType)i;

				auto& lights = lightData.GetLights(lightType);
				UINT32 count = lightData.GetNumShadowedLights(lightType);
				UINT32 offset = lightData.GetNumUnshadowedLights(lightType);

				for (UINT32 j = 0; j < count; j++)
				{
					rapi.SetRenderTarget(mLightOcclusionRT, FBT_DEPTH, RT_DEPTH_STENCIL);

					Rect2 area(0.0f, 0.0f, 1.0f, 1.0f);
					rapi.SetViewport(area);

					rapi.ClearViewport(FBT_COLOR, Color::ZERO);

					UINT32 lightIdx = offset + j;
					const RendererLight& light = *lights[lightIdx];
					shadowRenderer.RenderShadowOcclusion(inputs.view, light, gbuffer);

					rapi.SetRenderTarget(output->renderTarget, FBT_DEPTH | FBT_STENCIL, RT_COLOR0 | RT_DEPTH_STENCIL);
					StandardDeferred::Instance().RenderLight(lightType, light, inputs.view, gbuffer,
						lightOcclusionTex->texture);
				}
			}
		}

		// Makes sure light accumulation can be read by following passes
		rapi.SetRenderTarget(nullptr);
	}

	void RCNodeDeferredDirectLighting::Clear()
	{
		output = nullptr;
	}

	SmallVector<StringID, 4> RCNodeDeferredDirectLighting::GetDependencies(const RendererView& view)
	{
		SmallVector<StringID, 4> deps;
		deps.Add(RCNodeLightAccumulation::GetNodeId());
		deps.Add(RCNodeBasePass::GetNodeId());
		deps.Add(RCNodeSceneDepth::GetNodeId());
		deps.Add(RCNodeSceneColor::GetNodeId());
		deps.Add(RCNodeMSAACoverage::GetNodeId());

		return deps;
	}

	void RCNodeIndirectDiffuseLighting::Render(const RenderCompositorNodeInputs& inputs)
	{
		if (!inputs.view.GetRenderSettings().enableIndirectLighting)
			return;

		RCNodeBasePass* gbufferNode = static_cast<RCNodeBasePass*>(inputs.inputNodes[0]);
		RCNodeSceneDepth* sceneDepthNode = static_cast<RCNodeSceneDepth*>(inputs.inputNodes[1]);
		RCNodeLightAccumulation* lightAccumNode = static_cast <RCNodeLightAccumulation*>(inputs.inputNodes[2]);
		RCNodeSSAO* ssaoNode = static_cast<RCNodeSSAO*>(inputs.inputNodes[3]);

		GpuResourcePool& resPool = gGpuResourcePool();
		const RendererViewProperties& viewProps = inputs.view.GetProperties();

		const LightProbes& lightProbes = inputs.scene.lightProbes;
		LightProbesInfo lpInfo = lightProbes.GetInfo();

		IrradianceEvaluateMat* evaluateMat;
		SPtr<PooledRenderTexture> volumeIndices;
		if(lightProbes.HasAnyProbes())
		{
			POOLED_RENDER_TEXTURE_DESC volumeIndicesDesc;
			POOLED_RENDER_TEXTURE_DESC depthDesc;
			TetrahedraRenderMat::GetOutputDesc(inputs.view, volumeIndicesDesc, depthDesc);

			volumeIndices = resPool.Get(volumeIndicesDesc);
			SPtr<PooledRenderTexture> depthTex = resPool.Get(depthDesc);

			RENDER_TEXTURE_DESC rtDesc;
			rtDesc.colorSurfaces[0].texture = volumeIndices->texture;
			rtDesc.depthStencilSurface.texture = depthTex->texture;

			SPtr<RenderTexture> rt = RenderTexture::Create(rtDesc);

			RenderAPI& rapi = RenderAPI::Instance();
			rapi.SetRenderTarget(rt);
			rapi.ClearRenderTarget(FBT_DEPTH);
			gRendererUtility().Clear(-1);

			TetrahedraRenderMat* renderTetrahedra =
				TetrahedraRenderMat::GetVariation(viewProps.target.numSamples > 1, true);
			renderTetrahedra->Execute(inputs.view, sceneDepthNode->depthTex->texture, lpInfo.tetrahedraVolume, rt);

			rt = nullptr;
			depthTex = nullptr;

			evaluateMat = IrradianceEvaluateMat::GetVariation(viewProps.target.numSamples > 1, true, false);
		}
		else // Sky only
		{
			evaluateMat = IrradianceEvaluateMat::GetVariation(viewProps.target.numSamples > 1, true, true);
		}

		GBufferTextures gbuffer;
		gbuffer.albedo = gbufferNode->albedoTex->texture;
		gbuffer.normals = gbufferNode->normalTex->texture;
		gbuffer.roughMetal = gbufferNode->roughMetalTex->texture;
		gbuffer.depth = sceneDepthNode->depthTex->texture;

		SPtr<Texture> volumeIndicesTex;
		if (volumeIndices)
			volumeIndicesTex = volumeIndices->texture;

		Skybox* skybox = nullptr;
		if(inputs.view.GetRenderSettings().enableSkybox)
			skybox = inputs.scene.skybox;

		evaluateMat->Execute(inputs.view, gbuffer, volumeIndicesTex, lpInfo, skybox, ssaoNode->output,
			lightAccumNode->renderTarget);

		volumeIndices = nullptr;
	}

	void RCNodeIndirectDiffuseLighting::Clear()
	{
		// Do nothing
	}

	SmallVector<StringID, 4> RCNodeIndirectDiffuseLighting::GetDependencies(const RendererView& view)
	{
		SmallVector<StringID, 4> deps;
		deps.Add(RCNodeBasePass::GetNodeId());
		deps.Add(RCNodeSceneDepth::GetNodeId());
		deps.Add(RCNodeLightAccumulation::GetNodeId());
		deps.Add(RCNodeSSAO::GetNodeId());
		deps.Add(RCNodeDeferredDirectLighting::GetNodeId());

		return deps;
	}

	void RCNodeDeferredIndirectSpecularLighting::Render(const RenderCompositorNodeInputs& inputs)
	{
		RCNodeSceneColor* sceneColorNode = static_cast<RCNodeSceneColor*>(inputs.inputNodes[0]);
		RCNodeBasePass* gbufferNode = static_cast<RCNodeBasePass*>(inputs.inputNodes[1]);
		RCNodeSceneDepth* sceneDepthNode = static_cast<RCNodeSceneDepth*>(inputs.inputNodes[2]);
		RCNodeLightAccumulation* lightAccumNode = static_cast <RCNodeLightAccumulation*>(inputs.inputNodes[3]);
		RCNodeSSR* ssrNode = static_cast<RCNodeSSR*>(inputs.inputNodes[4]);
		RCNodeSSAO* ssaoNode = static_cast<RCNodeSSAO*>(inputs.inputNodes[5]);

		GBufferTextures gbuffer;
		gbuffer.albedo = gbufferNode->albedoTex->texture;
		gbuffer.normals = gbufferNode->normalTex->texture;
		gbuffer.roughMetal = gbufferNode->roughMetalTex->texture;
		gbuffer.depth = sceneDepthNode->depthTex->texture;

		const RendererViewProperties& viewProps = inputs.view.GetProperties();

		bool tiledDeferredSupported = inputs.featureSet != RenderBeastFeatureSet::DesktopMacOS;
		if(tiledDeferredSupported)
		{
			SPtr<Texture> msaaCoverage;
			if (viewProps.target.numSamples > 1)
			{
				RCNodeMSAACoverage* coverageNode = static_cast<RCNodeMSAACoverage*>(inputs.inputNodes[6]);
				msaaCoverage = coverageNode->output->texture;
			}

			TiledDeferredImageBasedLightingMat* material =
				TiledDeferredImageBasedLightingMat::GetVariation(viewProps.target.numSamples);

			TiledDeferredImageBasedLightingMat::Inputs iblInputs;
			iblInputs.gbuffer = gbuffer;
			iblInputs.sceneColorTex = sceneColorNode->sceneColorTex->texture;
			iblInputs.lightAccumulation = lightAccumNode->lightAccumulationTex->texture;
			iblInputs.preIntegratedGF = RendererTextures::preintegratedEnvGF;
			iblInputs.ambientOcclusion = ssaoNode->output;
			iblInputs.ssr = ssrNode->output;
			iblInputs.msaaCoverage = msaaCoverage;

			if (sceneColorNode->sceneColorTexArray)
				iblInputs.sceneColorTexArray = sceneColorNode->sceneColorTexArray->texture;

			material->Execute(inputs.view, inputs.scene, inputs.viewGroup.GetVisibleReflProbeData(), iblInputs);

			if(viewProps.target.numSamples > 1)
				sceneColorNode->MsaaTexArrayToTexture();
		}
		else // Standard deferred
		{
			SPtr<RenderTexture>	outputRT = lightAccumNode->renderTarget;

			UINT32 width = viewProps.target.viewRect.width;
			UINT32 height = viewProps.target.viewRect.height;
			UINT32 numSamples = viewProps.target.numSamples;

			RenderAPI& rapi = RenderAPI::Instance();

			bool isMSAA = viewProps.target.numSamples > 1;

			SPtr<PooledRenderTexture> iblRadianceTex = gGpuResourcePool().Get(
				POOLED_RENDER_TEXTURE_DESC::Create2D(PF_RGBA16F, width, height, TU_RENDERTARGET, numSamples, false));

			RENDER_TEXTURE_DESC rtDesc;
			rtDesc.colorSurfaces[0].texture = iblRadianceTex->texture;
			rtDesc.depthStencilSurface.texture = sceneDepthNode->depthTex->texture;

			SPtr<GpuParamBlockBuffer> perViewBuffer = inputs.view.GetPerViewBuffer();

			SPtr<RenderTexture> iblRadianceRT = RenderTexture::Create(rtDesc);
			rapi.SetRenderTarget(iblRadianceRT, FBT_DEPTH | FBT_STENCIL, RT_DEPTH_STENCIL);

			const VisibleReflProbeData& probeData = inputs.viewGroup.GetVisibleReflProbeData();

			Skybox* skybox = nullptr;
			if(inputs.view.GetRenderSettings().enableSkybox)
				skybox = inputs.scene.skybox;

			ReflProbeParamBuffer reflProbeParams;
			reflProbeParams.Populate(skybox, probeData.GetNumProbes(), inputs.scene.reflProbeCubemapsTex,
				viewProps.capturingReflections);

			// Prepare the texture for refl. probe and skybox rendering
			{
				DeferredIBLSetupMat* mat = DeferredIBLSetupMat::GetVariation(isMSAA, true);
				mat->Bind(gbuffer, perViewBuffer, ssrNode->output, ssaoNode->output, reflProbeParams.buffer);

				gRendererUtility().DrawScreenQuad();

				// Draw pixels requiring per-sample evaluation
				if (isMSAA)
				{
					DeferredIBLSetupMat* msaaMat = DeferredIBLSetupMat::GetVariation(true, false);
					msaaMat->Bind(gbuffer, perViewBuffer, ssrNode->output, ssaoNode->output, reflProbeParams.buffer);

					gRendererUtility().DrawScreenQuad();
				}
			}

			if (!viewProps.capturingReflections)
			{
				// Render refl. probes
				UINT32 numProbes = probeData.GetNumProbes();
				for (UINT32 i = 0; i < numProbes; i++)
				{
					const ReflProbeData& probe = probeData.GetProbeData(i);

					StandardDeferred::Instance().RenderReflProbe(probe, inputs.view, gbuffer, inputs.scene,
						reflProbeParams.buffer);
				}

				// Render sky
				SPtr<Texture> skyFilteredRadiance;
				if (skybox)
					skyFilteredRadiance = skybox->GetFilteredRadiance();

				if (skyFilteredRadiance)
				{
					DeferredIBLSkyMat* skymat = DeferredIBLSkyMat::GetVariation(isMSAA, true);
					skymat->Bind(gbuffer, perViewBuffer, skybox, reflProbeParams.buffer);

					gRendererUtility().DrawScreenQuad();

					// Draw pixels requiring per-sample evaluation
					if (isMSAA)
					{
						DeferredIBLSkyMat* msaaMat = DeferredIBLSkyMat::GetVariation(true, false);
						msaaMat->Bind(gbuffer, perViewBuffer, skybox, reflProbeParams.buffer);

						gRendererUtility().DrawScreenQuad();
					}
				}
			}

			// Finalize rendered reflections and output them to main render target
			{
				rapi.SetRenderTarget(outputRT, FBT_DEPTH | FBT_STENCIL, RT_COLOR0 | RT_DEPTH_STENCIL);

				DeferredIBLFinalizeMat* mat = DeferredIBLFinalizeMat::GetVariation(isMSAA, true);
				mat->Bind(gbuffer, perViewBuffer, iblRadianceTex->texture, RendererTextures::preintegratedEnvGF,
					reflProbeParams.buffer);

				gRendererUtility().DrawScreenQuad();

				// Draw pixels requiring per-sample evaluation
				if (isMSAA)
				{
					DeferredIBLFinalizeMat* msaaMat = DeferredIBLFinalizeMat::GetVariation(true, false);
					msaaMat->Bind(gbuffer, perViewBuffer, iblRadianceTex->texture, RendererTextures::preintegratedEnvGF,
						reflProbeParams.buffer);

					gRendererUtility().DrawScreenQuad();
				}
			}

			// Makes sure light accumulation can be read by following passes
			rapi.SetRenderTarget(nullptr);
		}
	}

	void RCNodeDeferredIndirectSpecularLighting::Clear()
	{
		output = nullptr;
	}

	SmallVector<StringID, 4> RCNodeDeferredIndirectSpecularLighting::GetDependencies(const RendererView& view)
	{
		SmallVector<StringID, 4> deps;
		deps.Add(RCNodeSceneColor::GetNodeId());
		deps.Add(RCNodeBasePass::GetNodeId());
		deps.Add(RCNodeSceneDepth::GetNodeId());
		deps.Add(RCNodeLightAccumulation::GetNodeId());
		deps.Add(RCNodeSSR::GetNodeId());
		deps.Add(RCNodeSSAO::GetNodeId());
		deps.Add(RCNodeMSAACoverage::GetNodeId());
		deps.Add(RCNodeIndirectDiffuseLighting::GetNodeId());

		return deps;
	}

	void RCNodeClusteredForward::Render(const RenderCompositorNodeInputs& inputs)
	{
		const SceneInfo& sceneInfo = inputs.scene;
		const RendererViewProperties& viewProps = inputs.view.GetProperties();

		const VisibleLightData& visibleLightData = inputs.viewGroup.GetVisibleLightData();
		const VisibleReflProbeData& visibleReflProbeData = inputs.viewGroup.GetVisibleReflProbeData();

		LightGridOutputs lightGridOutputs;

		struct StandardForwardBuffers
		{
			SPtr<GpuParamBlockBuffer> lightsParamBlock;
			SPtr<GpuParamBlockBuffer> reflProbesParamBlock;
			SPtr<GpuParamBlockBuffer> lightAndReflProbeParamsParamBlock;
		} standardForwardBuffers;

		const bool supportsClusteredForward = gRenderBeast()->GetFeatureSet() == RenderBeastFeatureSet::Desktop;
		if(supportsClusteredForward)
		{
			const LightGrid& lightGrid = inputs.view.GetLightGrid();
			lightGridOutputs = lightGrid.GetOutputs();
		}
		else
		{
			// Note: Store these instead of creating them every time?
			standardForwardBuffers.lightsParamBlock = gLightsParamDef.CreateBuffer();
			standardForwardBuffers.reflProbesParamBlock = gReflProbesParamDef.CreateBuffer();
			standardForwardBuffers.lightAndReflProbeParamsParamBlock = gLightAndReflProbeParamsParamDef.CreateBuffer();
		}

		Skybox* skybox = nullptr;
		if(inputs.view.GetRenderSettings().enableSkybox)
			skybox = sceneInfo.skybox;

		// Prepare refl. probe param buffer
		ReflProbeParamBuffer reflProbeParamBuffer;
		reflProbeParamBuffer.Populate(skybox, visibleReflProbeData.GetNumProbes(), sceneInfo.reflProbeCubemapsTex,
			viewProps.capturingReflections);

		SPtr<Texture> skyFilteredRadiance;
		if(skybox)
			skyFilteredRadiance = skybox->GetFilteredRadiance();

		const auto bindParamsForClustered = [&lightGridOutputs, &visibleLightData, &visibleReflProbeData]
			(GpuParams& gpuParams, const ForwardLightingParams& fwdParams, const ImageBasedLightingParams& iblParams)
		{
			for (UINT32 j = 0; j < GPT_COUNT; j++)
			{
				const GpuParamBinding& binding = fwdParams.gridParamsBindings[j];
				if (binding.slot != (UINT32)-1)
					gpuParams.SetParamBlockBuffer(binding.set, binding.slot, lightGridOutputs.gridParams);
			}

			fwdParams.gridLightOffsetsAndSizeParam.Set(lightGridOutputs.gridLightOffsetsAndSize);
			fwdParams.gridProbeOffsetsAndSizeParam.Set(lightGridOutputs.gridProbeOffsetsAndSize);

			fwdParams.gridLightIndicesParam.Set(lightGridOutputs.gridLightIndices);
			iblParams.reflectionProbeIndicesParam.Set(lightGridOutputs.gridProbeIndices);

			fwdParams.lightsBufferParam.Set(visibleLightData.GetLightBuffer());
			iblParams.reflectionProbesParam.Set(visibleReflProbeData.GetProbeBuffer());
		};

		const auto bindParamsForStandardForward = [&standardForwardBuffers, &visibleLightData, &visibleReflProbeData]
			(GpuParams& gpuParams, const Bounds& bounds, const ForwardLightingParams& fwdParams,
				const ImageBasedLightingParams& iblParams)
		{
			// Populate light & probe buffers
			Vector3I lightCounts;
			const LightData* lights[STANDARD_FORWARD_MAX_NUM_LIGHTS];
			visibleLightData.GatherInfluencingLights(bounds, lights, lightCounts);

			Vector4I lightOffsets;
			lightOffsets.x = lightCounts.x;
			lightOffsets.y = lightCounts.x;
			lightOffsets.z = lightOffsets.y + lightCounts.y;
			lightOffsets.w = lightOffsets.z + lightCounts.z;

			for (INT32 j = 0; j < lightOffsets.w; j++)
				gLightsParamDef.gLights.Set(standardForwardBuffers.lightsParamBlock, *lights[j], j);

			INT32 numReflProbes = std::min(visibleReflProbeData.GetNumProbes(), STANDARD_FORWARD_MAX_NUM_PROBES);
			for (INT32 j = 0; j < numReflProbes; j++)
			{
				gReflProbesParamDef.gReflectionProbes.Set(standardForwardBuffers.reflProbesParamBlock,
					visibleReflProbeData.GetProbeData(j), j);
			}

			gLightAndReflProbeParamsParamDef.gLightOffsets.Set(standardForwardBuffers.lightAndReflProbeParamsParamBlock,
				lightOffsets);
			gLightAndReflProbeParamsParamDef.gReflProbeCount.Set(standardForwardBuffers.lightAndReflProbeParamsParamBlock,
				numReflProbes);

			if (iblParams.reflProbesBinding.set != (UINT32)-1)
			{
				gpuParams.SetParamBlockBuffer(
					iblParams.reflProbesBinding.set,
					iblParams.reflProbesBinding.slot,
					standardForwardBuffers.reflProbesParamBlock);
			}

			if (fwdParams.lightsParamBlockBinding.set != (UINT32)-1)
			{
				gpuParams.SetParamBlockBuffer(
					fwdParams.lightsParamBlockBinding.set,
					fwdParams.lightsParamBlockBinding.slot,
					standardForwardBuffers.lightsParamBlock);
			}

			if (fwdParams.lightAndReflProbeParamsParamBlockBinding.set != (UINT32)-1)
			{
				gpuParams.SetParamBlockBuffer(
					fwdParams.lightAndReflProbeParamsParamBlockBinding.set,
					fwdParams.lightAndReflProbeParamsParamBlockBinding.slot,
					standardForwardBuffers.lightAndReflProbeParamsParamBlock);
			}
		};

		const auto bindCommonIBLParams = [&reflProbeParamBuffer, &skyFilteredRadiance, &sceneInfo]
			(GpuParams& gpuParams, ImageBasedLightingParams& iblParams)
		{
			// Note: Ideally these should be bound once (they are the same for all renderables)
			if (iblParams.reflProbeParamBindings.set != (UINT32)-1)
			{
				gpuParams.SetParamBlockBuffer(
					iblParams.reflProbeParamBindings.set,
					iblParams.reflProbeParamBindings.slot,
					reflProbeParamBuffer.buffer);
			}

			iblParams.skyReflectionsTexParam.Set(skyFilteredRadiance);
			iblParams.ambientOcclusionTexParam.Set(Texture::WHITE); // Note: Add SSAO here?
			iblParams.ssrTexParam.Set(Texture::BLACK); // Note: Add SSR here?

			iblParams.reflectionProbeCubemapsTexParam.Set(sceneInfo.reflProbeCubemapsTex);
			iblParams.preintegratedEnvBRDFParam.Set(RendererTextures::preintegratedEnvGF);
		};

		// Prepare render target
		auto sceneColorNode = static_cast<RCNodeSceneColor*>(inputs.inputNodes[0]);
		auto sceneDepthNode = static_cast<RCNodeSceneDepth*>(inputs.inputNodes[2]);
		auto resolvedSceneDepthNode = static_cast<RCNodeResolvedSceneDepth*>(inputs.inputNodes[5]);

		bool rebuildRT;
		if (renderTarget != nullptr)
		{
			rebuildRT = renderTarget->GetColorTexture(0) != sceneColorNode->sceneColorTex->texture;
			rebuildRT |= renderTarget->GetDepthStencilTexture() != sceneDepthNode->depthTex->texture;
		}
		else
			rebuildRT = true;

		if (rebuildRT)
		{
			RENDER_TEXTURE_DESC rtDesc;
			rtDesc.colorSurfaces[0].texture = sceneColorNode->sceneColorTex->texture;
			rtDesc.colorSurfaces[0].face = 0;
			rtDesc.colorSurfaces[0].numFaces = 1;
			rtDesc.colorSurfaces[0].mipLevel = 0;

			rtDesc.depthStencilSurface.texture = sceneDepthNode->depthTex->texture;
			rtDesc.depthStencilSurface.face = 0;
			rtDesc.depthStencilSurface.numFaces = 1;
			rtDesc.depthStencilSurface.mipLevel = 0;

			renderTarget = RenderTexture::Create(rtDesc);
		}

		// Prepare objects for rendering by binding forward lighting data
		//// Normal renderables
		const VisibilityInfo& visibility = inputs.view.GetVisibilityMasks();
		const auto numRenderables = (UINT32)sceneInfo.renderables.size();
		for (UINT32 i = 0; i < numRenderables; i++)
		{
			if (!visibility.renderables[i])
				continue;

			for (auto& element : sceneInfo.renderables[i]->elements)
			{
				ShaderFlags shaderFlags = element.material->GetShader()->GetFlags();

				const bool useForwardRendering = shaderFlags.IsSet(ShaderFlag::Forward) || shaderFlags.IsSet(ShaderFlag::Transparent);
				if (!useForwardRendering)
					continue;

				// Note: It would be nice to be able to set this once and keep it, only updating if the buffers actually
				// change (e.g. when growing).
				const SPtr<GpuParams> gpuParams = element.params->GetGpuParams();
				if(supportsClusteredForward)
					bindParamsForClustered(*gpuParams, element.forwardLightingParams, element.imageBasedParams);
				else
				{
					// Populate light & probe buffers
					const Bounds& bounds = sceneInfo.renderableCullInfos[i].bounds;
					bindParamsForStandardForward(*gpuParams, bounds, element.forwardLightingParams, element.imageBasedParams);
				}

				bindCommonIBLParams(*gpuParams, element.imageBasedParams);
			}
		}

		//// Particle systems
		const ParticlePerFrameData* particleData = inputs.frameInfo.perFrameData.particles;
		if(particleData)
		{
			const auto numParticleSystems = (UINT32)inputs.scene.particleSystems.size();

			for (UINT32 i = 0; i < numParticleSystems; i++)
			{
				if (!visibility.particleSystems[i])
					continue;

				const RendererParticles& rendererParticles = inputs.scene.particleSystems[i];
				ParticlesRenderElement& renderElement = rendererParticles.renderElement;

				ShaderFlags shaderFlags = renderElement.material->GetShader()->GetFlags();

				if(shaderFlags.IsSet(ShaderFlag::Transparent))
					renderElement.depthInputTexture.Set(resolvedSceneDepthNode->output->texture);

				const bool requiresForwardLighting = shaderFlags.IsSet(ShaderFlag::Forward);
				if (!requiresForwardLighting)
					continue;

				if(!renderElement.IsValid())
					continue;

				const SPtr<GpuParams> gpuParams = renderElement.params->GetGpuParams();

				// Note: It would be nice to be able to set this once and keep it, only updating if the buffers actually
				// change (e.g. when growing).
				if(supportsClusteredForward)
					bindParamsForClustered(*gpuParams, renderElement.forwardLightingParams, renderElement.imageBasedParams);
				else
				{
					// Populate light & probe buffers
					const Bounds& bounds = sceneInfo.particleSystemCullInfos[i].bounds;
					bindParamsForStandardForward(*gpuParams, bounds, renderElement.forwardLightingParams, renderElement.imageBasedParams);
				}

				bindCommonIBLParams(*gpuParams, renderElement.imageBasedParams);
			}
		}

		// TODO: Forward pipeline rendering doesn't support shadows. In order to support this I'd have to render the light
		// occlusion for all lights affecting this object into a single (or a few) textures. I can likely use texture
		// arrays for this, or to avoid sampling many textures, perhaps just jam it all in one or few texture channels.

		// Render everything
		RenderAPI& rapi = RenderAPI::Instance();

		RenderQueue* opaqueQueue = inputs.view.GetOpaqueQueue(true).get();
		RenderQueue* transparentQueue = inputs.view.GetTransparentQueue().get();

		rapi.SetRenderTarget(renderTarget, 0, RT_ALL);
		renderQueueElements(opaqueQueue->GetSortedElements());

		rapi.SetRenderTarget(renderTarget, FBT_DEPTH, RT_ALL);
		renderQueueElements(transparentQueue->GetSortedElements());

		// Note: Perhaps delay clearing this one frame, so previous frame textures have a better chance of being done
		ParticleRenderer::Instance().GetTexturePool().Clear();

		// Trigger post-lighting callbacks
		Camera* sceneCamera = inputs.view.GetSceneCamera();
		if (sceneCamera != nullptr)
		{
			inputs.view.NotifyCompositorTargetChangedInternal(renderTarget);

			for(auto& extension : inputs.extPostLighting)
				extension->Render(*sceneCamera, inputs.view.GetContext());
		}
	}

	void RCNodeClusteredForward::Clear()
	{
		// Do nothing
	}

	SmallVector<StringID, 4> RCNodeClusteredForward::GetDependencies(const RendererView& view)
	{
		return {
			RCNodeSceneColor::GetNodeId(),
			RCNodeSkybox::GetNodeId(),
			RCNodeSceneDepth::GetNodeId(),
			RCNodeParticleSimulate::GetNodeId(),
			RCNodeParticleSort::GetNodeId(),
			RCNodeResolvedSceneDepth::GetNodeId()
		};
	}

	void RCNodeSkybox::Render(const RenderCompositorNodeInputs& inputs)
	{
		Skybox* skybox = nullptr;
		if(inputs.view.GetRenderSettings().enableSkybox)
			skybox = inputs.scene.skybox;

		SPtr<Texture> radiance = skybox ? skybox->GetTexture() : nullptr;

		if (radiance != nullptr)
		{
			SkyboxMat* material = SkyboxMat::GetVariation(false);
			material->Bind(inputs.view.GetPerViewBuffer(), radiance, Color::White);
		}
		else
		{
			// Cancel out the linear->SRGB conversion
			Color clearColor = inputs.view.GetProperties().target.clearColor.GetLinear();

			SkyboxMat* material = SkyboxMat::GetVariation(true);
			material->Bind(inputs.view.GetPerViewBuffer(), nullptr, clearColor);
		}

		RCNodeSceneColor* sceneColorNode = static_cast<RCNodeSceneColor*>(inputs.inputNodes[0]);
		int readOnlyFlags = FBT_DEPTH | FBT_STENCIL;

		RenderAPI& rapi = RenderAPI::Instance();
		rapi.SetRenderTarget(sceneColorNode->renderTarget, readOnlyFlags, RT_COLOR0 | RT_DEPTH_STENCIL);

		Rect2 area(0.0f, 0.0f, 1.0f, 1.0f);
		rapi.SetViewport(area);

		SPtr<Mesh> mesh = gRendererUtility().GetSkyBoxMesh();
		gRendererUtility().Draw(mesh, mesh->GetProperties().GetSubMesh(0));
	}

	void RCNodeSkybox::Clear()
	{ }

	SmallVector<StringID, 4> RCNodeSkybox::GetDependencies(const RendererView& view)
	{
		SmallVector<StringID, 4> deps;
		deps.Add(RCNodeSceneColor::GetNodeId());
		deps.Add(RCNodeDeferredIndirectSpecularLighting::GetNodeId());

		return deps;
	}

	void RCNodeFinalResolve::Render(const RenderCompositorNodeInputs& inputs)
	{
		const RendererViewProperties& viewProps = inputs.view.GetProperties();

		SPtr<Texture> input;
		if(viewProps.runPostProcessing)
		{
			RCNodePostProcess* postProcessNode = static_cast<RCNodePostProcess*>(inputs.inputNodes[0]);

			// Note: Ideally the last PP effect could write directly to the final target and we could avoid this copy
			input = postProcessNode->GetLastOutput();
		}
		else
		{
			RCNodeSceneColor* sceneColorNode = static_cast<RCNodeSceneColor*>(inputs.inputNodes[0]);
			input = sceneColorNode->sceneColorTex->texture;
		}

		SPtr<RenderTarget> target = viewProps.target.target;

		RenderAPI& rapi = RenderAPI::Instance();
		rapi.SetRenderTarget(target);
		rapi.SetViewport(viewProps.target.nrmViewRect);

		gRendererUtility().Blit(input, Rect2I::EMPTY, viewProps.flipView);

		if(viewProps.encodeDepth)
		{
			RCNodeResolvedSceneDepth* resolvedSceneDepthNode = static_cast<RCNodeResolvedSceneDepth*>(inputs.inputNodes[0]);

			EncodeDepthMat* encodeDepthMat = EncodeDepthMat::Get();
			encodeDepthMat->Execute(resolvedSceneDepthNode->output->texture, viewProps.depthEncodeNear,
				viewProps.depthEncodeFar, target);
		}

		// Trigger overlay callbacks
		Camera* sceneCamera = inputs.view.GetSceneCamera();
		if (sceneCamera != nullptr)
		{
			inputs.view.NotifyCompositorTargetChangedInternal(target);

			for(auto& extension : inputs.extOverlay)
				extension->Render(*sceneCamera, inputs.view.GetContext());
		}

		inputs.view.NotifyCompositorTargetChangedInternal(nullptr);
	}

	void RCNodeFinalResolve::Clear()
	{ }

	SmallVector<StringID, 4> RCNodeFinalResolve::GetDependencies(const RendererView& view)
	{
		const RendererViewProperties& viewProps = view.GetProperties();

		SmallVector<StringID, 4> deps;
		if(viewProps.runPostProcessing)
		{
			deps.Add(RCNodePostProcess::GetNodeId());
			deps.Add(RCNodeFilmGrain::GetNodeId());
		}
		else
		{
			deps.Add(RCNodeSceneColor::GetNodeId());
			deps.Add(RCNodeClusteredForward::GetNodeId());
		}

		if(viewProps.encodeDepth)
			deps.Add(RCNodeResolvedSceneDepth::GetNodeId());

		return deps;
	}

	void RCNodePostProcess::GetAndSwitch(const RendererView& view, SPtr<RenderTexture>& output, SPtr<Texture>& lastFrame) const
	{
		const RendererViewProperties& viewProps = view.GetProperties();
		UINT32 width = viewProps.target.viewRect.width;
		UINT32 height = viewProps.target.viewRect.height;

		if(!mOutput[mCurrentIdx])
		{
			mOutput[mCurrentIdx] = gGpuResourcePool().Get(
				POOLED_RENDER_TEXTURE_DESC::Create2D(PF_RGBA8, width, height, TU_RENDERTARGET, 1, false));
		}

		output = mOutput[mCurrentIdx]->renderTexture;

		UINT32 otherIdx = (mCurrentIdx + 1) % 2;
		if (mOutput[otherIdx])
			lastFrame = mOutput[otherIdx]->texture;

		mCurrentIdx = otherIdx;
	}

	SPtr<Texture> RCNodePostProcess::GetLastOutput() const
	{
		UINT32 otherIdx = (mCurrentIdx + 1) % 2;
		if (mOutput[otherIdx])
			return mOutput[otherIdx]->texture;
		
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

	SmallVector<StringID, 4> RCNodePostProcess::GetDependencies(const RendererView& view)
	{
		return {};
	}

	void RCNodeEyeAdaptation::Render(const RenderCompositorNodeInputs& inputs)
	{
		GpuResourcePool& resPool = gGpuResourcePool();

		const RenderSettings& settings = inputs.view.GetRenderSettings();

		const bool hdr = settings.enableHDR;

		if(hdr && settings.enableAutoExposure)
		{
			// Get downsample scene
			auto* halfSceneColorNode = static_cast<RCNodeHalfSceneColor*>(inputs.inputNodes[1]);
			const SPtr<PooledRenderTexture>& downsampledScene = halfSceneColorNode->output;

			if(UseHistogramEyeAdapatation(inputs))
			{
				// Generate histogram
				SPtr<PooledRenderTexture> eyeAdaptHistogram =
					resPool.Get(EyeAdaptHistogramMat::GetOutputDesc(downsampledScene->texture));
				EyeAdaptHistogramMat* eyeAdaptHistogramMat = EyeAdaptHistogramMat::Get();
				eyeAdaptHistogramMat->Execute(downsampledScene->texture, eyeAdaptHistogram->texture, settings.autoExposure);

				// Reduce histogram
				SPtr<PooledRenderTexture> reducedHistogram = resPool.Get(EyeAdaptHistogramReduceMat::GetOutputDesc());

				SPtr<Texture> prevFrameEyeAdaptation;
				if (previous != nullptr)
					prevFrameEyeAdaptation = previous->texture;

				EyeAdaptHistogramReduceMat* eyeAdaptHistogramReduce = EyeAdaptHistogramReduceMat::Get();
				eyeAdaptHistogramReduce->Execute(
					downsampledScene->texture,
					eyeAdaptHistogram->texture,
					prevFrameEyeAdaptation,
					reducedHistogram->renderTexture);

				eyeAdaptHistogram = nullptr;

				// Generate eye adaptation value
				output = resPool.Get(EyeAdaptationMat::GetOutputDesc());
				EyeAdaptationMat* eyeAdaptationMat = EyeAdaptationMat::Get();
				eyeAdaptationMat->Execute(
					reducedHistogram->texture,
					output->renderTexture,
					inputs.frameInfo.timings.timeDelta,
					settings.autoExposure,
					settings.exposureScale);
			}
			else
			{
				// Populate alpha values of the downsampled texture with luminance
				SPtr<PooledRenderTexture> luminanceTex =
					resPool.Get(EyeAdaptationBasicSetupMat::GetOutputDesc(downsampledScene->texture));

				EyeAdaptationBasicSetupMat* setupMat = EyeAdaptationBasicSetupMat::Get();
				setupMat->Execute(
					downsampledScene->texture,
					luminanceTex->renderTexture,
					inputs.frameInfo.timings.timeDelta,
					settings.autoExposure,
					settings.exposureScale);

				SPtr<Texture> downsampleInput = luminanceTex->texture;
				luminanceTex = nullptr;

				// Downsample some more
				for(UINT32 i = 0; i < 5; i++)
				{
					DownsampleMat* downsampleMat = DownsampleMat::GetVariation(1, false);
					SPtr<PooledRenderTexture> downsampledLuminance =
						resPool.Get(DownsampleMat::GetOutputDesc(downsampleInput));

					downsampleMat->Execute(downsampleInput, downsampledLuminance->renderTexture);
					downsampleInput = downsampledLuminance->texture;
				}

				// Generate eye adaptation value
				EyeAdaptationBasicMat* eyeAdaptationMat = EyeAdaptationBasicMat::Get();

				SPtr<Texture> prevFrameEyeAdaptation;
				if (previous != nullptr)
					prevFrameEyeAdaptation = previous->texture;

				output = resPool.Get(EyeAdaptationBasicMat::GetOutputDesc());
				eyeAdaptationMat->Execute(
					downsampleInput,
					prevFrameEyeAdaptation,
					output->renderTexture,
					inputs.frameInfo.timings.timeDelta,
					settings.autoExposure,
					settings.exposureScale);
			}

			const RendererView& view = inputs.view;

			// Notify the view eye adaptation value will change
			SPtr<CommandBuffer> cb = RenderAPI::Instance().GetMainCommandBuffer();
			view.NotifyLuminanceUpdatedInternal(inputs.frameInfo.timings.frameIdx, cb, output);
		}
		else
		{
			previous = nullptr;
			output = nullptr;
		}
	}

	void RCNodeEyeAdaptation::Clear()
	{
		std::swap(output, previous);
		output = nullptr;
	}
	
	bool RCNodeEyeAdaptation::UseHistogramEyeAdapatation(const RenderCompositorNodeInputs& inputs)
	{
		return inputs.featureSet == RenderBeastFeatureSet::Desktop;
	}

	SmallVector<StringID, 4> RCNodeEyeAdaptation::GetDependencies(const RendererView& view)
	{
		SmallVector<StringID, 4> deps;
		deps.Add(RCNodeClusteredForward::GetNodeId());

		const RenderSettings& settings = view.GetRenderSettings();
		if(settings.enableHDR && settings.enableAutoExposure)
			deps.Add(RCNodeHalfSceneColor::GetNodeId());

		return deps;
	}

	void RCNodeTonemapping::Render(const RenderCompositorNodeInputs& inputs)
	{
		const RendererViewProperties& viewProps = inputs.view.GetProperties();
		const RenderSettings& settings = inputs.view.GetRenderSettings();

		auto* eyeAdaptationNode = static_cast<RCNodeEyeAdaptation*>(inputs.inputNodes[0]);
		auto* sceneColorNode = static_cast<RCNodeSceneColor*>(inputs.inputNodes[1]);
		auto* postProcessNode = static_cast<RCNodePostProcess*>(inputs.inputNodes[3]);
		const SPtr<Texture>& sceneColor = sceneColorNode->sceneColorTex->texture;

		const bool hdr = settings.enableHDR;
		const bool msaa = sceneColor->GetProperties().GetNumSamples() > 1;

		const bool volumeLUT = inputs.featureSet == RenderBeastFeatureSet::Desktop;
		bool gammaOnly;
		bool autoExposure;
		if (hdr)
		{
			if (settings.enableTonemapping)
			{
				const UINT64 latestHash = inputs.view.GetRenderSettingsHash();
				const bool tonemapLUTDirty = mTonemapLastUpdateHash != latestHash;

				if (tonemapLUTDirty) // Rebuild LUT if PP settings changed
				{
					CreateTonemapLUTMat* createLUT = CreateTonemapLUTMat::GetVariation(volumeLUT);
					if(mTonemapLUT == nullptr)
						mTonemapLUT = gGpuResourcePool().Get(createLUT->GetOutputDesc());

					if(volumeLUT)
						createLUT->Execute3D(mTonemapLUT->texture, settings);
					else
						createLUT->Execute2D(mTonemapLUT->renderTexture, settings);

					mTonemapLastUpdateHash = latestHash;
				}

				gammaOnly = false;
			}
			else
				gammaOnly = true;

			autoExposure = settings.enableAutoExposure;
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
		postProcessNode->GetAndSwitch(inputs.view, ppOutput, ppLastFrame);

		SPtr<Texture> eyeAdaptationTex;
		if (eyeAdaptationNode->output)
			eyeAdaptationTex = eyeAdaptationNode->output->texture;

		SPtr<Texture> tonemapLUTTex;
		if (mTonemapLUT)
			tonemapLUTTex = mTonemapLUT->texture;

		SPtr<Texture> bloomTex;
		if(settings.bloom.enabled)
		{
			auto* bloomNode = static_cast<RCNodeBloom*>(inputs.inputNodes[5]);
			bloomTex = bloomNode->output;
		}

		tonemapping->Execute(sceneColor, eyeAdaptationTex, bloomTex, tonemapLUTTex, ppOutput, settings);
	}

	void RCNodeTonemapping::Clear()
	{
		// Do nothing
	}
	
	SmallVector<StringID, 4> RCNodeTonemapping::GetDependencies(const RendererView& view)
	{
		SmallVector<StringID, 4> deps = {
			RCNodeEyeAdaptation::GetNodeId(),
			RCNodeSceneColor::GetNodeId(),
			RCNodeMotionBlur::GetNodeId(),
			RCNodePostProcess::GetNodeId(),
			RCNodeHalfSceneColor::GetNodeId()
		};

		if(view.GetRenderSettings().bloom.enabled)
			deps.Add(RCNodeBloom::GetNodeId());

		if(view.GetRenderSettings().screenSpaceLensFlare.enabled)
			deps.Add(RCNodeScreenSpaceLensFlare::GetNodeId());

		return deps;
	}

	void RCNodeBokehDOF::Render(const RenderCompositorNodeInputs& inputs)
	{
		const DepthOfFieldSettings& settings = inputs.view.GetRenderSettings().depthOfField;
		if(!settings.enabled || settings.type != DepthOfFieldType::Bokeh)
			return;

		const RendererViewProperties& viewProps = inputs.view.GetProperties();
		const bool msaa = viewProps.target.numSamples > 1;

		RCNodeSceneColor* sceneColorNode = static_cast<RCNodeSceneColor*>(inputs.inputNodes[1]);
		RCNodeSceneDepth* sceneDepthNode = static_cast<RCNodeSceneDepth*>(inputs.inputNodes[2]);
		RCNodeLightAccumulation* lightAccumNode = static_cast<RCNodeLightAccumulation*>(inputs.inputNodes[3]);

		BokehDOFPrepareMat* prepareMat = BokehDOFPrepareMat::GetVariation(msaa);
		BokehDOFMat* renderMat = BokehDOFMat::GetVariation(settings.bokehOcclusion);
		BokehDOFCombineMat* combineMat = BokehDOFCombineMat::GetVariation(msaa ? MSAAMode::Full : MSAAMode::None);

		SPtr<Texture> depth = sceneDepthNode->depthTex->texture;

		// Downsample scene and store depth in .w
		SPtr<PooledRenderTexture> halfResSceneAndDepth =
			gGpuResourcePool().Get(BokehDOFPrepareMat::GetOutputDesc(sceneColorNode->sceneColorTex->texture));

		prepareMat->Execute(sceneColorNode->sceneColorTex->texture, depth, inputs.view, settings,
			halfResSceneAndDepth->renderTexture);

		SPtr<PooledRenderTexture> unfocusedTex =
			gGpuResourcePool().Get(BokehDOFMat::GetOutputDesc(halfResSceneAndDepth->texture));

		renderMat->Execute(halfResSceneAndDepth->texture, inputs.view, settings, unfocusedTex->renderTexture);
		halfResSceneAndDepth = nullptr;

		// Combine the unfocused and focused textures to form the final image
		combineMat->Execute(unfocusedTex->texture, sceneColorNode->sceneColorTex->texture, depth, inputs.view, settings,
			lightAccumNode->lightAccumulationTex->renderTexture);

		// TODO - This might be incorrect when not supporting tiled deferred? As light accum is the same as scene color
		sceneColorNode->Swap(lightAccumNode);
	}

	void RCNodeBokehDOF::Clear()
	{
		// Do nothing
	}

	SmallVector<StringID, 4> RCNodeBokehDOF::GetDependencies(const RendererView& view)
	{
		return 
		{ 
			RCNodeClusteredForward::GetNodeId(), 
			RCNodeSceneColor::GetNodeId(), 
			RCNodeSceneDepth::GetNodeId(), 
			RCNodeLightAccumulation::GetNodeId() 
		};
	}

	RCNodeTemporalAA::~RCNodeTemporalAA()
	{
		DeallocOutputs();
	}

	void RCNodeTemporalAA::Render(const RenderCompositorNodeInputs& inputs)
	{
		auto* sceneColorNode = static_cast<RCNodeSceneColor*>(inputs.inputNodes[1]);
		SPtr<PooledRenderTexture> sceneColor = sceneColorNode->sceneColorTex;

		const TemporalAASettings& settings = inputs.view.GetRenderSettings().temporalAA;
		if (!settings.enabled)
		{
			DeallocOutputs();

			mPooledOutput = nullptr;
			output = sceneColor->texture;
			return;
		}

		RenderAPI& rapi = RenderAPI::Instance();

		// TODO - Resolve scene color MSAA (in a way that can be shared by multiple effects)

		auto* sceneDepthNode = static_cast<RCNodeSceneDepth*>(inputs.inputNodes[2]);
		auto* basePassNode = static_cast<RCNodeBasePass*>(inputs.inputNodes[3]);

		GpuResourcePool& resPool = gGpuResourcePool();
		const RendererViewProperties& viewProps = inputs.view.GetProperties();

		UINT32 width = viewProps.target.viewRect.width;
		UINT32 height = viewProps.target.viewRect.height;

		// Resolve multiple samples if MSAA is used
		SPtr<PooledRenderTexture> resolvedSceneColor;
		if (viewProps.target.numSamples > 1)
		{
			resolvedSceneColor = resPool.Get(POOLED_RENDER_TEXTURE_DESC::Create2D(PF_RGBA16F, width, height,
				TU_RENDERTARGET));

			rapi.SetRenderTarget(resolvedSceneColor->renderTexture);
			gRendererUtility().Blit(sceneColor->texture);

			sceneColor = resolvedSceneColor;
		}

		if (mPrevFrame)
		{
			mPooledOutput = resPool.Get(POOLED_RENDER_TEXTURE_DESC::Create2D(PF_RGBA16F, width, height, TU_RENDERTARGET));

			rapi.SetRenderTarget(mPooledOutput->renderTexture);
			rapi.ClearRenderTarget(FBT_COLOR);

			SPtr<Texture> velocityTex;
			if (basePassNode->velocityTex)
				velocityTex = basePassNode->velocityTex->texture;

			float exposure = inputs.view.GetCurrentExposure();

			TemporalFilteringMat* temporalFilteringMat =
				TemporalFilteringMat::GetVariation(TemporalFilteringType::FullScreenAA, true, viewProps.target.numSamples > 1);
			temporalFilteringMat->Execute(inputs.view, mPrevFrame->texture, sceneColor->texture, velocityTex,
				sceneDepthNode->depthTex->texture, viewProps.temporalJitter, exposure, mPooledOutput->renderTexture);

			sceneColorNode->SetExternalTexture(mPooledOutput);
		}
		else
			mPooledOutput = sceneColor;

		RenderAPI::Instance().SetRenderTarget(nullptr);
		output = mPooledOutput->texture;
	}

	void RCNodeTemporalAA::Clear()
	{
		mPrevFrame = mPooledOutput;
		mPooledOutput = nullptr;
		output = nullptr;
	}

	void RCNodeTemporalAA::DeallocOutputs()
	{
		mPrevFrame = nullptr;
		output = nullptr;
	}

	SmallVector<StringID, 4> RCNodeTemporalAA::GetDependencies(const RendererView& view)
	{
		return
		{
			RCNodeBokehDOF::GetNodeId(),
			RCNodeSceneColor::GetNodeId(),
			RCNodeSceneDepth::GetNodeId(),
			RCNodeBasePass::GetNodeId()
		};
	}

	void RCNodeMotionBlur::Render(const RenderCompositorNodeInputs& inputs)
	{
		const MotionBlurSettings& settings = inputs.view.GetRenderSettings().motionBlur;
		if (!settings.enabled)
			return;

		// TODO - Account for settings such as filter type and domain by grabbing correct shader variations

		// TODO - WIP
		//const RendererViewProperties& viewProps = inputs.view.getProperties();

		//RCNodeSceneColor* sceneColorNode = static_cast<RCNodeSceneColor*>(inputs.inputNodes[1]);
		//RCNodeSceneDepth* sceneDepthNode = static_cast<RCNodeSceneDepth*>(inputs.inputNodes[2]);
		//RCNodeLightAccumulation* lightAccumNode = static_cast<RCNodeLightAccumulation*>(inputs.inputNodes[3]);

		//MotionBlurMat* motionBlurMat = MotionBlurMat::Get();

		//SPtr<Texture> depth = sceneDepthNode->depthTex->texture;
		//motionBlurMat->Execute(sceneColorNode->sceneColorTex->texture, depth, inputs.view, settings,
		//	lightAccumNode->lightAccumulationTex->renderTexture);

		//sceneColorNode->swap(lightAccumNode);
	}

	void RCNodeMotionBlur::Clear()
	{
		// Do nothing
	}

	SmallVector<StringID, 4> RCNodeMotionBlur::GetDependencies(const RendererView & view)
	{
		return
		{
			RCNodeTemporalAA::GetNodeId(),
			RCNodeSceneColor::GetNodeId(),
			RCNodeSceneDepth::GetNodeId(),
			RCNodeLightAccumulation::GetNodeId()
		};
	}

	void RCNodeGaussianDOF::Render(const RenderCompositorNodeInputs& inputs)
	{
		RCNodeSceneDepth* sceneDepthNode = static_cast<RCNodeSceneDepth*>(inputs.inputNodes[1]);
		RCNodePostProcess* postProcessNode = static_cast<RCNodePostProcess*>(inputs.inputNodes[2]);

		const DepthOfFieldSettings& settings = inputs.view.GetRenderSettings().depthOfField;
		bool near = settings.nearBlurAmount > 0.0f;
		bool far = settings.farBlurAmount > 0.0f;

		bool enabled = settings.enabled && settings.type == DepthOfFieldType::Gaussian && (near || far);
		if(!enabled)
			return;

		GaussianDOFSeparateMat* separateMat = GaussianDOFSeparateMat::GetVariation(near, far);
		GaussianDOFCombineMat* combineMat = GaussianDOFCombineMat::GetVariation(near, far);
		GaussianBlurMat* blurMat = GaussianBlurMat::Get();

		SPtr<RenderTexture> ppOutput;
		SPtr<Texture> ppLastFrame;
		postProcessNode->GetAndSwitch(inputs.view, ppOutput, ppLastFrame);

		separateMat->Execute(ppLastFrame, sceneDepthNode->depthTex->texture, inputs.view, settings);

		SPtr<PooledRenderTexture> nearTex, farTex;
		if(near && far)
		{
			nearTex = separateMat->GetOutput(0);
			farTex = separateMat->GetOutput(1);
		}
		else
		{
			if (near)
				nearTex = separateMat->GetOutput(0);
			else
				farTex = separateMat->GetOutput(0);
		}

		// Blur the out of focus pixels
		// Note: Perhaps set up stencil so I can avoid performing blur on unused parts of the textures?
		const TextureProperties& texProps = nearTex ? nearTex->texture->GetProperties() : farTex->texture->GetProperties();
		POOLED_RENDER_TEXTURE_DESC tempTexDesc = POOLED_RENDER_TEXTURE_DESC::Create2D(texProps.GetFormat(),
			texProps.GetWidth(), texProps.GetHeight(), TU_RENDERTARGET);
		SPtr<PooledRenderTexture> tempTexture = gGpuResourcePool().Get(tempTexDesc);

		SPtr<Texture> blurredNearTex;
		if(nearTex)
		{
			blurMat->Execute(nearTex->texture, settings.nearBlurAmount, tempTexture->renderTexture);
			blurredNearTex = tempTexture->texture;
		}

		SPtr<Texture> blurredFarTex;
		if(farTex)
		{
			// If temporary texture is used up, re-use the original near texture for the blurred result
			if(blurredNearTex)
			{
				blurMat->Execute(farTex->texture, settings.farBlurAmount, nearTex->renderTexture);
				blurredFarTex = nearTex->texture;
			}
			else // Otherwise just use the temporary
			{
				blurMat->Execute(farTex->texture, settings.farBlurAmount, tempTexture->renderTexture);
				blurredFarTex = tempTexture->texture;
			}
		}

		combineMat->Execute(ppLastFrame, blurredNearTex, blurredFarTex,
			sceneDepthNode->depthTex->texture, ppOutput, inputs.view, settings);

		separateMat->Release();
	}

	void RCNodeGaussianDOF::Clear()
	{
		// Do nothing
	}

	SmallVector<StringID, 4> RCNodeGaussianDOF::GetDependencies(const RendererView& view)
	{
		return { RCNodeTonemapping::GetNodeId(), RCNodeSceneDepth::GetNodeId(), RCNodePostProcess::GetNodeId() };
	}

	void RCNodeFXAA::Render(const RenderCompositorNodeInputs& inputs)
	{
		const RenderSettings& settings = inputs.view.GetRenderSettings();
		if (!settings.enableFXAA)
			return;

		RCNodePostProcess* postProcessNode = static_cast<RCNodePostProcess*>(inputs.inputNodes[1]);

		SPtr<RenderTexture> ppOutput;
		SPtr<Texture> ppLastFrame;
		postProcessNode->GetAndSwitch(inputs.view, ppOutput, ppLastFrame);

		// Note: I could skip executing FXAA over DOF and motion blurred pixels
		FXAAMat* fxaa = FXAAMat::Get();
		fxaa->Execute(ppLastFrame, ppOutput);
	}

	void RCNodeFXAA::Clear()
	{
		// Do nothing
	}

	SmallVector<StringID, 4> RCNodeFXAA::GetDependencies(const RendererView& view)
	{
		return { RCNodeGaussianDOF::GetNodeId(), RCNodePostProcess::GetNodeId() };
	}

	void RCNodeChromaticAberration::Render(const RenderCompositorNodeInputs& inputs)
	{
		const RenderSettings& settings = inputs.view.GetRenderSettings();
		if (!settings.chromaticAberration.enabled)
			return;

		auto* postProcessNode = static_cast<RCNodePostProcess*>(inputs.inputNodes[1]);

		SPtr<RenderTexture> ppOutput;
		SPtr<Texture> ppLastFrame;
		postProcessNode->GetAndSwitch(inputs.view, ppOutput, ppLastFrame);

		ChromaticAberrationMat* chromaticAberration = ChromaticAberrationMat::GetVariation(settings.chromaticAberration.type);
		chromaticAberration->Execute(ppLastFrame, settings.chromaticAberration, ppOutput);
	}

	void RCNodeChromaticAberration::Clear()
	{
		// Do nothing
	}

	SmallVector<StringID, 4> RCNodeChromaticAberration::GetDependencies(const RendererView & view)
	{
		return { RCNodeFXAA::GetNodeId(), RCNodePostProcess::GetNodeId() };
	}

	void RCNodeFilmGrain::Render(const RenderCompositorNodeInputs& inputs)
	{
		const RenderSettings& settings = inputs.view.GetRenderSettings();
		if (!settings.filmGrain.enabled)
			return;

		auto* postProcessNode = static_cast<RCNodePostProcess*>(inputs.inputNodes[1]);

		SPtr<RenderTexture> ppOutput;
		SPtr<Texture> ppLastFrame;
		postProcessNode->GetAndSwitch(inputs.view, ppOutput, ppLastFrame);

		FilmGrainMat* filmGrain = FilmGrainMat::Get();
		filmGrain->Execute(ppLastFrame, inputs.frameInfo.timings.time, settings.filmGrain, ppOutput);
	}

	void RCNodeFilmGrain::Clear()
	{
		// Do nothing
	}

	SmallVector<StringID, 4> RCNodeFilmGrain::GetDependencies(const RendererView & view)
	{
		return { RCNodeChromaticAberration::GetNodeId(), RCNodePostProcess::GetNodeId() };
	}
	
	void RCNodeHalfSceneColor::Render(const RenderCompositorNodeInputs& inputs)
	{
		const RendererViewProperties& viewProps = inputs.view.GetProperties();

		auto* sceneColorNode = static_cast<RCNodeSceneColor*>(inputs.inputNodes[0]);
		const SPtr<Texture>& input = sceneColorNode->sceneColorTex->texture;

		// Downsample scene
		const bool msaa = viewProps.target.numSamples > 1;
		DownsampleMat* downsampleMat = DownsampleMat::GetVariation(1, msaa);

		output = gGpuResourcePool().Get(DownsampleMat::GetOutputDesc(input));

		downsampleMat->Execute(input, output->renderTexture);
	}

	void RCNodeHalfSceneColor::Clear()
	{
		output = nullptr;
	}

	SmallVector<StringID, 4> RCNodeHalfSceneColor::GetDependencies(const RendererView& view)
	{
		return { RCNodeSceneColor::GetNodeId() };
	}

	constexpr UINT32 RCNodeSceneColorDownsamples::MAX_NUM_DOWNSAMPLES;

	void RCNodeSceneColorDownsamples::Render(const RenderCompositorNodeInputs& inputs)
	{
		GpuResourcePool& resPool = gGpuResourcePool();

		auto* halfSceneColorNode = static_cast<RCNodeHalfSceneColor*>(inputs.inputNodes[0]);
		const TextureProperties& halfSceneProps = halfSceneColorNode->output->texture->GetProperties();

		const UINT32 totalDownsampleLevels = PixelUtil::GetMaxMipmaps(
			halfSceneProps.GetWidth(),
			halfSceneProps.GetHeight(),
			1,
			halfSceneProps.GetFormat()
		) + 1;

		availableDownsamples = Math::Min(MAX_NUM_DOWNSAMPLES, totalDownsampleLevels);

		{
			output[0] = halfSceneColorNode->output;

			DownsampleMat* downsampleMat = DownsampleMat::GetVariation(1, false);
			for (UINT32 i = 1; i < availableDownsamples; i++)
			{
				output[i] = resPool.Get(DownsampleMat::GetOutputDesc(output[i - 1]->texture));
				downsampleMat->Execute(output[i - 1]->texture, output[i]->renderTexture);
			}
		}
	}

	void RCNodeSceneColorDownsamples::Clear()
	{
		for(UINT32 i = 0; i < MAX_NUM_DOWNSAMPLES; i++)
			output[i] = nullptr;
	}

	SmallVector<StringID, 4> RCNodeSceneColorDownsamples::GetDependencies(const RendererView& view)
	{
		return { RCNodeHalfSceneColor::GetNodeId() };
	}

	void RCNodeResolvedSceneDepth::Render(const RenderCompositorNodeInputs& inputs)
	{
		const RendererViewProperties& viewProps = inputs.view.GetProperties();
		RCNodeSceneDepth* sceneDepthNode = static_cast<RCNodeSceneDepth*>(inputs.inputNodes[0]);

		if (viewProps.target.numSamples > 1)
		{
			UINT32 width = viewProps.target.viewRect.width;
			UINT32 height = viewProps.target.viewRect.height;

			output = gGpuResourcePool().Get(
				POOLED_RENDER_TEXTURE_DESC::Create2D(PF_D32_S8X24, width, height, TU_DEPTHSTENCIL, 1, false));

			RenderAPI& rapi = RenderAPI::Instance();
			rapi.SetRenderTarget(output->renderTexture);
			rapi.ClearRenderTarget(FBT_STENCIL);
			gRendererUtility().Blit(sceneDepthNode->depthTex->texture, Rect2I::EMPTY, false, true);
		}
		else
			output = sceneDepthNode->depthTex;
	}

	void RCNodeResolvedSceneDepth::Clear()
	{
		output = nullptr;
	}

	SmallVector<StringID, 4> RCNodeResolvedSceneDepth::GetDependencies(const RendererView& view)
	{
		// GBuffer require because it renders the base pass (populates the depth buffer)
		return { RCNodeSceneDepth::GetNodeId(), RCNodeBasePass::GetNodeId() };
	}

	void RCNodeHiZ::Render(const RenderCompositorNodeInputs& inputs)
	{
		const RendererViewProperties& viewProps = inputs.view.GetProperties();

		RCNodeResolvedSceneDepth* resolvedSceneDepth = static_cast<RCNodeResolvedSceneDepth*>(inputs.inputNodes[0]);

		UINT32 width = viewProps.target.viewRect.width;
		UINT32 height = viewProps.target.viewRect.height;

		UINT32 size = Bitwise::NextPow2(std::max(width, height));
		UINT32 numMips = PixelUtil::GetMaxMipmaps(size, size, 1, PF_R32F);
		size = 1 << numMips;

		// Note: Use the 32-bit buffer here as 16-bit causes too much banding (most of the scene gets assigned 4-5 different
		// depth values).
		//  - When I add UNORM 16-bit format I should be able to switch to that
		output = gGpuResourcePool().Get(
			POOLED_RENDER_TEXTURE_DESC::Create2D(PF_R32F, size, size, TU_RENDERTARGET, 1, false, 1, numMips));

		Rect2 srcRect = viewProps.target.nrmViewRect;

		// If viewport size is odd, adjust UV
		srcRect.width += (viewProps.target.viewRect.width % 2) * (1.0f / viewProps.target.viewRect.width);
		srcRect.height += (viewProps.target.viewRect.height % 2) * (1.0f / viewProps.target.viewRect.height);

		bool noTextureViews = !gCaps().HasCapability(RSC_TEXTURE_VIEWS);

		BuildHiZMat* material = BuildHiZMat::GetVariation(noTextureViews);

		// Generate first mip
		RENDER_TEXTURE_DESC rtDesc;
		rtDesc.colorSurfaces[0].texture = output->texture;
		rtDesc.colorSurfaces[0].mipLevel = 0;

		SPtr<RenderTexture> rt = RenderTexture::Create(rtDesc);

		Rect2 destRect;
		bool downsampledFirstMip = false; // Not used currently
		if (downsampledFirstMip)
		{
			// Make sure that 1 pixel in HiZ maps to a 2x2 block in source
			destRect = Rect2(0, 0,
				Math::CeilToInt(viewProps.target.viewRect.width / 2.0f) / (float)size,
				Math::CeilToInt(viewProps.target.viewRect.height / 2.0f) / (float)size);

			material->Execute(resolvedSceneDepth->output->texture, 0, srcRect, destRect, rt);
		}
		else // First level is just a copy of the depth buffer
		{
			destRect = Rect2(0, 0,
				viewProps.target.viewRect.width / (float)size,
				viewProps.target.viewRect.height / (float)size);

			RenderAPI& rapi = RenderAPI::Instance();
			rapi.SetRenderTarget(rt);
			rapi.SetViewport(destRect);

			Rect2I srcAreaInt;
			srcAreaInt.x = (INT32)(srcRect.x * viewProps.target.viewRect.width);
			srcAreaInt.y = (INT32)(srcRect.y * viewProps.target.viewRect.height);
			srcAreaInt.width = (UINT32)(srcRect.width * viewProps.target.viewRect.width);
			srcAreaInt.height = (UINT32)(srcRect.height * viewProps.target.viewRect.height);

			gRendererUtility().Blit(resolvedSceneDepth->output->texture, srcAreaInt);
			rapi.SetViewport(Rect2(0, 0, 1, 1));
		}

		// Generate remaining mip levels
		for(UINT32 i = 1; i <= numMips; i++)
		{
			rtDesc.colorSurfaces[0].mipLevel = i;
			rt = RenderTexture::Create(rtDesc);

			material->Execute(output->texture, i - 1, destRect, destRect, rt);
		}
	}

	void RCNodeHiZ::Clear()
	{
		output = nullptr;
	}

	SmallVector<StringID, 4> RCNodeHiZ::GetDependencies(const RendererView& view)
	{
		// Note: This doesn't actually use any gbuffer textures, but node is a dependency because it renders to the depth
		// buffer. In order to avoid keeping gbuffer textures alive I could separate out the base pass into its own node
		// perhaps. But at the moment it doesn't matter, as anything using HiZ also needs gbuffer.
		return { RCNodeResolvedSceneDepth::GetNodeId(), RCNodeBasePass::GetNodeId() };
	}

	void RCNodeSSAO::Render(const RenderCompositorNodeInputs& inputs)
	{
		/** Maximum valid depth range within samples in a sample set. In meters. */
		static const float DEPTH_RANGE = 1.0f;

		const AmbientOcclusionSettings& settings = inputs.view.GetRenderSettings().ambientOcclusion;
		if(!settings.enabled)
		{
			output = Texture::WHITE;
			mPooledOutput = nullptr;
			return;
		}

		GpuResourcePool& resPool = gGpuResourcePool();
		const RendererViewProperties& viewProps = inputs.view.GetProperties();

		RCNodeResolvedSceneDepth* resolvedDepthNode = static_cast<RCNodeResolvedSceneDepth*>(inputs.inputNodes[0]);
		RCNodeBasePass* gbufferNode = static_cast<RCNodeBasePass*>(inputs.inputNodes[1]);

		SPtr<Texture> sceneDepth = resolvedDepthNode->output->texture;
		SPtr<Texture> sceneNormals = gbufferNode->normalTex->texture;

		const TextureProperties& normalsProps = sceneNormals->GetProperties();
		SPtr<PooledRenderTexture> resolvedNormals;

		RenderAPI& rapi = RenderAPI::Instance();
		if(sceneNormals->GetProperties().GetNumSamples() > 1)
		{
			POOLED_RENDER_TEXTURE_DESC desc = POOLED_RENDER_TEXTURE_DESC::Create2D(normalsProps.GetFormat(),
				normalsProps.GetWidth(), normalsProps.GetHeight(), TU_RENDERTARGET);
			resolvedNormals = resPool.Get(desc);

			rapi.SetRenderTarget(resolvedNormals->renderTexture);
			gRendererUtility().Blit(sceneNormals);

			sceneNormals = resolvedNormals->texture;
		}

		// Multiple downsampled AO levels are used to minimize cache trashing. Downsampled AO targets use larger radius,
		// whose contents are then blended with the higher level.
		UINT32 quality = settings.quality;
		UINT32 numDownsampleLevels = 0;
		if (quality == 2)
			numDownsampleLevels = 1;
		else if (quality > 2)
			numDownsampleLevels = 2;

		SSAODownsampleMat* downsample = SSAODownsampleMat::Get();

		SPtr<PooledRenderTexture> setupTex0;
		if(numDownsampleLevels > 0)
		{
			Vector2I downsampledSize(
				std::max(1, Math::DivideAndRoundUp((INT32)viewProps.target.viewRect.width, 2)),
				std::max(1, Math::DivideAndRoundUp((INT32)viewProps.target.viewRect.height, 2))
			);

			POOLED_RENDER_TEXTURE_DESC desc = POOLED_RENDER_TEXTURE_DESC::Create2D(PF_RGBA16F, downsampledSize.x,
				downsampledSize.y, TU_RENDERTARGET);
			setupTex0 = resPool.Get(desc);

			downsample->Execute(inputs.view, sceneDepth, sceneNormals, setupTex0->renderTexture, DEPTH_RANGE);
		}

		SPtr<PooledRenderTexture> setupTex1;
		if(numDownsampleLevels > 1)
		{
			Vector2I downsampledSize(
				std::max(1, Math::DivideAndRoundUp((INT32)viewProps.target.viewRect.width, 4)),
				std::max(1, Math::DivideAndRoundUp((INT32)viewProps.target.viewRect.height, 4))
			);

			POOLED_RENDER_TEXTURE_DESC desc = POOLED_RENDER_TEXTURE_DESC::Create2D(PF_RGBA16F, downsampledSize.x,
				downsampledSize.y, TU_RENDERTARGET);
			setupTex1 = resPool.Get(desc);

			downsample->Execute(inputs.view, sceneDepth, sceneNormals, setupTex1->renderTexture, DEPTH_RANGE);
		}

		SSAOTextureInputs textures;
		textures.sceneDepth = sceneDepth;
		textures.sceneNormals = sceneNormals;
		textures.randomRotations = RendererTextures::ssaoRandomization4x4;

		SPtr<PooledRenderTexture> downAOTex1;
		if(numDownsampleLevels > 1)
		{
			textures.aoSetup = setupTex1->texture;

			Vector2I downsampledSize(
				std::max(1, Math::DivideAndRoundUp((INT32)viewProps.target.viewRect.width, 4)),
				std::max(1, Math::DivideAndRoundUp((INT32)viewProps.target.viewRect.height, 4))
			);

			POOLED_RENDER_TEXTURE_DESC desc = POOLED_RENDER_TEXTURE_DESC::Create2D(PF_R8, downsampledSize.x,
				downsampledSize.y, TU_RENDERTARGET);
			downAOTex1 = resPool.Get(desc);

			SSAOMat* ssaoMat = SSAOMat::GetVariation(false, false, quality);
			ssaoMat->Execute(inputs.view, textures, downAOTex1->renderTexture, settings);

			setupTex1 = nullptr;
		}

		SPtr<PooledRenderTexture> downAOTex0;
		if(numDownsampleLevels > 0)
		{
			textures.aoSetup = setupTex0->texture;

			if(downAOTex1)
				textures.aoDownsampled = downAOTex1->texture;

			Vector2I downsampledSize(
				std::max(1, Math::DivideAndRoundUp((INT32)viewProps.target.viewRect.width, 2)),
				std::max(1, Math::DivideAndRoundUp((INT32)viewProps.target.viewRect.height, 2))
			);

			POOLED_RENDER_TEXTURE_DESC desc = POOLED_RENDER_TEXTURE_DESC::Create2D(PF_R8, downsampledSize.x,
				downsampledSize.y, TU_RENDERTARGET);
			downAOTex0 = resPool.Get(desc);

			bool upsample = numDownsampleLevels > 1;
			SSAOMat* ssaoMat = SSAOMat::GetVariation(upsample, false, quality);
			ssaoMat->Execute(inputs.view, textures, downAOTex0->renderTexture, settings);

			if(upsample)
				downAOTex1 = nullptr;
		}

		UINT32 width = viewProps.target.viewRect.width;
		UINT32 height = viewProps.target.viewRect.height;
		mPooledOutput = resPool.Get(POOLED_RENDER_TEXTURE_DESC::Create2D(PF_R8, width, height, TU_RENDERTARGET));

		{
			if(setupTex0)
				textures.aoSetup = setupTex0->texture;

			if(downAOTex0)
				textures.aoDownsampled = downAOTex0->texture;

			bool upsample = numDownsampleLevels > 0;
			SSAOMat* ssaoMat = SSAOMat::GetVariation(upsample, true, quality);
			ssaoMat->Execute(inputs.view, textures, mPooledOutput->renderTexture, settings);
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
			const RenderTargetProperties& rtProps = mPooledOutput->renderTexture->GetProperties();

			POOLED_RENDER_TEXTURE_DESC desc = POOLED_RENDER_TEXTURE_DESC::Create2D(PF_R8, rtProps.width,
				rtProps.height, TU_RENDERTARGET);
			SPtr<PooledRenderTexture> blurIntermediateTex = resPool.Get(desc);

			SSAOBlurMat* blurHorz = SSAOBlurMat::GetVariation(true);
			SSAOBlurMat* blurVert = SSAOBlurMat::GetVariation(false);

			blurHorz->Execute(inputs.view, mPooledOutput->texture, sceneDepth, blurIntermediateTex->renderTexture, DEPTH_RANGE);
			blurVert->Execute(inputs.view, blurIntermediateTex->texture, sceneDepth, mPooledOutput->renderTexture, DEPTH_RANGE);
		}

		RenderAPI::Instance().SetRenderTarget(nullptr);
		output = mPooledOutput->texture;
	}

	void RCNodeSSAO::Clear()
	{
		mPooledOutput = nullptr;
		output = nullptr;
	}

	SmallVector<StringID, 4> RCNodeSSAO::GetDependencies(const RendererView& view)
	{
		return { RCNodeResolvedSceneDepth::GetNodeId(), RCNodeBasePass::GetNodeId() };
	}

	RCNodeSSR::~RCNodeSSR()
	{
		DeallocOutputs();
	}

	void RCNodeSSR::Render(const RenderCompositorNodeInputs& inputs)
	{
		const ScreenSpaceReflectionsSettings& settings = inputs.view.GetRenderSettings().screenSpaceReflections;
		if (!settings.enabled)
		{
			DeallocOutputs();

			mPooledOutput = nullptr;
			output = Texture::BLACK;
			return;
		}

		RenderAPI& rapi = RenderAPI::Instance();

		RCNodeSceneDepth* sceneDepthNode = static_cast<RCNodeSceneDepth*>(inputs.inputNodes[0]);
		RCNodeLightAccumulation* lightAccumNode = static_cast<RCNodeLightAccumulation*>(inputs.inputNodes[1]);
		RCNodeBasePass* gbufferNode = static_cast<RCNodeBasePass*>(inputs.inputNodes[2]);
		RCNodeHiZ* hiZNode = static_cast<RCNodeHiZ*>(inputs.inputNodes[3]);
		RCNodeResolvedSceneDepth* resolvedSceneDepthNode = static_cast<RCNodeResolvedSceneDepth*>(inputs.inputNodes[4]);

		GpuResourcePool& resPool = gGpuResourcePool();
		const RendererViewProperties& viewProps = inputs.view.GetProperties();

		UINT32 width = viewProps.target.viewRect.width;
		UINT32 height = viewProps.target.viewRect.height;

		SPtr<Texture> hiZ = hiZNode->output->texture;

		// This will be executing before scene color is resolved, so get the light accum buffer instead
		SPtr<Texture> sceneColor = lightAccumNode->lightAccumulationTex->texture;

		// Resolve multiple samples if MSAA is used
		SPtr<PooledRenderTexture> resolvedSceneColor;
		if (viewProps.target.numSamples > 1)
		{
			resolvedSceneColor = resPool.Get(POOLED_RENDER_TEXTURE_DESC::Create2D(PF_RGBA16F, width, height,
				TU_RENDERTARGET));

			rapi.SetRenderTarget(resolvedSceneColor->renderTexture);
			gRendererUtility().Blit(sceneColor);

			sceneColor = resolvedSceneColor->texture;
		}

		GBufferTextures gbuffer;
		gbuffer.albedo = gbufferNode->albedoTex->texture;
		gbuffer.normals = gbufferNode->normalTex->texture;
		gbuffer.roughMetal = gbufferNode->roughMetalTex->texture;
		gbuffer.depth = sceneDepthNode->depthTex->texture;

		SSRStencilMat* stencilMat = SSRStencilMat::GetVariation(viewProps.target.numSamples > 1, true);

		// Note: Making the assumption that the stencil buffer is clear at this point
		rapi.SetRenderTarget(resolvedSceneDepthNode->output->renderTexture, FBT_DEPTH, RT_DEPTH_STENCIL);
		stencilMat->Execute(inputs.view, gbuffer, settings);

		SPtr<PooledRenderTexture> traceOutput = resPool.Get(POOLED_RENDER_TEXTURE_DESC::Create2D(PF_RGBA16F, width,
			height, TU_RENDERTARGET));

		RENDER_TEXTURE_DESC traceRtDesc;
		traceRtDesc.colorSurfaces[0].texture = traceOutput->texture;
		traceRtDesc.depthStencilSurface.texture = resolvedSceneDepthNode->output->texture;

		SPtr<RenderTexture> traceRt = RenderTexture::Create(traceRtDesc);

		rapi.SetRenderTarget(traceRt, FBT_DEPTH | FBT_STENCIL, RT_DEPTH_STENCIL);
		rapi.ClearRenderTarget(FBT_COLOR, Color::ZERO);

		SSRTraceMat* traceMat = SSRTraceMat::GetVariation(settings.quality, viewProps.target.numSamples > 1, true);
		traceMat->Execute(inputs.view, gbuffer, sceneColor, hiZ, settings, traceRt);

		resolvedSceneColor = nullptr;

		mUsingTemporalAA = inputs.view.GetRenderSettings().temporalAA.enabled;
		if (mPrevFrame && !mUsingTemporalAA)
		{
			mPooledOutput = resPool.Get(POOLED_RENDER_TEXTURE_DESC::Create2D(PF_RGBA16F, width, height, TU_RENDERTARGET));

			rapi.SetRenderTarget(mPooledOutput->renderTexture);
			rapi.ClearRenderTarget(FBT_COLOR);

			TemporalFilteringMat* temporalFilteringMat =
				TemporalFilteringMat::GetVariation(TemporalFilteringType::SSR, false, viewProps.target.numSamples > 1);
			temporalFilteringMat->Execute(inputs.view, mPrevFrame->texture, traceOutput->texture, nullptr,
				sceneDepthNode->depthTex->texture, Vector2::ZERO, 1.0f, mPooledOutput->renderTexture);

			traceOutput = nullptr;
		}
		else
			mPooledOutput = traceOutput;

		RenderAPI::Instance().SetRenderTarget(nullptr);
		output = mPooledOutput->texture;
	}

	void RCNodeSSR::Clear()
	{
		if(!mUsingTemporalAA)
			mPrevFrame = mPooledOutput;

		mPooledOutput = nullptr;
		output = nullptr;
	}

	void RCNodeSSR::DeallocOutputs()
	{
		mPrevFrame = nullptr;
		output = nullptr;
	}

	SmallVector<StringID, 4> RCNodeSSR::GetDependencies(const RendererView& view)
	{
		SmallVector<StringID, 4> deps;
		if (view.GetRenderSettings().screenSpaceReflections.enabled)
		{
			deps.Add(RCNodeSceneDepth::GetNodeId());
			deps.Add(RCNodeLightAccumulation::GetNodeId());
			deps.Add(RCNodeBasePass::GetNodeId());
			deps.Add(RCNodeHiZ::GetNodeId());
			deps.Add(RCNodeResolvedSceneDepth::GetNodeId());
			deps.Add(RCNodeIndirectDiffuseLighting::GetNodeId());
		}

		return deps;
	}

	void RCNodeBloom::Render(const RenderCompositorNodeInputs& inputs)
	{
		const RenderSettings& settings = inputs.view.GetRenderSettings();

		// Grab downsampled scene color to use as input
		auto* sceneDownsamplesNode = static_cast<RCNodeSceneColorDownsamples*>(inputs.inputNodes[1]);

		constexpr UINT32 PREFERRED_NUM_DOWNSAMPLE_LEVELS = 6;
		const UINT32 availableDownsamples = sceneDownsamplesNode->availableDownsamples;
		const UINT32 numDownsamples = Math::Min(availableDownsamples, PREFERRED_NUM_DOWNSAMPLE_LEVELS);
		assert(numDownsamples >= 1);

		// Blur & clip the downsampled entries and add them together
		const UINT32 quality = Math::Clamp(settings.bloom.quality, 0U, 3U);
		constexpr UINT32 NUM_STEPS_PER_QUALITY[] = { 3, 4, 5, 6  };

		GaussianBlurMat* filterMat = GaussianBlurMat::GetVariation(true);

		const bool autoExposure = settings.enableHDR && settings.enableAutoExposure;
		BloomClipMat* clipMat = BloomClipMat::GetVariation(autoExposure);

		SPtr<Texture> eyeAdaptationTex = nullptr;
		if (autoExposure)
		{
			auto* eyeAdapatationNode = static_cast<RCNodeEyeAdaptation*>(inputs.inputNodes[2]);

			if (eyeAdapatationNode->output)
				eyeAdaptationTex = eyeAdapatationNode->output->texture;
		}

		const UINT32 numSteps = NUM_STEPS_PER_QUALITY[quality];
		SPtr<PooledRenderTexture> prevOutput;
		for(UINT32 i = 0; i < numSteps; i++)
		{
			const UINT32 srcIdx = numDownsamples - i - 1;
			const SPtr<PooledRenderTexture> downsampledTex = sceneDownsamplesNode->output[srcIdx];

			const TextureProperties& inputProps = downsampledTex->texture->GetProperties();

			SPtr<PooledRenderTexture> filterOutput = gGpuResourcePool().Get(
				POOLED_RENDER_TEXTURE_DESC::Create2D(
					inputProps.GetFormat(),
					inputProps.GetWidth(),
					inputProps.GetHeight(),
					TU_RENDERTARGET)
			);

			SPtr<PooledRenderTexture> blurInput = downsampledTex;
			SPtr<PooledRenderTexture> blurOutput = filterOutput;
			if(settings.bloom.threshold > 0.0f)
			{
				clipMat->Execute(downsampledTex->texture, settings.bloom.threshold, eyeAdaptationTex, settings,
					filterOutput->renderTexture);

				blurOutput = blurInput;
				blurInput = filterOutput;
			}

			SPtr<Texture> additiveInput;
			if(prevOutput)
				additiveInput = prevOutput->texture;

			const Color tint = Color::White * (settings.bloom.intensity / (float)numSteps);
			filterMat->Execute(blurInput->texture, settings.bloom.filterSize, blurOutput->renderTexture,
				tint, additiveInput);
			prevOutput = blurOutput;
		}

		mPooledOutput = prevOutput;
		output = mPooledOutput->texture;
	}

	void RCNodeBloom::Clear()
	{
		mPooledOutput = nullptr;
		output = nullptr;
	}

	SmallVector<StringID, 4> RCNodeBloom::GetDependencies(const RendererView& view)
	{
		return
		{
			RCNodeClusteredForward::GetNodeId(),
			RCNodeSceneColorDownsamples::GetNodeId(),
			RCNodeEyeAdaptation::GetNodeId()
		};
	}

	void RCNodeScreenSpaceLensFlare::Render(const RenderCompositorNodeInputs& inputs)
	{
		GpuResourcePool& resPool = GpuResourcePool::Instance();
		const RenderSettings& settings = inputs.view.GetRenderSettings();
		const ScreenSpaceLensFlareSettings& lensFlareSettings = settings.screenSpaceLensFlare;

		// Grab downsampled scene color to use as input
		auto* sceneDownsamplesNode = static_cast<RCNodeSceneColorDownsamples*>(inputs.inputNodes[2]);

		const UINT32 availableDownsamples = sceneDownsamplesNode->availableDownsamples;
		const UINT32 numDownsamples = Math::Clamp(settings.screenSpaceLensFlare.downsampleCount, 1U, availableDownsamples);
		assert(numDownsamples >= 1);

		SPtr<PooledRenderTexture> downsampledTex = sceneDownsamplesNode->output[numDownsamples - 1];
		const TextureProperties& sceneTexProps = downsampledTex->texture->GetProperties();

		// Ghost features
		SPtr<PooledRenderTexture> featureTex = resPool.Get(
			POOLED_RENDER_TEXTURE_DESC::Create2D(
				sceneTexProps.GetFormat(),
				sceneTexProps.GetWidth(),
				sceneTexProps.GetHeight(),
				TU_RENDERTARGET));

		bool haloAspect = lensFlareSettings.haloAspectRatio != 1.0f;
		ScreenSpaceLensFlareMat* lensFlareMat = ScreenSpaceLensFlareMat::GetVariation(
			lensFlareSettings.halo,
			haloAspect,
			lensFlareSettings.chromaticAberration);
		lensFlareMat->Execute(downsampledTex->texture, lensFlareSettings, featureTex->renderTexture);

		// Blur
		GaussianBlurMat* filterMat = GaussianBlurMat::Get();
		filterMat->Execute(featureTex->texture, lensFlareSettings.filterSize, downsampledTex->renderTexture, Color::White);

		auto* sceneColorNode = static_cast<RCNodeSceneColor*>(inputs.inputNodes[1]);

		if(lensFlareSettings.bicubicUpsampling)
		{
			BicubicUpsampleMat* upsampleMat = BicubicUpsampleMat::GetVariation(false);
			upsampleMat->Execute(
				downsampledTex->texture,
				sceneColorNode->renderTarget,
				Color::White * lensFlareSettings.brightness);
		}
		else
		{
			CompositeMat* upsampleMat = CompositeMat::Get();
			upsampleMat->Execute(
				downsampledTex->texture,
				sceneColorNode->renderTarget,
				Color::White * lensFlareSettings.brightness);
		}
	}

	void RCNodeScreenSpaceLensFlare::Clear()
	{
		// Do nothing
	}

	SmallVector<StringID, 4> RCNodeScreenSpaceLensFlare::GetDependencies(const RendererView& view)
	{
		return
		{
			RCNodeClusteredForward::GetNodeId(),
			RCNodeSceneColor::GetNodeId(),
			RCNodeSceneColorDownsamples::GetNodeId()
		};
	}
}}
