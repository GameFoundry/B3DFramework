//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsRenderCompositor.h"
#include "BsGpuResourcePool.h"
#include "BsRendererView.h"
#include "BsRendererUtility.h"
#include "BsMesh.h"
#include "BsGpuBuffer.h"
#include "BsStandardDeferredLighting.h"
#include "BsGlobalMaterials.h"
#include "BsRenderBeastOptions.h"
#include "BsCamera.h"
#include "BsRendererScene.h"
#include "BsIBLUtility.h"

namespace bs { namespace ct
{
	UnorderedMap<StringID, RenderCompositor::NodeType*> RenderCompositor::mNodeTypes;

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

	void RenderCompositor::execute(const RendererViewGroup& viewGroup, const RendererView& view, const SceneInfo& scene, 
		const RenderBeastOptions& options) const
	{
		if (!mIsValid)
			return;

		bs_frame_mark();
		{
			FrameVector<const NodeInfo*> activeNodes;

			UINT32 idx = 0;
			for (auto& entry : mNodeInfos)
			{
				RenderCompositorNodeInputs inputs(viewGroup, view, scene, options, entry.inputs);
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
		albedoTex = resPool.get(POOLED_RENDER_TEXTURE_DESC::create2D(PF_R8G8B8A8, width, height, TU_RENDERTARGET,
			numSamples, true));
		normalTex = resPool.get(POOLED_RENDER_TEXTURE_DESC::create2D(PF_UNORM_R10G10B10A2, width, height, TU_RENDERTARGET,
			numSamples, false));
		roughMetalTex = resPool.get(POOLED_RENDER_TEXTURE_DESC::create2D(PF_FLOAT16_RG, width, height, TU_RENDERTARGET,
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

		// Render base pass
		RenderAPI& rapi = RenderAPI::instance();
		rapi.setRenderTarget(renderTarget);

		Rect2 area(0.0f, 0.0f, 1.0f, 1.0f);
		rapi.setViewport(area);

		// Clear depth & stencil according to user defined values, don't clear color as all values will get written to
		UINT32 clearFlags = viewProps.clearFlags & ~FBT_COLOR;
		if (clearFlags != 0)
		{
			rapi.clearViewport(clearFlags, viewProps.clearColor, viewProps.clearDepthValue, 
				viewProps.clearStencilValue, 0x01);
		}

		// Clear all non primary targets (Note: I could perhaps clear all but albedo, since it stores a per-pixel write mask)
		rapi.clearViewport(FBT_COLOR, Color::ZERO, 1.0f, 0, 0xFF & ~0x01);

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
		sceneColorTex = resPool.get(POOLED_RENDER_TEXTURE_DESC::create2D(PF_FLOAT16_RGBA, width, height, TU_RENDERTARGET | 
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

		lightAccumulationTex = resPool.get(POOLED_RENDER_TEXTURE_DESC::create2D(PF_FLOAT16_RGBA, width,
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
		ITiledDeferredLightingMat* tiledDeferredMat = 
			GlobalMaterials::instance().getTileDeferredLighting(viewProps.numSamples);

		GBufferInput gbuffer;
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

		GBufferInput gbuffer;
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
		FlatFramebufferToTextureMat* material = GlobalMaterials::instance().getUnflattenBuffer();

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

	void RCNodeTiledDeferredIBL::render(const RenderCompositorNodeInputs& inputs)
	{
		RCNodeSceneColor* sceneColorNode = static_cast<RCNodeSceneColor*>(inputs.inputNodes[0]);
		RCNodeGBuffer* gbufferNode = static_cast<RCNodeGBuffer*>(inputs.inputNodes[1]);
		RCNodeSceneDepth* sceneDepthNode = static_cast<RCNodeSceneDepth*>(inputs.inputNodes[2]);
		RCNodeLightAccumulation* lightAccumNode = static_cast <RCNodeLightAccumulation*>(inputs.inputNodes[3]);

		const RendererViewProperties& viewProps = inputs.view.getProperties();
		ITiledDeferredImageBasedLightingMat* material = GlobalMaterials::instance().getTileDeferredIBL(viewProps.numSamples);

		TiledDeferredImageBasedLighting::Inputs iblInputs;
		iblInputs.gbuffer.albedo = gbufferNode->albedoTex->texture;
		iblInputs.gbuffer.normals = gbufferNode->normalTex->texture;
		iblInputs.gbuffer.roughMetal = gbufferNode->roughMetalTex->texture;
		iblInputs.gbuffer.depth = sceneDepthNode->depthTex->texture;
		iblInputs.sceneColorTex = sceneColorNode->sceneColorTex->texture;
		iblInputs.lightAccumulation = lightAccumNode->lightAccumulationTex->texture;
		iblInputs.preIntegratedGF = IBLUtility::getPreintegratedEnvBRDF();

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
		deps.push_back(RCNodeStandardDeferredLighting::getNodeId());

		return deps;
	}

	void RCNodeUnflattenSceneColor::render(const RenderCompositorNodeInputs& inputs)
	{
		RCNodeSceneColor* sceneColorNode = static_cast<RCNodeSceneColor*>(inputs.inputNodes[0]);
		FlatFramebufferToTextureMat* material = GlobalMaterials::instance().getUnflattenBuffer();

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

	void RCNodeSkybox::render(const RenderCompositorNodeInputs& inputs)
	{
		const SkyInfo& sky = inputs.scene.sky;

		if (sky.radiance != nullptr)
		{
			SkyboxMat<false>* material = GlobalMaterials::instance().getSkyboxTexture();
			material->bind(inputs.view.getPerViewBuffer());
			material->setParams(sky.radiance, Color::White);
		}
		else
		{
			Color clearColor = inputs.view.getProperties().clearColor;

			SkyboxMat<true>* material = GlobalMaterials::instance().getSkyboxColor();
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
		RCNodeSceneColor* sceneColorNode = static_cast<RCNodeSceneColor*>(inputs.inputNodes[0]);

		SPtr<RenderTarget> target = viewProps.target;

		RenderAPI& rapi = RenderAPI::instance();
		rapi.setRenderTarget(target);
		rapi.setViewport(viewProps.nrmViewRect);

		SPtr<Texture> sceneColor = sceneColorNode->sceneColorTex->texture;
		gRendererUtility().blit(sceneColor, Rect2I::EMPTY, viewProps.flipView);
	}

	void RCNodeFinalResolve::clear()
	{ }

	SmallVector<StringID, 4> RCNodeFinalResolve::getDependencies(const RendererView& view)
	{
		return{ RCNodeSceneColor::getNodeId(), RCNodeSkybox::getNodeId() };
	}
}}
