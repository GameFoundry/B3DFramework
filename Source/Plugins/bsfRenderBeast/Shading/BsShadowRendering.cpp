//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsShadowRendering.h"
#include "BsRendererView.h"
#include "BsRendererScene.h"
#include "Renderer/BsLight.h"
#include "Renderer/BsRendererUtility.h"
#include "Material/BsGpuParamsSet.h"
#include "Mesh/BsMesh.h"
#include "Renderer/BsCamera.h"
#include "Utility/BsBitwise.h"
#include "RenderAPI/BsVertexDataDesc.h"
#include "Renderer/BsRenderer.h"
#include "BsRendererRenderable.h"

namespace bs { namespace ct
{
	ShadowParamsDef gShadowParamsDef;

	void ShadowDepthNormalMat::bind(const SPtr<GpuParamBlockBuffer>& shadowParams)
	{
		mParams->setParamBlockBuffer("ShadowParams", shadowParams);

		RenderAPI::instance().setGraphicsPipeline(mGfxPipeline);
		RenderAPI::instance().setStencilRef(mStencilRef);
	}
	
	void ShadowDepthNormalMat::setPerObjectBuffer(const SPtr<GpuParamBlockBuffer>& perObjectParams)
	{
		mParams->setParamBlockBuffer("PerObject", perObjectParams);

		RenderAPI::instance().setGpuParams(mParams);
	}
	
	ShadowDepthNormalMat* ShadowDepthNormalMat::getVariation(bool skinned, bool morph)
	{
		if(skinned)
		{
			if(morph)
				return get(getVariation<true, true>());

			return get(getVariation<true, false>());
		}
		else
		{
			if(morph)
				return get(getVariation<false, true>());

			return get(getVariation<false, false>());
		}
	}

	ShadowDepthNormalNoPSMat::ShadowDepthNormalNoPSMat()
	{ }

	void ShadowDepthNormalNoPSMat::bind(const SPtr<GpuParamBlockBuffer>& shadowParams)
	{
		mParams->setParamBlockBuffer("ShadowParams", shadowParams);

		RenderAPI::instance().setGraphicsPipeline(mGfxPipeline);
		RenderAPI::instance().setStencilRef(mStencilRef);
	}

	void ShadowDepthNormalNoPSMat::setPerObjectBuffer(const SPtr<GpuParamBlockBuffer>& perObjectParams)
	{
		mParams->setParamBlockBuffer("PerObject", perObjectParams);

		RenderAPI::instance().setGpuParams(mParams);
	}

	ShadowDepthNormalNoPSMat* ShadowDepthNormalNoPSMat::getVariation(bool skinned, bool morph)
	{
		if(skinned)
		{
			if(morph)
				return get(getVariation<true, true>());

			return get(getVariation<true, false>());
		}
		else
		{
			if(morph)
				return get(getVariation<false, true>());

			return get(getVariation<false, false>());
		}
	}

	ShadowDepthDirectionalMat::ShadowDepthDirectionalMat()
	{ }

	void ShadowDepthDirectionalMat::bind(const SPtr<GpuParamBlockBuffer>& shadowParams)
	{
		mParams->setParamBlockBuffer("ShadowParams", shadowParams);

		RenderAPI::instance().setGraphicsPipeline(mGfxPipeline);
		RenderAPI::instance().setStencilRef(mStencilRef);
	}
	
	void ShadowDepthDirectionalMat::setPerObjectBuffer(const SPtr<GpuParamBlockBuffer>& perObjectParams)
	{
		mParams->setParamBlockBuffer("PerObject", perObjectParams);
		RenderAPI::instance().setGpuParams(mParams);
	}
	
	ShadowDepthDirectionalMat* ShadowDepthDirectionalMat::getVariation(bool skinned, bool morph)
	{
		if(skinned)
		{
			if(morph)
				return get(getVariation<true, true>());

			return get(getVariation<true, false>());
		}
		else
		{
			if(morph)
				return get(getVariation<false, true>());

			return get(getVariation<false, false>());
		}
	}

	ShadowCubeMatricesDef gShadowCubeMatricesDef;
	ShadowCubeMasksDef gShadowCubeMasksDef;

	ShadowDepthCubeMat::ShadowDepthCubeMat()
	{ }

	void ShadowDepthCubeMat::bind(const SPtr<GpuParamBlockBuffer>& shadowParams, 
		const SPtr<GpuParamBlockBuffer>& shadowCubeMatrices)
	{
		mParams->setParamBlockBuffer("ShadowParams", shadowParams);
		mParams->setParamBlockBuffer("ShadowCubeMatrices", shadowCubeMatrices);

		RenderAPI::instance().setGraphicsPipeline(mGfxPipeline);
		RenderAPI::instance().setStencilRef(mStencilRef);
	}

	void ShadowDepthCubeMat::setPerObjectBuffer(const SPtr<GpuParamBlockBuffer>& perObjectParams,
		const SPtr<GpuParamBlockBuffer>& shadowCubeMasks)
	{
		mParams->setParamBlockBuffer("PerObject", perObjectParams);
		mParams->setParamBlockBuffer("ShadowCubeMasks", shadowCubeMasks);

		RenderAPI::instance().setGpuParams(mParams);
	}
	
	ShadowDepthCubeMat* ShadowDepthCubeMat::getVariation(bool skinned, bool morph)
	{
		if(skinned)
		{
			if(morph)
				return get(getVariation<true, true>());

			return get(getVariation<true, false>());
		}
		else
		{
			if(morph)
				return get(getVariation<false, true>());

			return get(getVariation<false, false>());
		}
	}

	ShadowProjectParamsDef gShadowProjectParamsDef;
	ShadowProjectVertParamsDef gShadowProjectVertParamsDef;

	ShadowProjectStencilMat::ShadowProjectStencilMat()
	{
		mVertParams = gShadowProjectVertParamsDef.createBuffer();
		if(mParams->hasParamBlock(GPT_VERTEX_PROGRAM, "VertParams"))
			mParams->setParamBlockBuffer(GPT_VERTEX_PROGRAM, "VertParams", mVertParams);
	}

	void ShadowProjectStencilMat::bind(const SPtr<GpuParamBlockBuffer>& perCamera)
	{
		Vector4 lightPosAndScale(0, 0, 0, 1);
		gShadowProjectVertParamsDef.gPositionAndScale.set(mVertParams, lightPosAndScale);

		mParams->setParamBlockBuffer("PerCamera", perCamera);

		RendererMaterial::bind();
	}

	ShadowProjectStencilMat* ShadowProjectStencilMat::getVariation(bool directional, bool useZFailStencil)
	{
		if(directional)
			return get(getVariation<true, true>());
		else
		{
			if (useZFailStencil)
				return get(getVariation<false, true>());
			else
				return get(getVariation<false, false>());
		}
	}

	ShadowProjectMat::ShadowProjectMat()
		: mGBufferParams(GPT_FRAGMENT_PROGRAM, mParams)
	{
		mParams->getTextureParam(GPT_FRAGMENT_PROGRAM, "gShadowTex", mShadowMapParam);
		if(mParams->hasSamplerState(GPT_FRAGMENT_PROGRAM, "gShadowSampler"))
			mParams->getSamplerStateParam(GPT_FRAGMENT_PROGRAM, "gShadowSampler", mShadowSamplerParam);
		else
			mParams->getSamplerStateParam(GPT_FRAGMENT_PROGRAM, "gShadowTex", mShadowSamplerParam);

		SAMPLER_STATE_DESC desc;
		desc.minFilter = FO_POINT;
		desc.magFilter = FO_POINT;
		desc.mipFilter = FO_POINT;
		desc.addressMode.u = TAM_CLAMP;
		desc.addressMode.v = TAM_CLAMP;
		desc.addressMode.w = TAM_CLAMP;

		mSamplerState = SamplerState::create(desc);

		mVertParams = gShadowProjectVertParamsDef.createBuffer();
		if(mParams->hasParamBlock(GPT_VERTEX_PROGRAM, "VertParams"))
			mParams->setParamBlockBuffer(GPT_VERTEX_PROGRAM, "VertParams", mVertParams);
	}

	void ShadowProjectMat::bind(const ShadowProjectParams& params)
	{
		Vector4 lightPosAndScale(Vector3(0.0f, 0.0f, 0.0f), 1.0f);
		gShadowProjectVertParamsDef.gPositionAndScale.set(mVertParams, lightPosAndScale);

		mGBufferParams.bind(params.gbuffer);

		mShadowMapParam.set(params.shadowMap);
		mShadowSamplerParam.set(mSamplerState);

		mParams->setParamBlockBuffer("Params", params.shadowParams);
		mParams->setParamBlockBuffer("PerCamera", params.perCamera);

		RendererMaterial::bind();
	}

	ShadowProjectMat* ShadowProjectMat::getVariation(UINT32 quality, bool directional, bool MSAA)
	{
#define BIND_MAT(QUALITY)											\
	{																\
		if(directional)												\
			if (MSAA)												\
				return get(getVariation<QUALITY, true, true>());	\
			else													\
				return get(getVariation<QUALITY, true, false>());	\
		else														\
			if (MSAA)												\
				return get(getVariation<QUALITY, false, true>());	\
			else													\
				return get(getVariation<QUALITY, false, false>());	\
	}

		if(quality <= 1)
			BIND_MAT(1)
		else if(quality == 2)
			BIND_MAT(2)
		else if(quality == 3)
			BIND_MAT(3)
		else // 4 or higher
			BIND_MAT(4)

#undef BIND_MAT
	}

	ShadowProjectOmniParamsDef gShadowProjectOmniParamsDef;

	ShadowProjectOmniMat::ShadowProjectOmniMat()
		: mGBufferParams(GPT_FRAGMENT_PROGRAM, mParams)
	{
		mParams->getTextureParam(GPT_FRAGMENT_PROGRAM, "gShadowCubeTex", mShadowMapParam);

		if(mParams->hasSamplerState(GPT_FRAGMENT_PROGRAM, "gShadowCubeSampler"))
			mParams->getSamplerStateParam(GPT_FRAGMENT_PROGRAM, "gShadowCubeSampler", mShadowSamplerParam);
		else
			mParams->getSamplerStateParam(GPT_FRAGMENT_PROGRAM, "gShadowCubeTex", mShadowSamplerParam);

		SAMPLER_STATE_DESC desc;
		desc.minFilter = FO_LINEAR;
		desc.magFilter = FO_LINEAR;
		desc.mipFilter = FO_POINT;
		desc.addressMode.u = TAM_CLAMP;
		desc.addressMode.v = TAM_CLAMP;
		desc.addressMode.w = TAM_CLAMP;
		desc.comparisonFunc = CMPF_GREATER_EQUAL;

		mSamplerState = SamplerState::create(desc);

		mVertParams = gShadowProjectVertParamsDef.createBuffer();
		if(mParams->hasParamBlock(GPT_VERTEX_PROGRAM, "VertParams"))
			mParams->setParamBlockBuffer(GPT_VERTEX_PROGRAM, "VertParams", mVertParams);
	}

