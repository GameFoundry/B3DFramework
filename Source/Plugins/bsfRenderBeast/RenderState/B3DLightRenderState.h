//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DRenderBeastPrerequisites.h"
#include "Renderer/B3DRendererMaterial.h"
#include "Renderer/B3DGpuUniformBuffer.h"
#include "Components/B3DLight.h"
#include "GpuBackend/B3DGpuPipelineParameterLayout.h"

namespace b3d
{
	namespace render
	{
		class RenderBeastScene;
		class RendererViewGroup;

		/** @addtogroup RenderBeast
		 *  @{
		 */

		/** Render-thread state for a single light, indexed by PackedRendererId. */
		struct LightRenderState
		{
			u32 TypeArrayIndex = 0;
		};

		/** Information about a single light, as seen by the lighting shader. */
		struct LightData
		{
			Vector3 Position;
			float BoundsRadius;
			Vector3 Direction;
			float Luminance;
			Vector3 SpotAngles;
			float AttRadiusSqrdInv;
			Vector3 Color;
			float SrcRadius;
			Vector3 ShiftedLightPosition;
			float Padding;
		};

		/** Populates the structure with light parameters. */
		void GetLightParameters(const LightProxy& proxy, LightData& output);

		/**
		 * Populates the provided uniform buffer with information about the light. Provided buffer's structure
		 * must match PerLightUniformDefinition.
		 */
		void PopulateLightUniformBuffer(const LightProxy& proxy, TShared<GpuBuffer>& buffer, u32 index = 0);

		/**
		 * Calculates the light position that is shifted in order to account for area spot lights. For non-spot lights
		 * this method will return normal light position. The position will be shifted back from the light direction,
		 * magnitude of the shift depending on the source radius.
		 */
		Vector3 GetShiftedLightPosition(const LightProxy& proxy);

		/** Container for all GBuffer textures. */
		struct GBufferTextures
		{
			TShared<Texture> Albedo;
			TShared<Texture> Normals;
			TShared<Texture> RoughMetal;
			TShared<Texture> Depth;
		};

		/** Allows you to easily bind GBuffer textures to some material. */
		class GBufferParameterBinding
		{
		public:

			static constexpr const char* kAlbedoTextureName = "gGBufferATex";
			static constexpr const char* kNormalsTextureName = "gGBufferBTex";
			static constexpr const char* kRoughMetalTextureName = "gGBufferCTex";
			static constexpr const char* kDepthTextureName = "gDepthBufferTex";
			static constexpr const char* kDepthSamplerName = "gDepthBufferSamp";

			/** Initializes the required parameters. To be called once before use. */
			void Initialize(GpuDevice& gpuDevice, GpuProgramType type, const TShared<GpuParameterSet>& gpuParams);

			/** Binds the GBuffer textures to the pipeline. */
			void Bind(const GBufferTextures& gbuffer);

			/**
			 * Assigns the GBuffer textures, and the point sampler of the depth texture, to @p set. The set must be created for the
			 * material whose parameter set the binding was initialized with.
			 */
			void Bind(const TShared<GpuParameterSet>& set, const GBufferTextures& gbuffer) const;

			/** Assigns the provided GBuffer textures to the provided GPU parameters object. */
			static void Set(GpuDevice& gpuDevice, const TShared<GpuParameterSet>& gpuParameters, const GBufferTextures& textures);

		private:
			TShared<GpuParameterSet> mParams;

			GpuParameterSampledTexture mGBufferA;
			GpuParameterSampledTexture mGBufferB;
			GpuParameterSampledTexture mGBufferC;
			GpuParameterSampledTexture mGBufferDepth;
			GpuParameterSampler mGBufferDepthSampler;
			TShared<SamplerState> mGBufferDepthSamplerState;
		};

		/**
		 * Contains lights that are visible from a specific set of views, determined by scene information provided to
		 * setLights().
		 */
		class VisibleLightData
		{
		public:
			VisibleLightData();

			/**
			 * Updates the internal buffers with a new set of lights. Before calling make sure that light visibility has
			 * been calculated for the provided view group.
			 */
			void Update(const RenderBeastScene& scene, const RendererViewGroup& viewGroup);

			/** Returns a GPU bindable buffer containing information about every light. */
			TShared<GpuBuffer> GetLightBuffer() const { return mLightBuffer; }

			/** Returns the number of directional lights in the lights buffer. */
			u32 GetDirectionalLightCount() const { return mLightCounts[0]; }

			/** Returns the number of radial point lights in the lights buffer. */
			u32 GetRadialLightCount() const { return mLightCounts[1]; }

			/** Returns the number of spot point lights in the lights buffer. */
			u32 GetSpotLightCount() const { return mLightCounts[2]; }

			/** Returns the number of visible lights of the specified type. */
			u32 GetLightCount(LightType type) const { return mLightCounts[(u32)type]; }

			/** Returns the number of visible shadowed lights of the specified type. */
			u32 GetShadowedLightCount(LightType type) const { return mShadowedLightCounts[(u32)type]; }

			/** Returns the number of visible unshadowed lights of the specified type. */
			u32 GetUnshadowedLightCount(LightType type) const { return mLightCounts[(u32)type] - mShadowedLightCounts[(u32)type]; }

			/** Returns a list of all visible light packed IDs of the specified type. */
			const Vector<PackedRendererId>& GetLights(LightType type) const { return mVisibleLights[(u32)type]; }

		private:
			TShared<GpuBuffer> mLightBuffer;

			u32 mLightCounts[(u32)LightType::Count];
			u32 mShadowedLightCounts[(u32)LightType::Count];

			// These are rebuilt every call to update()
			Vector<PackedRendererId> mVisibleLights[(u32)LightType::Count];
			Vector<LightData> mVisibleLightData;
		};

		/** @} */
	} // namespace render
} // namespace b3d
