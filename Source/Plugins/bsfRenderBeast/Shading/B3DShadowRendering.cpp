//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DShadowRendering.h"
#include "B3DRendererView.h"
#include "B3DRenderBeastScene.h"
#include "B3DRenderBeast.h"
#include "Components/B3DLight.h"
#include "Renderer/B3DRendererUtility.h"
#include "Material/B3DMaterialParameterAdapter.h"
#include "Mesh/B3DMesh.h"
#include "Components/B3DCamera.h"
#include "Utility/B3DBitwise.h"
#include "RenderAPI/B3DVertexDescription.h"
#include "Renderer/B3DRenderer.h"
#include "B3DRendererRenderable.h"
#include "RenderAPI/B3DGpuCommandBuffer.h"
#include "RenderAPI/B3DRenderTexture.h"
#include "RenderAPI/B3DGpuProgramParameterDescription.h"
#include "RenderAPI/B3DGpuDevice.h"
#include "Math/B3DMath.h"

namespace b3d
{
	namespace render
	{

		ShadowUniformDefinition gShadowUniformDefinition;

		void ShadowDepthNormalMaterial::Bind(GpuCommandBuffer& commandBuffer, const SPtr<GpuParameterSet>& gpuParameters)
		{
			commandBuffer.SetGpuGraphicsPipelineState(mGraphicsPipeline);
			commandBuffer.SetStencilReferenceValue(mStencilReferenceValue);
			commandBuffer.SetGpuParameterSet(gpuParameters);
		}

		void ShadowDepthNormalMaterial::PopulateParameters(const SPtr<GpuParameterSet>& gpuParameters, const GpuBufferSuballocation& shadowUniforms)
		{
			gpuParameters->SetUniformBuffer("ShadowParams", shadowUniforms);
		}

		ShadowDepthNormalMaterial* ShadowDepthNormalMaterial::GetVariation(bool skinned, bool morph)
		{
			if(skinned)
			{
				if(morph)
					return Get(GetVariation<true, true>());

				return Get(GetVariation<true, false>());
			}
			else
			{
				if(morph)
					return Get(GetVariation<false, true>());

				return Get(GetVariation<false, false>());
			}
		}

		void ShadowDepthNormalNoPSMaterial::Bind(GpuCommandBuffer& commandBuffer, const SPtr<GpuParameterSet>& gpuParameters)
		{
			commandBuffer.SetGpuGraphicsPipelineState(mGraphicsPipeline);
			commandBuffer.SetStencilReferenceValue(mStencilReferenceValue);
			commandBuffer.SetGpuParameterSet(gpuParameters);
		}

		void ShadowDepthNormalNoPSMaterial::PopulateParameters(const SPtr<GpuParameterSet>& gpuParameters, const GpuBufferSuballocation& shadowUniforms)
		{
			gpuParameters->SetUniformBuffer("ShadowParams", shadowUniforms);
		}

		ShadowDepthNormalNoPSMaterial* ShadowDepthNormalNoPSMaterial::GetVariation(bool skinned, bool morph)
		{
			if(skinned)
			{
				if(morph)
					return Get(GetVariation<true, true>());

				return Get(GetVariation<true, false>());
			}
			else
			{
				if(morph)
					return Get(GetVariation<false, true>());

				return Get(GetVariation<false, false>());
			}
		}

		void ShadowDepthDirectionalMaterial::Bind(GpuCommandBuffer& commandBuffer, const SPtr<GpuParameterSet>& gpuParameters)
		{
			commandBuffer.SetGpuGraphicsPipelineState(mGraphicsPipeline);
			commandBuffer.SetStencilReferenceValue(mStencilReferenceValue);
			commandBuffer.SetGpuParameterSet(gpuParameters);
		}

		void ShadowDepthDirectionalMaterial::PopulateParameters(const SPtr<GpuParameterSet>& gpuParameters, const GpuBufferSuballocation& shadowUniforms)
		{
			gpuParameters->SetUniformBuffer("ShadowParams", shadowUniforms);
		}

		ShadowDepthDirectionalMaterial* ShadowDepthDirectionalMaterial::GetVariation(bool skinned, bool morph)
		{
			if(skinned)
			{
				if(morph)
					return Get(GetVariation<true, true>());

				return Get(GetVariation<true, false>());
			}
			else
			{
				if(morph)
					return Get(GetVariation<false, true>());

				return Get(GetVariation<false, false>());
			}
		}

		ShadowCubeMatricesUniformDefinition gShadowCubeMatricesUniformDefinition;
		ShadowCubeMasksUniformDefinition gShadowCubeMasksUniformDefinition;

		void ShadowDepthCubeMaterial::Bind(GpuCommandBuffer& commandBuffer, const SPtr<GpuParameterSet>& gpuParameters)
		{
			commandBuffer.SetGpuGraphicsPipelineState(mGraphicsPipeline);
			commandBuffer.SetStencilReferenceValue(mStencilReferenceValue);
			commandBuffer.SetGpuParameterSet(gpuParameters);
		}

		void ShadowDepthCubeMaterial::PopulateParameters(const SPtr<GpuParameterSet>& gpuParameters, const GpuBufferSuballocation& shadowUniforms, const GpuBufferSuballocation& shadowCubeMatrices, const GpuBufferSuballocation& shadowCubeMasks)
		{
			gpuParameters->SetUniformBuffer("ShadowParams", shadowUniforms);
			gpuParameters->SetUniformBuffer("ShadowCubeMatrices", shadowCubeMatrices);
			gpuParameters->SetUniformBuffer("ShadowCubeMasks", shadowCubeMasks);
		}

		ShadowDepthCubeMaterial* ShadowDepthCubeMaterial::GetVariation(bool skinned, bool morph)
		{
			if(skinned)
			{
				if(morph)
					return Get(GetVariation<true, true>());

				return Get(GetVariation<true, false>());
			}
			else
			{
				if(morph)
					return Get(GetVariation<false, true>());

				return Get(GetVariation<false, false>());
			}
		}

		ShadowProjectUniformDefinition gShadowProjectUniformDefinition;
		ShadowProjectVertUniformDefinition gShadowProjectVertUniformDefinition;

		void ShadowProjectStencilMaterial::Bind(GpuCommandBuffer& commandBuffer)
		{
			RendererMaterial::Bind(commandBuffer, false);
		}

		ShadowProjectStencilMaterial* ShadowProjectStencilMaterial::GetVariation(bool directional, bool useZFailStencil)
		{
			if(directional)
				return Get(GetVariation<true, true>());
			else
			{
				if(useZFailStencil)
					return Get(GetVariation<false, true>());
				else
					return Get(GetVariation<false, false>());
			}
		}

		void ShadowProjectMaterial::Bind(GpuCommandBuffer& commandBuffer)
		{
			RendererMaterial::Bind(commandBuffer, false);
		}

		SPtr<SamplerState> ShadowProjectMaterial::GetShadowSampler(GpuDevice& gpuDevice)
		{
			SamplerStateInformation desc;
			desc.MinFilter = FO_POINT;
			desc.MagFilter = FO_POINT;
			desc.MipFilter = FO_POINT;
			desc.AddressMode.U = TAM_CLAMP;
			desc.AddressMode.V = TAM_CLAMP;
			desc.AddressMode.W = TAM_CLAMP;

			return gpuDevice.FindOrCreateSamplerState(desc);
		}