	void ShadowProjectOmniMat::bind(const ShadowProjectParams& params)
	{
		Vector4 lightPosAndScale(params.light.getTransform().getPosition(), params.light.getAttenuationRadius());
		gShadowProjectVertParamsDef.gPositionAndScale.set(mVertParams, lightPosAndScale);

		mGBufferParams.bind(params.gbuffer);

		mShadowMapParam.set(params.shadowMap);
		mShadowSamplerParam.set(mSamplerState);

		mParams->setParamBlockBuffer("Params", params.shadowParams);
		mParams->setParamBlockBuffer("PerCamera", params.perCamera);

		RendererMaterial::bind();
	}

	ShadowProjectOmniMat* ShadowProjectOmniMat::getVariation(UINT32 quality, bool inside, bool MSAA)
	{
#define BIND_MAT(QUALITY)											\
	{																\
		if(inside)													\
			if (MSAA)												\
				return get(getVariation<QUALITY, true, true>());	\
			else													\
				return get(getVariation<QUALITY, true, false>());	\
		else														\
			if (MSAA)												\
				return get(getVariation<QUALITY, false, true>());	\
			else													\
				return get(getVariation<QUALITY, false, false>());	\
	}

		if(quality <= 1)
			BIND_MAT(1)
		else if(quality == 2)
			BIND_MAT(2)
		else if(quality == 3)
			BIND_MAT(3)
		else // 4 or higher
			BIND_MAT(4)

#undef BIND_MAT
	}

	void ShadowInfo::updateNormArea(UINT32 atlasSize)
	{
		normArea.x = area.x / (float)atlasSize;
		normArea.y = area.y / (float)atlasSize;
		normArea.width = area.width / (float)atlasSize;
		normArea.height = area.height / (float)atlasSize;
	}

	ShadowMapAtlas::ShadowMapAtlas(UINT32 size)
		: mLayout(0, 0, size, size, true), mLastUsedCounter(0)
	{
		mAtlas = GpuResourcePool::instance().get(
			POOLED_RENDER_TEXTURE_DESC::create2D(SHADOW_MAP_FORMAT, size, size, TU_DEPTHSTENCIL));
	}

	bool ShadowMapAtlas::addMap(UINT32 size, Rect2I& area, UINT32 border)
	{
		UINT32 sizeWithBorder = size + border * 2;

		UINT32 x, y;
		if (!mLayout.addElement(sizeWithBorder, sizeWithBorder, x, y))
			return false;

		area.width = area.height = size;
		area.x = x + border;
		area.y = y + border;

		mLastUsedCounter = 0;
		return true;
	}

	void ShadowMapAtlas::clear()
	{
		mLayout.clear();
		mLastUsedCounter++;
	}

	bool ShadowMapAtlas::isEmpty() const
	{
		return mLayout.isEmpty();
	}

	SPtr<Texture> ShadowMapAtlas::getTexture() const
	{
		return mAtlas->texture;
	}

	SPtr<RenderTexture> ShadowMapAtlas::getTarget() const
	{
		return mAtlas->renderTexture;
	}

	ShadowMapBase::ShadowMapBase(UINT32 size)
		: mSize(size), mIsUsed(false), mLastUsedCounter (0)
	{ }

	SPtr<Texture> ShadowMapBase::getTexture() const
	{
		return mShadowMap->texture;
	}

	ShadowCubemap::ShadowCubemap(UINT32 size)
		:ShadowMapBase(size)
	{
		mShadowMap = GpuResourcePool::instance().get(
			POOLED_RENDER_TEXTURE_DESC::createCube(SHADOW_MAP_FORMAT, size, size, TU_DEPTHSTENCIL));
	}

	SPtr<RenderTexture> ShadowCubemap::getTarget() const
	{
		return mShadowMap->renderTexture;
	}

	ShadowCascadedMap::ShadowCascadedMap(UINT32 size, UINT32 numCascades)
		:ShadowMapBase(size), mNumCascades(numCascades), mTargets(numCascades), mShadowInfos(numCascades)
	{
		mShadowMap = GpuResourcePool::instance().get(POOLED_RENDER_TEXTURE_DESC::create2D(SHADOW_MAP_FORMAT, size, size, 
			TU_DEPTHSTENCIL, 0, false, numCascades));

		RENDER_TEXTURE_DESC rtDesc;
		rtDesc.depthStencilSurface.texture = mShadowMap->texture;
		rtDesc.depthStencilSurface.numFaces = 1;

		for (UINT32 i = 0; i < mNumCascades; ++i)
		{
			rtDesc.depthStencilSurface.face = i;
			mTargets[i] = RenderTexture::create(rtDesc);
		}
	}

	SPtr<RenderTexture> ShadowCascadedMap::getTarget(UINT32 cascadeIdx) const
	{
		return mTargets[cascadeIdx];
	}

	/** 
	 * Provides a common way for all types of shadow depth rendering to render the relevant objects into the depth map. 
	 * Iterates over all relevant objects in the scene, binds the relevant materials and renders the objects into the depth
	 * map.
	 */
	class ShadowRenderQueue
	{
	public:
		struct Command
		{
			Command()
			{ }

			Command(RenderableElement* element)
				:element(element), isElement(true)
			{ }

			union
			{
				RenderableElement* element;
				RendererRenderable* renderable;
			};


			bool isElement : 1;
			UINT32 mask : 6;
		};

		template<class Options>
		static void execute(RendererScene& scene, const FrameInfo& frameInfo, const Options& opt)
		{
			static_assert((UINT32)RenderableAnimType::Count == 4, "RenderableAnimType is expected to have four sequential entries.");

			const SceneInfo& sceneInfo = scene.getSceneInfo();

			bs_frame_mark();
			{
				FrameVector<Command> commands[4];

				// Make a list of relevant renderables and prepare them for rendering
				for (UINT32 i = 0; i < sceneInfo.renderables.size(); i++)
				{
					const Sphere& bounds = sceneInfo.renderableCullInfos[i].bounds.getSphere();
					if (!opt.intersects(bounds))
						continue;

					scene.prepareRenderable(i, frameInfo);

					Command renderableCommand;
					renderableCommand.mask = 0;

					RendererRenderable* renderable = sceneInfo.renderables[i];
					renderableCommand.isElement = false;
					renderableCommand.renderable = renderable;

					opt.prepare(renderableCommand, bounds);

					bool renderableBound[4];
					bs_zero_out(renderableBound);

					for (auto& element : renderable->elements)
					{
						UINT32 arrayIdx = (int)element.animType;

						if (!renderableBound[arrayIdx])
						{
							commands[arrayIdx].push_back(renderableCommand);
							renderableBound[arrayIdx] = true;
						}

						commands[arrayIdx].push_back(Command(&element));
					}
				}

				static const ShaderVariation* VAR_LOOKUP[4];
				VAR_LOOKUP[0] = &getVertexInputVariation<false, false>();
				VAR_LOOKUP[1] = &getVertexInputVariation<true, false>();
				VAR_LOOKUP[2] = &getVertexInputVariation<false, true>();
				VAR_LOOKUP[3] = &getVertexInputVariation<true, true>();

				for (UINT32 i = 0; i < (UINT32)RenderableAnimType::Count; i++)
				{
					opt.bindMaterial(*VAR_LOOKUP[i]);

					for (auto& command : commands[i])
					{
						if (command.isElement)
						{
							const RenderableElement& element = *command.element;

							if (element.morphVertexDeclaration == nullptr)
								gRendererUtility().draw(element.mesh, element.subMesh);
							else
								gRendererUtility().drawMorph(element.mesh, element.subMesh, element.morphShapeBuffer,
									element.morphVertexDeclaration);
						}
						else
							opt.bindRenderable(command);
					}
				}
			}
			bs_frame_clear();
		}
	};

	/** Specialization used for ShadowRenderQueue when rendering cube (omnidirectional) shadow maps (all faces at once). */
	struct ShadowRenderQueueCubeOptions
	{
		ShadowRenderQueueCubeOptions(
			const ConvexVolume (&frustums)[6], 
			const ConvexVolume& boundingVolume, 
			const SPtr<GpuParamBlockBuffer>& shadowParamsBuffer, 
			const SPtr<GpuParamBlockBuffer>& shadowCubeMatricesBuffer,
			const SPtr<GpuParamBlockBuffer>& shadowCubeMasksBuffer)
			: frustums(frustums), boundingVolume(boundingVolume), shadowParamsBuffer(shadowParamsBuffer)
			, shadowCubeMatricesBuffer(shadowCubeMatricesBuffer), shadowCubeMasksBuffer(shadowCubeMasksBuffer)
		{ }

		bool intersects(const Sphere& bounds) const
		{
			return boundingVolume.intersects(bounds);
		}

		void prepare(ShadowRenderQueue::Command& command, const Sphere& bounds) const
		{
			for (UINT32 j = 0; j < 6; j++)
				command.mask |= (frustums[j].intersects(bounds) ? 1 : 0) << j;
		}

		void bindMaterial(const ShaderVariation& variation) const
		{
			material = ShadowDepthCubeMat::get(variation);
			material->bind(shadowParamsBuffer, shadowCubeMatricesBuffer);
		}

		void bindRenderable(ShadowRenderQueue::Command& command) const
		{
			RendererRenderable* renderable = command.renderable;

			for (UINT32 j = 0; j < 6; j++)
				gShadowCubeMasksDef.gFaceMasks.set(shadowCubeMasksBuffer, (command.mask & (1 << j)), j);

			material->setPerObjectBuffer(renderable->perObjectParamBuffer, shadowCubeMasksBuffer);
		}
		
		const ConvexVolume (&frustums)[6];
		const ConvexVolume& boundingVolume;
		const SPtr<GpuParamBlockBuffer>& shadowParamsBuffer;
		const SPtr<GpuParamBlockBuffer>& shadowCubeMatricesBuffer;
		const SPtr<GpuParamBlockBuffer>& shadowCubeMasksBuffer;

