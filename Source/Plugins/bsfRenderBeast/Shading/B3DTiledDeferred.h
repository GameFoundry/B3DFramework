//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DRenderBeastPrerequisites.h"
#include "Renderer/B3DRendererMaterial.h"
#include "Renderer/B3DGpuUniformBuffer.h"
#include "RenderAPI/B3DGpuPipelineParameterLayout.h"
#include "B3DRendererLight.h"
#include "B3DRendererReflectionProbe.h"

namespace b3d
{
	namespace render
	{
		struct SkyInfo;
		struct SceneInfo;
		class RendererViewGroup;

		/** @addtogroup RenderBeast
		 *  @{
		 */

		B3D_UNIFORM_BUFFER_BEGIN(TiledLightingUniformDefinition)
			B3D_UNIFORM_BUFFER_MEMBER(Vector4I, gLightCounts)
			B3D_UNIFORM_BUFFER_MEMBER(Vector2I, gLightStrides)
			B3D_UNIFORM_BUFFER_MEMBER(Vector2I, gFramebufferSize)
		B3D_UNIFORM_BUFFER_END

		extern TiledLightingUniformDefinition gTiledLightingUniformDefinition;

		/** Shader that performs a lighting pass over data stored in the Gbuffer. */
		class TiledDeferredLightingMaterial : public RendererMaterial<TiledDeferredLightingMaterial>
		{
			RMAT_DEF_CUSTOMIZED("TiledDeferredLighting.bsl");

			/** Helper method used for initializing variations of this material. */
			template <u32 msaa>
			static const ShaderVariationParameters& GetVariation()
			{
				static ShaderVariationParameters variation = ShaderVariationParameters(
					{ ShaderVariationParameter("MSAA_COUNT", msaa) });

				return variation;
			}

		public:
			TiledDeferredLightingMaterial() = default;
			void Initialize() override;

			/** Binds the material for rendering, sets up parameters and executes it. */
			void Execute(GpuCommandBuffer& commandBuffer, const RendererView& view, const VisibleLightData& lightData, const GBufferTextures& gbuffer, const SPtr<Texture>& inputTexture, const SPtr<Texture>& lightAccumTex, const SPtr<Texture>& lightAccumTexArray, const SPtr<Texture>& msaaCoverage);

			/** Returns the material variation matching the provided parameters. */
			static TiledDeferredLightingMaterial* GetVariation(u32 msaaCount);

		private:
			u32 mSampleCount;
			GBufferParameterBinding mGBufferParams;

			GpuParameterStorageBuffer mLightBufferParam;
			GpuParameterStorageTexture mOutputTextureParam;

			GpuParameterSampledTexture mInColorTextureParam;
			GpuParameterSampledTexture mMSAACoverageTexParam;

			GpuParameterUniformBuffer mUniformBufferParameter;

			static const u32 kTileSize;
		};

		/**
		 * Moves data from a texture array into a MSAA texture. Primarily useful when needing to do unordered writes to a
		 * MSAA texture which isn't directly supported on some backends, so writes are done to a texture array instead. The
		 * array is expected to have the same number of layers as the number of samples in the MSAA texture, each layer
		 * containing a sample for that specific pixel.
		 */
		class TextureArrayToMSAATexture : public RendererMaterial<TextureArrayToMSAATexture>
		{
			RMAT_DEF("TextureArrayToMSAATexture.bsl");

		public:
			TextureArrayToMSAATexture() = default;
			void Initialize() override;

			/** Prepares the material for rendering by setting up parameter. Must be called at least once before Execute. */
			void Prepare(const SPtr<Texture>& inputArray, const SPtr<Texture>& target);

			/** Binds the material for rendering, sets up parameters and executes it. */
			void Execute(GpuCommandBuffer& commandBuffer, const SPtr<Texture>& target);

		private:
			GpuParameterSampledTexture mInputParam;
		};

		B3D_UNIFORM_BUFFER_BEGIN(ClearLoadStoreUniformDefinition)
			B3D_UNIFORM_BUFFER_MEMBER(Vector2I, gSize)
			B3D_UNIFORM_BUFFER_MEMBER(Vector4, gFloatClearVal)
			B3D_UNIFORM_BUFFER_MEMBER(Vector4I, gIntClearVal)
		B3D_UNIFORM_BUFFER_END

		extern ClearLoadStoreUniformDefinition gClearLoadStoreUniformDefinition;

		/** Possible object types used as clear destinations by ClearLoadStoreMaterial. */
		enum class ClearLoadStoreType
		{
			Texture,
			TextureArray,
			Buffer,
			StructuredBuffer
		};

		/** Possible data types used in destination objects in ClearLoadStoreMaterial. */
		enum class ClearLoadStoreDataType
		{
			Float,
			Int
		};