		ShadowProjectMaterial* ShadowProjectMaterial::GetVariation(u32 quality, bool directional, bool MSAA)
		{
#define BIND_MAT(QUALITY)                                         \
	{                                                             \
		if(directional)                                           \
			if(MSAA)                                              \
				return Get(GetVariation<QUALITY, true, true>());  \
			else                                                  \
				return Get(GetVariation<QUALITY, true, false>()); \
		else if(MSAA)                                             \
			return Get(GetVariation<QUALITY, false, true>());     \
		else                                                      \
			return Get(GetVariation<QUALITY, false, false>());    \
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

		ShadowProjectOmniUniformDefinition gShadowProjectOmniUniformDefinition;

		void ShadowProjectOmniMaterial::Bind(GpuCommandBuffer& commandBuffer)
		{
			RendererMaterial::Bind(commandBuffer, false);
		}

		SPtr<SamplerState> ShadowProjectOmniMaterial::GetShadowSampler(GpuDevice& gpuDevice)
		{
			SamplerStateInformation desc;
			desc.MinFilter = FO_LINEAR;
			desc.MagFilter = FO_LINEAR;
			desc.MipFilter = FO_POINT;
			desc.AddressMode.U = TAM_CLAMP;
			desc.AddressMode.V = TAM_CLAMP;
			desc.AddressMode.W = TAM_CLAMP;
			desc.ComparisonFunc = CMPF_GREATER_EQUAL;

			return gpuDevice.FindOrCreateSamplerState(desc);
		}

		ShadowProjectOmniMaterial* ShadowProjectOmniMaterial::GetVariation(u32 quality, bool inside, bool MSAA)
		{
#define BIND_MAT(QUALITY)                                         \
	{                                                             \
		if(inside)                                                \
			if(MSAA)                                              \
				return Get(GetVariation<QUALITY, true, true>());  \
			else                                                  \
				return Get(GetVariation<QUALITY, true, false>()); \
		else if(MSAA)                                             \
			return Get(GetVariation<QUALITY, false, true>());     \
		else                                                      \
			return Get(GetVariation<QUALITY, false, false>());    \
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

		/** Helper class containing utilities for shadow projection material binding. */
		struct ShadowProjectionParameterBinding
		{
			/**
			 * Binds common shadow parameters that are shared by all shadow projection materials.
			 *
			 * @param gpuParameters          The GPU parameters object to set parameters on.
			 * @param vertexParameterBuffer  The vertex parameter buffer containing position and scale.
			 * @param perCameraBuffer        The per-camera parameter buffer.
			 * @param shadowParameterBuffer  Optional shadow-specific parameter buffer (for projection materials).
			 */
			static void BindCommonParameters(const SPtr<GpuParameterSet>& gpuParameters, const SPtr<GpuBuffer>& vertexParameterBuffer, const GpuBufferSuballocation& perCameraBuffer, const SPtr<GpuBuffer>& shadowParameterBuffer = nullptr)
			{
				// Set vertex parameters
				gpuParameters->TrySetUniformBuffer("VertParams", vertexParameterBuffer);

				// Set per-camera buffer
				gpuParameters->SetUniformBuffer("PerCamera", perCameraBuffer);

				// Set shadow-specific parameters buffer (if provided, for projection materials)
				if(shadowParameterBuffer != nullptr)
					gpuParameters->SetUniformBuffer("Params", shadowParameterBuffer);
			}

			/**
			 * Binds shadow projection stencil material parameters for standard (non-omnidirectional) shadows.
			 *
			 * @param gpuParameters The GPU parameters object to set parameters on.
			 */
			static void BindStencilProjectionParameters(const SPtr<GpuParameterSet>& gpuParameters, const GpuBufferSuballocation& perCameraBuffer, const SPtr<GpuBuffer>& vertexParameterBuffer)
			{
				// Bind common parameters (VertParams, PerCamera, Params)
				// Default light position for spot/directional lights
				BindCommonParameters(gpuParameters, vertexParameterBuffer, perCameraBuffer, nullptr);
			}

			/**
			 * Binds shadow projection material parameters for standard (non-omnidirectional) shadows.
			 *
			 * @param gpuParameters The GPU parameters object to set parameters on.
			 * @param gpuDevice     The GPU device used to retrieve the sampler state.
			 */
			static void BindProjectionParameters(const SPtr<GpuParameterSet>& gpuParameters, GpuDevice& gpuDevice, const SPtr<Texture>& shadowMap, const SPtr<GpuBuffer>& shadowParameterBuffer, const GpuBufferSuballocation& perCameraBuffer, const SPtr<GpuBuffer>& vertexParameterBuffer, const GBufferTextures& gbuffer)
			{
				// Bind common parameters (VertParams, PerCamera, Params)
				// Default light position for spot/directional lights
				BindCommonParameters(gpuParameters, vertexParameterBuffer, perCameraBuffer, shadowParameterBuffer);

				// Set GBuffer textures
				GBufferParameterBinding::Set(gpuDevice, gpuParameters, gbuffer);

				// Set shadow texture and sampler
				gpuParameters->SetSampledTexture("gShadowTex", shadowMap);

				if(gpuParameters->HasSamplerState("gShadowSampler"))
					gpuParameters->SetSamplerState("gShadowSampler", ShadowProjectMaterial::GetShadowSampler(gpuDevice));
				else
					gpuParameters->SetSamplerState("gShadowTex", ShadowProjectMaterial::GetShadowSampler(gpuDevice));
			}

			/**
			 * Binds omnidirectional shadow projection material parameters.
			 *
			 * @param gpuParameters The GPU parameters object to set parameters on.
			 * @param gpuDevice     The GPU device used to retrieve the sampler state.
			 */
			static void BindOmnidirectionalProjectionParameters(const SPtr<GpuParameterSet>& gpuParameters, GpuDevice& gpuDevice, const Light& light, const SPtr<Texture>& shadowMap, const SPtr<GpuBuffer>& shadowParameterBuffer, const GpuBufferSuballocation& perCameraBuffer, const SPtr<GpuBuffer>& vertexParameterBuffer, const GBufferTextures& gbuffer)
			{
				// Bind common parameters (VertParams, PerCamera, Params)
				// Set light position and radius for omnidirectional light
				const Transform& transform = light.GetWorldTransform();
				BindCommonParameters(gpuParameters, vertexParameterBuffer, perCameraBuffer, shadowParameterBuffer);

				// Set GBuffer textures
				GBufferParameterBinding::Set(gpuDevice, gpuParameters, gbuffer);

				// Set shadow cubemap texture and sampler
				gpuParameters->SetSampledTexture("gShadowCubeTex", shadowMap);

				if(gpuParameters->HasSamplerState("gShadowCubeSampler"))
					gpuParameters->SetSamplerState("gShadowCubeSampler", ShadowProjectOmniMaterial::GetShadowSampler(gpuDevice));
				else
					gpuParameters->SetSamplerState("gShadowCubeTex", ShadowProjectOmniMaterial::GetShadowSampler(gpuDevice));
			}
		};

		void ShadowInfo::UpdateNormArea(u32 atlasSize)
		{
			NormArea.X = Area.X / (float)atlasSize;
			NormArea.Y = Area.Y / (float)atlasSize;
			NormArea.Width = Area.Width / (float)atlasSize;
			NormArea.Height = Area.Height / (float)atlasSize;
		}

		ShadowMapAtlas::ShadowMapAtlas(u32 size)
			: mLayout(0, 0, size, size, true), mLastUsedCounter(0)
		{
			mAtlas = GpuResourcePool::Instance().Get(
				PooledRenderTextureCreateInformation::Create2D(kShadowMapFormat, size, size, TextureUsageFlag::DepthStencil));
		}

		bool ShadowMapAtlas::AddMap(u32 size, Area2I& area, u32 border)
		{
			u32 sizeWithBorder = size + border * 2;

			u32 x, y;
			if(!mLayout.AddElement(sizeWithBorder, sizeWithBorder, x, y))
				return false;

			area.Width = area.Height = size;
			area.X = x + border;
			area.Y = y + border;

			mLastUsedCounter = 0;
			return true;
		}

		void ShadowMapAtlas::MarkAsUnused()
		{
			mLayout.Clear();
			mLastUsedCounter++;
		}

		bool ShadowMapAtlas::IsEmpty() const
		{
			return mLayout.IsEmpty();
		}

		SPtr<Texture> ShadowMapAtlas::GetTexture() const
		{
			return mAtlas->Texture;
		}

		SPtr<RenderTexture> ShadowMapAtlas::GetTarget() const
		{
			return mAtlas->RenderTexture;
		}

		ShadowMapBase::ShadowMapBase(u32 size)
			: mSize(size), mIsUsed(false), mLastUsedCounter(0)
		{}

		SPtr<Texture> ShadowMapBase::GetTexture() const
		{
			return mShadowMap->Texture;
		}

		ShadowCubemap::ShadowCubemap(u32 size)
			: ShadowMapBase(size)
		{
			mShadowMap = GpuResourcePool::Instance().Get(
				PooledRenderTextureCreateInformation::CreateCube(kShadowMapFormat, size, size, TextureUsageFlag::DepthStencil));
		}

		SPtr<RenderTexture> ShadowCubemap::GetTarget() const
		{
			return mShadowMap->RenderTexture;
		}

		ShadowCascadedMap::ShadowCascadedMap(u32 size, u32 numCascades)
			: ShadowMapBase(size), mNumCascades(numCascades), mTargets(numCascades), mShadowInfos(numCascades)
		{
			mShadowMap = GpuResourcePool::Instance().Get(PooledRenderTextureCreateInformation::Create2D(kShadowMapFormat, size, size, TextureUsageFlag::DepthStencil, 0, false, numCascades));

			RenderTextureCreateInformation rtDesc;
			rtDesc.DepthStencilSurface.Texture = mShadowMap->Texture;
			rtDesc.DepthStencilSurface.FaceCount = 1;

			for(u32 i = 0; i < mNumCascades; ++i)
			{
				rtDesc.DepthStencilSurface.Face = i;
				mTargets[i] = RenderTexture::Create(rtDesc);
			}
		}

		SPtr<RenderTexture> ShadowCascadedMap::GetTarget(u32 cascadeIdx) const
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
				{}

				Command(RenderableElement* element)
					: Element(element), IsElement(true)
				{}

				union
				{
					RenderableElement* Element;
					RendererRenderable* Renderable;
				};

				SPtr<GpuParameterSet> GpuParameterSet;

				bool IsElement : 1;
			};

			template <class Options>
			static void Execute(GpuCommandBuffer& commandBuffer, RenderBeastScene& scene, const FrameInfo& frameInfo, const SPtr<RenderTarget>& renderTarget, const Options& opt)
			{
				static_assert((u32)RenderableAnimType::Count == 4, "RenderableAnimType is expected to have four sequential entries.");

				const SceneInfo& sceneInfo = scene.GetSceneInfo();

				B3DMarkAllocatorFrame();
				{
					FrameVector<Command> commands[4];

					static const ShaderVariationParameters* kVariationLookup[4];
					kVariationLookup[0] = &GetVertexInputVariation<false, false, false>(false);
					kVariationLookup[1] = &GetVertexInputVariation<true, false, false>(false);
					kVariationLookup[2] = &GetVertexInputVariation<false, true, false>(false);
					kVariationLookup[3] = &GetVertexInputVariation<true, true, false>(false);

					RenderPassCreateInformation passCreateInformation(renderTarget);
					TInlineArray<SPtr<GpuParameterSet>, 4> perObjectParameterSets;

					// Make a list of relevant renderables and prepare them for rendering
					for(u32 renderableIndex = 0; renderableIndex < sceneInfo.Renderables.size(); renderableIndex++)
					{
						const Sphere& bounds = sceneInfo.RenderableCullInfos[renderableIndex].Bounds.GetSphere();
						if(!opt.Intersects(bounds))
							continue;

						scene.PrepareVisibleRenderable(renderableIndex, frameInfo);

						RendererRenderable* renderable = sceneInfo.Renderables[renderableIndex];

						// Register per-object shadow parameter set if not already registered
						const SPtr<GpuParameterSet>& perObjectParameterSet = opt.GetShadowParameterSet(renderable);
						auto found = std::find(perObjectParameterSets.begin(), perObjectParameterSets.end(), perObjectParameterSet);
						if(found == perObjectParameterSets.end())
						{
							passCreateInformation.Parameters.Add(perObjectParameterSet);
							perObjectParameterSets.Add(perObjectParameterSet);
						}

						Command renderableCommand;
						renderableCommand.IsElement = false;
						renderableCommand.Renderable = renderable;

						bool renderableBound[4];
						B3DZeroOut(renderableBound);

						for(auto& element : renderable->Elements)
						{
							const u32 animationTypeIndex = (u32)element.AnimType;

							if(!renderableBound[animationTypeIndex])
							{
								opt.Prepare(renderableCommand, bounds, *kVariationLookup[animationTypeIndex]);
								passCreateInformation.Parameters.Add(renderableCommand.GpuParameterSet);

								commands[animationTypeIndex].push_back(renderableCommand);
								renderableBound[animationTypeIndex] = true;
							}

							commands[animationTypeIndex].push_back(Command(&element));
						}
					}

					commandBuffer.BeginRenderPass(passCreateInformation);
					opt.PrepareRenderTarget(commandBuffer);

					const u32 perObjectDynamicOffsetIndex = GetRenderBeast()->GetRenderableParameterSetInfo().PerObjectDynamicOffsetIndex;

					for(u32 i = 0; i < (u32)RenderableAnimType::Count; i++)
					{
						for(auto& command : commands[i])
						{
							if(command.IsElement)
							{
								const RenderableElement& element = *command.Element;

								if(element.MorphVertexDefinition == nullptr)
									GetRendererUtility().Draw(commandBuffer, element.Mesh, element.SubMesh);
								else
									GetRendererUtility().DrawMorph(commandBuffer, element.Mesh, element.SubMesh, element.MorphShapeBuffer, element.MorphVertexDefinition);
							}
							else
							{
								opt.Bind(commandBuffer, command);

								// Bind per-object shadow parameter set and dynamic offset
								const GpuBufferSuballocation& perObjectSuballocation = command.Renderable->PerObjectSuballocation;
								SPtr<GpuParameterSet> shadowParameterSet = opt.GetShadowParameterSet(command.Renderable); // TODO - Should sort objects by set if possible, to avoid switching sets
								commandBuffer.SetGpuParameterSet(shadowParameterSet);
								commandBuffer.SetDynamicBufferOffset(GpuPipelineSet::kPerObject, perObjectDynamicOffsetIndex, perObjectSuballocation.GetSuballocationOffset());
							}
						}
					}

					commandBuffer.EndRenderPass();
				}
				B3DClearAllocatorFrame();
			}
		};

		/** Specialization used for ShadowRenderQueue when rendering cube (omnidirectional) shadow maps (all faces at once). */
		struct ShadowRenderQueueCubeOptions
		{
			ShadowRenderQueueCubeOptions(
				ShadowRendering& shadowRendering,
				const ConvexVolume (&frustums)[6],
				const ConvexVolume& boundingVolume,
				const GpuBufferSuballocation& shadowParamsBuffer,
				const GpuBufferSuballocation& shadowCubeMatricesBuffer)
				: ShadowRenderer(shadowRendering), Frustums(frustums), BoundingVolume(boundingVolume), ShadowUniformBuffer(shadowParamsBuffer), ShadowCubeMatricesBuffer(shadowCubeMatricesBuffer)
			{}

			void PrepareRenderTarget(GpuCommandBuffer& commandBuffer) const
			{
				commandBuffer.ClearRenderTarget(RT_DEPTH);
			}

			bool Intersects(const Sphere& bounds) const
			{
				return BoundingVolume.Intersects(bounds);
			}

			void Prepare(ShadowRenderQueue::Command& command, const Sphere& bounds, const ShaderVariationParameters& variation) const
			{
				Material = ShadowDepthCubeMaterial::Get(variation);

				command.GpuParameterSet = Material->CreateGpuParameterSet();
				GpuBufferMappedScope shadowCubeMatricesUniforms = gShadowCubeMasksUniformDefinition.AllocateTransient().Map();

				for(u32 j = 0; j < 6; j++)
					gShadowCubeMasksUniformDefinition.gFaceMasks.Set(shadowCubeMatricesUniforms, (Frustums[j].Intersects(bounds) ? 1 : 0), j);

				ShadowDepthCubeMaterial::PopulateParameters(command.GpuParameterSet, ShadowUniformBuffer, ShadowCubeMatricesBuffer, shadowCubeMatricesUniforms);
			}

			void Bind(GpuCommandBuffer& commandBuffer, ShadowRenderQueue::Command& command) const
			{
				Material->Bind(commandBuffer, command.GpuParameterSet);
			}

			SPtr<GpuParameterSet> GetShadowParameterSet(RendererRenderable* renderable) const
			{
				// Need to create a new parameter set since we also use geometry shaders, and the default parameter set doesn't support it
				return ShadowRenderer.GetOrCreateCubemapShadowParameterSet(renderable->PerObjectSuballocation.GetBuffer());
			}

			ShadowRendering& ShadowRenderer;
			const ConvexVolume (&Frustums)[6];
			const ConvexVolume& BoundingVolume;
			const GpuBufferSuballocation& ShadowUniformBuffer;
			const GpuBufferSuballocation& ShadowCubeMatricesBuffer;

			mutable ShadowDepthCubeMaterial* Material = nullptr;
		};

		/** Specialization used for ShadowRenderQueue when rendering cube (omnidirectional) shadow maps (one face at a time). */
		struct ShadowRenderQueueCubeSingleOptions
		{
			ShadowRenderQueueCubeSingleOptions(
				ShadowRendering& shadowRendering,
				const ConvexVolume& boundingVolume,
				const GpuBufferSuballocation& shadowUniformBuffer)
				: ShadowRenderer(shadowRendering), BoundingVolume(boundingVolume), ShadowUniformBuffer(shadowUniformBuffer)
			{}

			void PrepareRenderTarget(GpuCommandBuffer& commandBuffer) const
			{
				commandBuffer.ClearRenderTarget(RT_DEPTH);
			}

			bool Intersects(const Sphere& bounds) const
			{
				return BoundingVolume.Intersects(bounds);
			}

			void Prepare(ShadowRenderQueue::Command& command, const Sphere& bounds, const ShaderVariationParameters& variation) const
			{
				Material = ShadowDepthNormalNoPSMaterial::Get(variation);

				command.GpuParameterSet = Material->CreateGpuParameterSet();
				ShadowDepthNormalNoPSMaterial::PopulateParameters(command.GpuParameterSet, ShadowUniformBuffer);
			}

			void Bind(GpuCommandBuffer& commandBuffer, ShadowRenderQueue::Command& command) const
			{
				Material->Bind(commandBuffer, command.GpuParameterSet);
			}

			SPtr<GpuParameterSet> GetShadowParameterSet(RendererRenderable* renderable) const
			{
				return renderable->PerObjectParameterSet;
			}

			ShadowRendering& ShadowRenderer;
			const ConvexVolume& BoundingVolume;
			const GpuBufferSuballocation& ShadowUniformBuffer;

			mutable ShadowDepthNormalNoPSMaterial* Material = nullptr;
		};

		/** Specialization used for ShadowRenderQueue when rendering spot light shadow maps. */
		struct ShadowRenderQueueSpotOptions
		{
			ShadowRenderQueueSpotOptions(
				ShadowRendering& shadowRendering,
				Area2 viewportArea,
				const ConvexVolume& boundingVolume,
				const GpuBufferSuballocation& shadowUniformBuffer)
				: ShadowRenderer(shadowRendering), ViewportArea(viewportArea), BoundingVolume(boundingVolume), ShadowUniformBuffer(shadowUniformBuffer)
			{}

			void PrepareRenderTarget(GpuCommandBuffer& commandBuffer) const
			{
				commandBuffer.SetViewport(ViewportArea);
				commandBuffer.ClearViewport(RT_DEPTH);
			}

			bool Intersects(const Sphere& bounds) const
			{
				return BoundingVolume.Intersects(bounds);
			}

			void Prepare(ShadowRenderQueue::Command& command, const Sphere& bounds, const ShaderVariationParameters& variation) const
			{
				Material = ShadowDepthNormalMaterial::Get(variation);

				command.GpuParameterSet = Material->CreateGpuParameterSet();
				ShadowDepthNormalMaterial::PopulateParameters(command.GpuParameterSet, ShadowUniformBuffer);
			}

			void Bind(GpuCommandBuffer& commandBuffer, ShadowRenderQueue::Command& command) const
			{
				Material->Bind(commandBuffer, command.GpuParameterSet);
			}

			SPtr<GpuParameterSet> GetShadowParameterSet(RendererRenderable* renderable) const
			{
				return renderable->PerObjectParameterSet;
			}

			ShadowRendering& ShadowRenderer;
			const Area2& ViewportArea;
			const ConvexVolume& BoundingVolume;
			const GpuBufferSuballocation& ShadowUniformBuffer;

			mutable ShadowDepthNormalMaterial* Material = nullptr;
		};

		/** Specialization used for ShadowRenderQueue when rendering directional light shadow maps. */
		struct ShadowRenderQueueDirOptions
		{
			ShadowRenderQueueDirOptions(
				ShadowRendering& shadowRendering,
				const ConvexVolume& boundingVolume,
				const GpuBufferSuballocation& shadowUniformBuffer)
				: ShadowRenderer(shadowRendering), BoundingVolume(boundingVolume), ShadowUniformBuffer(shadowUniformBuffer)
			{}

			void PrepareRenderTarget(GpuCommandBuffer& commandBuffer) const
			{
				commandBuffer.ClearRenderTarget(RT_DEPTH);
			}

			bool Intersects(const Sphere& bounds) const
			{
				return BoundingVolume.Intersects(bounds);
			}

			void Prepare(ShadowRenderQueue::Command& command, const Sphere& bounds, const ShaderVariationParameters& variation) const
			{
				Material = ShadowDepthDirectionalMaterial::Get(variation);

				command.GpuParameterSet = Material->CreateGpuParameterSet();
				ShadowDepthDirectionalMaterial::PopulateParameters(command.GpuParameterSet, ShadowUniformBuffer);
			}

			void Bind(GpuCommandBuffer& commandBuffer, ShadowRenderQueue::Command& command) const
			{
				Material->Bind(commandBuffer, command.GpuParameterSet);
			}

			SPtr<GpuParameterSet> GetShadowParameterSet(RendererRenderable* renderable) const
			{
				return renderable->PerObjectParameterSet;
			}

			ShadowRendering& ShadowRenderer;
			const ConvexVolume& BoundingVolume;
			const GpuBufferSuballocation& ShadowUniformBuffer;

			mutable ShadowDepthDirectionalMaterial* Material = nullptr;
		};

		const u32 ShadowRendering::kMaxAtlasSize = 4096;
		const u32 ShadowRendering::kMaxUnusedFrames = 60;
		const u32 ShadowRendering::kMinShadowMapSize = 32;
		const u32 ShadowRendering::kShadowMapFadeSize = 64;
		const u32 ShadowRendering::kShadowMapBorder = 4;
		const float ShadowRendering::kCascadeFractionFade = 0.1f;

		ShadowRendering::ShadowRendering(u32 shadowMapSize)
			: mShadowMapSize(shadowMapSize)
		{
			const SPtr<GpuDevice>& gpuDevice = GetApplication().GetPrimaryGpuDevice();

			TInlineArray<VertexElement, 8> vertexElements;
			vertexElements.Add(VertexElement(VET_FLOAT3, VES_POSITION));

			mPositionOnlyVertexDescription = B3DMakeShared<VertexDescription>(vertexElements);

			// Create plane index and vertex buffers
			{
				GpuBufferCreateInformation vertexBufferCreateInformation;
				vertexBufferCreateInformation.Type = GpuBufferType::Vertex;
				vertexBufferCreateInformation.Flags = GpuBufferFlag::StoreOnCPUWithGPUAccess;
				vertexBufferCreateInformation.Vertex.Count = 8;
				vertexBufferCreateInformation.Vertex.ElementSize = mPositionOnlyVertexDescription->GetVertexStride(0);

				mPlaneVB = gpuDevice->CreateGpuBuffer(vertexBufferCreateInformation);

				GpuBufferCreateInformation indexBufferCreateInformation;
				indexBufferCreateInformation.Type = GpuBufferType::Index;
				indexBufferCreateInformation.Index.Type = IT_32BIT;
				indexBufferCreateInformation.Index.Count = 12;

				mPlaneIB = gpuDevice->CreateGpuBuffer(indexBufferCreateInformation);

				u32 indices[] = {
					// Far plane, back facing
					4, 7, 6,
					4, 6, 5,

					// Near plane, front facing
					0, 1, 2,
					0, 2, 3
				};

				GpuBufferUtility::Write(mPlaneIB, 0, sizeof(indices), indices);
			}

			// Create frustum index and vertex buffers
			{
				GpuBufferCreateInformation vertexBufferCreateInformation;
				vertexBufferCreateInformation.Type = GpuBufferType::Vertex;
				vertexBufferCreateInformation.Flags = GpuBufferFlag::StoreOnCPUWithGPUAccess;
				vertexBufferCreateInformation.Vertex.Count = 8;
				vertexBufferCreateInformation.Vertex.ElementSize = mPositionOnlyVertexDescription->GetVertexStride(0);

				mFrustumVB = gpuDevice->CreateGpuBuffer(vertexBufferCreateInformation);

				GpuBufferCreateInformation indexBufferCreateInformation;
				indexBufferCreateInformation.Type = GpuBufferType::Index;
				indexBufferCreateInformation.Index.Type = IT_32BIT;
				indexBufferCreateInformation.Index.Count = 36;

				mFrustumIB = gpuDevice->CreateGpuBuffer(indexBufferCreateInformation);
				GpuBufferUtility::Write(mFrustumIB, 0, sizeof(AABox::kCubeIndices), AABox::kCubeIndices);
			}

			// Create shadow per-object layout with vertex+geometry stage (for point light cubemap shadows)
			{
				GpuUniformBufferInformation perObjectInfo;
				perObjectInfo.Name = "PerObject";
				perObjectInfo.Set = GpuPipelineSet::kPerObject;
				perObjectInfo.Slot = 0;
				perObjectInfo.Size = Math::CeilToMultiple(gPerObjectUniformDefinition.GetSize() / 4u, 4u);
				perObjectInfo.Stages = GpuProgramStageBit::Vertex | GpuProgramStageBit::Geometry;
				perObjectInfo.IsShareable = true;

				GpuProgramParameterDescription description;
				description.UniformBuffers["PerObject"] = perObjectInfo;
				mCubemapShadowPerObjectLayout = gpuDevice->CreateGpuPipelineParameterSetLayout(description);
			}
		}

		void ShadowRendering::SetShadowMapSize(u32 size)
		{
			if(mShadowMapSize == size)
				return;

			mCascadedShadowMaps.clear();
			mAtlasShadowMaps.clear();
			mShadowCubemaps.clear();

			mShadowMapSize = size;
		}

		SPtr<GpuParameterSet> ShadowRendering::GetOrCreateCubemapShadowParameterSet(const SPtr<GpuBuffer>& perObjectBuffer)
		{
			GpuBuffer* key = perObjectBuffer.get();

			auto iter = mCubemapShadowParameterSets.find(key);
			if(iter != mCubemapShadowParameterSets.end())
				return iter->second.ParameterSet;

			GpuParameterSetPool& pool = GetRenderer()->GetParameterSetPool();
			SPtr<GpuParameterSet> parameterSet = pool.Create(mCubemapShadowPerObjectLayout, GpuPipelineSet::kPerObject);
			parameterSet->SetUniformBuffer("PerObject", perObjectBuffer, 0);

			CubemapShadowParameterSetEntry entry;
			entry.ParameterSet = parameterSet;
			entry.RefCount = 1;

			mCubemapShadowParameterSets[key] = entry;
			return parameterSet;
		}

		void ShadowRendering::RenderShadowMaps(GpuCommandBuffer& commandBuffer, RenderBeastScene& scene, const RendererViewGroup& viewGroup, const FrameInfo& frameInfo)
		{
			// Note: Currently all shadows are dynamic and are rebuilt every frame. I should later added support for static
			// shadow maps which can be used for immovable lights. Such a light can then maintain a set of shadow maps,
			// one of which is static and only effects the static geometry, while the rest are per-object shadow maps used
			// for dynamic objects. Then only a small subset of geometry needs to be redrawn, instead of everything.

			// Note: Add support for per-object shadows and a way to force a renderable to use per-object shadows. This can be
			// used for adding high quality shadows on specific objects (e.g. important characters during cinematics).

			const SceneInfo& sceneInfo = scene.GetSceneInfo();
			const VisibilityInfo& visibility = viewGroup.GetVisibilityInfo();

			// Clear all transient data from last frame
			mShadowInfos.clear();
			mCubemapShadowParameterSets.clear();

			mSpotLightShadows.resize(sceneInfo.SpotLights.size());
			mRadialLightShadows.resize(sceneInfo.RadialLights.size());
			mDirectionalLightShadows.resize(sceneInfo.DirectionalLights.size());

			mSpotLightShadowOptions.clear();
			mRadialLightShadowOptions.clear();

			// Clear all dynamic light atlases
			for(auto& entry : mCascadedShadowMaps)
				entry.MarkAsUnused();

			for(auto& entry : mAtlasShadowMaps)
				entry.MarkAsUnused();

			for(auto& entry : mShadowCubemaps)
				entry.MarkAsUnused();

			// Determine shadow map sizes and sort them
			u32 shadowInfoCount = 0;
			for(u32 i = 0; i < (u32)sceneInfo.SpotLights.size(); ++i)
			{
				const RendererLight& light = sceneInfo.SpotLights[i];
				mSpotLightShadows[i].StartIndex = shadowInfoCount;
				mSpotLightShadows[i].ShadowCount = 0;

				// Note: I'm using visibility across all views, while I could be using visibility for every view individually,
				// if I kept that information somewhere
				if(!light.Internal->GetCastsShadow() || !visibility.SpotLights[i])
					continue;

				ShadowMapOptions options;
				options.LightIdx = i;

				float maxFadePercent;
				CalcShadowMapProperties(light, viewGroup, kShadowMapBorder, options.MapSize, options.FadePercents, maxFadePercent);

				// Don't render shadow maps that will end up nearly completely faded out
				if(maxFadePercent < 0.005f)
					continue;

				mSpotLightShadowOptions.push_back(options);
				shadowInfoCount++; // For now, always a single fully dynamic shadow for a single light, but that may change
			}

			for(u32 i = 0; i < (u32)sceneInfo.RadialLights.size(); ++i)
			{
				const RendererLight& light = sceneInfo.RadialLights[i];
				mRadialLightShadows[i].StartIndex = shadowInfoCount;
				mRadialLightShadows[i].ShadowCount = 0;

				// Note: I'm using visibility across all views, while I could be using visibility for every view individually,
				// if I kept that information somewhere
				if(!light.Internal->GetCastsShadow() || !visibility.RadialLights[i])
					continue;

				ShadowMapOptions options;
				options.LightIdx = i;

				float maxFadePercent;
				CalcShadowMapProperties(light, viewGroup, 0, options.MapSize, options.FadePercents, maxFadePercent);

				// Don't render shadow maps that will end up nearly completely faded out
				if(maxFadePercent < 0.005f)
					continue;

				mRadialLightShadowOptions.push_back(options);

				shadowInfoCount++; // For now, always a single fully dynamic shadow for a single light, but that may change
			}

			// Sort spot lights by size so they fit neatly in the texture atlas
			std::sort(mSpotLightShadowOptions.begin(), mSpotLightShadowOptions.end(), [](const ShadowMapOptions& a, const ShadowMapOptions& b)
					  { return a.MapSize > b.MapSize; });

			// Reserve space for shadow infos
			mShadowInfos.resize(shadowInfoCount);

			// Deallocate unused textures (must be done before rendering shadows, in order to ensure indices don't change)
			for(auto iter = mAtlasShadowMaps.begin(); iter != mAtlasShadowMaps.end(); ++iter)
			{
				if(iter->GetLastUsedCounter() >= kMaxUnusedFrames)
				{
					// These are always populated in order, so we can assume all following atlases are also empty
					mAtlasShadowMaps.erase(iter, mAtlasShadowMaps.end());
					break;
				}
			}

			for(auto iter = mCascadedShadowMaps.begin(); iter != mCascadedShadowMaps.end();)
			{
				if(iter->GetLastUsedCounter() >= kMaxUnusedFrames)
					iter = mCascadedShadowMaps.erase(iter);
				else
					++iter;
			}

			for(auto iter = mShadowCubemaps.begin(); iter != mShadowCubemaps.end();)
			{
				if(iter->GetLastUsedCounter() >= kMaxUnusedFrames)
					iter = mShadowCubemaps.erase(iter);
				else
					++iter;
			}

			// Render shadow maps
			for(u32 i = 0; i < (u32)sceneInfo.DirectionalLights.size(); ++i)
			{
				const RendererLight& light = sceneInfo.DirectionalLights[i];

				if(!light.Internal->GetCastsShadow())
					continue;

				u32 numViews = viewGroup.GetViewCount();
				mDirectionalLightShadows[i].ViewShadows.Resize(numViews);

				for(u32 j = 0; j < numViews; ++j)
					RenderCascadedShadowMaps(commandBuffer, *viewGroup.GetView(j), i, scene, frameInfo);
			}

			for(auto& entry : mSpotLightShadowOptions)
			{
				u32 lightIdx = entry.LightIdx;
				RenderSpotShadowMap(commandBuffer, sceneInfo.SpotLights[lightIdx], entry, scene, frameInfo);
			}

			for(auto& entry : mRadialLightShadowOptions)
			{
				u32 lightIdx = entry.LightIdx;
				RenderRadialShadowMap(commandBuffer, sceneInfo.RadialLights[lightIdx], entry, scene, frameInfo);
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
		std::array<Vector3, 8> GetFrustum(const Matrix4& invVP, ConvexVolume& worldFrustum)
		{
			std::array<Vector3, 8> output;

			const SPtr<GpuDevice>& gpuDevice = GetApplication().GetPrimaryGpuDevice();
			const GpuDeviceCapabilities& caps = gpuDevice->GetCapabilities();

			float flipY = 1.0f;
			if(caps.Conventions.NdcYAxis == GpuBackendConventions::Axis::Down)
				flipY = -1.0f;

			AABox frustumCube(
				Vector3(-1, -1 * flipY, caps.MinDepth),
				Vector3(1, 1 * flipY, caps.MaxDepth));

			for(size_t i = 0; i < output.size(); i++)
			{
				Vector3 corner = frustumCube.GetCorner((AABox::Corner)i);
				output[i] = invVP.Multiply(corner);
			}

			Vector<Plane> planes(6);
			planes[FRUSTUM_PLANE_NEAR] = Plane(output[AABox::NearLeftBottom], output[AABox::NearRightBottom], output[AABox::NearRightTop]);
			planes[FRUSTUM_PLANE_FAR] = Plane(output[AABox::FarLeftBottom], output[AABox::FarLeftTop], output[AABox::FarRightTop]);
			planes[FRUSTUM_PLANE_LEFT] = Plane(output[AABox::NearLeftBottom], output[AABox::NearLeftTop], output[AABox::FarLeftTop]);
			planes[FRUSTUM_PLANE_RIGHT] = Plane(output[AABox::FarRightTop], output[AABox::NearRightTop], output[AABox::NearRightBottom]);
			planes[FRUSTUM_PLANE_TOP] = Plane(output[AABox::NearLeftTop], output[AABox::NearRightTop], output[AABox::FarRightTop]);
			planes[FRUSTUM_PLANE_BOTTOM] = Plane(output[AABox::NearLeftBottom], output[AABox::FarLeftBottom], output[AABox::FarRightBottom]);

			worldFrustum = ConvexVolume(planes);
			return output;
		}

		/**
		 * Converts a point in mixed space (clip_x, clip_y, view_z, view_w) to UV coordinates on a shadow map (x, y),
		 * and normalized linear depth from the shadow caster's perspective (z).
		 */
		Matrix4 CreateMixedToShadowUvMatrix(const Matrix4& viewP, const Matrix4& viewInvVP, const Area2& shadowMapArea, float depthScale, float depthOffset, const Matrix4& shadowViewProj)
		{
			// Projects a point from (clip_x, clip_y, view_z, view_w) into clip space
			Matrix4 mixedToShadow = Matrix4::kIdentity;
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
			const SPtr<GpuDevice>& gpuDevice = GetApplication().GetPrimaryGpuDevice();
			const GpuBackendConventions& gpuBackendConventions = gpuDevice->GetCapabilities().Conventions;

			float flipY = -1.0f;
			// Either of these flips the Y axis, but if they're both true they cancel out
			if((gpuBackendConventions.UvYAxis == GpuBackendConventions::Axis::Up) ^ (gpuBackendConventions.NdcYAxis == GpuBackendConventions::Axis::Down))
				flipY = -flipY;

			Matrix4 shadowMapTfrm(
				shadowMapArea.Width * 0.5f, 0, 0, shadowMapArea.X + 0.5f * shadowMapArea.Width,
				0, flipY * shadowMapArea.Height * 0.5f, 0, shadowMapArea.Y + 0.5f * shadowMapArea.Height,
				0, 0, depthScale, depthOffset,
				0, 0, 0, 1);

			return shadowMapTfrm * mixedToShadow;
		}

		ShadowRendering::ProjectedShadowRenderingBatchInformation ShadowRendering::PrepareParametersForRenderShadowProjection(GpuDevice& gpuDevice, const RendererView& view, const RendererLight& rendererLight, GBufferTextures gbuffer) const
		{
			const u32 shadowQuality = view.GetRenderSettings().ShadowSettings.ShadowFilteringQuality;

			const Light* light = rendererLight.Internal;
			const u32 lightRendererId = light->GetRendererId();

			const auto& viewProperties = view.GetProperties();
			const GpuBufferSuballocation& perViewBuffer = view.GetPerViewBuffer();

			const GpuDeviceCapabilities& deviceCapabilities = gpuDevice.GetCapabilities();
			// TODO - Calculate and set a scissor rectangle for the light

			ProjectedShadowRenderingBatchInformation batchRenderingInfo;
			batchRenderingInfo.MSAA = viewProperties.Target.NumSamples > 1;

			TInlineArray<const ShadowInfo*, 6> shadowInfos;

			const u32 viewIndex = view.GetViewIdx();
			// Prepare parameters
			if(light->GetType() == LightType::Radial)
			{
				const LocalLightShadows& shadows = mRadialLightShadows[lightRendererId];
				const Transform& lightTransform = light->GetWorldTransform();

				// Check if viewer is inside the light bounds
				//// Expand the light bounds slightly to handle the case when the near plane is intersecting the light volume
				float lightRadius = light->GetAttenuationRadius() + viewProperties.NearPlane * 3.0f;
				const bool viewerInsideVolume = (lightTransform.GetPosition() - viewProperties.ViewOrigin).Length() < lightRadius;

				for(u32 localShadowIndex = 0; localShadowIndex < shadows.ShadowCount; ++localShadowIndex)
				{
					const u32 shadowIndex = shadows.StartIndex + localShadowIndex;
					const ShadowInfo& shadowInfo = mShadowInfos[shadowIndex];

					if(shadowInfo.FadePerView[viewIndex] < 0.005f)
						continue;

					shadowInfos.Add(&shadowInfo);
				}

				SPtr<GpuBuffer> shadowOmniParamBuffer = gShadowProjectOmniUniformDefinition.CreateBuffer((u32)shadowInfos.Size());
				SPtr<GpuBuffer> shadowProjectVertBuffer = gShadowProjectVertUniformDefinition.CreateBuffer((u32)shadowInfos.Size());

				batchRenderingInfo.UniformBufferSuballocationSize = shadowOmniParamBuffer->GetSuballocationSize();
				batchRenderingInfo.VertexUniformBufferSuballocationSize = shadowProjectVertBuffer->GetSuballocationSize();

				GpuBufferMappedScope omnidirectionalShadowUniforms = shadowOmniParamBuffer->Map(GpuMapOption::Write);
				GpuBufferMappedScope shadowProjectVertexUniforms =shadowProjectVertBuffer->Map(GpuMapOption::Write); 

				for(u32 visibleShadowIndex = 0; visibleShadowIndex < (u32)shadowInfos.Size(); ++visibleShadowIndex)
				{
					const ShadowInfo& shadowInfo = *shadowInfos[visibleShadowIndex];

					for(u32 faceIndex = 0; faceIndex < 6; faceIndex++)
						gShadowProjectOmniUniformDefinition.gFaceVPMatrices.Set(omnidirectionalShadowUniforms, shadowInfo.ShadowVpTransforms[faceIndex], faceIndex, visibleShadowIndex);

					gShadowProjectOmniUniformDefinition.gDepthBias.Set(omnidirectionalShadowUniforms, shadowInfo.DepthBias, 0, visibleShadowIndex);
					gShadowProjectOmniUniformDefinition.gFadePercent.Set(omnidirectionalShadowUniforms, shadowInfo.FadePerView[viewIndex], 0, visibleShadowIndex);
					gShadowProjectOmniUniformDefinition.gInvResolution.Set(omnidirectionalShadowUniforms, 1.0f / shadowInfo.Area.Width, 0, visibleShadowIndex);

					Vector4 lightPosAndRadius(lightTransform.GetPosition(), light->GetAttenuationRadius());
					gShadowProjectOmniUniformDefinition.gLightPosAndRadius.Set(omnidirectionalShadowUniforms, lightPosAndRadius, 0, visibleShadowIndex);

					Vector4 lightPosAndScale(lightTransform.GetPosition(), light->GetAttenuationRadius());
					gShadowProjectVertUniformDefinition.gPositionAndScale.Set(shadowProjectVertexUniforms, lightPosAndScale, 0, visibleShadowIndex);

					// Reduce shadow quality based on shadow map resolution for spot lights
					u32 effectiveShadowQuality = GetShadowQuality(shadowQuality, shadowInfo.Area.Width, 2);

					SPtr<Texture> shadowMap = mShadowCubemaps[shadowInfo.TextureIdx].GetTexture();

					ShadowProjectOmniMaterial* const shadowProjectOmniMaterial = ShadowProjectOmniMaterial::GetVariation(effectiveShadowQuality, viewerInsideVolume, viewProperties.Target.NumSamples > 1);

					const SPtr<GpuParameterSet>& gpuParameters = shadowProjectOmniMaterial->CreateGpuParameterSet();

					ProjectedShadowRenderingInformation shadowRenderingInfo;
					shadowRenderingInfo.ShadowInfo = &shadowInfo;
					shadowRenderingInfo.ShadowQuality = effectiveShadowQuality;
					shadowRenderingInfo.IsViewerInsideLightVolume = viewerInsideVolume;
					shadowRenderingInfo.PrimaryGpuParameters = gpuParameters;

					ShadowProjectionParameterBinding::BindOmnidirectionalProjectionParameters(gpuParameters, gpuDevice, *light, shadowMap, shadowOmniParamBuffer, perViewBuffer, shadowProjectVertBuffer, gbuffer);

					const SPtr<GpuPipelineParameterSetLayout>& pipelineParameterSetLayout = gpuParameters->GetLayout();
					shadowRenderingInfo.PrimaryUniformBufferDynamicIndex = pipelineParameterSetLayout->GetDynamicOffsetIndex("Params");
					shadowRenderingInfo.PrimaryVertexUniformBufferDynamicIndex = pipelineParameterSetLayout->GetDynamicOffsetIndex("VertParams");

					batchRenderingInfo.Shadows.Add(shadowRenderingInfo);
				}
			}
			else // Directional & spot
			{
				bool isCSM = light->GetType() == LightType::Directional;
				if(!isCSM)
				{
					const LocalLightShadows& shadows = mSpotLightShadows[lightRendererId];
					for(u32 localShadowIndex = 0; localShadowIndex < shadows.ShadowCount; ++localShadowIndex)
					{
						const u32 shadowIndex = shadows.StartIndex + localShadowIndex;
						const ShadowInfo& shadowInfo = mShadowInfos[shadowIndex];

						if(shadowInfo.FadePerView[viewIndex] < 0.005f)
							continue;

						shadowInfos.Add(&shadowInfo);
					}
				}
				else // Directional
				{
					const LocalLightShadows& shadows = mDirectionalLightShadows[lightRendererId].ViewShadows[viewIndex];
					if(shadows.ShadowCount > 0)
					{
						u32 shadowMapIndex = shadows.StartIndex;
						const ShadowCascadedMap& cascadedMap = mCascadedShadowMaps[shadowMapIndex];

						// Render cascades in far to near order.
						// Note: If rendering other non-cascade maps they should be rendered after cascades.
						for(i32 cascadeIndex = cascadedMap.GetNumCascades() - 1; cascadeIndex >= 0; cascadeIndex--)
							shadowInfos.Add(&cascadedMap.GetShadowInfo(cascadeIndex));
					}
				}

				if(shadowInfos.Empty())
					return batchRenderingInfo;

				const u32 shadowCount = (u32)shadowInfos.size();
				SPtr<GpuBuffer> shadowParamBuffer = gShadowProjectUniformDefinition.CreateBuffer(shadowCount);
				SPtr<GpuBuffer> shadowProjectVertBuffer = gShadowProjectVertUniformDefinition.CreateBuffer(shadowCount);

				batchRenderingInfo.UniformBufferSuballocationSize = shadowParamBuffer->GetSuballocationSize();
				batchRenderingInfo.VertexUniformBufferSuballocationSize = shadowProjectVertBuffer->GetSuballocationSize();

				GpuBufferMappedScope shadowUniforms = shadowParamBuffer->Map(GpuMapOption::Write);
				GpuBufferMappedScope shadowProjectVertexUniforms =shadowProjectVertBuffer->Map(GpuMapOption::Write); 

				for(u32 shadowIndex = 0; shadowIndex < (u32)shadowInfos.size(); ++shadowIndex)
				{
					const ShadowInfo* shadowInfo = shadowInfos[shadowIndex];
					float depthScale, depthOffset;

					// Depth range scale is already baked into the ortho projection matrix, so avoid doing it here
					if(isCSM)
					{
						// Need to map from API-specific clip space depth to [0, 1] range
						depthScale = 1.0f / (deviceCapabilities.MaxDepth - deviceCapabilities.MinDepth);
						depthOffset = -deviceCapabilities.MinDepth * depthScale;
					}
					else
					{
						depthScale = 1.0f / shadowInfo->DepthRange;
						depthOffset = 0.0f;
					}

					SPtr<Texture> shadowMap;
					u32 shadowMapFace = 0;
					if(!isCSM)
						shadowMap = mAtlasShadowMaps[shadowInfo->TextureIdx].GetTexture();
					else
					{
						shadowMap = mCascadedShadowMaps[shadowInfo->TextureIdx].GetTexture();
						shadowMapFace = shadowInfo->CascadeIdx;
					}

					const Matrix4& cameraProjection = viewProperties.ProjTransform;
					const Matrix4& cameraInverseViewProjection = viewProperties.ViewProjTransform.Inverse();

					Matrix4 mixedToShadowUV = CreateMixedToShadowUvMatrix(cameraProjection, cameraInverseViewProjection, shadowInfo->NormArea, depthScale, depthOffset, shadowInfo->ShadowVpTransform);

					auto shadowMapProps = shadowMap->GetProperties();

					Vector2 shadowMapSize((float)shadowMapProps.Width, (float)shadowMapProps.Height);
					float transitionScale = GetFadeTransition(*light, shadowInfo->SubjectBounds.Radius, shadowInfo->DepthRange, shadowInfo->Area.Width);

					gShadowProjectUniformDefinition.gFadePlaneDepth.Set(shadowUniforms, shadowInfo->DepthFade, 0, shadowIndex);
					gShadowProjectUniformDefinition.gMixedToShadowSpace.Set(shadowUniforms, mixedToShadowUV, 0, shadowIndex);
					gShadowProjectUniformDefinition.gShadowMapSize.Set(shadowUniforms, shadowMapSize, 0, shadowIndex);
					gShadowProjectUniformDefinition.gShadowMapSizeInv.Set(shadowUniforms, 1.0f / shadowMapSize, 0, shadowIndex);
					gShadowProjectUniformDefinition.gSoftTransitionScale.Set(shadowUniforms, transitionScale, 0, shadowIndex);

					if(isCSM)
						gShadowProjectUniformDefinition.gFadePercent.Set(shadowUniforms, 1.0f, 0, shadowIndex);
					else
						gShadowProjectUniformDefinition.gFadePercent.Set(shadowUniforms, shadowInfo->FadePerView[viewIndex], 0, shadowIndex);

					if(shadowInfo->FadeRange == 0.0f)
						gShadowProjectUniformDefinition.gInvFadePlaneRange.Set(shadowUniforms, 0.0f, 0, shadowIndex);
					else
						gShadowProjectUniformDefinition.gInvFadePlaneRange.Set(shadowUniforms, 1.0f / shadowInfo->FadeRange, 0, shadowIndex);

					gShadowProjectVertUniformDefinition.gPositionAndScale.Set(shadowProjectVertexUniforms, Vector4(0.0f, 0.0f, 0.0f, 1.0f), 0, shadowIndex);

					ProjectedShadowRenderingInformation shadowRenderingInfo;
					shadowRenderingInfo.ShadowInfo = shadowInfo;

					// Generate a stencil buffer to avoid evaluating pixels without any receiver geometry in the shadow area
					u32 effectiveShadowQuality = shadowQuality;
					if(!isCSM)
					{
						ConvexVolume shadowFrustum;
						const std::array<Vector3, 8> frustumVertices = GetFrustum(shadowInfo->ShadowVpTransform.Inverse(), shadowFrustum);

						// Check if viewer is inside the frustum. Frustum is slightly expanded so that if the near plane is
						// intersecting the shadow frustum, it is counted as inside. This needs to be conservative as the code
						// for handling viewer outside the frustum will not properly render intersections with the near plane.
						bool viewerInsideFrustum = shadowFrustum.Contains(viewProperties.ViewOrigin, viewProperties.NearPlane * 3.0f);

						ShadowProjectStencilMaterial* const stencilMaterial = ShadowProjectStencilMaterial::GetVariation(false, viewerInsideFrustum);

						// Bind GPU parameters for stencil material
						SPtr<GpuParameterSet> stencilGpuParameters = stencilMaterial->CreateGpuParameterSet();
						ShadowProjectionParameterBinding::BindStencilProjectionParameters(stencilGpuParameters, perViewBuffer, shadowProjectVertBuffer);

						shadowRenderingInfo.StencilGpuParameters = stencilGpuParameters;
						shadowRenderingInfo.IsViewerInsideLightVolume = viewerInsideFrustum;

						// Reduce shadow quality based on shadow map resolution for spot lights
						effectiveShadowQuality = GetShadowQuality(shadowQuality, shadowInfo->Area.Width, 2);
					}
					else
					{
						ShadowProjectStencilMaterial* const stencilMaterial = ShadowProjectStencilMaterial::GetVariation(true, true);

						// Bind GPU parameters for stencil material
						SPtr<GpuParameterSet> stencilGpuParameters = stencilMaterial->CreateGpuParameterSet();
						ShadowProjectionParameterBinding::BindStencilProjectionParameters(stencilGpuParameters, perViewBuffer, shadowProjectVertBuffer);

						shadowRenderingInfo.StencilGpuParameters = stencilGpuParameters;
						shadowRenderingInfo.IsViewerInsideLightVolume = true;
					}

					shadowRenderingInfo.ShadowQuality = effectiveShadowQuality;

					const SPtr<GpuPipelineParameterSetLayout>& stencilPipelineParameterSetLayout = shadowRenderingInfo.StencilGpuParameters->GetLayout();
					shadowRenderingInfo.StencilVertexUniformBufferDynamicIndex = stencilPipelineParameterSetLayout->GetDynamicOffsetIndex("VertParams");

					gShadowProjectUniformDefinition.gFace.Set(shadowUniforms, (float)shadowMapFace, 0, shadowIndex);

					ShadowProjectMaterial* const primaryMaterial = ShadowProjectMaterial::GetVariation(effectiveShadowQuality, isCSM, viewProperties.Target.NumSamples > 1);

					// Bind GPU parameters explicitly
					SPtr<GpuParameterSet> gpuParameters = primaryMaterial->CreateGpuParameterSet();
					ShadowProjectionParameterBinding::BindProjectionParameters(gpuParameters, gpuDevice, shadowMap, shadowParamBuffer, perViewBuffer, shadowProjectVertBuffer, gbuffer);

					shadowRenderingInfo.PrimaryGpuParameters = gpuParameters;

					const SPtr<GpuPipelineParameterSetLayout>& pipelineParameterSetLayout = gpuParameters->GetLayout();
					shadowRenderingInfo.PrimaryUniformBufferDynamicIndex = pipelineParameterSetLayout->GetDynamicOffsetIndex("Params");
					shadowRenderingInfo.PrimaryVertexUniformBufferDynamicIndex = pipelineParameterSetLayout->GetDynamicOffsetIndex("VertParams");

					batchRenderingInfo.Shadows.Add(shadowRenderingInfo);
				}
			}

			return batchRenderingInfo;
		}

		void ShadowRendering::RenderShadowProjectionBatch(GpuCommandBuffer& commandBuffer, const RendererView& view, const RendererLight& rendererLight, const ProjectedShadowRenderingBatchInformation& batch) const
		{
			const Light* light = rendererLight.Internal;
			const auto& viewProperties = view.GetProperties();

			// TODO - Calculate and set a scissor rectangle for the light

			ProfileGPUBlock sampleBlock(commandBuffer, "Render shadow occlusion");

			if(light->GetType() == LightType::Radial)
			{
				for(u32 visibleShadowIndex = 0; visibleShadowIndex < (u32)batch.Shadows.Size(); ++visibleShadowIndex)
				{
					const ProjectedShadowRenderingInformation& shadowRenderingInformation = batch.Shadows[visibleShadowIndex];

					ShadowProjectOmniMaterial* const shadowProjectOmniMaterial = ShadowProjectOmniMaterial::GetVariation(shadowRenderingInformation.ShadowQuality, shadowRenderingInformation.IsViewerInsideLightVolume, batch.MSAA);
					shadowProjectOmniMaterial->Bind(commandBuffer);

					// Bind parameters to pipeline
					commandBuffer.SetGpuParameterSet(shadowRenderingInformation.PrimaryGpuParameters);

					// Calculate sub-allocation offsets
					const u32 shadowSuballocationOffset = visibleShadowIndex * batch.UniformBufferSuballocationSize;
					const u32 vertSuballocationOffset = visibleShadowIndex * batch.VertexUniformBufferSuballocationSize;

					const u32 setIndex = shadowRenderingInformation.PrimaryGpuParameters->GetSet();
					commandBuffer.SetDynamicBufferOffset(setIndex, shadowRenderingInformation.PrimaryUniformBufferDynamicIndex, shadowSuballocationOffset);
					commandBuffer.SetDynamicBufferOffset(setIndex, shadowRenderingInformation.PrimaryVertexUniformBufferDynamicIndex, vertSuballocationOffset);

					GetRendererUtility().Draw(commandBuffer, GetRendererUtility().GetSphereStencil());
				}
			}
			else // Directional & spot
			{
				bool isCSM = light->GetType() == LightType::Directional;

				for(u32 visibleShadowIndex = 0; visibleShadowIndex < (u32)batch.Shadows.Size(); ++visibleShadowIndex)
				{
					const ProjectedShadowRenderingInformation& shadowRenderingInformation = batch.Shadows[visibleShadowIndex];
					const ShadowInfo& shadowInfo = *shadowRenderingInformation.ShadowInfo;

					// Generate a stencil buffer to avoid evaluating pixels without any receiver geometry in the shadow area
					std::array<Vector3, 8> frustumVertices;
					if(!isCSM)
					{
						ConvexVolume shadowFrustum;
						frustumVertices = GetFrustum(shadowInfo.ShadowVpTransform.Inverse(), shadowFrustum);

						ShadowProjectStencilMaterial* stencilMat = ShadowProjectStencilMaterial::GetVariation(false, shadowRenderingInformation.IsViewerInsideLightVolume);
						stencilMat->Bind(commandBuffer);

						// Bind parameters to pipeline
						commandBuffer.SetGpuParameterSet(shadowRenderingInformation.StencilGpuParameters);

						const u32 vertSuballocationOffset = visibleShadowIndex * batch.VertexUniformBufferSuballocationSize;
						const u32 setIndex = shadowRenderingInformation.StencilGpuParameters->GetSet();
						commandBuffer.SetDynamicBufferOffset(setIndex, shadowRenderingInformation.StencilVertexUniformBufferDynamicIndex, vertSuballocationOffset);

						DrawFrustum(commandBuffer, frustumVertices);
					}
					else
					{
						// Need to generate near and far planes to clip the geometry within the current CSM slice.
						// Note: If the render API supports built-in depth bound tests that could be used instead.

						Vector3 near = viewProperties.ProjTransform.Multiply(Vector3(0, 0, -shadowInfo.DepthNear));
						Vector3 far = viewProperties.ProjTransform.Multiply(Vector3(0, 0, -shadowInfo.DepthFar));

						ShadowProjectStencilMaterial* stencilMaterial = ShadowProjectStencilMaterial::GetVariation(true, true);
						stencilMaterial->Bind(commandBuffer);

						commandBuffer.SetGpuParameterSet(shadowRenderingInformation.StencilGpuParameters);

						const u32 vertSuballocationOffset = visibleShadowIndex * batch.VertexUniformBufferSuballocationSize;
						const u32 setIndex = shadowRenderingInformation.StencilGpuParameters->GetSet();
						commandBuffer.SetDynamicBufferOffset(setIndex, shadowRenderingInformation.StencilVertexUniformBufferDynamicIndex, vertSuballocationOffset);

						DrawNearFarPlanes(commandBuffer, near.Z, far.Z, shadowInfo.CascadeIdx != 0);
					}

					ShadowProjectMaterial* mat = ShadowProjectMaterial::GetVariation(shadowRenderingInformation.ShadowQuality, isCSM, batch.MSAA);
					mat->Bind(commandBuffer);

					commandBuffer.SetGpuParameterSet(shadowRenderingInformation.PrimaryGpuParameters);

					// Calculate sub-allocation offsets
					const u32 shadowSuballocationOffset = visibleShadowIndex * batch.UniformBufferSuballocationSize;
					const u32 vertSuballocationOffset = visibleShadowIndex * batch.VertexUniformBufferSuballocationSize;
					const u32 setIndex = shadowRenderingInformation.PrimaryGpuParameters->GetSet();

					commandBuffer.SetDynamicBufferOffset(setIndex, shadowRenderingInformation.PrimaryUniformBufferDynamicIndex, shadowSuballocationOffset);
					commandBuffer.SetDynamicBufferOffset(setIndex, shadowRenderingInformation.PrimaryVertexUniformBufferDynamicIndex, vertSuballocationOffset);

					if(!isCSM)
						DrawFrustum(commandBuffer, frustumVertices);
					else
						GetRendererUtility().DrawScreenQuad(commandBuffer);
				}
			}
		}

		void ShadowRendering::RenderCascadedShadowMaps(GpuCommandBuffer& commandBuffer, const RendererView& view, u32 lightIdx, RenderBeastScene& scene, const FrameInfo& frameInfo)
		{
			u32 viewIdx = view.GetViewIdx();
			LocalLightShadows& lightShadows = mDirectionalLightShadows[lightIdx].ViewShadows[viewIdx];

			if(!view.GetRenderSettings().EnableShadows)
			{
				lightShadows.StartIndex = -1;
				lightShadows.ShadowCount = 0;
				return;
			}

			// Note: Currently I'm using spherical bounds for the cascaded frustum which might result in non-optimal usage
			// of the shadow map. A different approach would be to generate a bounding box and then both adjust the aspect
			// ratio (and therefore dimensions) of the shadow map, as well as rotate the camera so the visible area best fits
			// in the map. It remains to be seen if this is viable.
			//  - Note2: Actually both of these will likely have serious negative impact on shadow stability.
			const SceneInfo& sceneInfo = scene.GetSceneInfo();

			const RendererLight& rendererLight = sceneInfo.DirectionalLights[lightIdx];
			Light* light = rendererLight.Internal;

			const Transform& tfrm = light->GetWorldTransform();
			Vector3 lightDir = -tfrm.GetRotation().ZAxis();
			GpuBufferMappedScope shadowUniforms = gShadowUniformDefinition.AllocateTransient().Map();

			ShadowInfo shadowInfo;
			shadowInfo.LightId = lightIdx;
			shadowInfo.TextureIdx = -1;

			u32 mapSize = std::min(mShadowMapSize, kMaxAtlasSize);
			shadowInfo.Area = Area2I(0, 0, mapSize, mapSize);
			shadowInfo.UpdateNormArea(mapSize);

			u32 numCascades = view.GetRenderSettings().ShadowSettings.NumCascades;
			for(u32 i = 0; i < (u32)mCascadedShadowMaps.size(); i++)
			{
				ShadowCascadedMap& shadowMap = mCascadedShadowMaps[i];

				if(!shadowMap.IsUsed() && shadowMap.GetSize() == mapSize && shadowMap.GetNumCascades() == numCascades)
				{
					shadowInfo.TextureIdx = i;
					shadowMap.MarkAsUsed();

					break;
				}
			}

			if(shadowInfo.TextureIdx == (u32)-1)
			{
				shadowInfo.TextureIdx = (u32)mCascadedShadowMaps.size();
				mCascadedShadowMaps.push_back(ShadowCascadedMap(mapSize, numCascades));

				ShadowCascadedMap& shadowMap = mCascadedShadowMaps.back();
				shadowMap.MarkAsUsed();
			}

			ShadowCascadedMap& shadowMap = mCascadedShadowMaps[shadowInfo.TextureIdx];

			Quaternion lightRotation(BsIdentity);
			lightRotation.LookRotation(lightDir, Vector3::kUnitY);

			ProfileGPUBlock profileSample(commandBuffer, "Project directional light shadow");

			for(u32 i = 0; i < numCascades; ++i)
			{
				Sphere frustumBounds;
				ConvexVolume cascadeCullVolume = GetCsmSplitFrustum(view, lightDir, i, numCascades, frustumBounds);

				// Make sure the size of the projected area is in multiples of shadow map pixel size (for stability)
				float worldUnitsPerTexel = frustumBounds.Radius * 2.0f / shadowMap.GetSize();

				float orthoSize = floor(frustumBounds.Radius * 2.0f / worldUnitsPerTexel) * worldUnitsPerTexel * 0.5f;
				worldUnitsPerTexel = orthoSize * 2.0f / shadowMap.GetSize();

				// Snap caster origin to the shadow map pixel grid, to ensure shadow map stability
				Vector3 casterOrigin = frustumBounds.Center;
				Matrix4 shadowView = Matrix4::View(Vector3::kZero, lightRotation);
				Vector3 shadowSpaceOrigin = shadowView.MultiplyAffine(casterOrigin);

				Vector2 snapOffset(fmod(shadowSpaceOrigin.X, worldUnitsPerTexel), fmod(shadowSpaceOrigin.Y, worldUnitsPerTexel));
				shadowSpaceOrigin.X -= snapOffset.X;
				shadowSpaceOrigin.Y -= snapOffset.Y;

				Matrix4 shadowViewInv = shadowView.InverseAffine();
				casterOrigin = shadowViewInv.MultiplyAffine(shadowSpaceOrigin);

				// Move the light so it is centered at the subject frustum, with depth range covering the frustum bounds
				shadowInfo.DepthRange = frustumBounds.Radius * 2.0f;

				Vector3 offsetLightPos = casterOrigin - lightDir * frustumBounds.Radius;
				Matrix4 offsetViewMat = Matrix4::View(offsetLightPos, lightRotation);

				Matrix4 proj = Matrix4::ProjectionOrthographic(-orthoSize, orthoSize, orthoSize, -orthoSize, 0.0f, shadowInfo.DepthRange);

				GpuDevice& gpuDevice = commandBuffer.GetGpuDevice();
				gpuDevice.ConvertProjectionMatrix(proj, proj);

				shadowInfo.CascadeIdx = i;
				shadowInfo.ShadowVpTransform = proj * offsetViewMat;

				// Determine split range
				float splitNear = GetCsmSplitDistance(view, i, numCascades);
				float splitFar = GetCsmSplitDistance(view, i + 1, numCascades);

				shadowInfo.DepthNear = splitNear;
				shadowInfo.DepthFade = splitFar;
				shadowInfo.SubjectBounds = frustumBounds;

				if((u32)(i + 1) < numCascades)
					shadowInfo.FadeRange = kCascadeFractionFade * (shadowInfo.DepthFade - shadowInfo.DepthNear);
				else
					shadowInfo.FadeRange = 0.0f;

				shadowInfo.DepthFar = shadowInfo.DepthFade + shadowInfo.FadeRange;
				shadowInfo.DepthBias = GetDepthBias(*light, frustumBounds.Radius, shadowInfo.DepthRange, mapSize);

				gShadowUniformDefinition.gDepthBias.Set(shadowUniforms, shadowInfo.DepthBias);
				gShadowUniformDefinition.gInvDepthRange.Set(shadowUniforms, 1.0f / shadowInfo.DepthRange);
				gShadowUniformDefinition.gMatViewProj.Set(shadowUniforms, shadowInfo.ShadowVpTransform);
				gShadowUniformDefinition.gNDCZToDeviceZ.Set(shadowUniforms, RendererView::GetNdczToDeviceZ());

				// Render all renderables into the shadow map
				ShadowRenderQueueDirOptions dirOptions(
					*this,
					cascadeCullVolume,
					shadowUniforms);

				ShadowRenderQueue::Execute(commandBuffer, scene, frameInfo, shadowMap.GetTarget(i), dirOptions);

				shadowMap.SetShadowInfo(i, shadowInfo);
			}

			lightShadows.StartIndex = shadowInfo.TextureIdx;
			lightShadows.ShadowCount = 1;
		}

		void ShadowRendering::RenderSpotShadowMap(GpuCommandBuffer& commandBuffer, const RendererLight& rendererLight, const ShadowMapOptions& options, RenderBeastScene& scene, const FrameInfo& frameInfo)
		{
			Light* light = rendererLight.Internal;

			GpuBufferMappedScope shadowUniforms = gShadowUniformDefinition.AllocateTransient().Map();

			ShadowInfo mapInfo;
			mapInfo.FadePerView = options.FadePercents;
			mapInfo.LightId = options.LightIdx;
			mapInfo.CascadeIdx = -1;

			bool foundSpace = false;
			for(u32 i = 0; i < (u32)mAtlasShadowMaps.size(); i++)
			{
				ShadowMapAtlas& atlas = mAtlasShadowMaps[i];

				if(atlas.AddMap(options.MapSize, mapInfo.Area, kShadowMapBorder))
				{
					mapInfo.TextureIdx = i;

					foundSpace = true;
					break;
				}
			}

			if(!foundSpace)
			{
				mapInfo.TextureIdx = (u32)mAtlasShadowMaps.size();
				mAtlasShadowMaps.push_back(ShadowMapAtlas(kMaxAtlasSize));

				ShadowMapAtlas& atlas = mAtlasShadowMaps.back();
				atlas.AddMap(options.MapSize, mapInfo.Area, kShadowMapBorder);
			}

			mapInfo.UpdateNormArea(kMaxAtlasSize);
			ShadowMapAtlas& atlas = mAtlasShadowMaps[mapInfo.TextureIdx];

			ProfileGPUBlock profileSample(commandBuffer, "Project spot light shadows");

			mapInfo.DepthNear = 0.05f;
			mapInfo.DepthFar = light->GetAttenuationRadius();
			mapInfo.DepthFade = mapInfo.DepthFar;
			mapInfo.FadeRange = 0.0f;
			mapInfo.DepthRange = mapInfo.DepthFar - mapInfo.DepthNear;
			mapInfo.DepthBias = GetDepthBias(*light, light->GetBounds().Radius, mapInfo.DepthRange, options.MapSize);
			mapInfo.SubjectBounds = light->GetBounds();

			Quaternion lightRotation = light->GetWorldTransform().GetRotation();

			Matrix4 view = Matrix4::View(rendererLight.GetShiftedLightPosition(), lightRotation);
			Matrix4 proj = Matrix4::ProjectionPerspective(light->GetSpotAngle(), 1.0f, 0.05f, light->GetAttenuationRadius());

			ConvexVolume localFrustum = ConvexVolume(proj);

			GpuDevice& gpuDevice = commandBuffer.GetGpuDevice();
			gpuDevice.ConvertProjectionMatrix(proj, proj);

			mapInfo.ShadowVpTransform = proj * view;

			gShadowUniformDefinition.gDepthBias.Set(shadowUniforms, mapInfo.DepthBias);
			gShadowUniformDefinition.gInvDepthRange.Set(shadowUniforms, 1.0f / mapInfo.DepthRange);
			gShadowUniformDefinition.gMatViewProj.Set(shadowUniforms, mapInfo.ShadowVpTransform);
			gShadowUniformDefinition.gNDCZToDeviceZ.Set(shadowUniforms, RendererView::GetNdczToDeviceZ());

			const Vector<Plane>& frustumPlanes = localFrustum.GetPlanes();
			Matrix4 worldMatrix = view.InverseAffine();

			Vector<Plane> worldPlanes(frustumPlanes.size());
			u32 j = 0;
			for(auto& plane : frustumPlanes)
			{
				worldPlanes[j] = worldMatrix.MultiplyAffine(plane);
				j++;
			}

			ConvexVolume worldFrustum(worldPlanes);

			// Render all renderables into the shadow map
			ShadowRenderQueueSpotOptions spotOptions(
				*this,
				mapInfo.NormArea,
				worldFrustum,
				shadowUniforms);

			ShadowRenderQueue::Execute(commandBuffer, scene, frameInfo, atlas.GetTarget(), spotOptions);

			// Restore viewport
			commandBuffer.SetViewport(Area2(0.0f, 0.0f, 1.0f, 1.0f));

			LocalLightShadows& lightShadows = mSpotLightShadows[options.LightIdx];

			mShadowInfos[lightShadows.StartIndex + lightShadows.ShadowCount] = mapInfo;
			lightShadows.ShadowCount++;
		}

		void ShadowRendering::RenderRadialShadowMap(GpuCommandBuffer& commandBuffer, const RendererLight& rendererLight, const ShadowMapOptions& options, RenderBeastScene& scene, const FrameInfo& frameInfo)
		{
			Light* const light = rendererLight.Internal;

			GpuBufferMappedScope shadowUniforms = gShadowUniformDefinition.AllocateTransient().Map();

			ShadowInfo shadowInfo;
			shadowInfo.LightId = options.LightIdx;
			shadowInfo.TextureIdx = -1;
			shadowInfo.FadePerView = options.FadePercents;
			shadowInfo.CascadeIdx = -1;
			shadowInfo.Area = Area2I(0, 0, options.MapSize, options.MapSize);
			shadowInfo.UpdateNormArea(options.MapSize);

			for(u32 i = 0; i < (u32)mShadowCubemaps.size(); i++)
			{
				ShadowCubemap& cubemap = mShadowCubemaps[i];

				if(!cubemap.IsUsed() && cubemap.GetSize() == options.MapSize)
				{
					shadowInfo.TextureIdx = i;
					cubemap.MarkAsUsed();

					break;
				}
			}

			if(shadowInfo.TextureIdx == (u32)-1)
			{
				shadowInfo.TextureIdx = (u32)mShadowCubemaps.size();
				mShadowCubemaps.push_back(ShadowCubemap(options.MapSize));

				ShadowCubemap& cubemap = mShadowCubemaps.back();
				cubemap.MarkAsUsed();
			}

			ShadowCubemap& cubemap = mShadowCubemaps[shadowInfo.TextureIdx];

			shadowInfo.DepthNear = 0.05f;
			shadowInfo.DepthFar = light->GetAttenuationRadius();
			shadowInfo.DepthFade = shadowInfo.DepthFar;
			shadowInfo.FadeRange = 0.0f;
			shadowInfo.DepthRange = shadowInfo.DepthFar - shadowInfo.DepthNear;
			shadowInfo.DepthBias = GetDepthBias(*light, light->GetBounds().Radius, shadowInfo.DepthRange, options.MapSize);
			shadowInfo.SubjectBounds = light->GetBounds();

			// Note: Projecting on positive Z axis, because cubemaps use a left-handed coordinate system
			Matrix4 proj = Matrix4::ProjectionPerspective(Degree(90.0f), 1.0f, 0.05f, light->GetAttenuationRadius(), true);
			ConvexVolume localFrustum(proj);

			ProfileGPUBlock profileSample(commandBuffer, "Project radial light shadows");

			const GpuDeviceCapabilities& caps = commandBuffer.GetGpuDevice().GetCapabilities();

			GpuDevice& gpuDevice = commandBuffer.GetGpuDevice();
			gpuDevice.ConvertProjectionMatrix(proj, proj);

			// Render cubemaps upside down if necessary
			Matrix4 adjustedProj = proj;
			if(caps.Conventions.UvYAxis == GpuBackendConventions::Axis::Up)
			{
				// All big APIs use the same cubemap sampling coordinates, as well as the same face order. But APIs that
				// use bottom-up UV coordinates require the cubemap faces to be stored upside down in order to get the same
				// behaviour. APIs that use an upside-down NDC Y axis have the same problem as the rendered image will be
				// upside down, but this is handled by the projection matrix. If both of those are enabled, then the effect
				// cancels out.

				adjustedProj[1][1] = -proj[1][1];
			}

			bool renderAllFacesAtOnce = caps.HasCapability(RSC_RENDER_TARGET_LAYERS);

			GpuBufferSuballocation shadowCubeMatricesBuffer;
			if(renderAllFacesAtOnce)
				shadowCubeMatricesBuffer = gShadowCubeMatricesUniformDefinition.AllocateTransient();

			gShadowUniformDefinition.gDepthBias.Set(shadowUniforms, shadowInfo.DepthBias);
			gShadowUniformDefinition.gInvDepthRange.Set(shadowUniforms, 1.0f / shadowInfo.DepthRange);
			gShadowUniformDefinition.gMatViewProj.Set(shadowUniforms, Matrix4::kIdentity);
			gShadowUniformDefinition.gNDCZToDeviceZ.Set(shadowUniforms, RendererView::GetNdczToDeviceZ());

			ConvexVolume frustums[6];
			Vector<Plane> boundingPlanes;
			for(u32 i = 0; i < 6; i++)
			{
				// Calculate view matrix
				Vector3 forward;
				Vector3 up = Vector3::kUnitY;

				switch(i)
				{
				case CF_PositiveX:
					forward = Vector3::kUnitX;
					break;
				case CF_NegativeX:
					forward = -Vector3::kUnitX;
					break;
				case CF_PositiveY:
					forward = Vector3::kUnitY;
					up = -Vector3::kUnitZ;
					break;
				case CF_NegativeY:
					forward = -Vector3::kUnitY;
					up = Vector3::kUnitZ;
					break;
				case CF_PositiveZ:
					forward = Vector3::kUnitZ;
					break;
				case CF_NegativeZ:
					forward = -Vector3::kUnitZ;
					break;
				}

				Vector3 right = Vector3::Cross(up, forward);
				Matrix3 viewRotationMat = Matrix3(right, up, forward);

				Vector3 lightPos = light->GetWorldTransform().GetPosition();
				Matrix4 viewOffsetMat = Matrix4::Translation(-lightPos);

				Matrix4 view = Matrix4(viewRotationMat.Transpose()) * viewOffsetMat;
				shadowInfo.ShadowVpTransforms[i] = proj * view;

				Matrix4 shadowViewProj = adjustedProj * view;

				// Calculate world frustum for culling
				const Vector<Plane>& frustumPlanes = localFrustum.GetPlanes();

				Matrix4 worldMatrix = Matrix4::Translation(lightPos) * Matrix4(viewRotationMat);

				Vector<Plane> worldPlanes(frustumPlanes.size());
				u32 j = 0;
				for(auto& plane : frustumPlanes)
				{
					worldPlanes[j] = worldMatrix.MultiplyAffine(plane);
					j++;
				}

				ConvexVolume frustum(worldPlanes);

				if(renderAllFacesAtOnce)
				{
					frustums[i] = frustum;

					// Register far plane of all frustums
					boundingPlanes.push_back(worldPlanes[FRUSTUM_PLANE_FAR]);

					GpuBufferMappedScope shadowCubeMatricesUniforms = shadowCubeMatricesBuffer.Map();
					gShadowCubeMatricesUniformDefinition.gFaceVPMatrices.Set(shadowCubeMatricesUniforms, shadowViewProj, i);
				}
				else
				{
					gShadowUniformDefinition.gMatViewProj.Set(shadowUniforms, shadowViewProj);

					RenderTextureCreateInformation rtDesc;
					rtDesc.DepthStencilSurface.Texture = cubemap.GetTexture();
					rtDesc.DepthStencilSurface.Face = i;
					rtDesc.DepthStencilSurface.FaceCount = 1;

					SPtr<RenderTarget> faceRt = RenderTexture::Create(rtDesc);

					// Render all renderables into the shadow map
					ConvexVolume boundingVolume(boundingPlanes);
					ShadowRenderQueueCubeSingleOptions cubeOptions(
						*this,
						frustum,
						shadowUniforms);

					ShadowRenderQueue::Execute(commandBuffer, scene, frameInfo, faceRt, cubeOptions);
				}
			}

			if(renderAllFacesAtOnce)
			{
				// Render all renderables into the shadow map
				ConvexVolume boundingVolume(boundingPlanes);
				ShadowRenderQueueCubeOptions cubeOptions(
					*this,
					frustums,
					boundingVolume,
					shadowUniforms,
					shadowCubeMatricesBuffer);

				ShadowRenderQueue::Execute(commandBuffer, scene, frameInfo, cubemap.GetTarget(), cubeOptions);
			}

			LocalLightShadows& lightShadows = mRadialLightShadows[options.LightIdx];

			mShadowInfos[lightShadows.StartIndex + lightShadows.ShadowCount] = shadowInfo;
			lightShadows.ShadowCount++;
		}

		void ShadowRendering::CalcShadowMapProperties(const RendererLight& light, const RendererViewGroup& viewGroup, u32 border, u32& size, TInlineArray<float, 6>& fadePercents, float& maxFadePercent) const
		{
			const static float kShadowTexelsPerPixel = 1.0f;

			// Find a view in which the light has the largest radius
			float maxMapSize = 0.0f;
			maxFadePercent = 0.0f;
			for(int i = 0; i < (int)viewGroup.GetViewCount(); ++i)
			{
				const RendererView& view = *viewGroup.GetView(i);
				const RendererViewProperties& viewProps = view.GetProperties();
				const RenderSettings& viewSettings = view.GetRenderSettings();

				if(!viewSettings.EnableShadows)
					fadePercents.Add(0.0f);
				else
				{
					// Approximation for screen space sphere radius: screenSize * 0.5 * cot(fov) * radius / Z, where FOV is the
					// largest one
					//// First get sphere depth
					const Matrix4& viewVP = viewProps.ViewProjTransform;
					float depth = viewVP.Multiply(Vector4(light.Internal->GetWorldTransform().GetPosition(), 1.0f)).W;

					// This is just 1/tan(fov), for both horz. and vert. FOV
					float viewScaleX = viewProps.ProjTransform[0][0];
					float viewScaleY = viewProps.ProjTransform[1][1];

					float screenScaleX = viewScaleX * viewProps.Target.ViewRect.Width * 0.5f;
					float screenScaleY = viewScaleY * viewProps.Target.ViewRect.Height * 0.5f;

					float screenScale = std::max(screenScaleX, screenScaleY);

					//// Calc radius (clamp if too close to avoid massive numbers)
					float radiusNDC = light.Internal->GetBounds().Radius / std::max(depth, 1.0f);

					//// Radius of light bounds in percent of the view surface, multiplied by screen size in pixels
					float radiusScreen = radiusNDC * screenScale;

					float optimalMapSize = kShadowTexelsPerPixel * radiusScreen;
					maxMapSize = std::max(maxMapSize, optimalMapSize);

					// Determine if the shadow should fade out
					float fadePercent = Math::InvLerp(optimalMapSize, (float)kMinShadowMapSize, (float)kShadowMapFadeSize);
					fadePercents.Add(fadePercent);
					maxFadePercent = std::max(maxFadePercent, fadePercent);
				}
			}

			// If light fully (or nearly fully) covers the screen, use full shadow map resolution, otherwise
			// scale it down to smaller power of two, while clamping to minimal allowed resolution
			u32 effectiveMapSize = Bitwise::NextPow2((u32)maxMapSize);
			effectiveMapSize = Math::Clamp(effectiveMapSize, kMinShadowMapSize, mShadowMapSize);

			// Leave room for border
			size = std::max(effectiveMapSize - 2 * border, 1u);
		}

		void ShadowRendering::DrawNearFarPlanes(GpuCommandBuffer& commandBuffer, float near, float far, bool drawNear) const
		{
			const GpuBackendConventions& rapiConventions = commandBuffer.GetGpuDevice().GetCapabilities().Conventions;
			float flipY = (rapiConventions.NdcYAxis == GpuBackendConventions::Axis::Down) ? -1.0f : 1.0f;

			// Update VB with new vertices
			Vector3 vertices[8] = {
				// Near plane
				{ -1.0f, -1.0f * flipY, near },
				{ 1.0f, -1.0f * flipY, near },
				{ 1.0f, 1.0f * flipY, near },
				{ -1.0f, 1.0f * flipY, near },

				// Far plane
				{ -1.0f, -1.0f * flipY, far },
				{ 1.0f, -1.0f * flipY, far },
				{ 1.0f, 1.0f * flipY, far },
				{ -1.0f, 1.0f * flipY, far },
			};

			GpuBufferUtility::Write(mPlaneVB, 0, sizeof(vertices), vertices, GpuBufferWriteFlag::Discard);

			// Draw the mesh
			commandBuffer.SetVertexDescription(mPositionOnlyVertexDescription);
			commandBuffer.SetVertexBuffers(0, &mPlaneVB, 1);
			commandBuffer.SetIndexBuffer(mPlaneIB);
			commandBuffer.SetDrawOperation(DOT_TRIANGLE_LIST);

			commandBuffer.DrawIndexed(0, drawNear ? 12 : 6, 0, drawNear ? 8 : 4);
		}

		void ShadowRendering::DrawFrustum(GpuCommandBuffer& commandBuffer, const std::array<Vector3, 8>& corners) const
		{
			// Update VB with new vertices
			GpuBufferUtility::Write(mFrustumVB, 0, sizeof(Vector3) * 8, corners.data(), GpuBufferWriteFlag::Discard);

			// Draw the mesh
			commandBuffer.SetVertexDescription(mPositionOnlyVertexDescription);
			commandBuffer.SetVertexBuffers(0, &mFrustumVB, 1);
			commandBuffer.SetIndexBuffer(mFrustumIB);
			commandBuffer.SetDrawOperation(DOT_TRIANGLE_LIST);

			commandBuffer.DrawIndexed(0, 36, 0, 8);
		}

		u32 ShadowRendering::GetShadowQuality(u32 requestedQuality, u32 shadowMapResolution, u32 minAllowedQuality)
		{
			static const u32 kTargetResolution = 512;

			// If shadow map resolution is smaller than some target resolution drop the number of PCF samples (shadow quality)
			// so that the penumbra better matches with larger sized shadow maps.
			while(requestedQuality > minAllowedQuality && shadowMapResolution < kTargetResolution)
			{
				shadowMapResolution *= 2;
				requestedQuality = std::max(requestedQuality - 1, 1U);
			}

			return requestedQuality;
		}

		ConvexVolume ShadowRendering::GetCsmSplitFrustum(const RendererView& view, const Vector3& lightDir, u32 cascade, u32 numCascades, Sphere& outBounds)
		{
			// Determine split range
			float splitNear = GetCsmSplitDistance(view, cascade, numCascades);
			float splitFar = GetCsmSplitDistance(view, cascade + 1, numCascades);

			// Increase by fade range, unless last cascade
			if((u32)(cascade + 1) < numCascades)
				splitFar += kCascadeFractionFade * (splitFar - splitNear);

			// Calculate the eight vertices of the split frustum
			auto& viewProps = view.GetProperties();

			const Matrix4& projMat = viewProps.ProjTransform;

			float aspect;
			float nearHalfWidth, nearHalfHeight;
			float farHalfWidth, farHalfHeight;
			if(viewProps.ProjType == PT_PERSPECTIVE)
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

			const Matrix4& viewMat = viewProps.ViewTransform;
			Vector3 cameraRight = Vector3(viewMat[0]);
			Vector3 cameraUp = Vector3(viewMat[1]);

			const Vector3& viewOrigin = viewProps.ViewOrigin;
			const Vector3& viewDir = viewProps.ViewDirection;

			Vector3 frustumVerts[] = {
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
			float distToCenter = Math::Clamp(splitFar - offset, splitNear, splitFar);

			Vector3 center = viewOrigin + viewDir * distToCenter;

			float radius = 0.0f;
			for(auto& entry : frustumVerts)
				radius = std::max(radius, center.SquaredDistance(entry));

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
				if(entry.Normal.Dot(lightDir) < 0.0f)
					lightVolume.push_back(entry);
			}

			//// Determine edge planes by testing adjacent planes with different facing
			////// Pairs of frustum planes that share an edge
			u32 adjacentPlanes[][2] = {
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
			u32 sharedEdges[][2] = {
				{ 3, 0 }, { 1, 2 }, { 0, 1 }, { 2, 3 }, { 4, 7 }, { 6, 5 }, { 5, 4 }, { 7, 6 }, { 4, 0 }, { 5, 1 }, { 6, 2 }, { 7, 3 }
			};

			for(u32 i = 0; i < 12; i++)
			{
				const Plane& planeA = viewPlanes[adjacentPlanes[i][0]];
				const Plane& planeB = viewPlanes[adjacentPlanes[i][1]];

				float dotA = planeA.Normal.Dot(lightDir);
				float dotB = planeB.Normal.Dot(lightDir);

				if((dotA * dotB) < 0.0f)
				{
					const Vector3& vertA = frustumVerts[sharedEdges[i][0]];
					const Vector3& vertB = frustumVerts[sharedEdges[i][1]];
					Vector3 vertC = vertA + lightDir;

					if(dotA < 0.0f)
						lightVolume.push_back(Plane(vertA, vertB, vertC));
					else
						lightVolume.push_back(Plane(vertB, vertA, vertC));
				}
			}

			return ConvexVolume(lightVolume);
		}

		float ShadowRendering::GetCsmSplitDistance(const RendererView& view, u32 index, u32 numCascades)
		{
			auto& shadowSettings = view.GetRenderSettings().ShadowSettings;
			float distributionExponent = shadowSettings.CascadeDistributionExponent;

			// First determine the scale of the split, relative to the entire range
			float scaleModifier = 1.0f;
			float scale = 0.0f;
			float totalScale = 0.0f;

			//// Split 0 corresponds to near plane
			if(index > 0)
			{
				for(u32 i = 0; i < numCascades; i++)
				{
					if(i < index)
						scale += scaleModifier;

					totalScale += scaleModifier;
					scaleModifier *= distributionExponent;
				}

				scale = scale / totalScale;
			}

			// Calculate split distance in Z
			auto& viewProps = view.GetProperties();
			float near = viewProps.NearPlane;
			float far = Math::Clamp(shadowSettings.DirectionalShadowDistance, viewProps.NearPlane, viewProps.FarPlane);

			return near + (far - near) * scale;
		}

		float ShadowRendering::GetDepthBias(const Light& light, float radius, float depthRange, u32 mapSize)
		{
			const static float kRadialLightBias = 0.005f;
			const static float kSpotDepthBias = 0.01f;
			const static float kDirDepthBias = 0.001f; // In clip space units
			const static float kDefaultResolution = 512.0f;

			// Increase bias if map size smaller than some resolution
			float resolutionScale = 1.0f;

			if(light.GetType() != LightType::Directional)
				resolutionScale = kDefaultResolution / (float)mapSize;

			// Adjust range because in shader we compare vs. clip space depth
			float rangeScale = 1.0f;
			if(light.GetType() == LightType::Spot)
				rangeScale = 1.0f / depthRange;

			const SPtr<GpuDevice>& gpuDevice = GetApplication().GetPrimaryGpuDevice();
			const GpuDeviceCapabilities& caps = gpuDevice->GetCapabilities();
			float deviceDepthRange = caps.MaxDepth - caps.MinDepth;

			float defaultBias = 1.0f;
			switch(light.GetType())
			{
			case LightType::Directional:
				defaultBias = kDirDepthBias * deviceDepthRange;

				// Use larger bias for further away cascades
				defaultBias *= depthRange * 0.01f;
				break;
			case LightType::Radial:
				defaultBias = kRadialLightBias;
				break;
			case LightType::Spot:
				defaultBias = kSpotDepthBias;
				break;
			default:
				break;
			}

			return defaultBias * light.GetShadowBias() * resolutionScale * rangeScale;
		}

		float ShadowRendering::GetFadeTransition(const Light& light, float radius, float depthRange, u32 mapSize)
		{
			const static float kSpotLightScale = 1000.0f;
			const static float kDirLightScale = 50000000.0f;

			// Note: Currently fade transitions are only used in spot & directional (non omni-directional) lights, so no need
			// to account for radial light type.
			if(light.GetType() == LightType::Directional)
			{
				// Just use a large value, as we want a minimal transition region
				return kDirLightScale;
			}
			else
				return fabs(light.GetShadowBias()) * kSpotLightScale;
		}
	} // namespace render
} // namespace b3d