		mutable ShadowDepthCubeMat* material = nullptr;
	};

	/** Specialization used for ShadowRenderQueue when rendering cube (omnidirectional) shadow maps (one face at a time). */
	struct ShadowRenderQueueCubeSingleOptions
	{
		ShadowRenderQueueCubeSingleOptions(
				const ConvexVolume& boundingVolume,
				const SPtr<GpuParamBlockBuffer>& shadowParamsBuffer)
				: boundingVolume(boundingVolume), shadowParamsBuffer(shadowParamsBuffer)
		{ }

		bool intersects(const Sphere& bounds) const
		{
			return boundingVolume.intersects(bounds);
		}

		void prepare(ShadowRenderQueue::Command& command, const Sphere& bounds) const
		{
		}

		void bindMaterial(const ShaderVariation& variation) const
		{
			material = ShadowDepthNormalNoPSMat::get(variation);
			material->bind(shadowParamsBuffer);
		}

		void bindRenderable(ShadowRenderQueue::Command& command) const
		{
			RendererRenderable* renderable = command.renderable;

			material->setPerObjectBuffer(renderable->perObjectParamBuffer);
		}

		const ConvexVolume& boundingVolume;
		const SPtr<GpuParamBlockBuffer>& shadowParamsBuffer;

		mutable ShadowDepthNormalNoPSMat* material = nullptr;
	};

	/** Specialization used for ShadowRenderQueue when rendering spot light shadow maps. */
	struct ShadowRenderQueueSpotOptions
	{
		ShadowRenderQueueSpotOptions(
			const ConvexVolume& boundingVolume, 
			const SPtr<GpuParamBlockBuffer>& shadowParamsBuffer)
			: boundingVolume(boundingVolume), shadowParamsBuffer(shadowParamsBuffer)
		{ }

		bool intersects(const Sphere& bounds) const
		{
			return boundingVolume.intersects(bounds);
		}

		void prepare(ShadowRenderQueue::Command& command, const Sphere& bounds) const
		{
		}

		void bindMaterial(const ShaderVariation& variation) const
		{
			material = ShadowDepthNormalMat::get(variation);
			material->bind(shadowParamsBuffer);
		}

		void bindRenderable(ShadowRenderQueue::Command& command) const
		{
			RendererRenderable* renderable = command.renderable;

			material->setPerObjectBuffer(renderable->perObjectParamBuffer);
		}
		
		const ConvexVolume& boundingVolume;
		const SPtr<GpuParamBlockBuffer>& shadowParamsBuffer;

		mutable ShadowDepthNormalMat* material = nullptr;
	};

	/** Specialization used for ShadowRenderQueue when rendering directional light shadow maps. */
	struct ShadowRenderQueueDirOptions
	{
		ShadowRenderQueueDirOptions(
			const ConvexVolume& boundingVolume, 
			const SPtr<GpuParamBlockBuffer>& shadowParamsBuffer)
			: boundingVolume(boundingVolume), shadowParamsBuffer(shadowParamsBuffer)
		{ }

		bool intersects(const Sphere& bounds) const
		{
			return boundingVolume.intersects(bounds);
		}

		void prepare(ShadowRenderQueue::Command& command, const Sphere& bounds) const
		{
		}

		void bindMaterial(const ShaderVariation& variation) const
		{
			material = ShadowDepthDirectionalMat::get(variation);
			material->bind(shadowParamsBuffer);
		}

		void bindRenderable(ShadowRenderQueue::Command& command) const
		{
			RendererRenderable* renderable = command.renderable;

			material->setPerObjectBuffer(renderable->perObjectParamBuffer);
		}
		
		const ConvexVolume& boundingVolume;
		const SPtr<GpuParamBlockBuffer>& shadowParamsBuffer;

		mutable ShadowDepthDirectionalMat* material = nullptr;
	};

	const UINT32 ShadowRendering::MAX_ATLAS_SIZE = 4096;
	const UINT32 ShadowRendering::MAX_UNUSED_FRAMES = 60;
	const UINT32 ShadowRendering::MIN_SHADOW_MAP_SIZE = 32;
	const UINT32 ShadowRendering::SHADOW_MAP_FADE_SIZE = 64;
	const UINT32 ShadowRendering::SHADOW_MAP_BORDER = 4;
	const float ShadowRendering::CASCADE_FRACTION_FADE = 0.1f;

	ShadowRendering::ShadowRendering(UINT32 shadowMapSize)
		: mShadowMapSize(shadowMapSize)
	{
		SPtr<VertexDataDesc> vertexDesc = VertexDataDesc::create();
		vertexDesc->addVertElem(VET_FLOAT3, VES_POSITION);

		mPositionOnlyVD = VertexDeclaration::create(vertexDesc);

		// Create plane index and vertex buffers
		{
			VERTEX_BUFFER_DESC vbDesc;
			vbDesc.numVerts = 8;
			vbDesc.usage = GBU_DYNAMIC;
			vbDesc.vertexSize = mPositionOnlyVD->getProperties().getVertexSize(0);

			mPlaneVB = VertexBuffer::create(vbDesc);

			INDEX_BUFFER_DESC ibDesc;
			ibDesc.indexType = IT_32BIT;
			ibDesc.numIndices = 12;

			mPlaneIB = IndexBuffer::create(ibDesc);

			UINT32 indices[] =
			{
				// Far plane, back facing
				4, 7, 6,
				4, 6, 5,

				// Near plane, front facing
				0, 1, 2,
				0, 2, 3
			};

			mPlaneIB->writeData(0, sizeof(indices), indices);
		}

		// Create frustum index and vertex buffers
		{
			VERTEX_BUFFER_DESC vbDesc;
			vbDesc.numVerts = 8;
			vbDesc.usage = GBU_DYNAMIC;
			vbDesc.vertexSize = mPositionOnlyVD->getProperties().getVertexSize(0);

			mFrustumVB = VertexBuffer::create(vbDesc);

			INDEX_BUFFER_DESC ibDesc;
			ibDesc.indexType = IT_32BIT;
			ibDesc.numIndices = 36;

			mFrustumIB = IndexBuffer::create(ibDesc);
			mFrustumIB->writeData(0, sizeof(AABox::CUBE_INDICES), AABox::CUBE_INDICES);
		}
	}

	void ShadowRendering::setShadowMapSize(UINT32 size)
	{
		if (mShadowMapSize == size)
			return;

		mCascadedShadowMaps.clear();
		mDynamicShadowMaps.clear();
		mShadowCubemaps.clear();

		mShadowMapSize = size;
	}

	void ShadowRendering::renderShadowMaps(RendererScene& scene, const RendererViewGroup& viewGroup, 
		const FrameInfo& frameInfo)
	{
		// Note: Currently all shadows are dynamic and are rebuilt every frame. I should later added support for static
		// shadow maps which can be used for immovable lights. Such a light can then maintain a set of shadow maps,
		// one of which is static and only effects the static geometry, while the rest are per-object shadow maps used
		// for dynamic objects. Then only a small subset of geometry needs to be redrawn, instead of everything.

		// Note: Add support for per-object shadows and a way to force a renderable to use per-object shadows. This can be
		// used for adding high quality shadows on specific objects (e.g. important characters during cinematics).

		const SceneInfo& sceneInfo = scene.getSceneInfo();
		const VisibilityInfo& visibility = viewGroup.getVisibilityInfo();
		
		// Clear all transient data from last frame
		mShadowInfos.clear();

		mSpotLightShadows.resize(sceneInfo.spotLights.size());
		mRadialLightShadows.resize(sceneInfo.radialLights.size());
		mDirectionalLightShadows.resize(sceneInfo.directionalLights.size());

		mSpotLightShadowOptions.clear();
		mRadialLightShadowOptions.clear();

		// Clear all dynamic light atlases
		for (auto& entry : mCascadedShadowMaps)
			entry.clear();

		for (auto& entry : mDynamicShadowMaps)
			entry.clear();

		for (auto& entry : mShadowCubemaps)
			entry.clear();

		// Determine shadow map sizes and sort them
		UINT32 shadowInfoCount = 0;
		for (UINT32 i = 0; i < (UINT32)sceneInfo.spotLights.size(); ++i)
		{
			const RendererLight& light = sceneInfo.spotLights[i];
			mSpotLightShadows[i].startIdx = shadowInfoCount;
			mSpotLightShadows[i].numShadows = 0;

			// Note: I'm using visibility across all views, while I could be using visibility for every view individually,
			// if I kept that information somewhere
			if (!light.internal->getCastsShadow() || !visibility.spotLights[i])
				continue;

			ShadowMapOptions options;
			options.lightIdx = i;

			float maxFadePercent;
			calcShadowMapProperties(light, viewGroup, SHADOW_MAP_BORDER, options.mapSize, options.fadePercents, maxFadePercent);

			// Don't render shadow maps that will end up nearly completely faded out
			if (maxFadePercent < 0.005f)
				continue;

			mSpotLightShadowOptions.push_back(options);
			shadowInfoCount++; // For now, always a single fully dynamic shadow for a single light, but that may change
		}

		for (UINT32 i = 0; i < (UINT32)sceneInfo.radialLights.size(); ++i)
		{
			const RendererLight& light = sceneInfo.radialLights[i];
			mRadialLightShadows[i].startIdx = shadowInfoCount;
			mRadialLightShadows[i].numShadows = 0;

			// Note: I'm using visibility across all views, while I could be using visibility for every view individually,
			// if I kept that information somewhere
			if (!light.internal->getCastsShadow() || !visibility.radialLights[i])
				continue;

			ShadowMapOptions options;
			options.lightIdx = i;

			float maxFadePercent;
			calcShadowMapProperties(light, viewGroup, 0, options.mapSize, options.fadePercents, maxFadePercent);

			// Don't render shadow maps that will end up nearly completely faded out
			if (maxFadePercent < 0.005f)
				continue;

			mRadialLightShadowOptions.push_back(options);

			shadowInfoCount++; // For now, always a single fully dynamic shadow for a single light, but that may change
		}

		// Sort spot lights by size so they fit neatly in the texture atlas
		std::sort(mSpotLightShadowOptions.begin(), mSpotLightShadowOptions.end(),
			[](const ShadowMapOptions& a, const ShadowMapOptions& b) { return a.mapSize > b.mapSize; } );

		// Reserve space for shadow infos
		mShadowInfos.resize(shadowInfoCount);

		// Deallocate unused textures (must be done before rendering shadows, in order to ensure indices don't change)
		for(auto iter = mDynamicShadowMaps.begin(); iter != mDynamicShadowMaps.end(); ++iter)
		{
			if(iter->getLastUsedCounter() >= MAX_UNUSED_FRAMES)
			{
				// These are always populated in order, so we can assume all following atlases are also empty
				mDynamicShadowMaps.erase(iter, mDynamicShadowMaps.end());
				break;
			}
		}

		for(auto iter = mCascadedShadowMaps.begin(); iter != mCascadedShadowMaps.end();)
		{
			if (iter->getLastUsedCounter() >= MAX_UNUSED_FRAMES)
				iter = mCascadedShadowMaps.erase(iter);
			else
				++iter;
		}
		
		for(auto iter = mShadowCubemaps.begin(); iter != mShadowCubemaps.end();)
		{
			if (iter->getLastUsedCounter() >= MAX_UNUSED_FRAMES)
				iter = mShadowCubemaps.erase(iter);
			else
				++iter;
		}

		// Render shadow maps
		for (UINT32 i = 0; i < (UINT32)sceneInfo.directionalLights.size(); ++i)
		{
			const RendererLight& light = sceneInfo.directionalLights[i];

			if (!light.internal->getCastsShadow())
				return;

			UINT32 numViews = viewGroup.getNumViews();
			mDirectionalLightShadows[i].viewShadows.resize(numViews);

			for (UINT32 j = 0; j < numViews; ++j)
				renderCascadedShadowMaps(*viewGroup.getView(j), i, scene, frameInfo);
		}

		for(auto& entry : mSpotLightShadowOptions)
		{
			UINT32 lightIdx = entry.lightIdx;
			renderSpotShadowMap(sceneInfo.spotLights[lightIdx], entry, scene, frameInfo);
		}

		for (auto& entry : mRadialLightShadowOptions)
		{
			UINT32 lightIdx = entry.lightIdx;
			renderRadialShadowMap(sceneInfo.radialLights[lightIdx], entry, scene, frameInfo);
		}
	}

