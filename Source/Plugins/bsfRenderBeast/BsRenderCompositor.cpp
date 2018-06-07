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
#include "Utility/BsGpuResourcePool.h"
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

namespace bs { namespace ct
{
	UnorderedMap<StringID, RenderCompositor::NodeType*> RenderCompositor::mNodeTypes;

	RenderCompositor::~RenderCompositor()
	{
		clear();
	}

	void RenderCompositor::build(const RendererView& view, const StringID& finalNode)
	{
		clear();

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
					LOGERR("Cannot find render compositor node of type \"" + String(nodeId.cstr()) + "\".");
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
				SmallVector<StringID, 4> depIds = nodeType->getDependencies(view);
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
					nodeInfo.node = nodeType->create();
					nodeInfo.lastUseIdx = -1;

					for (auto& depId : depIds)
					{
						iterFind2 = processedNodes.find(depId);

						NodeInfo& depNodeInfo = mNodeInfos[iterFind2->second];
						nodeInfo.inputs.push_back(depNodeInfo.node);
					}
				}
				else // Existing node
				{
					curIdx = iterFind2->second;

					// Check if invalid
					if (curIdx == (UINT32)-1)
					{
						LOGERR("Render compositor nodes recursion detected. Node \"" + String(nodeId.cstr()) + "\" " +
							"depends on node \"" + String(iterFind->first.cstr()) + "\" which is not available at " +
							"this stage.");
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
				clear();
		}
		bs_frame_clear();
	}

	void RenderCompositor::execute(RenderCompositorNodeInputs& inputs) const
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
				entry.node->render(inputs);

				activeNodes.push_back(&entry);

				for (UINT32 i = 0; i < (UINT32)activeNodes.size(); ++i)
				{
					if (activeNodes[i] == nullptr)
						continue;

					if (activeNodes[i]->lastUseIdx <= idx)
					{
						activeNodes[i]->node->clear();
						activeNodes[i] = nullptr;
					}
				}

