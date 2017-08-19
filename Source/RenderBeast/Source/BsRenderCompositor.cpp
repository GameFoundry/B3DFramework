//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsRenderCompositor.h"
#include "BsGpuResourcePool.h"
#include "BsRendererView.h"
#include "BsRendererUtility.h"
#include "BsMesh.h"
#include "BsGpuBuffer.h"
#include "BsStandardDeferredLighting.h"
#include "BsRenderBeastOptions.h"
#include "BsCamera.h"
#include "BsRendererScene.h"
#include "BsRenderBeast.h"
#include "BsBitwise.h"
#include "BsRendererTextures.h"
#include "BsObjectRendering.h"
#include "BsGpuParamsSet.h"
#include "BsRendererExtension.h"
#include "BsSkybox.h"
#include "BsLightProbes.h"

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
					if (curIdx == -1)
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
					if (depNodeInfo.lastUseIdx == -1)
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

			RendererObject* rendererObject = inputs.scene.renderables[i];
			rendererObject->updatePerCallBuffer(viewProps.viewProjTransform);

			for (auto& element : inputs.scene.renderables[i]->elements)
			{
				if (element.perCameraBindingIdx != -1)
					element.params->setParamBlockBuffer(element.perCameraBindingIdx, inputs.view.getPerViewBuffer(), true);
			}
		}

		Camera* sceneCamera = inputs.view.getSceneCamera();

		// Trigger pre-base-pass callbacks
		if (sceneCamera != nullptr)
		{
			for(auto& extension : inputs.extPreBasePass)
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

		// Render all visible opaque elements
		const Vector<RenderQueueElement>& opaqueElements = inputs.view.getOpaqueQueue()->getSortedElements();
		for (auto iter = opaqueElements.begin(); iter != opaqueElements.end(); ++iter)
		{
			BeastRenderableElement* renderElem = static_cast<BeastRenderableElement*>(iter->renderElem);

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

		// Note: Consider customizable HDR format via options? e.g. smaller PF_FLOAT_R11G11B10 or larger 32-bit format
		sceneColorTex = resPool.get(POOLED_RENDER_TEXTURE_DESC::create2D(PF_RGBA16F, width, height, TU_RENDERTARGET | 
			TU_LOADSTORE, numSamples, false));

		RCNodeSceneDepth* sceneDepthNode = static_cast<RCNodeSceneDepth*>(inputs.inputNodes[0]);
		SPtr<PooledRenderTexture> sceneDepthTex = sceneDepthNode->depthTex;

		if (viewProps.numSamples > 1)
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

	SmallVector<StringID, 4> RCNodeSceneColor::getDependencies(const RendererView& view)
	{
		return { RCNodeSceneDepth::getNodeId() };
	}

	void RCNodeLightAccumulation::render(const RenderCompositorNodeInputs& inputs)
	{
		GpuResourcePool& resPool = GpuResourcePool::instance();
		const RendererViewProperties& viewProps = inputs.view.getProperties();

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
			rebuildRT = renderTarget->getColorTexture(0) != lightAccumulationTex->texture;
		else
			rebuildRT = true;

		if (rebuildRT)
		{
			RENDER_TEXTURE_DESC lightAccumulationRTDesc;
			lightAccumulationRTDesc.colorSurfaces[0].texture = lightAccumulationTex->texture;
			lightAccumulationRTDesc.colorSurfaces[0].face = 0;
			lightAccumulationRTDesc.colorSurfaces[0].numFaces = 1;
			lightAccumulationRTDesc.colorSurfaces[0].mipLevel = 0;

			renderTarget = RenderTexture::create(lightAccumulationRTDesc);
		}
	}

	void RCNodeLightAccumulation::clear()
	{
		GpuResourcePool& resPool = GpuResourcePool::instance();
		resPool.release(lightAccumulationTex);

		if (flattenedLightAccumBuffer)
			resPool.release(flattenedLightAccumBuffer);
	}

	SmallVector<StringID, 4> RCNodeLightAccumulation::getDependencies(const RendererView& view)
	{
		return {};
	}

	void RCNodeTiledDeferredLighting::render(const RenderCompositorNodeInputs& inputs)
	{
		output = static_cast<RCNodeLightAccumulation*>(inputs.inputNodes[0]);

		RCNodeGBuffer* gbufferNode = static_cast<RCNodeGBuffer*>(inputs.inputNodes[1]);
		RCNodeSceneDepth* sceneDepthNode = static_cast<RCNodeSceneDepth*>(inputs.inputNodes[2]);

		const RendererViewProperties& viewProps = inputs.view.getProperties();
		TiledDeferredLightingMat* tiledDeferredMat = TiledDeferredLightingMat::getVariation(viewProps.numSamples);

		GBufferTextures gbuffer;
		gbuffer.albedo = gbufferNode->albedoTex->texture;
		gbuffer.normals = gbufferNode->normalTex->texture;
		gbuffer.roughMetal = gbufferNode->roughMetalTex->texture;
		gbuffer.depth = sceneDepthNode->depthTex->texture;

		RenderAPI& rapi = RenderAPI::instance();
		rapi.setRenderTarget(output->renderTarget, FBT_DEPTH | FBT_STENCIL, RT_COLOR0 | RT_DEPTH_STENCIL);

		const VisibleLightData& lightData = inputs.viewGroup.getVisibleLightData();
		SPtr<GpuBuffer> flattenedLightAccumBuffer;
		if (output->flattenedLightAccumBuffer)
			flattenedLightAccumBuffer = output->flattenedLightAccumBuffer->buffer;

		tiledDeferredMat->execute(inputs.view, lightData, gbuffer, output->lightAccumulationTex->texture, 
			flattenedLightAccumBuffer);
	}

	void RCNodeTiledDeferredLighting::clear()
	{
		output = nullptr;
	}

	SmallVector<StringID, 4> RCNodeTiledDeferredLighting::getDependencies(const RendererView& view)
	{
		return { RCNodeLightAccumulation::getNodeId(), RCNodeGBuffer::getNodeId(), RCNodeSceneDepth::getNodeId() };
	}

	void RCNodeStandardDeferredLighting::render(const RenderCompositorNodeInputs& inputs)
	{
		GpuResourcePool& resPool = GpuResourcePool::instance();
		const RendererViewProperties& viewProps = inputs.view.getProperties();

		UINT32 width = viewProps.viewRect.width;
		UINT32 height = viewProps.viewRect.height;
		UINT32 numSamples = viewProps.numSamples;

		RCNodeTiledDeferredLighting* tileDeferredNode = static_cast<RCNodeTiledDeferredLighting*>(inputs.inputNodes[0]);
		output = tileDeferredNode->output;

		RCNodeGBuffer* gbufferNode = static_cast<RCNodeGBuffer*>(inputs.inputNodes[1]);
		RCNodeSceneDepth* sceneDepthNode = static_cast<RCNodeSceneDepth*>(inputs.inputNodes[2]);

		// Allocate light occlusion
		mLightOcclusionTex = resPool.get(POOLED_RENDER_TEXTURE_DESC::create2D(PF_R8, width,
			height, TU_RENDERTARGET, numSamples, false));

		bool rebuildRT = false;
		if (mRenderTarget != nullptr)
		{
			rebuildRT |= mRenderTarget->getColorTexture(0) != mLightOcclusionTex->texture;
			rebuildRT |= mRenderTarget->getDepthStencilTexture() != sceneDepthNode->depthTex->texture;
		}
		else
			rebuildRT = true;

		if (rebuildRT)
		{
			RENDER_TEXTURE_DESC lightOcclusionRTDesc;
			lightOcclusionRTDesc.colorSurfaces[0].texture = mLightOcclusionTex->texture;
			lightOcclusionRTDesc.colorSurfaces[0].face = 0;
			lightOcclusionRTDesc.colorSurfaces[0].numFaces = 1;
			lightOcclusionRTDesc.colorSurfaces[0].mipLevel = 0;

			lightOcclusionRTDesc.depthStencilSurface.texture = sceneDepthNode->depthTex->texture;
			lightOcclusionRTDesc.depthStencilSurface.face = 0;
			lightOcclusionRTDesc.depthStencilSurface.numFaces = 1;
			lightOcclusionRTDesc.depthStencilSurface.mipLevel = 0;

			mRenderTarget = RenderTexture::create(lightOcclusionRTDesc);
		}

		GBufferTextures gbuffer;
		gbuffer.albedo = gbufferNode->albedoTex->texture;
		gbuffer.normals = gbufferNode->normalTex->texture;
		gbuffer.roughMetal = gbufferNode->roughMetalTex->texture;
		gbuffer.depth = sceneDepthNode->depthTex->texture;

		const VisibleLightData& lightData = inputs.viewGroup.getVisibleLightData();
		const ShadowRendering& shadowRenderer = inputs.viewGroup.getShadowRenderer();

		Camera* sceneCamera = inputs.view.getSceneCamera();

		// Note: Currently skipping shadow rendering for any views that aren't part of the scene (like temporary ones)
		if (sceneCamera != nullptr)
		{
			RenderAPI& rapi = RenderAPI::instance();
			UINT32 viewIdx = sceneCamera->getRendererId();
			for (UINT32 i = 0; i < (UINT32)LightType::Count; i++)
			{
				LightType lightType = (LightType)i;

				auto& lights = lightData.getLights(lightType);
				UINT32 count = lightData.getNumShadowedLights(lightType);
				UINT32 offset = lightData.getNumUnshadowedLights(lightType);

				for (UINT32 j = 0; j < count; j++)
				{
					rapi.setRenderTarget(mRenderTarget, FBT_DEPTH, RT_DEPTH_STENCIL);

					Rect2 area(0.0f, 0.0f, 1.0f, 1.0f);
					rapi.setViewport(area);

					rapi.clearViewport(FBT_COLOR, Color::ZERO);

					UINT32 lightIdx = offset + j;
					const RendererLight& light = *lights[lightIdx];
					shadowRenderer.renderShadowOcclusion(inputs.scene, inputs.options.shadowFilteringQuality,
						light, viewIdx, gbuffer);

					rapi.setRenderTarget(output->renderTarget, FBT_DEPTH | FBT_STENCIL, RT_COLOR0 | RT_DEPTH_STENCIL);
					StandardDeferred::instance().renderLight(lightType, light, inputs.view, gbuffer,
						mLightOcclusionTex->texture);
				}
			}

			// Makes sure light accumulation can be read by following passes
			rapi.setRenderTarget(nullptr);
		}

		resPool.release(mLightOcclusionTex);
	}

	void RCNodeStandardDeferredLighting::clear()
	{
		output = nullptr;
	}

	SmallVector<StringID, 4> RCNodeStandardDeferredLighting::getDependencies(const RendererView& view)
	{
		SmallVector<StringID, 4> deps;

		deps.push_back(RCNodeTiledDeferredLighting::getNodeId());
		deps.push_back(RCNodeGBuffer::getNodeId());
		deps.push_back(RCNodeSceneDepth::getNodeId());

		if (view.getProperties().numSamples > 1)
			deps.push_back(RCNodeUnflattenLightAccum::getNodeId());

		return deps;
	}

	void RCNodeUnflattenLightAccum::render(const RenderCompositorNodeInputs& inputs)
	{
		RCNodeLightAccumulation* lightAccumNode = static_cast<RCNodeLightAccumulation*>(inputs.inputNodes[0]);
		FlatFramebufferToTextureMat* material = FlatFramebufferToTextureMat::get();

		RenderAPI& rapi = RenderAPI::instance();
		rapi.setRenderTarget(lightAccumNode->renderTarget, FBT_DEPTH | FBT_STENCIL, RT_COLOR0 | RT_DEPTH_STENCIL);
		material->execute(lightAccumNode->flattenedLightAccumBuffer->buffer, lightAccumNode->lightAccumulationTex->texture);
	}

	void RCNodeUnflattenLightAccum::clear()
	{
		output = nullptr;
	}

	SmallVector<StringID, 4> RCNodeUnflattenLightAccum::getDependencies(const RendererView& view)
	{
		return { RCNodeLightAccumulation::getNodeId() };
	}

	void RCNodeIndirectLighting::render(const RenderCompositorNodeInputs& inputs)
	{
		if (!inputs.view.getRenderSettings().enableIndirectLighting)
			return;

		RCNodeGBuffer* gbufferNode = static_cast<RCNodeGBuffer*>(inputs.inputNodes[0]);
		RCNodeSceneDepth* sceneDepthNode = static_cast<RCNodeSceneDepth*>(inputs.inputNodes[1]);
		RCNodeLightAccumulation* lightAccumNode = static_cast <RCNodeLightAccumulation*>(inputs.inputNodes[2]);

		SPtr<Texture> ssao;
		if (inputs.view.getRenderSettings().ambientOcclusion.enabled)
		{
			RCNodeSSAO* ssaoNode = static_cast<RCNodeSSAO*>(inputs.inputNodes[4]);
			ssao = ssaoNode->output->texture;
		}
		else
			ssao = Texture::WHITE;

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

			TetrahedraRenderMat* renderTetrahedra = TetrahedraRenderMat::getVariation(viewProps.numSamples > 1);
			renderTetrahedra->execute(inputs.view, sceneDepthNode->depthTex->texture, lpInfo.tetrahedraVolume, rt);

			rt = nullptr;
			resPool.release(depthTex);

			evaluateMat = IrradianceEvaluateMat::getVariation(viewProps.numSamples, false);
		}
		else // Sky only
		{
			evaluateMat = IrradianceEvaluateMat::getVariation(viewProps.numSamples, true);
		}

		GBufferTextures gbuffer;
		gbuffer.albedo = gbufferNode->albedoTex->texture;
		gbuffer.normals = gbufferNode->normalTex->texture;
		gbuffer.roughMetal = gbufferNode->roughMetalTex->texture;
		gbuffer.depth = sceneDepthNode->depthTex->texture;

		SPtr<Texture> volumeIndicesTex;
		if (volumeIndices)
			volumeIndicesTex = volumeIndices->texture;

		evaluateMat->execute(inputs.view, gbuffer, volumeIndicesTex, lpInfo, inputs.scene.skybox, ssao, 
			lightAccumNode->renderTarget);

		if(volumeIndices)
			resPool.release(volumeIndices);
	}

	void RCNodeIndirectLighting::clear()
	{
		// Do nothing
	}

	SmallVector<StringID, 4> RCNodeIndirectLighting::getDependencies(const RendererView& view)
	{
		SmallVector<StringID, 4> deps;

		deps.push_back(RCNodeGBuffer::getNodeId());
		deps.push_back(RCNodeSceneDepth::getNodeId());
		deps.push_back(RCNodeLightAccumulation::getNodeId());
		deps.push_back(RCNodeStandardDeferredLighting::getNodeId());

		if(view.getRenderSettings().ambientOcclusion.enabled)
			deps.push_back(RCNodeSSAO::getNodeId());

		if (view.getProperties().numSamples > 1)
			deps.push_back(RCNodeUnflattenLightAccum::getNodeId());

		return deps;
	}

	void RCNodeTiledDeferredIBL::render(const RenderCompositorNodeInputs& inputs)
	{
		const RenderSettings& rs = inputs.view.getRenderSettings();

		RCNodeSceneColor* sceneColorNode = static_cast<RCNodeSceneColor*>(inputs.inputNodes[0]);
		RCNodeGBuffer* gbufferNode = static_cast<RCNodeGBuffer*>(inputs.inputNodes[1]);
		RCNodeSceneDepth* sceneDepthNode = static_cast<RCNodeSceneDepth*>(inputs.inputNodes[2]);
		RCNodeLightAccumulation* lightAccumNode = static_cast <RCNodeLightAccumulation*>(inputs.inputNodes[3]);

		SPtr<Texture> ssr;
		if (rs.screenSpaceReflections.enabled)
		{
			RCNodeSSR* ssrNode = static_cast<RCNodeSSR*>(inputs.inputNodes[5]);
			ssr = ssrNode->output->texture;
		}
		else
			ssr = Texture::BLACK;

		SPtr<Texture> ssao;
		if (rs.ambientOcclusion.enabled)
		{
			RCNodeSSAO* ssaoNode = static_cast<RCNodeSSAO*>(inputs.inputNodes[6]);
			ssao = ssaoNode->output->texture;
		}
		else
			ssao = Texture::WHITE;

		const RendererViewProperties& viewProps = inputs.view.getProperties();
		TiledDeferredImageBasedLightingMat* material = TiledDeferredImageBasedLightingMat::getVariation(viewProps.numSamples);

		TiledDeferredImageBasedLightingMat::Inputs iblInputs;
		iblInputs.gbuffer.albedo = gbufferNode->albedoTex->texture;
		iblInputs.gbuffer.normals = gbufferNode->normalTex->texture;
		iblInputs.gbuffer.roughMetal = gbufferNode->roughMetalTex->texture;
		iblInputs.gbuffer.depth = sceneDepthNode->depthTex->texture;
		iblInputs.sceneColorTex = sceneColorNode->sceneColorTex->texture;
		iblInputs.lightAccumulation = lightAccumNode->lightAccumulationTex->texture;
		iblInputs.preIntegratedGF = RendererTextures::preintegratedEnvGF;
		iblInputs.ambientOcclusion = ssao;
		iblInputs.ssr = ssr;

		if(sceneColorNode->flattenedSceneColorBuffer)
			iblInputs.sceneColorBuffer = sceneColorNode->flattenedSceneColorBuffer->buffer;

		material->execute(inputs.view, inputs.scene, inputs.viewGroup.getVisibleReflProbeData(), iblInputs);
	}

	void RCNodeTiledDeferredIBL::clear()
	{
		output = nullptr;
	}

	SmallVector<StringID, 4> RCNodeTiledDeferredIBL::getDependencies(const RendererView& view)
	{
		SmallVector<StringID, 4> deps;

		deps.push_back(RCNodeSceneColor::getNodeId());
		deps.push_back(RCNodeGBuffer::getNodeId());
		deps.push_back(RCNodeSceneDepth::getNodeId());
		deps.push_back(RCNodeLightAccumulation::getNodeId());
		deps.push_back(RCNodeIndirectLighting::getNodeId());
		deps.push_back(RCNodeSSR::getNodeId());

		if(view.getRenderSettings().ambientOcclusion.enabled)
			deps.push_back(RCNodeSSAO::getNodeId());

		return deps;
	}

	void RCNodeUnflattenSceneColor::render(const RenderCompositorNodeInputs& inputs)
	{
		RCNodeSceneColor* sceneColorNode = static_cast<RCNodeSceneColor*>(inputs.inputNodes[0]);
		FlatFramebufferToTextureMat* material = FlatFramebufferToTextureMat::get();

		int readOnlyFlags = FBT_DEPTH | FBT_STENCIL;

		RenderAPI& rapi = RenderAPI::instance();
		rapi.setRenderTarget(sceneColorNode->renderTarget, readOnlyFlags, RT_COLOR0 | RT_DEPTH_STENCIL);

		Rect2 area(0.0f, 0.0f, 1.0f, 1.0f);
		rapi.setViewport(area);

		material->execute(sceneColorNode->flattenedSceneColorBuffer->buffer, sceneColorNode->sceneColorTex->texture);
	}

	void RCNodeUnflattenSceneColor::clear()
	{
		output = nullptr;
	}

	SmallVector<StringID, 4> RCNodeUnflattenSceneColor::getDependencies(const RendererView& view)
	{
		return { RCNodeSceneColor::getNodeId() };
	}

	void RCNodeClusteredForward::render(const RenderCompositorNodeInputs& inputs)
	{
		const SceneInfo& sceneInfo = inputs.scene;
		const RendererViewProperties& viewProps = inputs.view.getProperties();

		const VisibleLightData& visibleLightData = inputs.viewGroup.getVisibleLightData();
		const VisibleReflProbeData& visibleReflProbeData = inputs.viewGroup.getVisibleReflProbeData();

		const LightGrid& lightGrid = inputs.view.getLightGrid();

		SPtr<GpuParamBlockBuffer> gridParams;
		SPtr<GpuBuffer> gridLightOffsetsAndSize, gridLightIndices;
		SPtr<GpuBuffer> gridProbeOffsetsAndSize, gridProbeIndices;
		lightGrid.getOutputs(gridLightOffsetsAndSize, gridLightIndices, gridProbeOffsetsAndSize, gridProbeIndices, 
			gridParams);

		// Prepare refl. probe param buffer
		ReflProbeParamBuffer reflProbeParamBuffer;
		reflProbeParamBuffer.populate(sceneInfo.skybox, visibleReflProbeData, sceneInfo.reflProbeCubemapsTex, 
			viewProps.renderingReflections);

		SPtr<Texture> skyFilteredRadiance;
		if(sceneInfo.skybox)
			skyFilteredRadiance = sceneInfo.skybox->getFilteredRadiance();

		// Prepare objects for rendering
		const VisibilityInfo& visibility = inputs.view.getVisibilityMasks();
		UINT32 numRenderables = (UINT32)sceneInfo.renderables.size();
		for (UINT32 i = 0; i < numRenderables; i++)
		{
			if (!visibility.renderables[i])
				continue;

			for (auto& element : sceneInfo.renderables[i]->elements)
			{
				bool isTransparent = (element.material->getShader()->getFlags() & (UINT32)ShaderFlags::Transparent) != 0;
				if (!isTransparent)
					continue;

				// Note: It would be nice to be able to set this once and keep it, only updating if the buffers actually
				// change (e.g. when growing). Although technically the internal systems should be smart enough to
				// avoid updates unless objects actually changed.
				if (element.gridParamsBindingIdx != -1)
					element.params->setParamBlockBuffer(element.gridParamsBindingIdx, gridParams, true);

				element.gridLightOffsetsAndSizeParam.set(gridLightOffsetsAndSize);
				element.gridLightIndicesParam.set(gridLightIndices);
				element.lightsBufferParam.set(visibleLightData.getLightBuffer());

				// Image based lighting params
				ImageBasedLightingParams& iblParams = element.imageBasedParams;
				if (iblParams.reflProbeParamsBindingIdx != -1)
					element.params->setParamBlockBuffer(iblParams.reflProbeParamsBindingIdx, reflProbeParamBuffer.buffer);

				element.gridProbeOffsetsAndSizeParam.set(gridProbeOffsetsAndSize);

				iblParams.reflectionProbeIndicesParam.set(gridProbeIndices);
				iblParams.reflectionProbesParam.set(visibleReflProbeData.getProbeBuffer());

				iblParams.skyReflectionsTexParam.set(skyFilteredRadiance);
				iblParams.ambientOcclusionTexParam.set(Texture::WHITE); // Note: Add SSAO here?
				iblParams.ssrTexParam.set(Texture::BLACK); // Note: Add SSR here?

				iblParams.reflectionProbeCubemapsTexParam.set(sceneInfo.reflProbeCubemapsTex);
				iblParams.preintegratedEnvBRDFParam.set(RendererTextures::preintegratedEnvGF);
			}
		}

		// TODO: Transparent objects cannot receive shadows. In order to support this I'd have to render the light occlusion
		// for all lights affecting this object into a single (or a few) textures. I can likely use texture arrays for this,
		// or to avoid sampling many textures, perhaps just jam it all in one or few texture channels. 
		const Vector<RenderQueueElement>& transparentElements = inputs.view.getTransparentQueue()->getSortedElements();
		for (auto iter = transparentElements.begin(); iter != transparentElements.end(); ++iter)
		{
			BeastRenderableElement* renderElem = static_cast<BeastRenderableElement*>(iter->renderElem);
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
		return { RCNodeSceneColor::getNodeId(), RCNodeSkybox::getNodeId() };
	}

	void RCNodeSkybox::render(const RenderCompositorNodeInputs& inputs)
	{
		Skybox* skybox = inputs.scene.skybox;
		SPtr<Texture> radiance = skybox ? skybox->getTexture() : nullptr;

		if (radiance != nullptr)
		{
			SkyboxMat* material = SkyboxMat::getVariation(false);
			material->bind(inputs.view.getPerViewBuffer());
			material->setParams(radiance, Color::White);
		}
		else
		{
			Color clearColor = inputs.view.getProperties().clearColor;

			SkyboxMat* material = SkyboxMat::getVariation(true);
			material->bind(inputs.view.getPerViewBuffer());
			material->setParams(nullptr, clearColor);
		}

		RCNodeSceneColor* sceneColorNode = static_cast<RCNodeSceneColor*>(inputs.inputNodes[1]);
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

		deps.push_back(RCNodeTiledDeferredIBL::getNodeId());
		deps.push_back(RCNodeSceneColor::getNodeId());

		if (view.getProperties().numSamples > 1)
			deps.push_back(RCNodeUnflattenSceneColor::getNodeId());

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

			// Generate histogram
			SPtr<PooledRenderTexture> eyeAdaptHistogram = resPool.get(
				EyeAdaptHistogramMat::getOutputDesc(downsampledScene->texture));
			EyeAdaptHistogramMat* eyeAdaptHistogramMat = EyeAdaptHistogramMat::get();
			eyeAdaptHistogramMat->execute(downsampledScene->texture, eyeAdaptHistogram->texture, settings.autoExposure);

			// Reduce histogram
			SPtr<PooledRenderTexture> reducedHistogram = resPool.get(EyeAdaptHistogramReduceMat::getOutputDesc());

			SPtr<Texture> prevFrameEyeAdaptation;
			if (prevEyeAdaptation != nullptr)
				prevFrameEyeAdaptation = prevEyeAdaptation->texture;

			EyeAdaptHistogramReduceMat* eyeAdaptHistogramReduce = EyeAdaptHistogramReduceMat::get();
			eyeAdaptHistogramReduce->execute(downsampledScene->texture, eyeAdaptHistogram->texture, 
				prevFrameEyeAdaptation, reducedHistogram->renderTexture);

			resPool.release(downsampledScene);
			downsampledScene = nullptr;

			resPool.release(eyeAdaptHistogram);
			eyeAdaptHistogram = nullptr;

			// Generate eye adaptation value
			eyeAdaptation = resPool.get(EyeAdaptationMat::getOutputDesc());
			EyeAdaptationMat* eyeAdaptationMat = EyeAdaptationMat::get();
			eyeAdaptationMat->execute(reducedHistogram->texture, eyeAdaptation->renderTexture, inputs.frameInfo.timeDelta,
				settings.autoExposure, settings.exposureScale);

			resPool.release(reducedHistogram);
			reducedHistogram = nullptr;
		}
		else
		{
			if(prevEyeAdaptation)
				resPool.release(prevEyeAdaptation);

			prevEyeAdaptation = nullptr;
			eyeAdaptation = nullptr;
		}

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
					if(mTonemapLUT == nullptr)
						mTonemapLUT = resPool.get(CreateTonemapLUTMat::getOutputDesc());

					CreateTonemapLUTMat* createLUT = CreateTonemapLUTMat::get();
					createLUT->execute(mTonemapLUT->texture, settings);

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

		TonemappingMat* tonemapping = TonemappingMat::getVariation(gammaOnly, autoExposure, msaa);

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

			output = resPool.get(POOLED_RENDER_TEXTURE_DESC::create2D(PF_D32_S8X24, width, height, TU_RENDERTARGET, 1, false)); 

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

			BuildHiZMat* material = BuildHiZMat::get();
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

			BuildHiZMat* material = BuildHiZMat::get();
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

		GpuResourcePool& resPool = GpuResourcePool::instance();
		const RendererViewProperties& viewProps = inputs.view.getProperties();
		const AmbientOcclusionSettings& settings = inputs.view.getRenderSettings().ambientOcclusion;

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
		if (quality > 1)
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
		output = resPool.get(POOLED_RENDER_TEXTURE_DESC::create2D(PF_R8, width, height, TU_RENDERTARGET));

		{
			if(setupTex0)
				textures.aoSetup = setupTex0->texture;

			if(downAOTex0)
				textures.aoDownsampled = downAOTex0->texture;

			bool upsample = numDownsampleLevels > 0;
			SSAOMat* ssaoMat = SSAOMat::getVariation(upsample, true, quality);
			ssaoMat->execute(inputs.view, textures, output->renderTexture, settings);
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
			const RenderTargetProperties& rtProps = output->renderTexture->getProperties();

			POOLED_RENDER_TEXTURE_DESC desc = POOLED_RENDER_TEXTURE_DESC::create2D(PF_R8, rtProps.getWidth(), 
				rtProps.getHeight(), TU_RENDERTARGET);
			SPtr<PooledRenderTexture> blurIntermediateTex = GpuResourcePool::instance().get(desc);

			SSAOBlurMat* blurHorz = SSAOBlurMat::getVariation(true);
			SSAOBlurMat* blurVert = SSAOBlurMat::getVariation(false);

			blurHorz->execute(inputs.view, output->texture, sceneDepth, blurIntermediateTex->renderTexture, DEPTH_RANGE);
			blurVert->execute(inputs.view, blurIntermediateTex->texture, sceneDepth, output->renderTexture, DEPTH_RANGE);

			GpuResourcePool::instance().release(blurIntermediateTex);
		}
	}

	void RCNodeSSAO::clear()
	{
		GpuResourcePool& resPool = GpuResourcePool::instance();
		resPool.release(output);
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
		if (settings.enabled)
		{
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
			if(viewProps.numSamples > 1)
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

			SSRStencilMat* stencilMat = SSRStencilMat::getVariation(viewProps.numSamples > 1);

			// Note: Making the assumption that the stencil buffer is clear at this point
			rapi.setRenderTarget(resolvedSceneDepthNode->output->renderTexture);
			stencilMat->execute(inputs.view, gbuffer, settings);

			SPtr<PooledRenderTexture> traceOutput = resPool.get(POOLED_RENDER_TEXTURE_DESC::create2D(PF_RGBA16F, width, 
				height, TU_RENDERTARGET));

			RENDER_TEXTURE_DESC traceRtDesc;
			traceRtDesc.colorSurfaces[0].texture = traceOutput->texture;
			traceRtDesc.depthStencilSurface.texture = resolvedSceneDepthNode->output->texture;

			SPtr<RenderTexture> traceRt = RenderTexture::create(traceRtDesc);

			rapi.setRenderTarget(traceRt);
			rapi.clearRenderTarget(FBT_COLOR);

			SSRTraceMat* traceMat = SSRTraceMat::getVariation(settings.quality, viewProps.numSamples > 1);
			traceMat->execute(inputs.view, gbuffer, sceneColor, hiZ, settings, traceRt);

			if (resolvedSceneColor)
			{
				resPool.release(resolvedSceneColor);
				resolvedSceneColor = nullptr;
			}

			if (mPrevFrame)
			{
				output = resPool.get(POOLED_RENDER_TEXTURE_DESC::create2D(PF_RGBA16F, width, height, TU_RENDERTARGET));

				rapi.setRenderTarget(output->renderTexture);
				rapi.clearRenderTarget(FBT_COLOR);

				SSRResolveMat* resolveMat = SSRResolveMat::getVariation(viewProps.numSamples > 1);
				resolveMat->execute(inputs.view, mPrevFrame->texture, traceOutput->texture, sceneDepthNode->depthTex->texture,
					output->renderTexture);

				resPool.release(traceOutput);
			}
			else
				output = traceOutput;

			RenderAPI::instance().setRenderTarget(nullptr);
		}
		else
			deallocOutputs();
	}

	void RCNodeSSR::clear()
	{
		GpuResourcePool& resPool = GpuResourcePool::instance();

		if(mPrevFrame)
			resPool.release(mPrevFrame);

		mPrevFrame = output;
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

			if (view.getProperties().numSamples > 1)
				deps.push_back(RCNodeUnflattenLightAccum::getNodeId());
		}

		return deps;
	}
}}