	/**
	 * Generates a frustum from the provided view-projection matrix.
	 * 
	 * @param[in]	invVP			Inverse of the view-projection matrix to use for generating the frustum.
	 * @param[out]	worldFrustum	Generated frustum planes, in world space.
	 * @return						Individual vertices of the frustum corners, in world space. Ordered using the
	 *								AABox::CornerEnum.
	 */
	std::array<Vector3, 8> getFrustum(const Matrix4& invVP, ConvexVolume& worldFrustum)
	{
		std::array<Vector3, 8> output;

		const RenderAPICapabilities& caps = gCaps();

		float flipY = 1.0f;
		if (caps.conventions.ndcYAxis == Conventions::Axis::Down)
			flipY = -1.0f;

		AABox frustumCube(
			Vector3(-1, -1 * flipY, caps.minDepth),
			Vector3(1, 1 * flipY, caps.maxDepth)
		);

		for(size_t i = 0; i < output.size(); i++)
		{
			Vector3 corner = frustumCube.getCorner((AABox::Corner)i);
			output[i] = invVP.multiply(corner);
		}

		Vector<Plane> planes(6);
		planes[FRUSTUM_PLANE_NEAR] = Plane(output[AABox::NEAR_LEFT_BOTTOM], output[AABox::NEAR_RIGHT_BOTTOM], output[AABox::NEAR_RIGHT_TOP]);
		planes[FRUSTUM_PLANE_FAR] = Plane(output[AABox::FAR_LEFT_BOTTOM], output[AABox::FAR_LEFT_TOP], output[AABox::FAR_RIGHT_TOP]);
		planes[FRUSTUM_PLANE_LEFT] = Plane(output[AABox::NEAR_LEFT_BOTTOM], output[AABox::NEAR_LEFT_TOP], output[AABox::FAR_LEFT_TOP]);
		planes[FRUSTUM_PLANE_RIGHT] = Plane(output[AABox::FAR_RIGHT_TOP], output[AABox::NEAR_RIGHT_TOP], output[AABox::NEAR_RIGHT_BOTTOM]);
		planes[FRUSTUM_PLANE_TOP] = Plane(output[AABox::NEAR_LEFT_TOP], output[AABox::NEAR_RIGHT_TOP], output[AABox::FAR_RIGHT_TOP]);
		planes[FRUSTUM_PLANE_BOTTOM] = Plane(output[AABox::NEAR_LEFT_BOTTOM], output[AABox::FAR_LEFT_BOTTOM], output[AABox::FAR_RIGHT_BOTTOM]);

		worldFrustum = ConvexVolume(planes);
		return output;
	}

	/**
	 * Converts a point in mixed space (clip_x, clip_y, view_z, view_w) to UV coordinates on a shadow map (x, y),
	 * and normalized linear depth from the shadow caster's perspective (z).
	 */
	Matrix4 createMixedToShadowUVMatrix(const Matrix4& viewP, const Matrix4& viewInvVP, const Rect2& shadowMapArea,
		float depthScale, float depthOffset, const Matrix4& shadowViewProj)
	{
		// Projects a point from (clip_x, clip_y, view_z, view_w) into clip space
		Matrix4 mixedToShadow = Matrix4::IDENTITY;
		mixedToShadow[2][2] = viewP[2][2];
		mixedToShadow[2][3] = viewP[2][3];
		mixedToShadow[3][2] = viewP[3][2];
		mixedToShadow[3][3] = 0.0f;

		// Projects a point in clip space back to homogeneus world space
		mixedToShadow = viewInvVP * mixedToShadow;

		// Projects a point in world space to shadow clip space
		mixedToShadow = shadowViewProj * mixedToShadow;
		
		// Convert shadow clip space coordinates to UV coordinates relative to the shadow map rectangle, and normalize
		// depth
		const Conventions& rapiConventions = gCaps().conventions;

		float flipY = -1.0f;
		// Either of these flips the Y axis, but if they're both true they cancel out
		if ((rapiConventions.uvYAxis == Conventions::Axis::Up) ^ (rapiConventions.ndcYAxis == Conventions::Axis::Down))
			flipY = -flipY;

		Matrix4 shadowMapTfrm
		(
			shadowMapArea.width * 0.5f, 0, 0, shadowMapArea.x + 0.5f * shadowMapArea.width,
			0, flipY * shadowMapArea.height * 0.5f, 0, shadowMapArea.y + 0.5f * shadowMapArea.height,
			0, 0, depthScale, depthOffset,
			0, 0, 0, 1
		);

		return shadowMapTfrm * mixedToShadow;
	}