				idx++;
			}
		}
		bs_frame_clear();

		if (!mNodeInfos.empty())
			mNodeInfos.back().node->clear();
	}

	void RenderCompositor::clear()
	{
		for (auto& entry : mNodeInfos)
			bs_delete(entry.node);

		mNodeInfos.clear();
		mIsValid = false;
	}

	void RCNodeSceneDepth::render(const RenderCompositorNodeInputs& inputs)
	{
		GpuResourcePool& resPool = GpuResourcePool::instance();
		const RendererViewProperties& viewProps = inputs.view.getProperties();

		UINT32 width = viewProps.viewRect.width;
		UINT32 height = viewProps.viewRect.height;
		UINT32 numSamples = viewProps.numSamples;

		depthTex = resPool.get(POOLED_RENDER_TEXTURE_DESC::create2D(PF_D32_S8X24, width, height, TU_DEPTHSTENCIL,
			numSamples, false));
	}

	void RCNodeSceneDepth::clear()
	{
		GpuResourcePool& resPool = GpuResourcePool::instance();
		resPool.release(depthTex);
	}

	SmallVector<StringID, 4> RCNodeSceneDepth::getDependencies(const RendererView& view)
	{
		return {};
	}

	void RCNodeGBuffer::render(const RenderCompositorNodeInputs& inputs)
	{
		// Allocate necessary textures & targets
		GpuResourcePool& resPool = GpuResourcePool::instance();
		const RendererViewProperties& viewProps = inputs.view.getProperties();

		UINT32 width = viewProps.viewRect.width;
		UINT32 height = viewProps.viewRect.height;
		UINT32 numSamples = viewProps.numSamples;

		// Note: Consider customizable formats. e.g. for testing if quality can be improved with higher precision normals.
		albedoTex = resPool.get(POOLED_RENDER_TEXTURE_DESC::create2D(PF_RGBA8, width, height, TU_RENDERTARGET,
			numSamples, true));
		normalTex = resPool.get(POOLED_RENDER_TEXTURE_DESC::create2D(PF_RGB10A2, width, height, TU_RENDERTARGET,
			numSamples, false));
		roughMetalTex = resPool.get(POOLED_RENDER_TEXTURE_DESC::create2D(PF_RG16F, width, height, TU_RENDERTARGET,
			numSamples, false)); // Note: Metal doesn't need 16-bit float

		RCNodeSceneDepth* sceneDepthNode = static_cast<RCNodeSceneDepth*>(inputs.inputNodes[0]);
		SPtr<PooledRenderTexture> sceneDepthTex = sceneDepthNode->depthTex;

		bool rebuildRT = false;
		if (renderTarget != nullptr)
		{
			rebuildRT |= renderTarget->getColorTexture(0) != albedoTex->texture;
			rebuildRT |= renderTarget->getColorTexture(1) != normalTex->texture;
			rebuildRT |= renderTarget->getColorTexture(2) != roughMetalTex->texture;
			rebuildRT |= renderTarget->getDepthStencilTexture() != sceneDepthTex->texture;
		}
		else
			rebuildRT = true;

		if (renderTarget == nullptr || rebuildRT)
		{
			RENDER_TEXTURE_DESC gbufferDesc;
			gbufferDesc.colorSurfaces[0].texture = albedoTex->texture;
			gbufferDesc.colorSurfaces[0].face = 0;
			gbufferDesc.colorSurfaces[0].numFaces = 1;
			gbufferDesc.colorSurfaces[0].mipLevel = 0;

			gbufferDesc.colorSurfaces[1].texture = normalTex->texture;
			gbufferDesc.colorSurfaces[1].face = 0;
			gbufferDesc.colorSurfaces[1].numFaces = 1;
			gbufferDesc.colorSurfaces[1].mipLevel = 0;

			gbufferDesc.colorSurfaces[2].texture = roughMetalTex->texture;
			gbufferDesc.colorSurfaces[2].face = 0;
			gbufferDesc.colorSurfaces[2].numFaces = 1;
			gbufferDesc.colorSurfaces[2].mipLevel = 0;

			gbufferDesc.depthStencilSurface.texture = sceneDepthTex->texture;
			gbufferDesc.depthStencilSurface.face = 0;
			gbufferDesc.depthStencilSurface.mipLevel = 0;

			renderTarget = RenderTexture::create(gbufferDesc);
		}

		// Prepare all visible objects. Note that this also prepares non-opaque objects.
		const VisibilityInfo& visibility = inputs.view.getVisibilityMasks();
		UINT32 numRenderables = (UINT32)inputs.scene.renderables.size();
		for (UINT32 i = 0; i < numRenderables; i++)
		{
			if (!visibility.renderables[i])
				continue;

			RendererRenderable* rendererRenderable = inputs.scene.renderables[i];
			rendererRenderable->updatePerCallBuffer(viewProps.viewProjTransform);

			for (auto& element : inputs.scene.renderables[i]->elements)
			{
				SPtr<GpuParams> gpuParams = element.params->getGpuParams();
				for(UINT32 j = 0; j < GPT_COUNT; j++)
				{
					const GpuParamBinding& binding = element.perCameraBindings[j];
					if(binding.slot != (UINT32)-1)
						gpuParams->setParamBlockBuffer(binding.set, binding.slot, inputs.view.getPerViewBuffer());
				}
			}
		}

		Camera* sceneCamera = inputs.view.getSceneCamera();

		// Trigger prepare callbacks
		if (sceneCamera != nullptr)
		{
			for(auto& extension : inputs.extPrepare)
			{
				if (extension->check(*sceneCamera))
					extension->render(*sceneCamera);
			}
		}

		// Render base pass
		RenderAPI& rapi = RenderAPI::instance();
		rapi.setRenderTarget(renderTarget);

		Rect2 area(0.0f, 0.0f, 1.0f, 1.0f);
		rapi.setViewport(area);

		// Clear all targets
		rapi.clearViewport(FBT_COLOR | FBT_DEPTH | FBT_STENCIL, Color::ZERO, 1.0f, 0);

		// Trigger pre-base-pass callbacks
		if (sceneCamera != nullptr)
		{
			for(auto& extension : inputs.extPreBasePass)
			{
				if (extension->check(*sceneCamera))
					extension->render(*sceneCamera);
			}
		}

		// Render all visible opaque elements that use the deferred pipeline
		const Vector<RenderQueueElement>& opaqueElements = inputs.view.getOpaqueQueue(false)->getSortedElements();
		for (auto iter = opaqueElements.begin(); iter != opaqueElements.end(); ++iter)
		{
			const RenderableElement* renderElem = static_cast<const RenderableElement*>(iter->renderElem);

			SPtr<Material> material = renderElem->material;

			if (iter->applyPass)
				gRendererUtility().setPass(material, iter->passIdx, renderElem->techniqueIdx);

			gRendererUtility().setPassParams(renderElem->params, iter->passIdx);

			if(renderElem->morphVertexDeclaration == nullptr)
				gRendererUtility().draw(renderElem->mesh, renderElem->subMesh);
			else
				gRendererUtility().drawMorph(renderElem->mesh, renderElem->subMesh, renderElem->morphShapeBuffer, 
					renderElem->morphVertexDeclaration);
		}

		// Make sure that any compute shaders are able to read g-buffer by unbinding it
		rapi.setRenderTarget(nullptr);

		// Trigger post-base-pass callbacks
		if (sceneCamera != nullptr)
		{
			for(auto& extension : inputs.extPostBasePass)
			{
				if (extension->check(*sceneCamera))
					extension->render(*sceneCamera);
			}
		}
	}

	void RCNodeGBuffer::clear()
	{
		GpuResourcePool& resPool = GpuResourcePool::instance();

		resPool.release(albedoTex);
		resPool.release(normalTex);
		resPool.release(roughMetalTex);
	}

	SmallVector<StringID, 4> RCNodeGBuffer::getDependencies(const RendererView& view)
	{
		return { RCNodeSceneDepth::getNodeId() };
	}

	void RCNodeSceneColor::render(const RenderCompositorNodeInputs& inputs)
	{
		GpuResourcePool& resPool = GpuResourcePool::instance();
		const RendererViewProperties& viewProps = inputs.view.getProperties();

		UINT32 width = viewProps.viewRect.width;
		UINT32 height = viewProps.viewRect.height;
		UINT32 numSamples = viewProps.numSamples;

		UINT32 usageFlags = TU_RENDERTARGET;

		bool tiledDeferredSupported = inputs.featureSet != RenderBeastFeatureSet::DesktopMacOS;
		if(tiledDeferredSupported)
			usageFlags |= TU_LOADSTORE;

		// Note: Consider customizable HDR format via options? e.g. smaller PF_FLOAT_R11G11B10 or larger 32-bit format
		sceneColorTex = resPool.get(POOLED_RENDER_TEXTURE_DESC::create2D(PF_RGBA16F, width, height, usageFlags, 
			numSamples, false));

		RCNodeSceneDepth* sceneDepthNode = static_cast<RCNodeSceneDepth*>(inputs.inputNodes[0]);
		SPtr<PooledRenderTexture> sceneDepthTex = sceneDepthNode->depthTex;

		if (tiledDeferredSupported && viewProps.numSamples > 1)
		{
			UINT32 bufferNumElements = width * height * viewProps.numSamples;
			flattenedSceneColorBuffer = resPool.get(POOLED_STORAGE_BUFFER_DESC::createStandard(BF_16X4F, bufferNumElements));
		}
		else
			flattenedSceneColorBuffer = nullptr;

		bool rebuildRT = false;
		if (renderTarget != nullptr)
		{
			rebuildRT |= renderTarget->getColorTexture(0) != sceneColorTex->texture;
			rebuildRT |= renderTarget->getDepthStencilTexture() != sceneDepthTex->texture;
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

			renderTarget = RenderTexture::create(sceneColorDesc);
		}
	}

	void RCNodeSceneColor::clear()
	{
		GpuResourcePool& resPool = GpuResourcePool::instance();
		resPool.release(sceneColorTex);

		if (flattenedSceneColorBuffer != nullptr)
			resPool.release(flattenedSceneColorBuffer);
	}

	void RCNodeSceneColor::unflatten()
	{
		FlatFramebufferToTextureMat* material = FlatFramebufferToTextureMat::get();

		int readOnlyFlags = FBT_DEPTH | FBT_STENCIL;

		RenderAPI& rapi = RenderAPI::instance();
		rapi.setRenderTarget(renderTarget, readOnlyFlags, RT_DEPTH_STENCIL);

		Rect2 area(0.0f, 0.0f, 1.0f, 1.0f);
		rapi.setViewport(area);

		material->execute(flattenedSceneColorBuffer->buffer, sceneColorTex->texture);
	}

	SmallVector<StringID, 4> RCNodeSceneColor::getDependencies(const RendererView& view)
	{
		return { RCNodeSceneDepth::getNodeId() };
	}

	void RCNodeMSAACoverage::render(const RenderCompositorNodeInputs& inputs)
	{
		const RendererViewProperties& viewProps = inputs.view.getProperties();
		if(viewProps.numSamples <= 1)
		{
			// No need for MSAA coverage
			output = nullptr;
			return;
		}

		GpuResourcePool& resPool = GpuResourcePool::instance();

		UINT32 width = viewProps.viewRect.width;
		UINT32 height = viewProps.viewRect.height;

		output = resPool.get(POOLED_RENDER_TEXTURE_DESC::create2D(PF_R8, width, height, TU_RENDERTARGET));

		RCNodeGBuffer* gbufferNode = static_cast<RCNodeGBuffer*>(inputs.inputNodes[0]);
		RCNodeSceneDepth* sceneDepthNode = static_cast<RCNodeSceneDepth*>(inputs.inputNodes[1]);

		GBufferTextures gbuffer;
		gbuffer.albedo = gbufferNode->albedoTex->texture;
		gbuffer.normals = gbufferNode->normalTex->texture;
		gbuffer.roughMetal = gbufferNode->roughMetalTex->texture;
		gbuffer.depth = sceneDepthNode->depthTex->texture;

		MSAACoverageMat* mat = MSAACoverageMat::getVariation(viewProps.numSamples);

		RenderAPI& rapi = RenderAPI::instance();
		rapi.setRenderTarget(output->renderTexture);
		mat->execute(inputs.view, gbuffer);

		MSAACoverageStencilMat* stencilMat = MSAACoverageStencilMat::get();
		rapi.setRenderTarget(sceneDepthNode->depthTex->renderTexture);
		stencilMat->execute(inputs.view, output->texture);

		rapi.setRenderTarget(nullptr);
	}

	void RCNodeMSAACoverage::clear()
	{
		if(output)
		{
			GpuResourcePool& resPool = GpuResourcePool::instance();
			resPool.release(output);
		}
	}

	SmallVector<StringID, 4> RCNodeMSAACoverage::getDependencies(const RendererView& view)
	{
		return { RCNodeGBuffer::getNodeId(), RCNodeSceneDepth::getNodeId() };
	}

	void RCNodeLightAccumulation::render(const RenderCompositorNodeInputs& inputs)
	{
		bool supportsTiledDeferred = gRenderBeast()->getFeatureSet() != RenderBeastFeatureSet::DesktopMacOS;
		if(!supportsTiledDeferred)
		{
			// If tiled deferred is not supported, we don't need a separate texture for light accumulation, instead we
			// use scene color directly
			RCNodeSceneColor* sceneColorNode = static_cast<RCNodeSceneColor*>(inputs.inputNodes[0]);
			lightAccumulationTex = sceneColorNode->sceneColorTex;
			renderTarget = sceneColorNode->renderTarget;

			mOwnsTexture = false;
			return;
		}

		GpuResourcePool& resPool = GpuResourcePool::instance();
		const RendererViewProperties& viewProps = inputs.view.getProperties();

		RCNodeSceneDepth* depthNode = static_cast<RCNodeSceneDepth*>(inputs.inputNodes[0]);

		UINT32 width = viewProps.viewRect.width;
		UINT32 height = viewProps.viewRect.height;
		UINT32 numSamples = viewProps.numSamples;
		
		if (numSamples > 1)
		{
			UINT32 bufferNumElements = width * height * numSamples;
			flattenedLightAccumBuffer =
				resPool.get(POOLED_STORAGE_BUFFER_DESC::createStandard(BF_16X4F, bufferNumElements));

			SPtr<GpuBuffer> buffer = flattenedLightAccumBuffer->buffer;
			auto& bufferProps = buffer->getProperties();

			UINT32 bufferSize = bufferProps.getElementSize() * bufferProps.getElementCount();
			UINT16* data = (UINT16*)buffer->lock(0, bufferSize, GBL_WRITE_ONLY_DISCARD);
			{
				memset(data, 0, bufferSize);
			}
			buffer->unlock();
		}
		else
			flattenedLightAccumBuffer = nullptr;

		lightAccumulationTex = resPool.get(POOLED_RENDER_TEXTURE_DESC::create2D(PF_RGBA16F, width,
			height, TU_LOADSTORE | TU_RENDERTARGET, numSamples, false));

		bool rebuildRT;
		if (renderTarget != nullptr)
		{
			rebuildRT = renderTarget->getColorTexture(0) != lightAccumulationTex->texture;
			rebuildRT |= renderTarget->getDepthStencilTexture() != depthNode->depthTex->texture;
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

			renderTarget = RenderTexture::create(lightAccumulationRTDesc);
		}

		mOwnsTexture = true;
	}

	void RCNodeLightAccumulation::unflatten()
	{
		FlatFramebufferToTextureMat* material = FlatFramebufferToTextureMat::get();

		RenderAPI& rapi = RenderAPI::instance();
		rapi.setRenderTarget(renderTarget, FBT_DEPTH | FBT_STENCIL, RT_DEPTH_STENCIL);
		material->execute(flattenedLightAccumBuffer->buffer, lightAccumulationTex->texture);
	}

	void RCNodeLightAccumulation::clear()
	{
		GpuResourcePool& resPool = GpuResourcePool::instance();
		if(mOwnsTexture)
			resPool.release(lightAccumulationTex);
		else
		{
			lightAccumulationTex = nullptr;
			renderTarget = nullptr;
		}

		if (flattenedLightAccumBuffer)
			resPool.release(flattenedLightAccumBuffer);
	}

	SmallVector<StringID, 4> RCNodeLightAccumulation::getDependencies(const RendererView& view)
	{
		SmallVector<StringID, 4> deps;

		bool supportsTiledDeferred = gRenderBeast()->getFeatureSet() != RenderBeastFeatureSet::DesktopMacOS;
		if(!supportsTiledDeferred)
			deps.push_back(RCNodeSceneColor::getNodeId());
		else
			deps.push_back(RCNodeSceneDepth::getNodeId());

		return deps;
	}

	void RCNodeDeferredDirectLighting::render(const RenderCompositorNodeInputs& inputs)
	{
		output = static_cast<RCNodeLightAccumulation*>(inputs.inputNodes[0]);

		RCNodeGBuffer* gbufferNode = static_cast<RCNodeGBuffer*>(inputs.inputNodes[1]);
		RCNodeSceneDepth* sceneDepthNode = static_cast<RCNodeSceneDepth*>(inputs.inputNodes[2]);

		GBufferTextures gbuffer;
		gbuffer.albedo = gbufferNode->albedoTex->texture;
		gbuffer.normals = gbufferNode->normalTex->texture;
		gbuffer.roughMetal = gbufferNode->roughMetalTex->texture;
		gbuffer.depth = sceneDepthNode->depthTex->texture;

		const RendererViewProperties& viewProps = inputs.view.getProperties();

		if (!inputs.view.getRenderSettings().enableShadows)
			mLightOcclusionRT = nullptr;

		bool tiledDeferredSupported = inputs.featureSet != RenderBeastFeatureSet::DesktopMacOS;
		if(tiledDeferredSupported)
		{
			SPtr<Texture> msaaCoverage;
			if(viewProps.numSamples > 1)
			{
				RCNodeMSAACoverage* coverageNode = static_cast<RCNodeMSAACoverage*>(inputs.inputNodes[3]);
				msaaCoverage = coverageNode->output->texture;
			}

			TiledDeferredLightingMat* tiledDeferredMat = TiledDeferredLightingMat::getVariation(viewProps.numSamples);

			const VisibleLightData& lightData = inputs.viewGroup.getVisibleLightData();
			SPtr<GpuBuffer> flattenedLightAccumBuffer;
			if (output->flattenedLightAccumBuffer)
				flattenedLightAccumBuffer = output->flattenedLightAccumBuffer->buffer;

			tiledDeferredMat->execute(inputs.view, lightData, gbuffer, output->lightAccumulationTex->texture, 
				flattenedLightAccumBuffer, msaaCoverage);

			if (viewProps.numSamples > 1)
				output->unflatten();

			// If shadows are disabled we handle all lights through tiled deferred so we can exit immediately
			if (!inputs.view.getRenderSettings().enableShadows)
				return;
		}

		// Standard deferred used for shadowed lights, or when tiled deferred isn't supported
		GpuResourcePool& resPool = GpuResourcePool::instance();

		UINT32 width = viewProps.viewRect.width;
		UINT32 height = viewProps.viewRect.height;
		UINT32 numSamples = viewProps.numSamples;

		const VisibleLightData& lightData = inputs.viewGroup.getVisibleLightData();

		RenderAPI& rapi = RenderAPI::instance();

		// Render unshadowed lights
		if(!tiledDeferredSupported)
		{
			rapi.setRenderTarget(output->renderTarget, FBT_DEPTH | FBT_STENCIL, RT_DEPTH_STENCIL);
			rapi.clearRenderTarget(FBT_COLOR, Color::ZERO);

			for (UINT32 i = 0; i < (UINT32)LightType::Count; i++)
			{
				LightType lightType = (LightType)i;

				auto& lights = lightData.getLights(lightType);
				UINT32 count = lightData.getNumUnshadowedLights(lightType);

				for (UINT32 j = 0; j < count; j++)
				{
					UINT32 lightIdx = j;
					const RendererLight& light = *lights[lightIdx];

					StandardDeferred::instance().renderLight(lightType, light, inputs.view, gbuffer, Texture::BLACK);
				}
			}
		}

		// Allocate light occlusion
		SPtr<PooledRenderTexture> lightOcclusionTex = resPool.get(POOLED_RENDER_TEXTURE_DESC::create2D(PF_R8, width,
			height, TU_RENDERTARGET, numSamples, false));

		bool rebuildRT = false;
		if (mLightOcclusionRT != nullptr)
		{
			rebuildRT |= mLightOcclusionRT->getColorTexture(0) != lightOcclusionTex->texture;
			rebuildRT |= mLightOcclusionRT->getDepthStencilTexture() != sceneDepthNode->depthTex->texture;
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

			mLightOcclusionRT = RenderTexture::create(lightOcclusionRTDesc);
		}

		// Render shadowed lights
		const ShadowRendering& shadowRenderer = inputs.viewGroup.getShadowRenderer();
		for (UINT32 i = 0; i < (UINT32)LightType::Count; i++)
		{
			LightType lightType = (LightType)i;

			auto& lights = lightData.getLights(lightType);
			UINT32 count = lightData.getNumShadowedLights(lightType);
			UINT32 offset = lightData.getNumUnshadowedLights(lightType);

			for (UINT32 j = 0; j < count; j++)
			{
				rapi.setRenderTarget(mLightOcclusionRT, FBT_DEPTH, RT_DEPTH_STENCIL);

				Rect2 area(0.0f, 0.0f, 1.0f, 1.0f);
				rapi.setViewport(area);

				rapi.clearViewport(FBT_COLOR, Color::ZERO);

				UINT32 lightIdx = offset + j;
				const RendererLight& light = *lights[lightIdx];
				shadowRenderer.renderShadowOcclusion(inputs.view, light, gbuffer);

				rapi.setRenderTarget(output->renderTarget, FBT_DEPTH | FBT_STENCIL, RT_COLOR0 | RT_DEPTH_STENCIL);
				StandardDeferred::instance().renderLight(lightType, light, inputs.view, gbuffer,
					lightOcclusionTex->texture);
			}
		}

		// Makes sure light accumulation can be read by following passes
		rapi.setRenderTarget(nullptr);

		resPool.release(lightOcclusionTex);
	}

	void RCNodeDeferredDirectLighting::clear()
	{
		output = nullptr;
	}

	SmallVector<StringID, 4> RCNodeDeferredDirectLighting::getDependencies(const RendererView& view)
	{
		SmallVector<StringID, 4> deps;
		deps.push_back(RCNodeLightAccumulation::getNodeId());
		deps.push_back(RCNodeGBuffer::getNodeId());
		deps.push_back(RCNodeSceneDepth::getNodeId());
		deps.push_back(RCNodeMSAACoverage::getNodeId());

		return deps;
	}

	void RCNodeIndirectDiffuseLighting::render(const RenderCompositorNodeInputs& inputs)
	{
		if (!inputs.view.getRenderSettings().enableIndirectLighting)
			return;

		RCNodeGBuffer* gbufferNode = static_cast<RCNodeGBuffer*>(inputs.inputNodes[0]);
		RCNodeSceneDepth* sceneDepthNode = static_cast<RCNodeSceneDepth*>(inputs.inputNodes[1]);
		RCNodeLightAccumulation* lightAccumNode = static_cast <RCNodeLightAccumulation*>(inputs.inputNodes[2]);
		RCNodeSSAO* ssaoNode = static_cast<RCNodeSSAO*>(inputs.inputNodes[3]);

		GpuResourcePool& resPool = GpuResourcePool::instance();
		const RendererViewProperties& viewProps = inputs.view.getProperties();

		const LightProbes& lightProbes = inputs.scene.lightProbes;
		LightProbesInfo lpInfo = lightProbes.getInfo();

		IrradianceEvaluateMat* evaluateMat;
		SPtr<PooledRenderTexture> volumeIndices;
		if(lightProbes.hasAnyProbes())
		{
			POOLED_RENDER_TEXTURE_DESC volumeIndicesDesc;
			POOLED_RENDER_TEXTURE_DESC depthDesc;
			TetrahedraRenderMat::getOutputDesc(inputs.view, volumeIndicesDesc, depthDesc);

			volumeIndices = resPool.get(volumeIndicesDesc);
			SPtr<PooledRenderTexture> depthTex = resPool.get(depthDesc);

			RENDER_TEXTURE_DESC rtDesc;
			rtDesc.colorSurfaces[0].texture = volumeIndices->texture;
			rtDesc.depthStencilSurface.texture = depthTex->texture;

			SPtr<RenderTexture> rt = RenderTexture::create(rtDesc);

			RenderAPI& rapi = RenderAPI::instance();
			rapi.setRenderTarget(rt);
			rapi.clearRenderTarget(FBT_DEPTH);
			gRendererUtility().clear(-1);

			TetrahedraRenderMat* renderTetrahedra = TetrahedraRenderMat::getVariation(viewProps.numSamples > 1, true);
			renderTetrahedra->execute(inputs.view, sceneDepthNode->depthTex->texture, lpInfo.tetrahedraVolume, rt);

			rt = nullptr;
			resPool.release(depthTex);

			evaluateMat = IrradianceEvaluateMat::getVariation(viewProps.numSamples > 1, true, false);
		}
		else // Sky only
		{
			evaluateMat = IrradianceEvaluateMat::getVariation(viewProps.numSamples > 1, true, true);
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
		if(inputs.view.getRenderSettings().enableSkybox)
			skybox = inputs.scene.skybox;

		evaluateMat->execute(inputs.view, gbuffer, volumeIndicesTex, lpInfo, skybox, ssaoNode->output, 
			lightAccumNode->renderTarget);

		if(volumeIndices)
			resPool.release(volumeIndices);
	}

	void RCNodeIndirectDiffuseLighting::clear()
	{
		// Do nothing
	}

	SmallVector<StringID, 4> RCNodeIndirectDiffuseLighting::getDependencies(const RendererView& view)
	{
		SmallVector<StringID, 4> deps;

		deps.push_back(RCNodeGBuffer::getNodeId());
		deps.push_back(RCNodeSceneDepth::getNodeId());
		deps.push_back(RCNodeLightAccumulation::getNodeId());
		deps.push_back(RCNodeSSAO::getNodeId());
		deps.push_back(RCNodeDeferredDirectLighting::getNodeId());

		return deps;
	}

	void RCNodeDeferredIndirectSpecularLighting::render(const RenderCompositorNodeInputs& inputs)
	{
		RCNodeSceneColor* sceneColorNode = static_cast<RCNodeSceneColor*>(inputs.inputNodes[0]);
		RCNodeGBuffer* gbufferNode = static_cast<RCNodeGBuffer*>(inputs.inputNodes[1]);
		RCNodeSceneDepth* sceneDepthNode = static_cast<RCNodeSceneDepth*>(inputs.inputNodes[2]);
		RCNodeLightAccumulation* lightAccumNode = static_cast <RCNodeLightAccumulation*>(inputs.inputNodes[3]);
		RCNodeSSR* ssrNode = static_cast<RCNodeSSR*>(inputs.inputNodes[4]);
		RCNodeSSAO* ssaoNode = static_cast<RCNodeSSAO*>(inputs.inputNodes[5]);

		GBufferTextures gbuffer;
		gbuffer.albedo = gbufferNode->albedoTex->texture;
		gbuffer.normals = gbufferNode->normalTex->texture;
		gbuffer.roughMetal = gbufferNode->roughMetalTex->texture;
		gbuffer.depth = sceneDepthNode->depthTex->texture;

		const RendererViewProperties& viewProps = inputs.view.getProperties();

		bool tiledDeferredSupported = inputs.featureSet != RenderBeastFeatureSet::DesktopMacOS;
		if(tiledDeferredSupported)
		{
			SPtr<Texture> msaaCoverage;
			if (viewProps.numSamples > 1)
			{
				RCNodeMSAACoverage* coverageNode = static_cast<RCNodeMSAACoverage*>(inputs.inputNodes[6]);
				msaaCoverage = coverageNode->output->texture;
			}

			TiledDeferredImageBasedLightingMat* material = TiledDeferredImageBasedLightingMat::getVariation(viewProps.numSamples);

			TiledDeferredImageBasedLightingMat::Inputs iblInputs;
			iblInputs.gbuffer = gbuffer;
			iblInputs.sceneColorTex = sceneColorNode->sceneColorTex->texture;
			iblInputs.lightAccumulation = lightAccumNode->lightAccumulationTex->texture;
			iblInputs.preIntegratedGF = RendererTextures::preintegratedEnvGF;
			iblInputs.ambientOcclusion = ssaoNode->output;
			iblInputs.ssr = ssrNode->output;
			iblInputs.msaaCoverage = msaaCoverage;

			if (sceneColorNode->flattenedSceneColorBuffer)
				iblInputs.sceneColorBuffer = sceneColorNode->flattenedSceneColorBuffer->buffer;

			material->execute(inputs.view, inputs.scene, inputs.viewGroup.getVisibleReflProbeData(), iblInputs);

			if(viewProps.numSamples > 1)
				sceneColorNode->unflatten();
		}
		else // Standard deferred
		{
			SPtr<RenderTexture>	outputRT = lightAccumNode->renderTarget;

			GpuResourcePool& resPool = GpuResourcePool::instance();

			UINT32 width = viewProps.viewRect.width;
			UINT32 height = viewProps.viewRect.height;
			UINT32 numSamples = viewProps.numSamples;

			RenderAPI& rapi = RenderAPI::instance();

			bool isMSAA = viewProps.numSamples > 1;

			SPtr<PooledRenderTexture> iblRadianceTex = resPool.get(POOLED_RENDER_TEXTURE_DESC::create2D(PF_RGBA16F, width,
				height, TU_RENDERTARGET, numSamples, false));

			RENDER_TEXTURE_DESC rtDesc;
			rtDesc.colorSurfaces[0].texture = iblRadianceTex->texture;
			rtDesc.depthStencilSurface.texture = sceneDepthNode->depthTex->texture;

			SPtr<GpuParamBlockBuffer> perViewBuffer = inputs.view.getPerViewBuffer();

			SPtr<RenderTexture> iblRadianceRT = RenderTexture::create(rtDesc);
			rapi.setRenderTarget(iblRadianceRT, FBT_DEPTH | FBT_STENCIL, RT_DEPTH_STENCIL);

			const VisibleReflProbeData& probeData = inputs.viewGroup.getVisibleReflProbeData();

			Skybox* skybox = nullptr;
			if(inputs.view.getRenderSettings().enableSkybox)
				skybox = inputs.scene.skybox;

			ReflProbeParamBuffer reflProbeParams;
			reflProbeParams.populate(skybox, probeData.getNumProbes(), inputs.scene.reflProbeCubemapsTex,
				viewProps.capturingReflections);

			// Prepare the texture for refl. probe and skybox rendering
			{
				DeferredIBLSetupMat* mat = DeferredIBLSetupMat::getVariation(isMSAA, true);
				mat->bind(gbuffer, perViewBuffer, ssrNode->output, ssaoNode->output, reflProbeParams.buffer);

				gRendererUtility().drawScreenQuad();

				// Draw pixels requiring per-sample evaluation
				if (isMSAA)
				{
					DeferredIBLSetupMat* msaaMat = DeferredIBLSetupMat::getVariation(true, false);
					msaaMat->bind(gbuffer, perViewBuffer, ssrNode->output, ssaoNode->output, reflProbeParams.buffer);

					gRendererUtility().drawScreenQuad();
				}
			}

			if (!viewProps.capturingReflections)
			{
				// Render refl. probes
				UINT32 numProbes = probeData.getNumProbes();
				for (UINT32 i = 0; i < numProbes; i++)
				{
					const ReflProbeData& probe = probeData.getProbeData(i);

					StandardDeferred::instance().renderReflProbe(probe, inputs.view, gbuffer, inputs.scene,
						reflProbeParams.buffer);
				}

				// Render sky
				SPtr<Texture> skyFilteredRadiance;
				if (skybox)
					skyFilteredRadiance = skybox->getFilteredRadiance();

				if (skyFilteredRadiance)
				{
					DeferredIBLSkyMat* skymat = DeferredIBLSkyMat::getVariation(isMSAA, true);
					skymat->bind(gbuffer, perViewBuffer, skybox, reflProbeParams.buffer);

					gRendererUtility().drawScreenQuad();

					// Draw pixels requiring per-sample evaluation
					if (isMSAA)
					{
						DeferredIBLSkyMat* msaaMat = DeferredIBLSkyMat::getVariation(true, false);
						msaaMat->bind(gbuffer, perViewBuffer, skybox, reflProbeParams.buffer);

						gRendererUtility().drawScreenQuad();
					}
				}
			}

			// Finalize rendered reflections and output them to main render target
			{
				rapi.setRenderTarget(outputRT, FBT_DEPTH | FBT_STENCIL, RT_COLOR0 | RT_DEPTH_STENCIL);

				DeferredIBLFinalizeMat* mat = DeferredIBLFinalizeMat::getVariation(isMSAA, true);
				mat->bind(gbuffer, perViewBuffer, iblRadianceTex->texture, RendererTextures::preintegratedEnvGF,
					reflProbeParams.buffer);

				gRendererUtility().drawScreenQuad();

				// Draw pixels requiring per-sample evaluation
				if (isMSAA)
				{
					DeferredIBLFinalizeMat* msaaMat = DeferredIBLFinalizeMat::getVariation(true, false);
					msaaMat->bind(gbuffer, perViewBuffer, iblRadianceTex->texture, RendererTextures::preintegratedEnvGF,
						reflProbeParams.buffer);

					gRendererUtility().drawScreenQuad();
				}
			}

			// Makes sure light accumulation can be read by following passes
			rapi.setRenderTarget(nullptr);
		}
	}

	void RCNodeDeferredIndirectSpecularLighting::clear()
	{
		output = nullptr;
	}

	SmallVector<StringID, 4> RCNodeDeferredIndirectSpecularLighting::getDependencies(const RendererView& view)
	{
		SmallVector<StringID, 4> deps;

		deps.push_back(RCNodeSceneColor::getNodeId());
		deps.push_back(RCNodeGBuffer::getNodeId());
		deps.push_back(RCNodeSceneDepth::getNodeId());
		deps.push_back(RCNodeLightAccumulation::getNodeId());
		deps.push_back(RCNodeSSR::getNodeId());
		deps.push_back(RCNodeSSAO::getNodeId());
		deps.push_back(RCNodeMSAACoverage::getNodeId());
		deps.push_back(RCNodeIndirectDiffuseLighting::getNodeId());

		return deps;
	}

	RCNodeClusteredForward::RCNodeClusteredForward()
	{ }

	void RCNodeClusteredForward::render(const RenderCompositorNodeInputs& inputs)
	{
		const SceneInfo& sceneInfo = inputs.scene;
		const RendererViewProperties& viewProps = inputs.view.getProperties();

		const VisibleLightData& visibleLightData = inputs.viewGroup.getVisibleLightData();
		const VisibleReflProbeData& visibleReflProbeData = inputs.viewGroup.getVisibleReflProbeData();

		// Buffers used when clustered forward is available
		SPtr<GpuParamBlockBuffer> gridParams;
		SPtr<GpuBuffer> gridLightOffsetsAndSize, gridLightIndices;
		SPtr<GpuBuffer> gridProbeOffsetsAndSize, gridProbeIndices;

		// Buffers used when clustered forward is unavailable
		SPtr<GpuParamBlockBuffer> lightsParamBlock;
		SPtr<GpuParamBlockBuffer> reflProbesParamBlock;
		SPtr<GpuParamBlockBuffer> lightAndReflProbeParamsParamBlock;

		bool supportsClusteredForward = gRenderBeast()->getFeatureSet() == RenderBeastFeatureSet::Desktop;
		if(supportsClusteredForward)
		{
			const LightGrid& lightGrid = inputs.view.getLightGrid();

			lightGrid.getOutputs(gridLightOffsetsAndSize, gridLightIndices, gridProbeOffsetsAndSize, gridProbeIndices,
				gridParams);
		}
		else
		{
			// Note: Store these instead of creating them every time?
			lightsParamBlock = gLightsParamDef.createBuffer();
			reflProbesParamBlock = gReflProbesParamDef.createBuffer();
			lightAndReflProbeParamsParamBlock = gLightAndReflProbeParamsParamDef.createBuffer();
		}

		Skybox* skybox = nullptr;
		if(inputs.view.getRenderSettings().enableSkybox)
			skybox = sceneInfo.skybox;

		// Prepare refl. probe param buffer
		ReflProbeParamBuffer reflProbeParamBuffer;
		reflProbeParamBuffer.populate(skybox, visibleReflProbeData.getNumProbes(), sceneInfo.reflProbeCubemapsTex, 
			viewProps.capturingReflections);

		SPtr<Texture> skyFilteredRadiance;
		if(skybox)
			skyFilteredRadiance = skybox->getFilteredRadiance();

		// Prepare objects for rendering
		const VisibilityInfo& visibility = inputs.view.getVisibilityMasks();
		UINT32 numRenderables = (UINT32)sceneInfo.renderables.size();
		for (UINT32 i = 0; i < numRenderables; i++)
		{
			if (!visibility.renderables[i])
				continue;

			for (auto& element : sceneInfo.renderables[i]->elements)
			{
				ShaderFlags shaderFlags = element.material->getShader()->getFlags();

				bool useForwardRendering = shaderFlags.isSet(ShaderFlag::Forward) || shaderFlags.isSet(ShaderFlag::Transparent);
				if (!useForwardRendering)
					continue;

				// Note: It would be nice to be able to set this once and keep it, only updating if the buffers actually
				// change (e.g. when growing). 
				SPtr<GpuParams> gpuParams = element.params->getGpuParams();
				ImageBasedLightingParams& iblParams = element.imageBasedParams;
				if(supportsClusteredForward)
				{ 
					for (UINT32 j = 0; j < GPT_COUNT; j++)
					{
						const GpuParamBinding& binding = element.gridParamsBindings[j];
						if (binding.slot != (UINT32)-1)
							gpuParams->setParamBlockBuffer(binding.set, binding.slot, gridParams);
					}

					element.gridLightOffsetsAndSizeParam.set(gridLightOffsetsAndSize);
					element.gridProbeOffsetsAndSizeParam.set(gridProbeOffsetsAndSize);

					element.gridLightIndicesParam.set(gridLightIndices);
					iblParams.reflectionProbeIndicesParam.set(gridProbeIndices);

					element.lightsBufferParam.set(visibleLightData.getLightBuffer());
					iblParams.reflectionProbesParam.set(visibleReflProbeData.getProbeBuffer());
				}
				else
				{
					// Populate light & probe buffers
					const Bounds& bounds = sceneInfo.renderableCullInfos[i].bounds;

					Vector3I lightCounts;
					const LightData* lights[STANDARD_FORWARD_MAX_NUM_LIGHTS];
					visibleLightData.gatherInfluencingLights(bounds, lights, lightCounts);

					Vector4I lightOffsets;
					lightOffsets.x = lightCounts.x;
					lightOffsets.y = lightCounts.x;
					lightOffsets.z = lightOffsets.y + lightCounts.y;
					lightOffsets.w = lightOffsets.z + lightCounts.z;

					for(INT32 j = 0; j < lightOffsets.w; j++)
						gLightsParamDef.gLights.set(lightsParamBlock, *lights[j], j);

					INT32 numReflProbes = std::min(visibleReflProbeData.getNumProbes(), STANDARD_FORWARD_MAX_NUM_PROBES);
					for(INT32 j = 0; j < numReflProbes; j++)
						gReflProbesParamDef.gReflectionProbes.set(reflProbesParamBlock, visibleReflProbeData.getProbeData(j), j);

					gLightAndReflProbeParamsParamDef.gLightOffsets.set(lightAndReflProbeParamsParamBlock, lightOffsets);
					gLightAndReflProbeParamsParamDef.gReflProbeCount.set(lightAndReflProbeParamsParamBlock, numReflProbes);

					if(iblParams.reflProbesBinding.set != (UINT32)-1)
					{
						gpuParams->setParamBlockBuffer(
							iblParams.reflProbesBinding.set,
							iblParams.reflProbesBinding.slot,
							reflProbesParamBlock);
					}

					if(element.lightsParamBlockBinding.set != (UINT32)-1)
					{
						gpuParams->setParamBlockBuffer(
							element.lightsParamBlockBinding.set,
							element.lightsParamBlockBinding.slot,
							lightsParamBlock);
					}

					if(element.lightAndReflProbeParamsParamBlockBinding.set != (UINT32)-1)
					{
						gpuParams->setParamBlockBuffer(
							element.lightAndReflProbeParamsParamBlockBinding.set,
							element.lightAndReflProbeParamsParamBlockBinding.slot,
							lightAndReflProbeParamsParamBlock);
					}
				}

				// Image based lighting params
				// Note: Ideally these should be bound once (they are the same for all renderables)
				if (iblParams.reflProbeParamBindings.set != (UINT32)-1)
				{
					gpuParams->setParamBlockBuffer(
						iblParams.reflProbeParamBindings.set,
						iblParams.reflProbeParamBindings.slot,
						reflProbeParamBuffer.buffer);
				}

				iblParams.skyReflectionsTexParam.set(skyFilteredRadiance);
				iblParams.ambientOcclusionTexParam.set(Texture::WHITE); // Note: Add SSAO here?
				iblParams.ssrTexParam.set(Texture::BLACK); // Note: Add SSR here?

				iblParams.reflectionProbeCubemapsTexParam.set(sceneInfo.reflProbeCubemapsTex);
				iblParams.preintegratedEnvBRDFParam.set(RendererTextures::preintegratedEnvGF);
			}
		}

		// TODO: Forward pipeline rendering doesn't support shadows. In order to support this I'd have to render the light
		// occlusion for all lights affecting this object into a single (or a few) textures. I can likely use texture 
		// arrays for this, or to avoid sampling many textures, perhaps just jam it all in one or few texture channels. 

		// Render base pass
		RCNodeSceneColor* sceneColorNode = static_cast<RCNodeSceneColor*>(inputs.inputNodes[0]);
		RCNodeSceneDepth* sceneDepthNode = static_cast<RCNodeSceneDepth*>(inputs.inputNodes[2]);

		bool rebuildRT;
		if (renderTarget != nullptr)
		{
			rebuildRT = renderTarget->getColorTexture(0) != sceneColorNode->sceneColorTex->texture;
			rebuildRT |= renderTarget->getDepthStencilTexture() != sceneDepthNode->depthTex->texture;
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

			renderTarget = RenderTexture::create(rtDesc);
		}

		RenderAPI& rapi = RenderAPI::instance();
		rapi.setRenderTarget(renderTarget, 0, RT_ALL);

		RenderQueue* queues[] =
		{
			inputs.view.getOpaqueQueue(true).get(),
			inputs.view.getTransparentQueue().get()
		};

		// Prepare all visible particle systems
		const ParticleRenderDataGroup* particleData = inputs.frameInfo.perFrameData.particles;
		ParticleTexturePool& particlesTexPool = ParticleRenderer::instance().getTexturePool();
		if(particleData)
		{
			const auto numParticleSystems = (UINT32)inputs.scene.particleSystems.size();
			for (UINT32 i = 0; i < numParticleSystems; i++)
			{
				if (!visibility.particleSystems[i])
					continue;

				const RendererParticles& rendererParticles = inputs.scene.particleSystems[i];
				ParticlesRenderElement& renderElement = rendererParticles.renderElement;

				const auto iterFind = particleData->data.find(rendererParticles.particleSystem->getId());
				if (iterFind == particleData->data.end())
					continue;

				const ParticleRenderData* renderData = iterFind->second;
				const ParticleTextures* textures = particlesTexPool.alloc(*renderData);

				renderElement.positionAndRotTexture.set(textures->positionAndRotation);
				renderElement.colorTexture.set(textures->color);
				renderElement.sizeTexture.set(textures->size);
				renderElement.numParticles = renderData->numParticles;

				UINT32 texSize = textures->positionAndRotation->getProperties().getWidth();
				gParticlesParamDef.gTexSize.set(rendererParticles.particlesParamBuffer, texSize);

				SPtr<GpuParams> gpuParams = renderElement.params->getGpuParams();
				for (UINT32 j = 0; j < GPT_COUNT; j++)
				{
					const GpuParamBinding& binding = renderElement.perCameraBindings[j];
					if (binding.slot != (UINT32)-1)
						gpuParams->setParamBlockBuffer(binding.set, binding.slot, inputs.view.getPerViewBuffer());
				}
			}
		}

		for(UINT32 i = 0; i < bs_size(queues); i++)
		{
			const Vector<RenderQueueElement>& elements = queues[i]->getSortedElements();
			for (auto iter = elements.begin(); iter != elements.end(); ++iter)
			{
				if (iter->applyPass)
					gRendererUtility().setPass(iter->renderElem->material, iter->passIdx, iter->renderElem->techniqueIdx);

				gRendererUtility().setPassParams(iter->renderElem->params, iter->passIdx);

				if(iter->renderElem->type == (UINT32)RenderElementType::Particle)
				{
					const auto& renderElem = static_cast<const ParticlesRenderElement*>(iter->renderElem);
					ParticleRenderer::instance().drawBillboards(renderElem->numParticles);
				}
				else // Renderable
				{
					const auto& renderElem = static_cast<const RenderableElement*>(iter->renderElem);
					if (renderElem->morphVertexDeclaration == nullptr)
						gRendererUtility().draw(renderElem->mesh, renderElem->subMesh);
					else
						gRendererUtility().drawMorph(renderElem->mesh, renderElem->subMesh, renderElem->morphShapeBuffer,
							renderElem->morphVertexDeclaration);
				}
			}
		}

		// Note: Perhaps delay clearing this one frame, so previous frame textures have a better chance of being done
		particlesTexPool.clear();

		// Trigger post-lighting callbacks
		Camera* sceneCamera = inputs.view.getSceneCamera();
		if (sceneCamera != nullptr)
		{
			for(auto& extension : inputs.extPostLighting)
			{
				if (extension->check(*sceneCamera))
					extension->render(*sceneCamera);
			}
		}
	}

	void RCNodeClusteredForward::clear()
	{
		// Do nothing
	}

	SmallVector<StringID, 4> RCNodeClusteredForward::getDependencies(const RendererView& view)
	{
		return { RCNodeSceneColor::getNodeId(), RCNodeSkybox::getNodeId(), RCNodeSceneDepth::getNodeId() };
	}

	void RCNodeSkybox::render(const RenderCompositorNodeInputs& inputs)
	{
		Skybox* skybox = nullptr;
		if(inputs.view.getRenderSettings().enableSkybox)
			skybox = inputs.scene.skybox;

		SPtr<Texture> radiance = skybox ? skybox->getTexture() : nullptr;

		if (radiance != nullptr)
		{
			SkyboxMat* material = SkyboxMat::getVariation(false);
			material->bind(inputs.view.getPerViewBuffer(), radiance, Color::White);
		}
		else
		{
			Color clearColor = inputs.view.getProperties().clearColor;

			SkyboxMat* material = SkyboxMat::getVariation(true);
			material->bind(inputs.view.getPerViewBuffer(), nullptr, clearColor);
		}

		RCNodeSceneColor* sceneColorNode = static_cast<RCNodeSceneColor*>(inputs.inputNodes[0]);
		int readOnlyFlags = FBT_DEPTH | FBT_STENCIL;

		RenderAPI& rapi = RenderAPI::instance();
		rapi.setRenderTarget(sceneColorNode->renderTarget, readOnlyFlags, RT_COLOR0 | RT_DEPTH_STENCIL);

		Rect2 area(0.0f, 0.0f, 1.0f, 1.0f);
		rapi.setViewport(area);

		SPtr<Mesh> mesh = gRendererUtility().getSkyBoxMesh();
		gRendererUtility().draw(mesh, mesh->getProperties().getSubMesh(0));
	}

	void RCNodeSkybox::clear()
	{ }

	SmallVector<StringID, 4> RCNodeSkybox::getDependencies(const RendererView& view)
	{
		SmallVector<StringID, 4> deps;
		deps.push_back(RCNodeSceneColor::getNodeId());
		deps.push_back(RCNodeDeferredIndirectSpecularLighting::getNodeId());

		return deps;
	}

	void RCNodeFinalResolve::render(const RenderCompositorNodeInputs& inputs)
	{
		const RendererViewProperties& viewProps = inputs.view.getProperties();

		SPtr<Texture> input;
		if(viewProps.runPostProcessing)
		{
			RCNodePostProcess* postProcessNode = static_cast<RCNodePostProcess*>(inputs.inputNodes[0]);

			// Note: Ideally the last PP effect could write directly to the final target and we could avoid this copy
			input = postProcessNode->getLastOutput();
		}
		else
		{
			RCNodeSceneColor* sceneColorNode = static_cast<RCNodeSceneColor*>(inputs.inputNodes[0]);
			input = sceneColorNode->sceneColorTex->texture;
		}

		SPtr<RenderTarget> target = viewProps.target;

		RenderAPI& rapi = RenderAPI::instance();
		rapi.setRenderTarget(target);
		rapi.setViewport(viewProps.nrmViewRect);

		gRendererUtility().blit(input, Rect2I::EMPTY, viewProps.flipView);

		if(viewProps.encodeDepth)
		{
			RCNodeResolvedSceneDepth* resolvedSceneDepthNode = static_cast<RCNodeResolvedSceneDepth*>(inputs.inputNodes[0]);

			EncodeDepthMat* encodeDepthMat = EncodeDepthMat::get();
			encodeDepthMat->execute(resolvedSceneDepthNode->output->texture, viewProps.depthEncodeNear, 
				viewProps.depthEncodeFar, target);
		}

		// Trigger overlay callbacks
		Camera* sceneCamera = inputs.view.getSceneCamera();
		if (sceneCamera != nullptr)
		{
			for(auto& extension : inputs.extOverlay)
			{
				if (extension->check(*sceneCamera))
					extension->render(*sceneCamera);
			}
		}
	}

	void RCNodeFinalResolve::clear()
	{ }

	SmallVector<StringID, 4> RCNodeFinalResolve::getDependencies(const RendererView& view)
	{
		const RendererViewProperties& viewProps = view.getProperties();

		SmallVector<StringID, 4> deps;
		if(viewProps.runPostProcessing)
		{
			deps.push_back(RCNodePostProcess::getNodeId());
			deps.push_back(RCNodeFXAA::getNodeId());
		}
		else
		{
			deps.push_back(RCNodeSceneColor::getNodeId());
			deps.push_back(RCNodeClusteredForward::getNodeId());
		}

		if(viewProps.encodeDepth)
			deps.push_back(RCNodeResolvedSceneDepth::getNodeId());

		return deps;
	}

	RCNodePostProcess::RCNodePostProcess()
		:mOutput(), mAllocated()
	{ }

	void RCNodePostProcess::getAndSwitch(const RendererView& view, SPtr<RenderTexture>& output, SPtr<Texture>& lastFrame) const
	{
		GpuResourcePool& resPool = GpuResourcePool::instance();

		const RendererViewProperties& viewProps = view.getProperties();
		UINT32 width = viewProps.viewRect.width;
		UINT32 height = viewProps.viewRect.height;

		if(!mAllocated[mCurrentIdx])
		{
			mOutput[mCurrentIdx] = resPool.get(POOLED_RENDER_TEXTURE_DESC::create2D(PF_RGBA8, width, height,
					TU_RENDERTARGET, 1, false));

			mAllocated[mCurrentIdx] = true;
		}

		output = mOutput[mCurrentIdx]->renderTexture;

		UINT32 otherIdx = (mCurrentIdx + 1) % 2;
		if (mAllocated[otherIdx])
			lastFrame = mOutput[otherIdx]->texture;

		mCurrentIdx = otherIdx;
	}

	SPtr<Texture> RCNodePostProcess::getLastOutput() const
	{
		UINT32 otherIdx = (mCurrentIdx + 1) % 2;
		if (mAllocated[otherIdx])
			return mOutput[otherIdx]->texture;
		
		return nullptr;
	}

	void RCNodePostProcess::render(const RenderCompositorNodeInputs& inputs)
	{
		// Do nothing, this is just a helper node
	}

	void RCNodePostProcess::clear()
	{
		GpuResourcePool& resPool = GpuResourcePool::instance();

		if (mAllocated[0])
			resPool.release(mOutput[0]);

		if (mAllocated[1])
			resPool.release(mOutput[1]);

		mAllocated[0] = false;
		mAllocated[1] = false;
		mCurrentIdx = 0;
	}

	SmallVector<StringID, 4> RCNodePostProcess::getDependencies(const RendererView& view)
	{
		return {};
	}

	RCNodeTonemapping::~RCNodeTonemapping()
	{
		GpuResourcePool& resPool = GpuResourcePool::instance();

		if (mTonemapLUT)
			resPool.release(mTonemapLUT);

		if (prevEyeAdaptation)
			resPool.release(prevEyeAdaptation);
	}

	void RCNodeTonemapping::render(const RenderCompositorNodeInputs& inputs)
	{
		GpuResourcePool& resPool = GpuResourcePool::instance();

		const RendererViewProperties& viewProps = inputs.view.getProperties();
		const RenderSettings& settings = inputs.view.getRenderSettings();

		RCNodeSceneColor* sceneColorNode = static_cast<RCNodeSceneColor*>(inputs.inputNodes[0]);
		RCNodePostProcess* postProcessNode = static_cast<RCNodePostProcess*>(inputs.inputNodes[2]);
		SPtr<Texture> sceneColor = sceneColorNode->sceneColorTex->texture;

		bool hdr = settings.enableHDR;
		bool msaa = viewProps.numSamples > 1;

		if(hdr && settings.enableAutoExposure)
		{
			// Downsample scene
			DownsampleMat* downsampleMat = DownsampleMat::getVariation(1, msaa);
			SPtr<PooledRenderTexture> downsampledScene = resPool.get(DownsampleMat::getOutputDesc(sceneColor));

			downsampleMat->execute(sceneColor, downsampledScene->renderTexture);

			if(useHistogramEyeAdapatation(inputs))
			{
				// Generate histogram
				SPtr<PooledRenderTexture> eyeAdaptHistogram =
					resPool.get(EyeAdaptHistogramMat::getOutputDesc(downsampledScene->texture));
				EyeAdaptHistogramMat* eyeAdaptHistogramMat = EyeAdaptHistogramMat::get();
				eyeAdaptHistogramMat->execute(downsampledScene->texture, eyeAdaptHistogram->texture, settings.autoExposure);

				// Reduce histogram
				SPtr<PooledRenderTexture> reducedHistogram = resPool.get(EyeAdaptHistogramReduceMat::getOutputDesc());

				SPtr<Texture> prevFrameEyeAdaptation;
				if (prevEyeAdaptation != nullptr)
					prevFrameEyeAdaptation = prevEyeAdaptation->texture;

				EyeAdaptHistogramReduceMat* eyeAdaptHistogramReduce = EyeAdaptHistogramReduceMat::get();
				eyeAdaptHistogramReduce->execute(
					downsampledScene->texture,
					eyeAdaptHistogram->texture,
					prevFrameEyeAdaptation,
					reducedHistogram->renderTexture);

				resPool.release(downsampledScene);
				downsampledScene = nullptr;

				resPool.release(eyeAdaptHistogram);
				eyeAdaptHistogram = nullptr;

				// Generate eye adaptation value
				eyeAdaptation = resPool.get(EyeAdaptationMat::getOutputDesc());
				EyeAdaptationMat* eyeAdaptationMat = EyeAdaptationMat::get();
				eyeAdaptationMat->execute(
					reducedHistogram->texture,
					eyeAdaptation->renderTexture,
					inputs.frameInfo.timeDelta,
					settings.autoExposure,
					settings.exposureScale);

				resPool.release(reducedHistogram);
				reducedHistogram = nullptr;
			}
			else
			{
				// Populate alpha values of the downsampled texture with luminance
				SPtr<PooledRenderTexture> luminanceTex = 
					resPool.get(EyeAdaptationBasicSetupMat::getOutputDesc(downsampledScene->texture));

				EyeAdaptationBasicSetupMat* setupMat = EyeAdaptationBasicSetupMat::get();
				setupMat->execute(
					downsampledScene->texture,
					luminanceTex->renderTexture,
					inputs.frameInfo.timeDelta,
					settings.autoExposure,
					settings.exposureScale);

				SPtr<Texture> downsampleInput = luminanceTex->texture;
				luminanceTex = nullptr;

				// Downsample some more
				for(UINT32 i = 0; i < 5; i++)
				{
					downsampleMat = DownsampleMat::getVariation(1, false);
					SPtr<PooledRenderTexture> downsampledLuminance = 
						resPool.get(DownsampleMat::getOutputDesc(downsampleInput));

					downsampleMat->execute(downsampleInput, downsampledLuminance->renderTexture);
					downsampleInput = downsampledLuminance->texture;
				}

				// Generate eye adaptation value
				EyeAdaptationBasicMat* eyeAdaptationMat = EyeAdaptationBasicMat::get();

				SPtr<Texture> prevFrameEyeAdaptation;
				if (prevEyeAdaptation != nullptr)
					prevFrameEyeAdaptation = prevEyeAdaptation->texture;

				eyeAdaptation = resPool.get(EyeAdaptationBasicMat::getOutputDesc());
				eyeAdaptationMat->execute(
					downsampleInput,
					prevFrameEyeAdaptation,
					eyeAdaptation->renderTexture,
					inputs.frameInfo.timeDelta,
					settings.autoExposure,
					settings.exposureScale);
			}
		}
		else
		{
			if(prevEyeAdaptation)
				resPool.release(prevEyeAdaptation);

			prevEyeAdaptation = nullptr;
			eyeAdaptation = nullptr;
		}

		bool volumeLUT = inputs.featureSet == RenderBeastFeatureSet::Desktop;
		bool gammaOnly;
		bool autoExposure;
		if (hdr)
		{
			if (settings.enableTonemapping)
			{
				UINT64 latestHash = inputs.view.getRenderSettingsHash();
				bool tonemapLUTDirty = mTonemapLastUpdateHash != latestHash;

				if (tonemapLUTDirty) // Rebuild LUT if PP settings changed
				{
					CreateTonemapLUTMat* createLUT = CreateTonemapLUTMat::getVariation(volumeLUT);
					if(mTonemapLUT == nullptr)
						mTonemapLUT = resPool.get(createLUT->getOutputDesc());

					if(volumeLUT)
						createLUT->execute3D(mTonemapLUT->texture, settings);
					else
						createLUT->execute2D(mTonemapLUT->renderTexture, settings);

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
		{
			if(mTonemapLUT)
			{
				resPool.release(mTonemapLUT);
				mTonemapLUT = nullptr;
			}
		}

		TonemappingMat* tonemapping = TonemappingMat::getVariation(volumeLUT, gammaOnly, autoExposure, msaa);

		SPtr<RenderTexture> ppOutput;
		SPtr<Texture> ppLastFrame;
		postProcessNode->getAndSwitch(inputs.view, ppOutput, ppLastFrame);

		SPtr<Texture> eyeAdaptationTex;
		if (eyeAdaptation)
			eyeAdaptationTex = eyeAdaptation->texture;

		SPtr<Texture> tonemapLUTTex;
		if (mTonemapLUT)
			tonemapLUTTex = mTonemapLUT->texture;

		tonemapping->execute(sceneColor, eyeAdaptationTex, tonemapLUTTex, ppOutput, settings);
	}

	void RCNodeTonemapping::clear()
	{
		GpuResourcePool& resPool = GpuResourcePool::instance();

		// Save eye adaptation for next frame
		if(prevEyeAdaptation)
			resPool.release(prevEyeAdaptation);

		std::swap(eyeAdaptation, prevEyeAdaptation);
	}
	
	bool RCNodeTonemapping::useHistogramEyeAdapatation(const RenderCompositorNodeInputs& inputs)
	{
		return inputs.featureSet == RenderBeastFeatureSet::Desktop;
	}

	SmallVector<StringID, 4> RCNodeTonemapping::getDependencies(const RendererView& view)
	{
		return{ RCNodeSceneColor::getNodeId(), RCNodeClusteredForward::getNodeId(), RCNodePostProcess::getNodeId() };
	}

	void RCNodeGaussianDOF::render(const RenderCompositorNodeInputs& inputs)
	{
		RCNodeSceneDepth* sceneDepthNode = static_cast<RCNodeSceneDepth*>(inputs.inputNodes[1]);
		RCNodePostProcess* postProcessNode = static_cast<RCNodePostProcess*>(inputs.inputNodes[2]);

		const DepthOfFieldSettings& settings = inputs.view.getRenderSettings().depthOfField;
		bool near = settings.nearBlurAmount > 0.0f;
		bool far = settings.farBlurAmount > 0.0f;

		bool enabled = settings.enabled && (near || far);
		if(!enabled)
			return;

		GaussianDOFSeparateMat* separateMat = GaussianDOFSeparateMat::getVariation(near, far);
		GaussianDOFCombineMat* combineMat = GaussianDOFCombineMat::getVariation(near, far);
		GaussianBlurMat* blurMat = GaussianBlurMat::get();

		SPtr<RenderTexture> ppOutput;
		SPtr<Texture> ppLastFrame;
		postProcessNode->getAndSwitch(inputs.view, ppOutput, ppLastFrame);

		separateMat->execute(ppLastFrame, sceneDepthNode->depthTex->texture, inputs.view, settings);

		SPtr<PooledRenderTexture> nearTex, farTex;
		if(near && far)
		{
			nearTex = separateMat->getOutput(0);
			farTex = separateMat->getOutput(1);
		}
		else
		{
			if (near)
				nearTex = separateMat->getOutput(0);
			else
				farTex = separateMat->getOutput(0);
		}

		// Blur the out of focus pixels
		// Note: Perhaps set up stencil so I can avoid performing blur on unused parts of the textures?
		const TextureProperties& texProps = nearTex ? nearTex->texture->getProperties() : farTex->texture->getProperties();
		POOLED_RENDER_TEXTURE_DESC tempTexDesc = POOLED_RENDER_TEXTURE_DESC::create2D(texProps.getFormat(), 
			texProps.getWidth(), texProps.getHeight(), TU_RENDERTARGET);
		SPtr<PooledRenderTexture> tempTexture = GpuResourcePool::instance().get(tempTexDesc);

		SPtr<Texture> blurredNearTex;
		if(nearTex)
		{
			blurMat->execute(nearTex->texture, settings.nearBlurAmount, tempTexture->renderTexture);
			blurredNearTex = tempTexture->texture;
		}

		SPtr<Texture> blurredFarTex;
		if(farTex)
		{
			// If temporary texture is used up, re-use the original near texture for the blurred result
			if(blurredNearTex)
			{
				blurMat->execute(farTex->texture, settings.farBlurAmount, nearTex->renderTexture);
				blurredFarTex = nearTex->texture;
			}
			else // Otherwise just use the temporary
			{
				blurMat->execute(farTex->texture, settings.farBlurAmount, tempTexture->renderTexture);
				blurredFarTex = tempTexture->texture;
			}
		}

		combineMat->execute(ppLastFrame, blurredNearTex, blurredFarTex, 
			sceneDepthNode->depthTex->texture, ppOutput, inputs.view, settings);

		separateMat->release();
		GpuResourcePool::instance().release(tempTexture);
	}

	void RCNodeGaussianDOF::clear()
	{
		// Do nothing
	}

	SmallVector<StringID, 4> RCNodeGaussianDOF::getDependencies(const RendererView& view)
	{
		return { RCNodeTonemapping::getNodeId(), RCNodeSceneDepth::getNodeId(), RCNodePostProcess::getNodeId() };
	}

	void RCNodeFXAA::render(const RenderCompositorNodeInputs& inputs)
	{
		const RenderSettings& settings = inputs.view.getRenderSettings();
		if (!settings.enableFXAA)
			return;

		RCNodePostProcess* postProcessNode = static_cast<RCNodePostProcess*>(inputs.inputNodes[1]);

		SPtr<RenderTexture> ppOutput;
		SPtr<Texture> ppLastFrame;
		postProcessNode->getAndSwitch(inputs.view, ppOutput, ppLastFrame);

		// Note: I could skip executing FXAA over DOF and motion blurred pixels
		FXAAMat* fxaa = FXAAMat::get();
		fxaa->execute(ppLastFrame, ppOutput);
	}

	void RCNodeFXAA::clear()
	{
		// Do nothing
	}

	SmallVector<StringID, 4> RCNodeFXAA::getDependencies(const RendererView& view)
	{
		return { RCNodeGaussianDOF::getNodeId(), RCNodePostProcess::getNodeId() };
	}

	void RCNodeResolvedSceneDepth::render(const RenderCompositorNodeInputs& inputs)
	{
		GpuResourcePool& resPool = GpuResourcePool::instance();
		const RendererViewProperties& viewProps = inputs.view.getProperties();
		RCNodeSceneDepth* sceneDepthNode = static_cast<RCNodeSceneDepth*>(inputs.inputNodes[0]);

		if (viewProps.numSamples > 1)
		{
			UINT32 width = viewProps.viewRect.width;
			UINT32 height = viewProps.viewRect.height;

			output = resPool.get(POOLED_RENDER_TEXTURE_DESC::create2D(PF_D32_S8X24, width, height, 
				TU_DEPTHSTENCIL, 1, false)); 

			RenderAPI& rapi = RenderAPI::instance();
			rapi.setRenderTarget(output->renderTexture);
			rapi.clearRenderTarget(FBT_STENCIL);
			gRendererUtility().blit(sceneDepthNode->depthTex->texture, Rect2I::EMPTY, false, true);

			mPassThrough = false;
		}
		else
		{
			output = sceneDepthNode->depthTex;
			mPassThrough = true;
		}
	}

	void RCNodeResolvedSceneDepth::clear()
	{
		GpuResourcePool& resPool = GpuResourcePool::instance();

		if (!mPassThrough)
			resPool.release(output);
		else
			output = nullptr;

		mPassThrough = false;
	}

	SmallVector<StringID, 4> RCNodeResolvedSceneDepth::getDependencies(const RendererView& view)
	{
		// GBuffer require because it renders the base pass (populates the depth buffer)
		return { RCNodeSceneDepth::getNodeId(), RCNodeGBuffer::getNodeId() };
	}

	void RCNodeHiZ::render(const RenderCompositorNodeInputs& inputs)
	{
		GpuResourcePool& resPool = GpuResourcePool::instance();
		const RendererViewProperties& viewProps = inputs.view.getProperties();

		RCNodeResolvedSceneDepth* resolvedSceneDepth = static_cast<RCNodeResolvedSceneDepth*>(inputs.inputNodes[0]);

		UINT32 width = viewProps.viewRect.width;
		UINT32 height = viewProps.viewRect.height;

		UINT32 size = Bitwise::nextPow2(std::max(width, height));
		UINT32 numMips = PixelUtil::getMaxMipmaps(size, size, 1, PF_R32F);
		size = 1 << numMips;

		// Note: Use the 32-bit buffer here as 16-bit causes too much banding (most of the scene gets assigned 4-5 different
		// depth values). 
		//  - When I add UNORM 16-bit format I should be able to switch to that
		output = resPool.get(POOLED_RENDER_TEXTURE_DESC::create2D(PF_R32F, size, size, TU_RENDERTARGET, 1, false, 1, 
			numMips));

		Rect2 srcRect = viewProps.nrmViewRect;

		// If viewport size is odd, adjust UV
		srcRect.width += (viewProps.viewRect.width % 2) * (1.0f / viewProps.viewRect.width);
		srcRect.height += (viewProps.viewRect.height % 2) * (1.0f / viewProps.viewRect.height);

		const RenderAPIInfo& rapiInfo = RenderAPI::instance().getAPIInfo();
		bool noTextureViews = !rapiInfo.isFlagSet(RenderAPIFeatureFlag::TextureViews);

		BuildHiZMat* material = BuildHiZMat::getVariation(noTextureViews);

		// Generate first mip
		RENDER_TEXTURE_DESC rtDesc;
		rtDesc.colorSurfaces[0].texture = output->texture;
		rtDesc.colorSurfaces[0].mipLevel = 0;

		SPtr<RenderTexture> rt = RenderTexture::create(rtDesc);

		Rect2 destRect;
		bool downsampledFirstMip = false; // Not used currently
		if (downsampledFirstMip)
		{
			// Make sure that 1 pixel in HiZ maps to a 2x2 block in source
			destRect = Rect2(0, 0,
				Math::ceilToInt(viewProps.viewRect.width / 2.0f) / (float)size,
				Math::ceilToInt(viewProps.viewRect.height / 2.0f) / (float)size);

			material->execute(resolvedSceneDepth->output->texture, 0, srcRect, destRect, rt);
		}
		else // First level is just a copy of the depth buffer
		{
			destRect = Rect2(0, 0,
				viewProps.viewRect.width / (float)size,
				viewProps.viewRect.height / (float)size);

			RenderAPI& rapi = RenderAPI::instance();
			rapi.setRenderTarget(rt);
			rapi.setViewport(destRect);

			Rect2I srcAreaInt;
			srcAreaInt.x = (INT32)(srcRect.x * viewProps.viewRect.width);
			srcAreaInt.y = (INT32)(srcRect.y * viewProps.viewRect.height);
			srcAreaInt.width = (UINT32)(srcRect.width * viewProps.viewRect.width);
			srcAreaInt.height = (UINT32)(srcRect.height * viewProps.viewRect.height);

			gRendererUtility().blit(resolvedSceneDepth->output->texture, srcAreaInt);
			rapi.setViewport(Rect2(0, 0, 1, 1));
		}

		// Generate remaining mip levels
		for(UINT32 i = 1; i <= numMips; i++)
		{
			rtDesc.colorSurfaces[0].mipLevel = i;
			rt = RenderTexture::create(rtDesc);

			material->execute(output->texture, i - 1, destRect, destRect, rt);
		}
	}

	void RCNodeHiZ::clear()
	{
		GpuResourcePool& resPool = GpuResourcePool::instance();
		resPool.release(output);
	}

	SmallVector<StringID, 4> RCNodeHiZ::getDependencies(const RendererView& view)
	{
		// Note: This doesn't actually use any gbuffer textures, but node is a dependency because it renders to the depth
		// buffer. In order to avoid keeping gbuffer textures alive I could separate out the base pass into its own node
		// perhaps. But at the moment it doesn't matter, as anything using HiZ also needs gbuffer.
		return { RCNodeResolvedSceneDepth::getNodeId(), RCNodeGBuffer::getNodeId() };
	}

	void RCNodeSSAO::render(const RenderCompositorNodeInputs& inputs)
	{
		/** Maximum valid depth range within samples in a sample set. In meters. */
		static const float DEPTH_RANGE = 1.0f;

		const AmbientOcclusionSettings& settings = inputs.view.getRenderSettings().ambientOcclusion;
		if(!settings.enabled)
		{
			output = Texture::WHITE;
			mPooledOutput = nullptr;
			return;
		}

		GpuResourcePool& resPool = GpuResourcePool::instance();
		const RendererViewProperties& viewProps = inputs.view.getProperties();

		RCNodeResolvedSceneDepth* resolvedDepthNode = static_cast<RCNodeResolvedSceneDepth*>(inputs.inputNodes[0]);
		RCNodeGBuffer* gbufferNode = static_cast<RCNodeGBuffer*>(inputs.inputNodes[1]);

		SPtr<Texture> sceneDepth = resolvedDepthNode->output->texture;
		SPtr<Texture> sceneNormals = gbufferNode->normalTex->texture;

		const TextureProperties& normalsProps = sceneNormals->getProperties();
		SPtr<PooledRenderTexture> resolvedNormals;

		RenderAPI& rapi = RenderAPI::instance();
		if(sceneNormals->getProperties().getNumSamples() > 1)
		{
			POOLED_RENDER_TEXTURE_DESC desc = POOLED_RENDER_TEXTURE_DESC::create2D(normalsProps.getFormat(), 
				normalsProps.getWidth(), normalsProps.getHeight(), TU_RENDERTARGET);
			resolvedNormals = resPool.get(desc);

			rapi.setRenderTarget(resolvedNormals->renderTexture);
			gRendererUtility().blit(sceneNormals);

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

		SSAODownsampleMat* downsample = SSAODownsampleMat::get();

		SPtr<PooledRenderTexture> setupTex0;
		if(numDownsampleLevels > 0)
		{
			Vector2I downsampledSize(
				std::max(1, Math::divideAndRoundUp((INT32)viewProps.viewRect.width, 2)),
				std::max(1, Math::divideAndRoundUp((INT32)viewProps.viewRect.height, 2))
			);

			POOLED_RENDER_TEXTURE_DESC desc = POOLED_RENDER_TEXTURE_DESC::create2D(PF_RGBA16F, downsampledSize.x, 
				downsampledSize.y, TU_RENDERTARGET);
			setupTex0 = GpuResourcePool::instance().get(desc);

			downsample->execute(inputs.view, sceneDepth, sceneNormals, setupTex0->renderTexture, DEPTH_RANGE);
		}

		SPtr<PooledRenderTexture> setupTex1;
		if(numDownsampleLevels > 1)
		{
			Vector2I downsampledSize(
				std::max(1, Math::divideAndRoundUp((INT32)viewProps.viewRect.width, 4)),
				std::max(1, Math::divideAndRoundUp((INT32)viewProps.viewRect.height, 4))
			);

			POOLED_RENDER_TEXTURE_DESC desc = POOLED_RENDER_TEXTURE_DESC::create2D(PF_RGBA16F, downsampledSize.x, 
				downsampledSize.y, TU_RENDERTARGET);
			setupTex1 = GpuResourcePool::instance().get(desc);

			downsample->execute(inputs.view, sceneDepth, sceneNormals, setupTex1->renderTexture, DEPTH_RANGE);
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
				std::max(1, Math::divideAndRoundUp((INT32)viewProps.viewRect.width, 4)),
				std::max(1, Math::divideAndRoundUp((INT32)viewProps.viewRect.height, 4))
			);

			POOLED_RENDER_TEXTURE_DESC desc = POOLED_RENDER_TEXTURE_DESC::create2D(PF_R8, downsampledSize.x, 
				downsampledSize.y, TU_RENDERTARGET);
			downAOTex1 = GpuResourcePool::instance().get(desc);

			SSAOMat* ssaoMat = SSAOMat::getVariation(false, false, quality);
			ssaoMat->execute(inputs.view, textures, downAOTex1->renderTexture, settings);

			GpuResourcePool::instance().release(setupTex1);
			setupTex1 = nullptr;
		}

		SPtr<PooledRenderTexture> downAOTex0;
		if(numDownsampleLevels > 0)
		{
			textures.aoSetup = setupTex0->texture;

			if(downAOTex1)
				textures.aoDownsampled = downAOTex1->texture;

			Vector2I downsampledSize(
				std::max(1, Math::divideAndRoundUp((INT32)viewProps.viewRect.width, 2)),
				std::max(1, Math::divideAndRoundUp((INT32)viewProps.viewRect.height, 2))
			);

			POOLED_RENDER_TEXTURE_DESC desc = POOLED_RENDER_TEXTURE_DESC::create2D(PF_R8, downsampledSize.x, 
				downsampledSize.y, TU_RENDERTARGET);
			downAOTex0 = GpuResourcePool::instance().get(desc);

			bool upsample = numDownsampleLevels > 1;
			SSAOMat* ssaoMat = SSAOMat::getVariation(upsample, false, quality);
			ssaoMat->execute(inputs.view, textures, downAOTex0->renderTexture, settings);

			if(upsample)
			{
				GpuResourcePool::instance().release(downAOTex1);
				downAOTex1 = nullptr;
			}
		}

		UINT32 width = viewProps.viewRect.width;
		UINT32 height = viewProps.viewRect.height;
		mPooledOutput = resPool.get(POOLED_RENDER_TEXTURE_DESC::create2D(PF_R8, width, height, TU_RENDERTARGET));

		{
			if(setupTex0)
				textures.aoSetup = setupTex0->texture;

			if(downAOTex0)
				textures.aoDownsampled = downAOTex0->texture;

			bool upsample = numDownsampleLevels > 0;
			SSAOMat* ssaoMat = SSAOMat::getVariation(upsample, true, quality);
			ssaoMat->execute(inputs.view, textures, mPooledOutput->renderTexture, settings);
		}

		if(resolvedNormals)
		{
			GpuResourcePool::instance().release(resolvedNormals);
			resolvedNormals = nullptr;
		}

		if(numDownsampleLevels > 0)
		{
			GpuResourcePool::instance().release(setupTex0);
			GpuResourcePool::instance().release(downAOTex0);
		}

		// Blur the output
		// Note: If I implement temporal AA then this can probably be avoided. I can instead jitter the sample offsets
		// each frame, and averaging them out should yield blurred AO.
		if(quality > 1) // On level 0 we don't blur at all, on level 1 we use the ad-hoc blur in shader
		{
			const RenderTargetProperties& rtProps = mPooledOutput->renderTexture->getProperties();

			POOLED_RENDER_TEXTURE_DESC desc = POOLED_RENDER_TEXTURE_DESC::create2D(PF_R8, rtProps.width, 
				rtProps.height, TU_RENDERTARGET);
			SPtr<PooledRenderTexture> blurIntermediateTex = GpuResourcePool::instance().get(desc);

			SSAOBlurMat* blurHorz = SSAOBlurMat::getVariation(true);
			SSAOBlurMat* blurVert = SSAOBlurMat::getVariation(false);

			blurHorz->execute(inputs.view, mPooledOutput->texture, sceneDepth, blurIntermediateTex->renderTexture, DEPTH_RANGE);
			blurVert->execute(inputs.view, blurIntermediateTex->texture, sceneDepth, mPooledOutput->renderTexture, DEPTH_RANGE);

			GpuResourcePool::instance().release(blurIntermediateTex);
		}

		RenderAPI::instance().setRenderTarget(nullptr);
		output = mPooledOutput->texture;
	}

	void RCNodeSSAO::clear()
	{
		if(mPooledOutput)
		{
			GpuResourcePool& resPool = GpuResourcePool::instance();
			resPool.release(mPooledOutput);
		}

		output = nullptr;
	}

	SmallVector<StringID, 4> RCNodeSSAO::getDependencies(const RendererView& view)
	{
		return { RCNodeResolvedSceneDepth::getNodeId(), RCNodeGBuffer::getNodeId() };
	}

	RCNodeSSR::~RCNodeSSR()
	{
		deallocOutputs();
	}

	void RCNodeSSR::render(const RenderCompositorNodeInputs& inputs)
	{
		const ScreenSpaceReflectionsSettings& settings = inputs.view.getRenderSettings().screenSpaceReflections;
		if (!settings.enabled)
		{
			deallocOutputs();

			mPooledOutput = nullptr;
			output = Texture::BLACK;
			return;
		}

		RenderAPI& rapi = RenderAPI::instance();

		RCNodeSceneDepth* sceneDepthNode = static_cast<RCNodeSceneDepth*>(inputs.inputNodes[0]);
		RCNodeLightAccumulation* lightAccumNode = static_cast<RCNodeLightAccumulation*>(inputs.inputNodes[1]);
		RCNodeGBuffer* gbufferNode = static_cast<RCNodeGBuffer*>(inputs.inputNodes[2]);
		RCNodeHiZ* hiZNode = static_cast<RCNodeHiZ*>(inputs.inputNodes[3]);
		RCNodeResolvedSceneDepth* resolvedSceneDepthNode = static_cast<RCNodeResolvedSceneDepth*>(inputs.inputNodes[4]);

		GpuResourcePool& resPool = GpuResourcePool::instance();
		const RendererViewProperties& viewProps = inputs.view.getProperties();

		UINT32 width = viewProps.viewRect.width;
		UINT32 height = viewProps.viewRect.height;

		SPtr<Texture> hiZ = hiZNode->output->texture;

		// This will be executing before scene color is resolved, so get the light accum buffer instead
		SPtr<Texture> sceneColor = lightAccumNode->lightAccumulationTex->texture;

		// Resolve multiple samples if MSAA is used
		SPtr<PooledRenderTexture> resolvedSceneColor;
		if (viewProps.numSamples > 1)
		{
			resolvedSceneColor = resPool.get(POOLED_RENDER_TEXTURE_DESC::create2D(PF_RGBA16F, width, height,
				TU_RENDERTARGET));

			rapi.setRenderTarget(resolvedSceneColor->renderTexture);
			gRendererUtility().blit(sceneColor);

			sceneColor = resolvedSceneColor->texture;
		}

		GBufferTextures gbuffer;
		gbuffer.albedo = gbufferNode->albedoTex->texture;
		gbuffer.normals = gbufferNode->normalTex->texture;
		gbuffer.roughMetal = gbufferNode->roughMetalTex->texture;
		gbuffer.depth = sceneDepthNode->depthTex->texture;

		SSRStencilMat* stencilMat = SSRStencilMat::getVariation(viewProps.numSamples > 1, true);

		// Note: Making the assumption that the stencil buffer is clear at this point
		rapi.setRenderTarget(resolvedSceneDepthNode->output->renderTexture, FBT_DEPTH, RT_DEPTH_STENCIL);
		stencilMat->execute(inputs.view, gbuffer, settings);

		SPtr<PooledRenderTexture> traceOutput = resPool.get(POOLED_RENDER_TEXTURE_DESC::create2D(PF_RGBA16F, width,
			height, TU_RENDERTARGET));

		RENDER_TEXTURE_DESC traceRtDesc;
		traceRtDesc.colorSurfaces[0].texture = traceOutput->texture;
		traceRtDesc.depthStencilSurface.texture = resolvedSceneDepthNode->output->texture;

		SPtr<RenderTexture> traceRt = RenderTexture::create(traceRtDesc);

		rapi.setRenderTarget(traceRt, FBT_DEPTH | FBT_STENCIL, RT_DEPTH_STENCIL);
		rapi.clearRenderTarget(FBT_COLOR, Color::ZERO);

		SSRTraceMat* traceMat = SSRTraceMat::getVariation(settings.quality, viewProps.numSamples > 1, true);
		traceMat->execute(inputs.view, gbuffer, sceneColor, hiZ, settings, traceRt);

		if (resolvedSceneColor)
		{
			resPool.release(resolvedSceneColor);
			resolvedSceneColor = nullptr;
		}

		if (mPrevFrame)
		{
			mPooledOutput = resPool.get(POOLED_RENDER_TEXTURE_DESC::create2D(PF_RGBA16F, width, height, TU_RENDERTARGET));

			rapi.setRenderTarget(mPooledOutput->renderTexture);
			rapi.clearRenderTarget(FBT_COLOR);

			SSRResolveMat* resolveMat = SSRResolveMat::getVariation(viewProps.numSamples > 1);
			resolveMat->execute(inputs.view, mPrevFrame->texture, traceOutput->texture, sceneDepthNode->depthTex->texture,
				mPooledOutput->renderTexture);

			resPool.release(traceOutput);
		}
		else
			mPooledOutput = traceOutput;

		RenderAPI::instance().setRenderTarget(nullptr);
		output = mPooledOutput->texture;
	}

	void RCNodeSSR::clear()
	{
		GpuResourcePool& resPool = GpuResourcePool::instance();

		if(mPrevFrame)
			resPool.release(mPrevFrame);

		mPrevFrame = mPooledOutput;
		mPooledOutput = nullptr;
		output = nullptr;
	}

	void RCNodeSSR::deallocOutputs()
	{
		GpuResourcePool& resPool = GpuResourcePool::instance();
		
		if(mPrevFrame)
		{
			resPool.release(mPrevFrame);
			mPrevFrame = nullptr;
		}

		output = nullptr;
	}

	SmallVector<StringID, 4> RCNodeSSR::getDependencies(const RendererView& view)
	{
		SmallVector<StringID, 4> deps;
		if (view.getRenderSettings().screenSpaceReflections.enabled)
		{
			deps.push_back(RCNodeSceneDepth::getNodeId());
			deps.push_back(RCNodeLightAccumulation::getNodeId());
			deps.push_back(RCNodeGBuffer::getNodeId());
			deps.push_back(RCNodeHiZ::getNodeId());
			deps.push_back(RCNodeResolvedSceneDepth::getNodeId());
			deps.push_back(RCNodeIndirectDiffuseLighting::getNodeId());
		}

		return deps;
	}
}}