		/** Clears the provided texture to zero, using a compute shader. */
		class ClearLoadStoreMaterial : public RendererMaterial<ClearLoadStoreMaterial>
		{
			RMAT_DEF_CUSTOMIZED("ClearLoadStore.bsl");

		public:
			ClearLoadStoreMaterial() = default;
			void Initialize() override;

			/**
			 * Binds the material for rendering, sets up parameters and executes it. Only works on variations of
			 * this material intended for textures and texture arrays.
			 */
			void Execute(GpuCommandBuffer& commandBuffer, const SPtr<Texture>& target, const Color& clearValue = Color::kZero, const TextureSurface& surface = TextureSurface::kComplete);

			/**
			 * Binds the material for rendering, sets up parameters and executes it. Only works on variations of
			 * this material intended for buffers.
			 */
			void Execute(GpuCommandBuffer& commandBuffer, const SPtr<GpuBuffer>& target, const Color& clearValue = Color::kZero);

			/**
			 * Returns the material variation matching the provided parameters.
			 *
			 * @param		objType			Type of object used for clear source.
			 * @param		dataType		Base data type stored in the clear source object.
			 * @param		numComponents	Number of components in the source objects's data type (e.g. float2, float4).
			 * 									In range [1, 4].
			 * @return							Material variation matching the provided values.
			 */
			static ClearLoadStoreMaterial* GetVariation(ClearLoadStoreType objType, ClearLoadStoreDataType dataType, u32 numComponents);

		private:
			/** TILE_SIZE * TILE_SIZE is the number of pixels to process per thread. */
			static constexpr u32 kTileSize = 4;

			/** Number of threads to launch per work group. */
			static constexpr u32 kNumThreads = 128;

			GpuParameterStorageTexture mOutputTextureParam;
			GpuParameterStorageBuffer mOutputBufferParam;
			GpuParameterUniformBuffer mUniformBufferParameter;
		};

		B3D_UNIFORM_BUFFER_BEGIN(TiledImageBasedLightingUniformDefinition)
			B3D_UNIFORM_BUFFER_MEMBER(Vector2I, gFramebufferSize)
		B3D_UNIFORM_BUFFER_END

		extern TiledImageBasedLightingUniformDefinition gTiledImageBasedLightingUniformDefinition;

		/** Shader that performs a lighting pass over data stored in the Gbuffer. */
		class TiledDeferredImageBasedLightingMaterial : public RendererMaterial<TiledDeferredImageBasedLightingMaterial>
		{
			RMAT_DEF_CUSTOMIZED("TiledDeferredImageBasedLighting.bsl");

			/** Helper method used for initializing variations of this material. */
			template <u32 msaa>
			static const ShaderVariationParameters& GetVariation()
			{
				static ShaderVariationParameters variation = ShaderVariationParameters(
					{ ShaderVariationParameter("MSAA_COUNT", msaa) });

				return variation;
			}

		public:
			/** Container for parameters to be passed to the execute() method. */
			struct Inputs
			{
				GBufferTextures Gbuffer;
				SPtr<Texture> LightAccumulation;
				SPtr<Texture> SceneColorTex;
				SPtr<Texture> SceneColorTexArray;
				SPtr<Texture> PreIntegratedGf;
				SPtr<Texture> AmbientOcclusion;
				SPtr<Texture> Ssr;
				SPtr<Texture> MsaaCoverage;
			};

			TiledDeferredImageBasedLightingMaterial() = default;
			void Initialize() override;

			/** Binds the material for rendering, sets up parameters and executes it. */
			void Execute(GpuCommandBuffer& commandBuffer, const RendererView& view, const SceneInfo& sceneInfo, const VisibleReflectionProbeData& probeData, const Inputs& inputs);

			/** Returns the material variation matching the provided parameters. */
			static TiledDeferredImageBasedLightingMaterial* GetVariation(u32 msaaCount);

		private:
			u32 mSampleCount;

			GpuParameterSampledTexture mGBufferA;
			GpuParameterSampledTexture mGBufferB;
			GpuParameterSampledTexture mGBufferC;
			GpuParameterSampledTexture mGBufferDepth;

			GpuParameterSampledTexture mInColorTextureParam;
			GpuParameterSampledTexture mMSAACoverageTexParam;

			ImageBasedLightingParameterBinding mImageBasedParams;

			GpuParameterStorageTexture mOutputTextureParam;

			GpuParameterUniformBuffer mUniformBufferParameter;
			ReflectionProbeUniformBuffer mReflProbeParamBuffer;

			static const u32 kTileSize;
		};

		/** @} */
	} // namespace render
} // namespace b3d