	void ShadowRendering::renderShadowOcclusion(const RendererView& view, const RendererLight& rendererLight, 
		GBufferTextures gbuffer) const
	{
		UINT32 shadowQuality = view.getRenderSettings().shadowSettings.shadowFilteringQuality;

		const Light* light = rendererLight.internal;
		UINT32 lightIdx = light->getRendererId();

		auto viewProps = view.getProperties();

		const Matrix4& viewP = viewProps.projTransform;
		Matrix4 viewInvVP = viewProps.viewProjTransform.inverse();

		SPtr<GpuParamBlockBuffer> perViewBuffer = view.getPerViewBuffer();

		ProfileGPUBlock sampleBlock("Render shadow occlusion");

		const RenderAPICapabilities& caps = gCaps();
		// TODO - Calculate and set a scissor rectangle for the light

		SPtr<GpuParamBlockBuffer> shadowParamBuffer = gShadowProjectParamsDef.createBuffer();
		SPtr<GpuParamBlockBuffer> shadowOmniParamBuffer = gShadowProjectOmniParamsDef.createBuffer();

		UINT32 viewIdx = view.getViewIdx();
		Vector<const ShadowInfo*> shadowInfos;

		if(light->getType() == LightType::Radial)
		{
			const LightShadows& shadows = mRadialLightShadows[lightIdx];

			for(UINT32 i = 0; i < shadows.numShadows; ++i)
			{
				UINT32 shadowIdx = shadows.startIdx + i;
				const ShadowInfo& shadowInfo = mShadowInfos[shadowIdx];

				if (shadowInfo.fadePerView[viewIdx] < 0.005f)
					continue;

				for(UINT32 j = 0; j < 6; j++)
					gShadowProjectOmniParamsDef.gFaceVPMatrices.set(shadowOmniParamBuffer, shadowInfo.shadowVPTransforms[j], j);

				gShadowProjectOmniParamsDef.gDepthBias.set(shadowOmniParamBuffer, shadowInfo.depthBias);
				gShadowProjectOmniParamsDef.gFadePercent.set(shadowOmniParamBuffer, shadowInfo.fadePerView[viewIdx]);
				gShadowProjectOmniParamsDef.gInvResolution.set(shadowOmniParamBuffer, 1.0f / shadowInfo.area.width);

				const Transform& tfrm = light->getTransform();
				Vector4 lightPosAndRadius(tfrm.getPosition(), light->getAttenuationRadius());
				gShadowProjectOmniParamsDef.gLightPosAndRadius.set(shadowOmniParamBuffer, lightPosAndRadius);

				// Reduce shadow quality based on shadow map resolution for spot lights
				UINT32 effectiveShadowQuality = getShadowQuality(shadowQuality, shadowInfo.area.width, 2);

				// Check if viewer is inside the light bounds
				//// Expand the light bounds slightly to handle the case when the near plane is intersecting the light volume
				float lightRadius = light->getAttenuationRadius() + viewProps.nearPlane * 3.0f;
				bool viewerInsideVolume = (tfrm.getPosition() - viewProps.viewOrigin).length() < lightRadius;

				SPtr<Texture> shadowMap = mShadowCubemaps[shadowInfo.textureIdx].getTexture();
				ShadowProjectParams shadowParams(*light, shadowMap, shadowOmniParamBuffer, perViewBuffer, gbuffer);

				ShadowProjectOmniMat* mat = ShadowProjectOmniMat::getVariation(effectiveShadowQuality, viewerInsideVolume, 
					viewProps.target.numSamples > 1);
				mat->bind(shadowParams);

				gRendererUtility().draw(gRendererUtility().getSphereStencil());
			}
		}
		else // Directional & spot
		{
			shadowInfos.clear();

			bool isCSM = light->getType() == LightType::Directional;
			if(!isCSM)
			{
				const LightShadows& shadows = mSpotLightShadows[lightIdx];
				for (UINT32 i = 0; i < shadows.numShadows; ++i)
				{
					UINT32 shadowIdx = shadows.startIdx + i;
					const ShadowInfo& shadowInfo = mShadowInfos[shadowIdx];

					if (shadowInfo.fadePerView[viewIdx] < 0.005f)
						continue;

					shadowInfos.push_back(&shadowInfo);
				}
			}
			else // Directional
			{
				const LightShadows& shadows = mDirectionalLightShadows[lightIdx].viewShadows[viewIdx];
				if (shadows.numShadows > 0)
				{
					UINT32 mapIdx = shadows.startIdx;
					const ShadowCascadedMap& cascadedMap = mCascadedShadowMaps[mapIdx];

					// Render cascades in far to near order.
					// Note: If rendering other non-cascade maps they should be rendered after cascades.
					for (INT32 i = cascadedMap.getNumCascades() - 1; i >= 0; i--)
						shadowInfos.push_back(&cascadedMap.getShadowInfo(i));
				}
			}

			for(auto& shadowInfo : shadowInfos)
			{
				float depthScale, depthOffset;

				// Depth range scale is already baked into the ortho projection matrix, so avoid doing it here
				if (isCSM)
				{
					// Need to map from API-specific clip space depth to [0, 1] range
					depthScale = 1.0f / (caps.maxDepth - caps.minDepth);
					depthOffset = -caps.minDepth * depthScale;
				}
				else
				{
					depthScale = 1.0f / shadowInfo->depthRange;
					depthOffset = 0.0f;
				}

				SPtr<Texture> shadowMap;
				UINT32 shadowMapFace = 0;
				if(!isCSM)
					shadowMap = mDynamicShadowMaps[shadowInfo->textureIdx].getTexture();
				else
				{
					shadowMap = mCascadedShadowMaps[shadowInfo->textureIdx].getTexture();
					shadowMapFace = shadowInfo->cascadeIdx;
				}

				Matrix4 mixedToShadowUV = createMixedToShadowUVMatrix(viewP, viewInvVP, shadowInfo->normArea, 
					depthScale, depthOffset, shadowInfo->shadowVPTransform);

				auto shadowMapProps = shadowMap->getProperties();

				Vector2 shadowMapSize((float)shadowMapProps.getWidth(), (float)shadowMapProps.getHeight());
				float transitionScale = getFadeTransition(*light, shadowInfo->subjectBounds.getRadius(), 
					shadowInfo->depthRange, shadowInfo->area.width);

				gShadowProjectParamsDef.gFadePlaneDepth.set(shadowParamBuffer, shadowInfo->depthFade);
				gShadowProjectParamsDef.gMixedToShadowSpace.set(shadowParamBuffer, mixedToShadowUV);
				gShadowProjectParamsDef.gShadowMapSize.set(shadowParamBuffer, shadowMapSize);
				gShadowProjectParamsDef.gShadowMapSizeInv.set(shadowParamBuffer, 1.0f / shadowMapSize);
				gShadowProjectParamsDef.gSoftTransitionScale.set(shadowParamBuffer, transitionScale);

				if(isCSM)
					gShadowProjectParamsDef.gFadePercent.set(shadowParamBuffer, 1.0f);
				else
					gShadowProjectParamsDef.gFadePercent.set(shadowParamBuffer, shadowInfo->fadePerView[viewIdx]);

				if(shadowInfo->fadeRange == 0.0f)
					gShadowProjectParamsDef.gInvFadePlaneRange.set(shadowParamBuffer, 0.0f);
				else
					gShadowProjectParamsDef.gInvFadePlaneRange.set(shadowParamBuffer, 1.0f / shadowInfo->fadeRange);

				// Generate a stencil buffer to avoid evaluating pixels without any receiver geometry in the shadow area
				std::array<Vector3, 8> frustumVertices;
				UINT32 effectiveShadowQuality = shadowQuality;
				if(!isCSM)
				{
					ConvexVolume shadowFrustum;
					frustumVertices = getFrustum(shadowInfo->shadowVPTransform.inverse(), shadowFrustum);

					// Check if viewer is inside the frustum. Frustum is slightly expanded so that if the near plane is
					// intersecting the shadow frustum, it is counted as inside. This needs to be conservative as the code
					// for handling viewer outside the frustum will not properly render intersections with the near plane.
					bool viewerInsideFrustum = shadowFrustum.contains(viewProps.viewOrigin, viewProps.nearPlane * 3.0f);

					ShadowProjectStencilMat* mat = ShadowProjectStencilMat::getVariation(false, viewerInsideFrustum);
					mat->bind(perViewBuffer);
					drawFrustum(frustumVertices);

					// Reduce shadow quality based on shadow map resolution for spot lights
					effectiveShadowQuality = getShadowQuality(shadowQuality, shadowInfo->area.width, 2);
				}
				else
				{
					// Need to generate near and far planes to clip the geometry within the current CSM slice.
					// Note: If the render API supports built-in depth bound tests that could be used instead.

					Vector3 near = viewProps.projTransform.multiply(Vector3(0, 0, -shadowInfo->depthNear));
					Vector3 far = viewProps.projTransform.multiply(Vector3(0, 0, -shadowInfo->depthFar));

					ShadowProjectStencilMat* mat = ShadowProjectStencilMat::getVariation(true, true);
					mat->bind(perViewBuffer);

					drawNearFarPlanes(near.z, far.z, shadowInfo->cascadeIdx != 0);
				}

				gShadowProjectParamsDef.gFace.set(shadowParamBuffer, (float)shadowMapFace);
				ShadowProjectParams shadowParams(*light, shadowMap, shadowParamBuffer, perViewBuffer, gbuffer);

				ShadowProjectMat* mat = ShadowProjectMat::getVariation(effectiveShadowQuality, isCSM, 
					viewProps.target.numSamples > 1);
				mat->bind(shadowParams);

				if (!isCSM)
					drawFrustum(frustumVertices);
				else
					gRendererUtility().drawScreenQuad();
			}
		}
	}

	void ShadowRendering::renderCascadedShadowMaps(const RendererView& view, UINT32 lightIdx, RendererScene& scene, 
		const FrameInfo& frameInfo)
	{
		UINT32 viewIdx = view.getViewIdx();
		LightShadows& lightShadows = mDirectionalLightShadows[lightIdx].viewShadows[viewIdx];

		if (!view.getRenderSettings().enableShadows)
		{
			lightShadows.startIdx = -1;
			lightShadows.numShadows = 0;
			return;
		}

		// Note: Currently I'm using spherical bounds for the cascaded frustum which might result in non-optimal usage
		// of the shadow map. A different approach would be to generate a bounding box and then both adjust the aspect
		// ratio (and therefore dimensions) of the shadow map, as well as rotate the camera so the visible area best fits
		// in the map. It remains to be seen if this is viable.
		//  - Note2: Actually both of these will likely have serious negative impact on shadow stability.
		const SceneInfo& sceneInfo = scene.getSceneInfo();

		const RendererLight& rendererLight = sceneInfo.directionalLights[lightIdx];
		Light* light = rendererLight.internal;

		RenderAPI& rapi = RenderAPI::instance();

		const Transform& tfrm = light->getTransform();
		Vector3 lightDir = -tfrm.getRotation().zAxis();
		SPtr<GpuParamBlockBuffer> shadowParamsBuffer = gShadowParamsDef.createBuffer();

		ShadowInfo shadowInfo;
		shadowInfo.lightIdx = lightIdx;
		shadowInfo.textureIdx = -1;

		UINT32 mapSize = std::min(mShadowMapSize, MAX_ATLAS_SIZE);
		shadowInfo.area = Rect2I(0, 0, mapSize, mapSize);
		shadowInfo.updateNormArea(mapSize);

		UINT32 numCascades = view.getRenderSettings().shadowSettings.numCascades;
		for (UINT32 i = 0; i < (UINT32)mCascadedShadowMaps.size(); i++)
		{
			ShadowCascadedMap& shadowMap = mCascadedShadowMaps[i];

			if (!shadowMap.isUsed() && shadowMap.getSize() == mapSize && shadowMap.getNumCascades() == numCascades)
			{
				shadowInfo.textureIdx = i;
				shadowMap.markAsUsed();

				break;
			}
		}

		if (shadowInfo.textureIdx == (UINT32)-1)
		{
			shadowInfo.textureIdx = (UINT32)mCascadedShadowMaps.size();
			mCascadedShadowMaps.push_back(ShadowCascadedMap(mapSize, numCascades));

			ShadowCascadedMap& shadowMap = mCascadedShadowMaps.back();
			shadowMap.markAsUsed();
		}

		ShadowCascadedMap& shadowMap = mCascadedShadowMaps[shadowInfo.textureIdx];

		Quaternion lightRotation(BsIdentity);
		lightRotation.lookRotation(lightDir, Vector3::UNIT_Y);

		ProfileGPUBlock profileSample("Project directional light shadow");

		for (UINT32 i = 0; i < numCascades; ++i)
		{
			Sphere frustumBounds;
			ConvexVolume cascadeCullVolume = getCSMSplitFrustum(view, lightDir, i, numCascades, frustumBounds);

			// Make sure the size of the projected area is in multiples of shadow map pixel size (for stability)
			float worldUnitsPerTexel = frustumBounds.getRadius() * 2.0f / shadowMap.getSize();

			float orthoSize = floor(frustumBounds.getRadius() * 2.0f / worldUnitsPerTexel) * worldUnitsPerTexel * 0.5f;
			worldUnitsPerTexel = orthoSize * 2.0f / shadowMap.getSize();
			
			// Snap caster origin to the shadow map pixel grid, to ensure shadow map stability
			Vector3 casterOrigin = frustumBounds.getCenter();
			Matrix4 shadowView = Matrix4::view(Vector3::ZERO, lightRotation);
			Vector3 shadowSpaceOrigin = shadowView.multiplyAffine(casterOrigin);

			Vector2 snapOffset(fmod(shadowSpaceOrigin.x, worldUnitsPerTexel), fmod(shadowSpaceOrigin.y, worldUnitsPerTexel));
			shadowSpaceOrigin.x -= snapOffset.x;
			shadowSpaceOrigin.y -= snapOffset.y;

			Matrix4 shadowViewInv = shadowView.inverseAffine();
			casterOrigin = shadowViewInv.multiplyAffine(shadowSpaceOrigin);

			// Move the light so it is centered at the subject frustum, with depth range covering the frustum bounds
			shadowInfo.depthRange = frustumBounds.getRadius() * 2.0f;

			Vector3 offsetLightPos = casterOrigin - lightDir * frustumBounds.getRadius();
			Matrix4 offsetViewMat = Matrix4::view(offsetLightPos, lightRotation);

			Matrix4 proj = Matrix4::projectionOrthographic(-orthoSize, orthoSize, orthoSize, -orthoSize, 0.0f, 
				shadowInfo.depthRange);

			RenderAPI::instance().convertProjectionMatrix(proj, proj);

			shadowInfo.cascadeIdx = i;
			shadowInfo.shadowVPTransform = proj * offsetViewMat;

			// Determine split range
			float splitNear = getCSMSplitDistance(view, i, numCascades);
			float splitFar = getCSMSplitDistance(view, i + 1, numCascades);

			shadowInfo.depthNear = splitNear;
			shadowInfo.depthFade = splitFar;
			shadowInfo.subjectBounds = frustumBounds;
			
			if ((UINT32)(i + 1) < numCascades)
				shadowInfo.fadeRange = CASCADE_FRACTION_FADE * (shadowInfo.depthFade - shadowInfo.depthNear);
			else
				shadowInfo.fadeRange = 0.0f;

			shadowInfo.depthFar = shadowInfo.depthFade + shadowInfo.fadeRange;
			shadowInfo.depthBias = getDepthBias(*light, frustumBounds.getRadius(), shadowInfo.depthRange, mapSize);

			gShadowParamsDef.gDepthBias.set(shadowParamsBuffer, shadowInfo.depthBias);
			gShadowParamsDef.gInvDepthRange.set(shadowParamsBuffer, 1.0f / shadowInfo.depthRange);
			gShadowParamsDef.gMatViewProj.set(shadowParamsBuffer, shadowInfo.shadowVPTransform);
			gShadowParamsDef.gNDCZToDeviceZ.set(shadowParamsBuffer, RendererView::getNDCZToDeviceZ());

			rapi.setRenderTarget(shadowMap.getTarget(i));
			rapi.clearRenderTarget(FBT_DEPTH);

			ShadowDepthDirectionalMat* depthDirMat = ShadowDepthDirectionalMat::get();
			depthDirMat->bind(shadowParamsBuffer);

			// Render all renderables into the shadow map
			ShadowRenderQueueDirOptions dirOptions(
				cascadeCullVolume,
				shadowParamsBuffer);
			
			ShadowRenderQueue::execute(scene, frameInfo, dirOptions);

			shadowMap.setShadowInfo(i, shadowInfo);
		}

		lightShadows.startIdx = shadowInfo.textureIdx;
		lightShadows.numShadows = 1;
	}

	void ShadowRendering::renderSpotShadowMap(const RendererLight& rendererLight, const ShadowMapOptions& options,
		RendererScene& scene, const FrameInfo& frameInfo)
	{
		Light* light = rendererLight.internal;

		SPtr<GpuParamBlockBuffer> shadowParamsBuffer = gShadowParamsDef.createBuffer();

		ShadowInfo mapInfo;
		mapInfo.fadePerView = options.fadePercents;
		mapInfo.lightIdx = options.lightIdx;
		mapInfo.cascadeIdx = -1;

		bool foundSpace = false;
		for (UINT32 i = 0; i < (UINT32)mDynamicShadowMaps.size(); i++)
		{
			ShadowMapAtlas& atlas = mDynamicShadowMaps[i];

			if (atlas.addMap(options.mapSize, mapInfo.area, SHADOW_MAP_BORDER))
			{
				mapInfo.textureIdx = i;

				foundSpace = true;
				break;
			}
		}

		if (!foundSpace)
		{
			mapInfo.textureIdx = (UINT32)mDynamicShadowMaps.size();
			mDynamicShadowMaps.push_back(ShadowMapAtlas(MAX_ATLAS_SIZE));

			ShadowMapAtlas& atlas = mDynamicShadowMaps.back();
			atlas.addMap(options.mapSize, mapInfo.area, SHADOW_MAP_BORDER);
		}

		mapInfo.updateNormArea(MAX_ATLAS_SIZE);
		ShadowMapAtlas& atlas = mDynamicShadowMaps[mapInfo.textureIdx];

		ProfileGPUBlock profileSample("Project spot light shadows");

		RenderAPI& rapi = RenderAPI::instance();
		rapi.setRenderTarget(atlas.getTarget());
		rapi.setViewport(mapInfo.normArea);
		rapi.clearViewport(FBT_DEPTH);

		mapInfo.depthNear = 0.05f;
		mapInfo.depthFar = light->getAttenuationRadius();
		mapInfo.depthFade = mapInfo.depthFar;
		mapInfo.fadeRange = 0.0f;
		mapInfo.depthRange = mapInfo.depthFar - mapInfo.depthNear;
		mapInfo.depthBias = getDepthBias(*light, light->getBounds().getRadius(), mapInfo.depthRange, options.mapSize);
		mapInfo.subjectBounds = light->getBounds();

		Quaternion lightRotation = light->getTransform().getRotation();

		Matrix4 view = Matrix4::view(rendererLight.getShiftedLightPosition(), lightRotation);
		Matrix4 proj = Matrix4::projectionPerspective(light->getSpotAngle(), 1.0f, 0.05f, light->getAttenuationRadius());

		ConvexVolume localFrustum = ConvexVolume(proj);
		RenderAPI::instance().convertProjectionMatrix(proj, proj);

		mapInfo.shadowVPTransform = proj * view;

		gShadowParamsDef.gDepthBias.set(shadowParamsBuffer, mapInfo.depthBias);
		gShadowParamsDef.gInvDepthRange.set(shadowParamsBuffer, 1.0f / mapInfo.depthRange);
		gShadowParamsDef.gMatViewProj.set(shadowParamsBuffer, mapInfo.shadowVPTransform);
		gShadowParamsDef.gNDCZToDeviceZ.set(shadowParamsBuffer, RendererView::getNDCZToDeviceZ());

		const Vector<Plane>& frustumPlanes = localFrustum.getPlanes();
		Matrix4 worldMatrix = view.inverseAffine();

		Vector<Plane> worldPlanes(frustumPlanes.size());
		UINT32 j = 0;
		for (auto& plane : frustumPlanes)
		{
			worldPlanes[j] = worldMatrix.multiplyAffine(plane);
			j++;
		}

		ConvexVolume worldFrustum(worldPlanes);

		// Render all renderables into the shadow map
		ShadowRenderQueueSpotOptions spotOptions(
			worldFrustum,
			shadowParamsBuffer);

		ShadowRenderQueue::execute(scene, frameInfo, spotOptions);

		// Restore viewport
		rapi.setViewport(Rect2(0.0f, 0.0f, 1.0f, 1.0f));

		LightShadows& lightShadows = mSpotLightShadows[options.lightIdx];

		mShadowInfos[lightShadows.startIdx + lightShadows.numShadows] = mapInfo;
		lightShadows.numShadows++;
	}

	void ShadowRendering::renderRadialShadowMap(const RendererLight& rendererLight, 
		const ShadowMapOptions& options, RendererScene& scene, const FrameInfo& frameInfo)
	{
		Light* light = rendererLight.internal;

		SPtr<GpuParamBlockBuffer> shadowParamsBuffer = gShadowParamsDef.createBuffer();

		ShadowInfo mapInfo;
		mapInfo.lightIdx = options.lightIdx;
		mapInfo.textureIdx = -1;
		mapInfo.fadePerView = options.fadePercents;
		mapInfo.cascadeIdx = -1;
		mapInfo.area = Rect2I(0, 0, options.mapSize, options.mapSize);
		mapInfo.updateNormArea(options.mapSize);

		for (UINT32 i = 0; i < (UINT32)mShadowCubemaps.size(); i++)
		{
			ShadowCubemap& cubemap = mShadowCubemaps[i];

			if (!cubemap.isUsed() && cubemap.getSize() == options.mapSize)
			{
				mapInfo.textureIdx = i;
				cubemap.markAsUsed();

				break;
			}
		}

		if (mapInfo.textureIdx == (UINT32)-1)
		{
			mapInfo.textureIdx = (UINT32)mShadowCubemaps.size();
			mShadowCubemaps.push_back(ShadowCubemap(options.mapSize));

			ShadowCubemap& cubemap = mShadowCubemaps.back();
			cubemap.markAsUsed();
		}

		ShadowCubemap& cubemap = mShadowCubemaps[mapInfo.textureIdx];

		mapInfo.depthNear = 0.05f;
		mapInfo.depthFar = light->getAttenuationRadius();
		mapInfo.depthFade = mapInfo.depthFar;
		mapInfo.fadeRange = 0.0f;
		mapInfo.depthRange = mapInfo.depthFar - mapInfo.depthNear;
		mapInfo.depthBias = getDepthBias(*light, light->getBounds().getRadius(), mapInfo.depthRange, options.mapSize);
		mapInfo.subjectBounds = light->getBounds();

		// Note: Projecting on positive Z axis, because cubemaps use a left-handed coordinate system
		Matrix4 proj = Matrix4::projectionPerspective(Degree(90.0f), 1.0f, 0.05f, light->getAttenuationRadius(), true);
		ConvexVolume localFrustum(proj);

		ProfileGPUBlock profileSample("Project radial light shadows");

		const RenderAPICapabilities& caps = gCaps();
		const Conventions& rapiConventions = gCaps().conventions;

		RenderAPI& rapi = RenderAPI::instance();
		rapi.convertProjectionMatrix(proj, proj);

		// Render cubemaps upside down if necessary
		Matrix4 adjustedProj = proj;
		if(caps.conventions.uvYAxis == Conventions::Axis::Up)
		{
			// All big APIs use the same cubemap sampling coordinates, as well as the same face order. But APIs that
			// use bottom-up UV coordinates require the cubemap faces to be stored upside down in order to get the same
			// behaviour. APIs that use an upside-down NDC Y axis have the same problem as the rendered image will be
			// upside down, but this is handled by the projection matrix. If both of those are enabled, then the effect
			// cancels out.

			adjustedProj[1][1] = -proj[1][1];
		}

		bool renderAllFacesAtOnce = caps.hasCapability(RSC_RENDER_TARGET_LAYERS);

		SPtr<GpuParamBlockBuffer> shadowCubeMatricesBuffer;
		SPtr<GpuParamBlockBuffer> shadowCubeMasksBuffer;
		if(renderAllFacesAtOnce)
		{
			shadowCubeMatricesBuffer = gShadowCubeMatricesDef.createBuffer();
			shadowCubeMasksBuffer = gShadowCubeMasksDef.createBuffer();
		}

		gShadowParamsDef.gDepthBias.set(shadowParamsBuffer, mapInfo.depthBias);
		gShadowParamsDef.gInvDepthRange.set(shadowParamsBuffer, 1.0f / mapInfo.depthRange);
		gShadowParamsDef.gMatViewProj.set(shadowParamsBuffer, Matrix4::IDENTITY);
		gShadowParamsDef.gNDCZToDeviceZ.set(shadowParamsBuffer, RendererView::getNDCZToDeviceZ());

		ConvexVolume frustums[6];
		Vector<Plane> boundingPlanes;
		for (UINT32 i = 0; i < 6; i++)
		{
			// Calculate view matrix
			Vector3 forward;
			Vector3 up = Vector3::UNIT_Y;

			switch (i)
			{
			case CF_PositiveX:
				forward = Vector3::UNIT_X;
				break;
			case CF_NegativeX:
				forward = -Vector3::UNIT_X;
				break;
			case CF_PositiveY:
				forward = Vector3::UNIT_Y;
				up = -Vector3::UNIT_Z;
				break;
			case CF_NegativeY:
				forward = -Vector3::UNIT_Y;
				up = Vector3::UNIT_Z;
				break;
			case CF_PositiveZ:
				forward = Vector3::UNIT_Z;
				break;
			case CF_NegativeZ:
				forward = -Vector3::UNIT_Z;
				break;
			}

			Vector3 right = Vector3::cross(up, forward);
			Matrix3 viewRotationMat = Matrix3(right, up, forward);

			Vector3 lightPos = light->getTransform().getPosition();
			Matrix4 viewOffsetMat = Matrix4::translation(-lightPos);

			Matrix4 view = Matrix4(viewRotationMat.transpose()) * viewOffsetMat;
			mapInfo.shadowVPTransforms[i] = proj * view;

			Matrix4 shadowViewProj = adjustedProj * view;

			// Calculate world frustum for culling
			const Vector<Plane>& frustumPlanes = localFrustum.getPlanes();

			Matrix4 worldMatrix = Matrix4::translation(lightPos) * Matrix4(viewRotationMat);

			Vector<Plane> worldPlanes(frustumPlanes.size());
			UINT32 j = 0;
			for (auto& plane : frustumPlanes)
			{
				worldPlanes[j] = worldMatrix.multiplyAffine(plane);
				j++;
			}

			ConvexVolume frustum(worldPlanes);

			if(renderAllFacesAtOnce)
			{
				frustums[i] = frustum;

				// Register far plane of all frustums
				boundingPlanes.push_back(worldPlanes[FRUSTUM_PLANE_FAR]);
				gShadowCubeMatricesDef.gFaceVPMatrices.set(shadowCubeMatricesBuffer, shadowViewProj, i);
			}
			else
			{
				gShadowParamsDef.gMatViewProj.set(shadowParamsBuffer, shadowViewProj);

				RENDER_TEXTURE_DESC rtDesc;
				rtDesc.depthStencilSurface.texture = cubemap.getTexture();
				rtDesc.depthStencilSurface.face = i;
				rtDesc.depthStencilSurface.numFaces = 1;

				SPtr<RenderTarget> faceRt = RenderTexture::create(rtDesc);

				rapi.setRenderTarget(faceRt);
				rapi.clearRenderTarget(FBT_DEPTH);

				// Render all renderables into the shadow map
				ConvexVolume boundingVolume(boundingPlanes);
				ShadowRenderQueueCubeSingleOptions cubeOptions(
						frustum,
						shadowParamsBuffer
				);

				ShadowRenderQueue::execute(scene, frameInfo, cubeOptions);
			}
		}

		if(renderAllFacesAtOnce)
		{
			rapi.setRenderTarget(cubemap.getTarget());
			rapi.clearRenderTarget(FBT_DEPTH);

			// Render all renderables into the shadow map
			ConvexVolume boundingVolume(boundingPlanes);
			ShadowRenderQueueCubeOptions cubeOptions(
					frustums,
					boundingVolume,
					shadowParamsBuffer,
					shadowCubeMatricesBuffer,
					shadowCubeMasksBuffer
			);

			ShadowRenderQueue::execute(scene, frameInfo, cubeOptions);
		}

		LightShadows& lightShadows = mRadialLightShadows[options.lightIdx];

		mShadowInfos[lightShadows.startIdx + lightShadows.numShadows] = mapInfo;
		lightShadows.numShadows++;
	}

	void ShadowRendering::calcShadowMapProperties(const RendererLight& light, const RendererViewGroup& viewGroup, 
		UINT32 border, UINT32& size, SmallVector<float, 6>& fadePercents, float& maxFadePercent) const
	{
		const static float SHADOW_TEXELS_PER_PIXEL = 1.0f;

		// Find a view in which the light has the largest radius
		float maxMapSize = 0.0f;
		maxFadePercent = 0.0f;
		for (int i = 0; i < (int)viewGroup.getNumViews(); ++i)
		{
			const RendererView& view = *viewGroup.getView(i);
			const RendererViewProperties& viewProps = view.getProperties();
			const RenderSettings& viewSettings = view.getRenderSettings();

			if(!viewSettings.enableShadows)
				fadePercents.add(0.0f);
			else
			{
				// Approximation for screen space sphere radius: screenSize * 0.5 * cot(fov) * radius / Z, where FOV is the 
				// largest one
				//// First get sphere depth
				const Matrix4& viewVP = viewProps.viewProjTransform;
				float depth = viewVP.multiply(Vector4(light.internal->getTransform().getPosition(), 1.0f)).w;

				// This is just 1/tan(fov), for both horz. and vert. FOV
				float viewScaleX = viewProps.projTransform[0][0];
				float viewScaleY = viewProps.projTransform[1][1];

				float screenScaleX = viewScaleX * viewProps.target.viewRect.width * 0.5f;
				float screenScaleY = viewScaleY * viewProps.target.viewRect.height * 0.5f;

				float screenScale = std::max(screenScaleX, screenScaleY);

				//// Calc radius (clamp if too close to avoid massive numbers)
				float radiusNDC = light.internal->getBounds().getRadius() / std::max(depth, 1.0f);

				//// Radius of light bounds in percent of the view surface, multiplied by screen size in pixels
				float radiusScreen = radiusNDC * screenScale;

				float optimalMapSize = SHADOW_TEXELS_PER_PIXEL * radiusScreen;
				maxMapSize = std::max(maxMapSize, optimalMapSize);

				// Determine if the shadow should fade out
				float fadePercent = Math::invLerp(optimalMapSize, (float)MIN_SHADOW_MAP_SIZE, (float)SHADOW_MAP_FADE_SIZE);
				fadePercents.add(fadePercent);
				maxFadePercent = std::max(maxFadePercent, fadePercent);
			}
		}

		// If light fully (or nearly fully) covers the screen, use full shadow map resolution, otherwise
		// scale it down to smaller power of two, while clamping to minimal allowed resolution
		UINT32 effectiveMapSize = Bitwise::nextPow2((UINT32)maxMapSize);
		effectiveMapSize = Math::clamp(effectiveMapSize, MIN_SHADOW_MAP_SIZE, mShadowMapSize);

		// Leave room for border
		size = std::max(effectiveMapSize - 2 * border, 1u);
	}

	void ShadowRendering::drawNearFarPlanes(float near, float far, bool drawNear) const
	{
		const Conventions& rapiConventions = gCaps().conventions;
		float flipY = (rapiConventions.ndcYAxis == Conventions::Axis::Down) ? -1.0f : 1.0f;

		// Update VB with new vertices
		Vector3 vertices[8] =
		{
			// Near plane
			{ -1.0f, -1.0f * flipY, near },
			{  1.0f, -1.0f * flipY, near },
			{  1.0f,  1.0f * flipY, near },
			{ -1.0f,  1.0f * flipY, near },

			// Far plane
			{ -1.0f, -1.0f * flipY, far },
			{  1.0f, -1.0f * flipY, far },
			{  1.0f,  1.0f * flipY, far },
			{ -1.0f,  1.0f * flipY, far },
		};

		mPlaneVB->writeData(0, sizeof(vertices), vertices, BWT_DISCARD);

		// Draw the mesh
		RenderAPI& rapi = RenderAPI::instance();
		rapi.setVertexDeclaration(mPositionOnlyVD);
		rapi.setVertexBuffers(0, &mPlaneVB, 1);
		rapi.setIndexBuffer(mPlaneIB);
		rapi.setDrawOperation(DOT_TRIANGLE_LIST);

		rapi.drawIndexed(0, drawNear ? 12 : 6, 0, drawNear ? 8 : 4);
	}

	void ShadowRendering::drawFrustum(const std::array<Vector3, 8>& corners) const
	{
		RenderAPI& rapi = RenderAPI::instance();

		// Update VB with new vertices
		mFrustumVB->writeData(0, sizeof(Vector3) * 8, corners.data(), BWT_DISCARD);

		// Draw the mesh
		rapi.setVertexDeclaration(mPositionOnlyVD);
		rapi.setVertexBuffers(0, &mFrustumVB, 1);
		rapi.setIndexBuffer(mFrustumIB);
		rapi.setDrawOperation(DOT_TRIANGLE_LIST);

		rapi.drawIndexed(0, 36, 0, 8);
	}

	UINT32 ShadowRendering::getShadowQuality(UINT32 requestedQuality, UINT32 shadowMapResolution, UINT32 minAllowedQuality)
	{
		static const UINT32 TARGET_RESOLUTION = 512;

		// If shadow map resolution is smaller than some target resolution drop the number of PCF samples (shadow quality)
		// so that the penumbra better matches with larger sized shadow maps.
		while(requestedQuality > minAllowedQuality && shadowMapResolution < TARGET_RESOLUTION)
		{
			shadowMapResolution *= 2;
			requestedQuality = std::max(requestedQuality - 1, 1U);
		}

		return requestedQuality;
	}

	ConvexVolume ShadowRendering::getCSMSplitFrustum(const RendererView& view, const Vector3& lightDir, UINT32 cascade, 
		UINT32 numCascades, Sphere& outBounds)
	{
		// Determine split range
		float splitNear = getCSMSplitDistance(view, cascade, numCascades);
		float splitFar = getCSMSplitDistance(view, cascade + 1, numCascades);

		// Increase by fade range, unless last cascade
		if ((UINT32)(cascade + 1) < numCascades)
				splitFar += CASCADE_FRACTION_FADE * (splitFar - splitNear);
		
		// Calculate the eight vertices of the split frustum
		auto& viewProps = view.getProperties();

		const Matrix4& projMat = viewProps.projTransform;

		float aspect;
		float nearHalfWidth, nearHalfHeight;
		float farHalfWidth, farHalfHeight;
		if(viewProps.projType == PT_PERSPECTIVE)
		{
			aspect = fabs(projMat[0][0] / projMat[1][1]);
			float tanHalfFOV = 1.0f / projMat[0][0];

			nearHalfWidth = splitNear * tanHalfFOV;
			nearHalfHeight = nearHalfWidth * aspect;

			farHalfWidth = splitFar * tanHalfFOV;
			farHalfHeight = farHalfWidth * aspect;
		}
		else
		{
			aspect = projMat[0][0] / projMat[1][1];

			nearHalfWidth = farHalfWidth = projMat[0][0] / 4.0f;
			nearHalfHeight = farHalfHeight = projMat[1][1] / 4.0f;
		}

		const Matrix4& viewMat = viewProps.viewTransform;
		Vector3 cameraRight = Vector3(viewMat[0]);
		Vector3 cameraUp = Vector3(viewMat[1]);

		const Vector3& viewOrigin = viewProps.viewOrigin;
		const Vector3& viewDir = viewProps.viewDirection;

		Vector3 frustumVerts[] =
		{
			viewOrigin + viewDir * splitNear - cameraRight * nearHalfWidth + cameraUp * nearHalfHeight, // Near, left, top
			viewOrigin + viewDir * splitNear + cameraRight * nearHalfWidth + cameraUp * nearHalfHeight, // Near, right, top
			viewOrigin + viewDir * splitNear + cameraRight * nearHalfWidth - cameraUp * nearHalfHeight, // Near, right, bottom
			viewOrigin + viewDir * splitNear - cameraRight * nearHalfWidth - cameraUp * nearHalfHeight, // Near, left, bottom
			viewOrigin + viewDir * splitFar - cameraRight * farHalfWidth + cameraUp * farHalfHeight, // Far, left, top
			viewOrigin + viewDir * splitFar + cameraRight * farHalfWidth + cameraUp * farHalfHeight, // Far, right, top
			viewOrigin + viewDir * splitFar + cameraRight * farHalfWidth - cameraUp * farHalfHeight, // Far, right, bottom
			viewOrigin + viewDir * splitFar - cameraRight * farHalfWidth - cameraUp * farHalfHeight, // Far, left, bottom
		};

		// Calculate the bounding sphere of the frustum
		float diagonalNearSq = nearHalfWidth * nearHalfWidth + nearHalfHeight * nearHalfHeight;
		float diagonalFarSq = farHalfWidth * farHalfWidth + farHalfHeight * farHalfHeight;

		float length = splitFar - splitNear;
		float offset = (diagonalNearSq - diagonalFarSq) / (2 * length) + length * 0.5f;
		float distToCenter = Math::clamp(splitFar - offset, splitNear, splitFar);

		Vector3 center = viewOrigin + viewDir * distToCenter;

		float radius = 0.0f;
		for (auto& entry : frustumVerts)
			radius = std::max(radius, center.squaredDistance(entry));

		radius = std::max((float)sqrt(radius), 1.0f);
		outBounds = Sphere(center, radius);

		// Generate light frustum planes
		Plane viewPlanes[6];
		viewPlanes[FRUSTUM_PLANE_NEAR] = Plane(frustumVerts[0], frustumVerts[1], frustumVerts[2]);
		viewPlanes[FRUSTUM_PLANE_FAR] = Plane(frustumVerts[5], frustumVerts[4], frustumVerts[7]);
		viewPlanes[FRUSTUM_PLANE_LEFT] = Plane(frustumVerts[4], frustumVerts[0], frustumVerts[3]);
		viewPlanes[FRUSTUM_PLANE_RIGHT] = Plane(frustumVerts[1], frustumVerts[5], frustumVerts[6]);
		viewPlanes[FRUSTUM_PLANE_TOP] = Plane(frustumVerts[4], frustumVerts[5], frustumVerts[1]);
		viewPlanes[FRUSTUM_PLANE_BOTTOM] = Plane(frustumVerts[3], frustumVerts[2], frustumVerts[6]);

		//// Add camera's planes facing towards the lights (forming the back of the volume)
		Vector<Plane> lightVolume;
		for(auto& entry : viewPlanes)
		{
			if (entry.normal.dot(lightDir) < 0.0f)
				lightVolume.push_back(entry);
		}

		//// Determine edge planes by testing adjacent planes with different facing
		////// Pairs of frustum planes that share an edge
		UINT32 adjacentPlanes[][2] =
		{
			{ FRUSTUM_PLANE_NEAR, FRUSTUM_PLANE_LEFT },
			{ FRUSTUM_PLANE_NEAR, FRUSTUM_PLANE_RIGHT },
			{ FRUSTUM_PLANE_NEAR, FRUSTUM_PLANE_TOP },
			{ FRUSTUM_PLANE_NEAR, FRUSTUM_PLANE_BOTTOM },

			{ FRUSTUM_PLANE_FAR, FRUSTUM_PLANE_LEFT },
			{ FRUSTUM_PLANE_FAR, FRUSTUM_PLANE_RIGHT },
			{ FRUSTUM_PLANE_FAR, FRUSTUM_PLANE_TOP },
			{ FRUSTUM_PLANE_FAR, FRUSTUM_PLANE_BOTTOM },

			{ FRUSTUM_PLANE_LEFT, FRUSTUM_PLANE_TOP },
			{ FRUSTUM_PLANE_TOP, FRUSTUM_PLANE_RIGHT },
			{ FRUSTUM_PLANE_RIGHT, FRUSTUM_PLANE_BOTTOM },
			{ FRUSTUM_PLANE_BOTTOM, FRUSTUM_PLANE_LEFT },
		};

		////// Vertex indices of edges on the boundary between two planes
		UINT32 sharedEdges[][2] =
		{
			{ 3, 0 },{ 1, 2 },{ 0, 1 },{ 2, 3 },
			{ 4, 7 },{ 6, 5 },{ 5, 4 },{ 7, 6 },
			{ 4, 0 },{ 5, 1 },{ 6, 2 },{ 7, 3 }
		};

		for(UINT32 i = 0; i < 12; i++)
		{
			const Plane& planeA = viewPlanes[adjacentPlanes[i][0]];
			const Plane& planeB = viewPlanes[adjacentPlanes[i][1]];

			float dotA = planeA.normal.dot(lightDir);
			float dotB = planeB.normal.dot(lightDir);

			if((dotA * dotB) < 0.0f)
			{
				const Vector3& vertA = frustumVerts[sharedEdges[i][0]];
				const Vector3& vertB = frustumVerts[sharedEdges[i][1]];
				Vector3 vertC = vertA + lightDir;

				if (dotA < 0.0f)
					lightVolume.push_back(Plane(vertA, vertB, vertC));
				else
					lightVolume.push_back(Plane(vertB, vertA, vertC));
			}
		}

		return ConvexVolume(lightVolume);
	}

	float ShadowRendering::getCSMSplitDistance(const RendererView& view, UINT32 index, UINT32 numCascades)
	{
		auto& shadowSettings = view.getRenderSettings().shadowSettings;
		float distributionExponent = shadowSettings.cascadeDistributionExponent;

		// First determine the scale of the split, relative to the entire range
		float scaleModifier = 1.0f;
		float scale = 0.0f;
		float totalScale = 0.0f;

		//// Split 0 corresponds to near plane
		if (index > 0)
		{
			for (UINT32 i = 0; i < numCascades; i++)
			{
				if (i < index)
					scale += scaleModifier;

				totalScale += scaleModifier;
				scaleModifier *= distributionExponent;
			}

			scale = scale / totalScale;
		}

		// Calculate split distance in Z
		auto& viewProps = view.getProperties();
		float near = viewProps.nearPlane;
		float far = Math::clamp(shadowSettings.directionalShadowDistance, viewProps.nearPlane, viewProps.farPlane);

		return near + (far - near) * scale;
	}

	float ShadowRendering::getDepthBias(const Light& light, float radius, float depthRange, UINT32 mapSize)
	{
		const static float RADIAL_LIGHT_BIAS = 0.005f;
		const static float SPOT_DEPTH_BIAS = 0.01f;
		const static float DIR_DEPTH_BIAS = 0.001f; // In clip space units
		const static float DEFAULT_RESOLUTION = 512.0f;
		
		// Increase bias if map size smaller than some resolution
		float resolutionScale = 1.0f;
		
		if (light.getType() != LightType::Directional)
			resolutionScale = DEFAULT_RESOLUTION / (float)mapSize;

		// Adjust range because in shader we compare vs. clip space depth
		float rangeScale = 1.0f;
		if (light.getType() == LightType::Spot)
			rangeScale = 1.0f / depthRange;
		
		const RenderAPICapabilities& caps = gCaps();
		float deviceDepthRange = caps.maxDepth - caps.minDepth;

		float defaultBias = 1.0f;
		switch(light.getType())
		{
		case LightType::Directional: 
			defaultBias = DIR_DEPTH_BIAS * deviceDepthRange;

			// Use larger bias for further away cascades
			defaultBias *= depthRange * 0.01f;
			break;
		case LightType::Radial: 
			defaultBias = RADIAL_LIGHT_BIAS;
			break;
		case LightType::Spot: 
			defaultBias = SPOT_DEPTH_BIAS;
			break;
		default:
			break;
		}
		
		return defaultBias * light.getShadowBias() * resolutionScale * rangeScale;
	}

	float ShadowRendering::getFadeTransition(const Light& light, float radius, float depthRange, UINT32 mapSize)
	{
		const static float SPOT_LIGHT_SCALE = 1000.0f;
		const static float DIR_LIGHT_SCALE = 50000000.0f;

		// Note: Currently fade transitions are only used in spot & directional (non omni-directional) lights, so no need
		// to account for radial light type.
		if (light.getType() == LightType::Directional)
		{
			// Just use a large value, as we want a minimal transition region
			return DIR_LIGHT_SCALE;
		}
		else
			return fabs(light.getShadowBias()) * SPOT_LIGHT_SCALE;
	}
}}
